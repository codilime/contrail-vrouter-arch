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

typedef struct _SWITCH_OBJECT
{
    //
    // The extension context is the context used in the specific logic
    // of this extension.
    // This is allocated and returned in SxExtSwitchCreate
    //
    PNDIS_HANDLE ExtensionContext;

    // Ndis related fields.
    NDIS_HANDLE NdisFilterHandle;
    NDIS_SWITCH_CONTEXT NdisSwitchContext;
    NDIS_SWITCH_OPTIONAL_HANDLERS NdisSwitchHandlers;

    // Switch state.
    volatile BOOLEAN Running;
    
    // Management fields.
    volatile LONG PendingOidCount;
    
    // Control Path Management.
    PNDIS_SWITCH_NIC_OID_REQUEST OldNicRequest;
    
} SWITCH_OBJECT, *PSWITCH_OBJECT;

typedef struct _SX_OID_REQUEST
{
    NDIS_OID_REQUEST NdisOidRequest;
    NDIS_EVENT ReqEvent;
    NDIS_STATUS Status;
    ULONG BytesNeeded;

} SX_OID_REQUEST, *PSX_OID_REQUEST;


VOID
SxpNdisCompleteInternalOidRequest(
    _In_ PSWITCH_OBJECT Switch,
    _In_ PNDIS_OID_REQUEST NdisRequest,
    _In_ NDIS_STATUS Status
    );
