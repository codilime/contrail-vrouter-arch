/*++

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SxLibrary.c

Abstract:

    This file contains the common library functions that can be used
    by any extension using the SxBase library.


--*/

#include "precomp.h"

VOID
SxLibSendNetBufferListsIngress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG SendFlags,
    _In_ ULONG NumInjectedNetBufferLists
    )
{
    BOOLEAN dispatch;
    BOOLEAN sameSource;
    ULONG sendCompleteFlags;
    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwdDetail;
    PNET_BUFFER_LIST curNbl, nextNbl;
    ULONG numNbls = 0;
    PNET_BUFFER_LIST dropNbl = NULL;
    PNET_BUFFER_LIST *curDropNbl = &dropNbl;
    NDIS_SWITCH_PORT_ID curSourcePort;
    NDIS_STRING filterReason;
    
    dispatch = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendFlags);
    sameSource = NDIS_TEST_SEND_FLAG(SendFlags, NDIS_SEND_FLAGS_SWITCH_SINGLE_SOURCE);
    
    InterlockedAdd(&Switch->PendingInjectedNblCount, NumInjectedNetBufferLists);
    KeMemoryBarrier();
    
    if (Switch->DataFlowState != SxSwitchRunning)
    {
        RtlInitUnicodeString(&filterReason, L"Extension Paused");
        
        sendCompleteFlags = (dispatch) ? NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL : 0;
        sendCompleteFlags |= (sameSource) ? NDIS_SEND_COMPLETE_FLAGS_SWITCH_SINGLE_SOURCE : 0;
        
        fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(NetBufferLists);
    
        if (sameSource)
        {
            for (curNbl = NetBufferLists; curNbl != NULL; curNbl = curNbl->Next)
            {
                ++numNbls;
            }
            
            Switch->NdisSwitchHandlers.ReportFilteredNetBufferLists(
                                         Switch->NdisSwitchContext,
                                         &SxExtensionGuid,
                                         &SxExtensionFriendlyName,
                                         fwdDetail->SourcePortId,
                                         NDIS_SWITCH_REPORT_FILTERED_NBL_FLAGS_IS_INCOMING,
                                         numNbls,
                                         NetBufferLists,
                                         &filterReason);
                                         
            SxExtStartCompleteNetBufferListsIngress(Switch,
                                                    Switch->ExtensionContext,
                                                    NetBufferLists,
                                                    sendCompleteFlags);    
        }
        else
        {
            curSourcePort = fwdDetail->SourcePortId;
            for (curNbl = NetBufferLists; curNbl != NULL; curNbl = nextNbl)
            {
                nextNbl = curNbl->Next;
                curNbl->Next = NULL;
                
                fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);
                
                if(curSourcePort == fwdDetail->SourcePortId)
                {
                    *curDropNbl = curNbl;
                    curDropNbl = &(curNbl->Next);
                    ++numNbls;
                }
                else
                {
                    Switch->NdisSwitchHandlers.ReportFilteredNetBufferLists(
                                                 Switch->NdisSwitchContext,
                                                 &SxExtensionGuid,
                                                 &SxExtensionFriendlyName,
                                                 curSourcePort,
                                                 NDIS_SWITCH_REPORT_FILTERED_NBL_FLAGS_IS_INCOMING,
                                                 numNbls,
                                                 dropNbl,
                                                 &filterReason);
                     
                    SxExtStartCompleteNetBufferListsIngress(Switch,
                                                            Switch->ExtensionContext,
                                                            dropNbl,
                                                            sendCompleteFlags);

                    numNbls = 1;
                    dropNbl = curNbl;
                    curDropNbl = &(curNbl->Next);
                    curSourcePort = fwdDetail->SourcePortId;
                }
            }
            
            Switch->NdisSwitchHandlers.ReportFilteredNetBufferLists(
                                             Switch->NdisSwitchContext,
                                             &SxExtensionGuid,
                                             &SxExtensionFriendlyName,
                                             curSourcePort,
                                             NDIS_SWITCH_REPORT_FILTERED_NBL_FLAGS_IS_INCOMING,
                                             numNbls,
                                             dropNbl,
                                             &filterReason);
                 
            SxExtStartCompleteNetBufferListsIngress(Switch,
                                                    Switch->ExtensionContext,
                                                    dropNbl,
                                                    sendCompleteFlags);
        }                                
                                            
        goto Cleanup;
    }
    
    NdisFSendNetBufferLists(Switch->NdisFilterHandle,
                            NetBufferLists,
                            NDIS_DEFAULT_PORT_NUMBER,
                            SendFlags);
                                
Cleanup:
    return;
}


VOID
SxLibSendNetBufferListsEgress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG NumberOfNetBufferLists,
    _In_ ULONG ReceiveFlags
    )
{
    BOOLEAN dispatch, sameSource;
    NDIS_SWITCH_PORT_ID sourcePortId;
    PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwdDetail;
    ULONG returnFlags;
    NDIS_SWITCH_PORT_ID curSourcePort;
    PNET_BUFFER_LIST curNbl, nextNbl;
    ULONG numNbls;
    PNET_BUFFER_LIST dropNbl = NULL;
    PNET_BUFFER_LIST *curDropNbl = &dropNbl;
    NDIS_STRING filterReason;
    
    dispatch = NDIS_TEST_RECEIVE_AT_DISPATCH_LEVEL(ReceiveFlags);
    sameSource = NDIS_TEST_RECEIVE_FLAG(ReceiveFlags, NDIS_RECEIVE_FLAGS_SWITCH_SINGLE_SOURCE);
                          
    if (Switch->DataFlowState != SxSwitchRunning)
    {
        RtlInitUnicodeString(&filterReason, L"Extension Paused");
        
        returnFlags = (dispatch) ? NDIS_RETURN_FLAGS_DISPATCH_LEVEL : 0;
        returnFlags |= NDIS_RETURN_FLAGS_SWITCH_SINGLE_SOURCE;
        
        fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(NetBufferLists);
        
        if (sameSource)
        {
            sourcePortId = fwdDetail->SourcePortId;
            
            Switch->NdisSwitchHandlers.ReportFilteredNetBufferLists(
                                                     Switch->NdisSwitchContext,
                                                     &SxExtensionGuid,
                                                     &SxExtensionFriendlyName,
                                                     sourcePortId,
                                                     NDIS_SWITCH_REPORT_FILTERED_NBL_FLAGS_IS_INCOMING,
                                                     NumberOfNetBufferLists,
                                                     NetBufferLists,
                                                     &filterReason);
                                                     
            SxExtStartCompleteNetBufferListsEgress(Switch,
                                                   Switch->ExtensionContext,
                                                   NetBufferLists,
                                                   returnFlags);
        }
        else
        {
            curSourcePort = fwdDetail->SourcePortId;
            numNbls = 0;
            for (curNbl = NetBufferLists; curNbl != NULL; curNbl = nextNbl)
            {
                nextNbl = curNbl->Next;
                curNbl->Next = NULL;
                
                fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);
                
                if(curSourcePort == fwdDetail->SourcePortId)
                {
                    *curDropNbl = curNbl;
                    curDropNbl = &(curNbl->Next);
                    ++numNbls;
                }
                else
                {
                    Switch->NdisSwitchHandlers.ReportFilteredNetBufferLists(
                                                 Switch->NdisSwitchContext,
                                                 &SxExtensionGuid,
                                                 &SxExtensionFriendlyName,
                                                 curSourcePort,
                                                 NDIS_SWITCH_REPORT_FILTERED_NBL_FLAGS_IS_INCOMING,
                                                 numNbls,
                                                 dropNbl,
                                                 &filterReason);
                     
                    SxExtStartCompleteNetBufferListsEgress(Switch,
                                                           Switch->ExtensionContext,
                                                           dropNbl,
                                                           returnFlags);

                    numNbls = 1;
                    dropNbl = curNbl;
                    curDropNbl = &(curNbl->Next);
                    curSourcePort = fwdDetail->SourcePortId;
                }
            }
            
            Switch->NdisSwitchHandlers.ReportFilteredNetBufferLists(
                                             Switch->NdisSwitchContext,
                                             &SxExtensionGuid,
                                             &SxExtensionFriendlyName,
                                             curSourcePort,
                                             NDIS_SWITCH_REPORT_FILTERED_NBL_FLAGS_IS_INCOMING,
                                             numNbls,
                                             dropNbl,
                                             &filterReason);
                 
            SxExtStartCompleteNetBufferListsEgress(Switch,
                                                   Switch->ExtensionContext,
                                                   dropNbl,
                                                   returnFlags);
        }             
        
        goto Cleanup;
    }
    
    NdisFIndicateReceiveNetBufferLists(Switch->NdisFilterHandle,
                                       NetBufferLists,
                                       NDIS_DEFAULT_PORT_NUMBER,
                                       NumberOfNetBufferLists,
                                       ReceiveFlags);
                                
Cleanup:
    return;
}

VOID
SxLibCompleteNetBufferListsEgress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG ReturnFlags
    )
{
    NdisFReturnNetBufferLists(Switch->NdisFilterHandle,
                              NetBufferLists,
                              ReturnFlags);
}


VOID
SxLibCompleteNetBufferListsIngress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG SendCompleteFlags
    )
{
    NdisFSendNetBufferListsComplete(Switch->NdisFilterHandle,
                                    NetBufferLists,
                                    SendCompleteFlags);
}


NDIS_STATUS
SxLibIssueOidRequest(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_REQUEST_TYPE RequestType,
    _In_ NDIS_OID Oid,
    _In_opt_ PVOID InformationBuffer,
    _In_ ULONG InformationBufferLength,
    _In_ ULONG OutputBufferLength,
    _In_ ULONG MethodId,
    _In_ UINT Timeout,
    _Out_ PULONG BytesNeeded
    )
{
    NDIS_STATUS status;
    PSX_OID_REQUEST oidRequest;
    PNDIS_OID_REQUEST ndisOidRequest;
    ULONG bytesNeeded;
    BOOLEAN asyncCompletion;

    status = NDIS_STATUS_SUCCESS;
    oidRequest = NULL;
    bytesNeeded = 0;
    asyncCompletion = FALSE;

    NdisInterlockedIncrement(&Switch->PendingOidCount);

    if (Switch->ControlFlowState != SxSwitchAttached)
    {
        status = NDIS_STATUS_CLOSING;
        goto Cleanup;
    }

    //
    // Dynamically allocate filter request so that we can handle asynchronous
    // completion.
    //
    oidRequest = (PSX_OID_REQUEST)ExAllocatePoolWithTag(NonPagedPoolNx,
                                                        sizeof(SX_OID_REQUEST),
                                                        SxExtAllocationTag);
    if (oidRequest == NULL)
    {
        goto Cleanup;
    }

    NdisZeroMemory(oidRequest, sizeof(SX_OID_REQUEST));
    ndisOidRequest = &oidRequest->NdisOidRequest;
    NdisInitializeEvent(&oidRequest->ReqEvent);

    ndisOidRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
    ndisOidRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
    ndisOidRequest->Header.Size = sizeof(NDIS_OID_REQUEST);
    ndisOidRequest->RequestType = RequestType;
    ndisOidRequest->Timeout = Timeout;

    switch (RequestType)
    {
    case NdisRequestQueryInformation:
         ndisOidRequest->DATA.QUERY_INFORMATION.Oid = Oid;
         ndisOidRequest->DATA.QUERY_INFORMATION.InformationBuffer =
             InformationBuffer;
         ndisOidRequest->DATA.QUERY_INFORMATION.InformationBufferLength =
             InformationBufferLength;
        break;

    case NdisRequestSetInformation:
         ndisOidRequest->DATA.SET_INFORMATION.Oid = Oid;
         ndisOidRequest->DATA.SET_INFORMATION.InformationBuffer =
             InformationBuffer;
         ndisOidRequest->DATA.SET_INFORMATION.InformationBufferLength =
             InformationBufferLength;
        break;

    case NdisRequestMethod:
         ndisOidRequest->DATA.METHOD_INFORMATION.Oid = Oid;
         ndisOidRequest->DATA.METHOD_INFORMATION.MethodId = MethodId;
         ndisOidRequest->DATA.METHOD_INFORMATION.InformationBuffer =
             InformationBuffer;
         ndisOidRequest->DATA.METHOD_INFORMATION.InputBufferLength =
             InformationBufferLength;
         ndisOidRequest->DATA.METHOD_INFORMATION.OutputBufferLength =
             OutputBufferLength;
         break;

    default:
        NT_ASSERT(FALSE);
        break;
    }

    ndisOidRequest->RequestId = (PVOID)SxExtOidRequestId;
    status = NdisFOidRequest(Switch->NdisFilterHandle, ndisOidRequest);

    if (status == NDIS_STATUS_PENDING)
    {
        asyncCompletion = TRUE;
        NdisWaitEvent(&oidRequest->ReqEvent, 0);
    }
    else
    {
        SxpNdisCompleteInternalOidRequest(Switch, ndisOidRequest, status);
    }

    bytesNeeded = oidRequest->BytesNeeded;
    status = oidRequest->Status;

Cleanup:

    if (BytesNeeded != NULL)
    {
        *BytesNeeded = bytesNeeded;
    }
    
    if (!asyncCompletion)
    {
        NdisInterlockedDecrement(&Switch->PendingOidCount);
    }
    
    if (oidRequest != NULL)
    {
        ExFreePoolWithTag(oidRequest, SxExtAllocationTag);
    }
    
    return status;
}


NDIS_STATUS
SxLibGetNicArrayUnsafe(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Out_ PNDIS_SWITCH_NIC_ARRAY *NicArray
    )
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    ULONG BytesNeeded = 0;
    PNDIS_SWITCH_NIC_ARRAY nicArray = NULL;
    ULONG arrayLength = 0;
    
    do 
    {
        if (nicArray != NULL)
        {
            ExFreePoolWithTag(nicArray, SxExtAllocationTag);
        }
        
        if (BytesNeeded != 0)
        {
            arrayLength = BytesNeeded;
            nicArray = ExAllocatePoolWithTag(NonPagedPoolNx,
                                             arrayLength,
                                             SxExtAllocationTag);
            
            if (nicArray == NULL)
            {
                status = NDIS_STATUS_RESOURCES;
                goto Cleanup;
            }
            
            nicArray->Header.Revision = NDIS_SWITCH_PORT_ARRAY_REVISION_1;
            nicArray->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
            nicArray->Header.Size = (USHORT)arrayLength;
        }
    
        status = SxLibIssueOidRequest(Switch,
                                      NdisRequestQueryInformation,
                                      OID_SWITCH_NIC_ARRAY,
                                      nicArray,
                                      arrayLength,
                                      0,
                                      0,
                                      0,
                                      &BytesNeeded);
        
    } while(status == NDIS_STATUS_INVALID_LENGTH);
    
    *NicArray = nicArray;
Cleanup:
    if (status != NDIS_STATUS_SUCCESS &&
        nicArray != NULL)
    {
        ExFreePoolWithTag(nicArray, SxExtAllocationTag);
    }
    
    return status;
}
