/*++

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SxLibrary.c

Abstract:

    This file contains the common library functions that can be used
    by any extension using the SxBase library.


--*/

#include "precomp.h"

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