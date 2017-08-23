/*++

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SxBase.c

Abstract:

    This file contains the common code for building a switch extension. This
    file includes all the standard NDIS handling and exposes a function based
    interface for the control path and data path. This also has reusable code
    for basic operations such as pause/resume handling etc.


--*/

#include "precomp.h"

ULONG SxDebugLevel;// = ~((ULONG)0);
extern NDIS_HANDLE SxDriverHandle;
extern NDIS_HANDLE SxDriverObject;
extern NDIS_SPIN_LOCK SxExtensionListLock;
extern LIST_ENTRY SxExtensionList;

NDIS_STRING SxExtensionFriendlyName;
NDIS_STRING SxExtensionGuid;

NDIS_STATUS
SxpNdisProcessSetOid(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Out_ PBOOLEAN Complete
    );

NDIS_STATUS
SxpNdisProcessMethodOid(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Out_ PBOOLEAN Complete,
    _Out_ PULONG BytesNeeded
    );


//
// FilterSetOptions Function
// http://msdn.microsoft.com/en-us/library/ff549972(v=VS.85).aspx
//
_Use_decl_annotations_
NDIS_STATUS
SxNdisSetOptions(
    NDIS_HANDLE NdisDriverHandle,
    NDIS_HANDLE DriverContext
    )
{
    UNREFERENCED_PARAMETER(NdisDriverHandle);
    UNREFERENCED_PARAMETER(DriverContext);
    return NDIS_STATUS_SUCCESS;
}
    

//
// FilterSetModuleOptions Function
// http://msdn.microsoft.com/en-us/library/ff549970(v=VS.85).aspx
//
_Use_decl_annotations_
NDIS_STATUS
SxNdisSetFilterModuleOptions(
    NDIS_HANDLE FilterModuleContext
    )
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    return NDIS_STATUS_SUCCESS;
}

//
// FilterOidRequest Function
// http://msdn.microsoft.com/en-us/library/ff549954(v=VS.85).aspx
//
_Use_decl_annotations_
NDIS_STATUS
SxNdisOidRequest(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_OID_REQUEST OidRequest
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    NDIS_STATUS status;
    PNDIS_OID_REQUEST clonedRequest=NULL;
    PVOID *cloneRequestContext;
    BOOLEAN completeOid = FALSE;
    ULONG bytesNeeded = 0;
   
    status = NDIS_STATUS_SUCCESS;

    DEBUGP(DL_TRACE, ("===>SxOidRequest: OidRequest %p.\n", OidRequest));
    
    NdisInterlockedIncrement(&switchObject->PendingOidCount);

    status = NdisAllocateCloneOidRequest(switchObject->NdisFilterHandle,
                                         OidRequest,
                                         SxExtAllocationTag,
                                         &clonedRequest);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGP(DL_WARN, ("FilerOidRequest: Cannot Clone OidRequest\n"));
        goto Cleanup;
    }

    cloneRequestContext = (PVOID*)(&clonedRequest->SourceReserved[0]);
    *cloneRequestContext = OidRequest;
    
    switch (clonedRequest->RequestType)
    {           
        case NdisRequestSetInformation:
            status = SxpNdisProcessSetOid(switchObject,
                                          clonedRequest,
                                          &completeOid);
            break;
        
        case NdisRequestMethod:
            status = SxpNdisProcessMethodOid(switchObject,
                                             clonedRequest,
                                             &completeOid,
                                             &bytesNeeded);

            break;
    }
    
    if (completeOid)
    {
        NdisFreeCloneOidRequest(switchObject->NdisFilterHandle, clonedRequest);
        OidRequest->DATA.METHOD_INFORMATION.BytesNeeded = bytesNeeded;
        NdisInterlockedDecrement(&switchObject->PendingOidCount);
        goto Cleanup;
    }

    status = NdisFOidRequest(switchObject->NdisFilterHandle, clonedRequest);

    if (status != NDIS_STATUS_PENDING)
    {
        SxNdisOidRequestComplete(switchObject, clonedRequest, status);

        //
        // We must still return status as pending because we complete the
        // request using NdisFOidRequestComplete() in SxOidRequestComplete().
        //
        status = NDIS_STATUS_PENDING;
    }

Cleanup:

    DEBUGP(DL_TRACE, ("<===SxOidRequest: status %8x.\n", status));
    return status;
}


//
// FilterOidRequestComplete Function
// http://msdn.microsoft.com/en-us/library/ff549956(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisOidRequestComplete(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_OID_REQUEST NdisOidRequest,
    NDIS_STATUS Status
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    PNDIS_OID_REQUEST originalRequest;
    PVOID *oidRequestContext;
    PNDIS_SWITCH_NIC_OID_REQUEST nicOidRequestBuf;
    PNDIS_OBJECT_HEADER header;

    DEBUGP(DL_TRACE,
           ("===>SxOidRequestComplete, NdisOidRequest %p.\n", NdisOidRequest));

    oidRequestContext = (PVOID*)(&NdisOidRequest->SourceReserved[0]);
    originalRequest = (*oidRequestContext);

    //
    // This is the internal request
    //
    if (originalRequest == NULL)
    {
        SxpNdisCompleteInternalOidRequest(switchObject, NdisOidRequest, Status);
        goto Cleanup;
    }

    //
    // Copy the information from the returned request to the original request
    //
    switch(NdisOidRequest->RequestType)
    {
    case NdisRequestMethod:
        originalRequest->DATA.METHOD_INFORMATION.OutputBufferLength =
            NdisOidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;
        originalRequest->DATA.METHOD_INFORMATION.BytesRead =
            NdisOidRequest->DATA.METHOD_INFORMATION.BytesRead;
        originalRequest->DATA.METHOD_INFORMATION.BytesNeeded =
            NdisOidRequest->DATA.METHOD_INFORMATION.BytesNeeded;
        originalRequest->DATA.METHOD_INFORMATION.BytesWritten =
            NdisOidRequest->DATA.METHOD_INFORMATION.BytesWritten;
            
        if (NdisOidRequest->DATA.METHOD_INFORMATION.Oid == OID_SWITCH_NIC_REQUEST &&
                switchObject->OldNicRequest != NULL)
        {
            nicOidRequestBuf = NdisOidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
            Status = NDIS_STATUS_SUCCESS;
            
            originalRequest->DATA.METHOD_INFORMATION.InformationBuffer =
                                                    switchObject->OldNicRequest;
            switchObject->OldNicRequest = NULL;
            ExFreePoolWithTag(nicOidRequestBuf, SxExtAllocationTag);
        }
        
        break;

    case NdisRequestSetInformation:
        header = originalRequest->DATA.SET_INFORMATION.InformationBuffer;
        
        originalRequest->DATA.SET_INFORMATION.BytesRead =
            NdisOidRequest->DATA.SET_INFORMATION.BytesRead;
        originalRequest->DATA.SET_INFORMATION.BytesNeeded =
            NdisOidRequest->DATA.SET_INFORMATION.BytesNeeded;
            
        if (NdisOidRequest->DATA.METHOD_INFORMATION.Oid == OID_SWITCH_PORT_CREATE &&
            Status != NDIS_STATUS_SUCCESS)
        {
        }
        else if (NdisOidRequest->DATA.METHOD_INFORMATION.Oid == OID_SWITCH_PORT_CREATE &&
                 Status != NDIS_STATUS_SUCCESS)
        {
            SxExtDeleteNic(switchObject,
                           switchObject->ExtensionContext,
                           (PNDIS_SWITCH_NIC_PARAMETERS)header);
            
        }

        break;

    case NdisRequestQueryInformation:
    case NdisRequestQueryStatistics:
    default:
        originalRequest->DATA.QUERY_INFORMATION.BytesWritten =
            NdisOidRequest->DATA.QUERY_INFORMATION.BytesWritten;
        originalRequest->DATA.QUERY_INFORMATION.BytesNeeded =
            NdisOidRequest->DATA.QUERY_INFORMATION.BytesNeeded;
        break;
    }

    (*oidRequestContext) = NULL;

    NdisFreeCloneOidRequest(switchObject->NdisFilterHandle, NdisOidRequest);

    NdisFOidRequestComplete(switchObject->NdisFilterHandle,
                            originalRequest,
                            Status);

    DEBUGP(DL_TRACE, ("<===SxOidRequestComplete.\n"));
    
Cleanup:
    NdisInterlockedDecrement(&switchObject->PendingOidCount);
}


//
// FilterSendNetBufferListsComplete Function
// http://msdn.microsoft.com/en-us/library/ff549967(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisSendNetBufferListsComplete(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    ULONG SendCompleteFlags
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    SxExtStartCompleteNetBufferListsIngress(switchObject,
                                            switchObject->ExtensionContext,
                                            NetBufferLists,
                                            SendCompleteFlags);
}


//
// FilterReturnNetBufferLists Function
// http://msdn.microsoft.com/en-us/library/ff549964(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisReturnNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    ULONG ReturnFlags
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    SxExtStartCompleteNetBufferListsEgress(switchObject,
                                           switchObject->ExtensionContext,
                                           NetBufferLists,
                                           ReturnFlags);
}


NDIS_STATUS
SxpNdisProcessSetOid(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Out_ PBOOLEAN Complete
    )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_OID oid = OidRequest->DATA.SET_INFORMATION.Oid;
    PNDIS_OBJECT_HEADER header;
    
    *Complete = FALSE;
    
    header = OidRequest->DATA.SET_INFORMATION.InformationBuffer;
    
    if (OidRequest->DATA.SET_INFORMATION.InformationBufferLength != 0 &&
        OidRequest->DATA.SET_INFORMATION.InformationBufferLength < 
            sizeof(NDIS_OBJECT_HEADER))
    {
        status = NDIS_STATUS_NOT_SUPPORTED;
        *Complete = TRUE;
        goto Cleanup;
    }
    
    if (OidRequest->DATA.SET_INFORMATION.InformationBufferLength == 0)
    {
        *Complete = FALSE;
        goto Cleanup;
    }
    
    switch(oid)
    {
        case OID_SWITCH_PROPERTY_ADD:
        case OID_SWITCH_PROPERTY_UPDATE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PROPERTY_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PROPERTY_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            status = NDIS_STATUS_SUCCESS;
            *Complete = TRUE;
            goto Cleanup;
            
            break;
        case OID_SWITCH_PROPERTY_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PROPERTY_DELETE_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PROPERTY_DELETE_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            *Complete = FALSE;
                                                 
            break;
        
        case OID_SWITCH_PORT_PROPERTY_ADD:
        case OID_SWITCH_PORT_PROPERTY_UPDATE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PORT_PROPERTY_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PORT_PROPERTY_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            status = NDIS_STATUS_SUCCESS;
            *Complete = TRUE;
            goto Cleanup;
            
            break;
        
        case OID_SWITCH_PORT_PROPERTY_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PORT_PROPERTY_DELETE_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PORT_PROPERTY_DELETE_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            break;
            
        case OID_SWITCH_PORT_CREATE:
        case OID_SWITCH_PORT_UPDATED:
        case OID_SWITCH_PORT_TEARDOWN:
        case OID_SWITCH_PORT_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_PORT_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_PORT_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
        
            if (oid == OID_SWITCH_PORT_CREATE)
            {
                status = SxExtCreatePort(Switch,
                                         Switch->ExtensionContext,
                                         (PNDIS_SWITCH_PORT_PARAMETERS)header);
                                       
                if (status != NDIS_STATUS_SUCCESS)
                {
                    *Complete = TRUE;
                }
            }
        
            break;
        
        case OID_SWITCH_NIC_CREATE:
        case OID_SWITCH_NIC_CONNECT:
        case OID_SWITCH_NIC_UPDATED:
        case OID_SWITCH_NIC_DISCONNECT:
        case OID_SWITCH_NIC_DELETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            if (oid == OID_SWITCH_NIC_CREATE)
            {
                status = SxExtCreateNic(Switch,
                                        Switch->ExtensionContext,
                                        (PNDIS_SWITCH_NIC_PARAMETERS)header);
                if (status != NDIS_STATUS_SUCCESS)
                {
                    *Complete = TRUE;
                }
            }
            else if (oid == OID_SWITCH_NIC_CONNECT)
            {   
                SxExtConnectNic(Switch,
                                Switch->ExtensionContext,
                                (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            else if (oid == OID_SWITCH_NIC_UPDATED)
            {
                SxExtUpdateNic(Switch,
                               Switch->ExtensionContext,
                               (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            else if (oid == OID_SWITCH_NIC_DISCONNECT)
            {
                SxExtDisconnectNic(Switch,
                                   Switch->ExtensionContext,
                                   (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            else
            {
                SxExtDeleteNic(Switch,
                               Switch->ExtensionContext,
                               (PNDIS_SWITCH_NIC_PARAMETERS)header);
            }
            
            break;
        
        case OID_SWITCH_NIC_RESTORE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                goto Cleanup;
            }
                                      
            break;    

        case OID_SWITCH_NIC_SAVE_COMPLETE:
        case OID_SWITCH_NIC_RESTORE_COMPLETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            break;
            
        default:
            break;
    }
    
Cleanup:
    return status;
}


NDIS_STATUS
SxpNdisProcessMethodOid(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Inout_ PNDIS_OID_REQUEST OidRequest,
    _Out_ PBOOLEAN Complete,
    _Out_ PULONG BytesNeeded
    )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS_OID oid = OidRequest->DATA.SET_INFORMATION.Oid;
    PNDIS_OBJECT_HEADER header;
    PNDIS_SWITCH_NIC_OID_REQUEST nicOidRequest;
    PNDIS_SWITCH_NIC_OID_REQUEST newNicOidRequest = NULL;
    NDIS_SWITCH_PORT_ID destPort, sourcePort;
    NDIS_SWITCH_NIC_INDEX destNic, sourceNic;
    ULONG bytesNeeded = 0;
    
    *Complete = FALSE;
    *BytesNeeded = 0;
    
    header = OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
    
    switch(oid)
    {
        case OID_SWITCH_FEATURE_STATUS_QUERY:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            *Complete = FALSE;
                                                      
            if (*BytesNeeded > 0)
            {
                status = NDIS_STATUS_BUFFER_TOO_SHORT;
            }
        
            break;
            
        case OID_SWITCH_PORT_FEATURE_STATUS_QUERY:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_FEATURE_STATUS_PARAMETERS_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            *Complete = FALSE;
                                                    
            if (*BytesNeeded > 0)
            {
                status = NDIS_STATUS_BUFFER_TOO_SHORT;
            }
        
            break;
            
        case OID_SWITCH_NIC_REQUEST:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_OID_REQUEST_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_OID_REQUEST_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            nicOidRequest = (PNDIS_SWITCH_NIC_OID_REQUEST)header;
            
            sourcePort = nicOidRequest->SourcePortId;
            sourceNic = nicOidRequest->SourceNicIndex;
            destPort = nicOidRequest->DestinationPortId;
            destNic = nicOidRequest->DestinationNicIndex;
            
            status = NDIS_STATUS_SUCCESS;
                                           
            if (sourcePort != nicOidRequest->SourcePortId ||
                sourceNic != nicOidRequest->SourceNicIndex ||
                destPort != nicOidRequest->DestinationPortId ||
                destNic != nicOidRequest->DestinationNicIndex)
            {
                ASSERT(Switch->OldNicRequest == NULL);
                Switch->OldNicRequest = nicOidRequest;
                
                newNicOidRequest = (PNDIS_SWITCH_NIC_OID_REQUEST)ExAllocatePoolWithTag(
                                                                    NonPagedPoolNx,
                                                                    sizeof(NDIS_SWITCH_NIC_OID_REQUEST),
                                                                    SxExtAllocationTag);
                                                                    
                if (newNicOidRequest == NULL)
                {
                    status = NDIS_STATUS_RESOURCES;
                    *Complete = TRUE;
                    goto Cleanup;
                }
                                                                    
                newNicOidRequest->Header = nicOidRequest->Header;
                newNicOidRequest->SourcePortId = sourcePort;
                newNicOidRequest->SourceNicIndex = sourceNic;
                newNicOidRequest->DestinationPortId = destPort;
                newNicOidRequest->DestinationNicIndex = destNic;
                newNicOidRequest->OidRequest = nicOidRequest->OidRequest;
                        
                OidRequest->DATA.METHOD_INFORMATION.InformationBuffer = newNicOidRequest;
            }
        
            break;
            
        case OID_SWITCH_NIC_SAVE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }
            
            status = SxExtSaveNic(Switch,
                                  Switch->ExtensionContext,
                                  (PNDIS_SWITCH_NIC_SAVE_STATE)header,
                                  NULL,
                                  &bytesNeeded);
                                       
            if (status == NDIS_STATUS_BUFFER_TOO_SHORT)
            {
                *BytesNeeded = ((PNDIS_SWITCH_NIC_SAVE_STATE)header)->SaveDataOffset +
                                bytesNeeded;
                *Complete = TRUE;
            }
            else if (status != NDIS_STATUS_SUCCESS)
            {
                *Complete = TRUE;
            }
            
            break;
            
        default:
            break;
    }
    
Cleanup:
    return status;
}


VOID
SxpNdisCompleteInternalOidRequest(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ PNDIS_OID_REQUEST NdisRequest,
    _In_ NDIS_STATUS Status
    )
/*++

Routine Description:

    NDIS entry point indicating completion of a pended NDIS_OID_REQUEST.

Arguments:

    Switch - pointer to switch object.

    NdisRequest - pointer to NDIS request

    Status - Status of request completion

Return Value:

    None

--*/
{
    PSX_OID_REQUEST oidRequest;
    ULONG bytesNeeded;
    
    UNREFERENCED_PARAMETER(Switch);

    bytesNeeded = 0;
    oidRequest = NULL;

    switch (NdisRequest->RequestType)
    {
    case NdisRequestSetInformation:
        bytesNeeded = NdisRequest->DATA.SET_INFORMATION.BytesNeeded;
        break;

    case NdisRequestQueryInformation:
        bytesNeeded = NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded;
        break;

    case NdisRequestMethod:
        bytesNeeded = NdisRequest->DATA.METHOD_INFORMATION.BytesNeeded;
        break;
    }

    //
    // Get at the request context.
    //
    oidRequest = CONTAINING_RECORD(NdisRequest, SX_OID_REQUEST, NdisOidRequest);

    //
    // Save away the completion status.
    //
    oidRequest->Status = Status;
    
    //
    // Save bytesNeeded
    //
    oidRequest->BytesNeeded = bytesNeeded;

    //
    // Wake up the thread blocked for this request to complete.
    //
    NdisSetEvent(&oidRequest->ReqEvent);
}

