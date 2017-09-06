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


VOID
SxpNdisCompleteInternalOidRequest(
    _In_ PSWITCH_OBJECT Switch,
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

