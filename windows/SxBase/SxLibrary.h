/*++

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SxLibrary.h

Abstract:

    This file contains the common library function headers that can be
    used by any extension using the SxBase library.


--*/


/*++

SxLibIssueOidRequest

Routine Description:

    Utility routine that forms and sends an NDIS_OID_REQUEST to the
    miniport, waits for it to complete, and returns status
    to the caller.

    NOTE: this assumes that the calling routine ensures validity
    of the filter handle until this returns.
    
    This function can only be called at PASSIVE_LEVEL.

Arguments:

    Switch - pointer to our switch object.

    RequestType - NdisRequest[Set|Query|method]Information.

    Oid - the object being set/queried.

    InformationBuffer - data for the request.

    InformationBufferLength - length of the above.

    OutputBufferLength - valid only for method request.

    MethodId - valid only for method request.
    
    Timeout - The timeout in seconds for the OID. 

    BytesNeeded - place to return bytes read/written.

Return Value:

    NDIS_STATUS_***

--*/
NDIS_STATUS
SxLibIssueOidRequest(
    _In_ PSWITCH_OBJECT SxSwitch,
    _In_ NDIS_REQUEST_TYPE RequestType,
    _In_ NDIS_OID Oid,
    _In_opt_ PVOID InformationBuffer,
    _In_ ULONG InformationBufferLength,
    _In_ ULONG OutputBufferLength,
    _In_ ULONG MethodId,
    _In_ UINT Timeout,
    _Out_ PULONG BytesNeeded
    );


/*++

SxLibGetNicArrayUnsafe
  
Routine Description:
    This function is called to get the current array
    of NICs.
    
    NOTE: It is necessary to synchonize this with SxExtNicConnect
    and SxExtNicDisconnect.
    
Arguments:

    Switch - the Switch context
    
    NicArray - the returned NIC array
    
Return Value:
    NDIS_STATUS_SUCCESS - if NicArray was successfully allocated
                          and returned
                          
    NDIS_STATUS_*** - otherwise
   
--*/
NDIS_STATUS
SxLibGetNicArrayUnsafe(
    _In_ PSWITCH_OBJECT SxSwitch,
    _Out_ PNDIS_SWITCH_NIC_ARRAY *NicArray
    );
