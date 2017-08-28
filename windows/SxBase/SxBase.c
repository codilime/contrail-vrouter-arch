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

NDIS_HANDLE SxDriverObject;

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
// FilterRestart Function
// http://msdn.microsoft.com/en-us/library/ff549962(v=VS.85).aspx
//
_Use_decl_annotations_
NDIS_STATUS
SxNdisPause(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_FILTER_PAUSE_PARAMETERS PauseParameters
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)(FilterModuleContext);

    UNREFERENCED_PARAMETER(PauseParameters);

    DbgPrint("%s: SxInstance %p\r\n", __func__, FilterModuleContext);

    //
    // Set the flag that the filter is going to pause.
    //
    NT_ASSERT(switchObject->DataFlowState == SxSwitchRunning);
    switchObject->DataFlowState = SxSwitchPaused;
    
    KeMemoryBarrier();
    
    while(switchObject->PendingInjectedNblCount > 0)
    {
        NdisMSleep(1000);
    }

    return NDIS_STATUS_SUCCESS;
}


//
// FilterPause Function
// http://msdn.microsoft.com/en-us/library/ff549957(v=VS.85).aspx
//
_Use_decl_annotations_
NDIS_STATUS
SxNdisRestart(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_FILTER_RESTART_PARAMETERS RestartParameters
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(RestartParameters);

    DbgPrint("%s: FilterModuleContext %p\n", __func__, FilterModuleContext);
           
    status = SxExtRestartSwitch(switchObject,
                                switchObject->ExtensionContext);
    if (status != NDIS_STATUS_SUCCESS)
    {
        status = NDIS_STATUS_RESOURCES;
        goto Cleanup;
    }

    NT_ASSERT(switchObject->DataFlowState == SxSwitchPaused);
    switchObject->DataFlowState = SxSwitchRunning;

Cleanup:
    return status;
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

    DbgPrint("%s: OidRequest %p.\r\n", __func__, OidRequest);
    
    NdisInterlockedIncrement(&switchObject->PendingOidCount);

    status = NdisAllocateCloneOidRequest(switchObject->NdisFilterHandle,
                                         OidRequest,
                                         SxExtAllocationTag,
                                         &clonedRequest);
    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Cannot Clone OidRequest\r\n", __func__);
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
    return status;
}


//
// FilterCancelOidRequest Function
// http://msdn.microsoft.com/en-us/library/ff549911(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisCancelOidRequest(
    NDIS_HANDLE FilterModuleContext,
    PVOID RequestId
    )
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(RequestId);
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

    DbgPrint("%s: NdisOidRequest %p.\r\n", __func__, NdisOidRequest);

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
    
Cleanup:
    NdisInterlockedDecrement(&switchObject->PendingOidCount);
}


//
// FilterSendNetBufferLists Function
// http://msdn.microsoft.com/en-us/library/ff549966(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisSendNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG SendFlags
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    
    UNREFERENCED_PARAMETER(PortNumber);

    SxExtStartNetBufferListsIngress(switchObject,
                                    switchObject->ExtensionContext,
                                    NetBufferLists,
                                    SendFlags);
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
// FilterReceiveNetBufferLists Function
// http://msdn.microsoft.com/en-us/library/ff549960(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisReceiveNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG NumberOfNetBufferLists,
    ULONG ReceiveFlags
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    
    UNREFERENCED_PARAMETER(PortNumber);

    SxExtStartNetBufferListsEgress(switchObject,
                                   switchObject->ExtensionContext,
                                   NetBufferLists,
                                   NumberOfNetBufferLists,
                                   ReceiveFlags);
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


//
// FilterCancelSendNetBufferLists Function
// http://msdn.microsoft.com/en-us/library/ff549915(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisCancelSendNetBufferLists(
    NDIS_HANDLE FilterModuleContext,
    PVOID CancelId
    )
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(CancelId);
}


//
// FilterNetPnPEvent Function
// http://msdn.microsoft.com/en-us/library/ff549952(v=vs.85).aspx
//
_Use_decl_annotations_
NDIS_STATUS
SxNdisNetPnPEvent(
    NDIS_HANDLE FilterModuleContext,
    PNET_PNP_EVENT_NOTIFICATION NetPnPEvent
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    
    if (NetPnPEvent->NetPnPEvent.NetEvent == NetEventSwitchActivate)
    {
    }
    
    return NdisFNetPnPEvent(switchObject->NdisFilterHandle,
                            NetPnPEvent);
}


//
// FilterStatus Function
// http://msdn.microsoft.com/en-us/library/ff549973(v=VS.85).aspx
//
_Use_decl_annotations_
VOID
SxNdisStatus(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_STATUS_INDICATION StatusIndication
    )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    PNDIS_SWITCH_NIC_STATUS_INDICATION nicIndication;
    PNDIS_STATUS_INDICATION originalIndication;
    
    if (StatusIndication->Header.Type != NDIS_OBJECT_TYPE_STATUS_INDICATION ||
        StatusIndication->Header.Revision != NDIS_STATUS_INDICATION_REVISION_1 ||
        StatusIndication->Header.Size < NDIS_SIZEOF_STATUS_INDICATION_REVISION_1)
    {
        goto Cleanup;
    }
    
    //
    // Only NDIS_STATUS_SWITCH_NIC_STAUTUS indications need to be processed
    // by switch extensions.
    //
    if (StatusIndication->StatusCode != NDIS_STATUS_SWITCH_NIC_STATUS)
    {
        goto Cleanup;
    }
    
    nicIndication = StatusIndication->StatusBuffer;
    
    if (nicIndication->Header.Type != NDIS_OBJECT_TYPE_STATUS_INDICATION ||
        nicIndication->Header.Revision != NDIS_SWITCH_NIC_STATUS_INDICATION_REVISION_1 ||
        nicIndication->Header.Size < NDIS_SIZEOF_SWITCH_NIC_STATUS_REVISION_1)
    {
        goto Cleanup;
    }
    
    originalIndication = nicIndication->StatusIndication;
    
    status = NDIS_STATUS_SUCCESS;
                                   
Cleanup:
    if (status == NDIS_STATUS_SUCCESS)
    {
        NdisFIndicateStatus(switchObject->NdisFilterHandle,
                            StatusIndication);
    }

    return;
                           
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
    ULONG bytesRestored = 0;
    
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
            
            if (oid == OID_SWITCH_PROPERTY_ADD)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                status = NDIS_STATUS_SUCCESS;
            }
            
            if (status == NDIS_STATUS_NOT_SUPPORTED)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                *Complete = TRUE;
                goto Cleanup;
            }
            
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
        
            if (oid == OID_SWITCH_PORT_PROPERTY_ADD)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                status = NDIS_STATUS_SUCCESS;
            }
            
            if (status == NDIS_STATUS_NOT_SUPPORTED)
            {
                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                *Complete = TRUE;
                goto Cleanup;
            }
            
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
            
            *Complete = FALSE;
            
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
            else if (oid == OID_SWITCH_PORT_UPDATED)
            {
            }
            else if (oid == OID_SWITCH_PORT_TEARDOWN)
            {
            }
            else
            {
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
            
            status = NDIS_STATUS_SUCCESS;
                                     
            if (status != NDIS_STATUS_SUCCESS)
            {
                *Complete = TRUE;
            }
            else if (bytesRestored > 0)
            {
                *Complete = TRUE;
            }
                                      
            break;    

        case OID_SWITCH_NIC_SAVE_COMPLETE:
            if (header->Type != NDIS_OBJECT_TYPE_DEFAULT ||
                header->Revision < NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1 ||
                header->Size < NDIS_SIZEOF_NDIS_SWITCH_NIC_SAVE_STATE_REVISION_1)
            {
                status = NDIS_STATUS_NOT_SUPPORTED;
                *Complete = TRUE;
                goto Cleanup;
            }

            break;
            
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
    ULONG bytesWritten = 0;
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
                                           
            if (status != NDIS_STATUS_SUCCESS)
            {
                *Complete = TRUE;
                goto Cleanup;
            }
                                           
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
            
            status = NDIS_STATUS_SUCCESS;
                                       
            if (status == NDIS_STATUS_SUCCESS &&
                bytesWritten > 0)
            {
                *Complete = TRUE;
            }
            else if (status == NDIS_STATUS_BUFFER_TOO_SHORT)
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

