/*++

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SxApi.h

Abstract:

    This file contains the API that must be implemented to
    create a switch extension using the SxBase library.


--*/

//
// The major version of NDIS the driver is using.
// For NDIS 6.30, use NDIS_FILTER_MAJOR_VERSION.
//
extern UCHAR  SxExtMajorNdisVersion;

//
// The minor version of NDIS the driver is using.
// For NDIS 6.30, use NDIS_FILTER_MINOR_VERSION.
//
extern UCHAR  SxExtMinorNdisVersion;

//
// The friendly name of the extension.
//
extern PWCHAR SxExtFriendlyName;

//
// The unique name of the extension.
//
extern PWCHAR SxExtUniqueName;

//
// The service name of the extension from the INF.
//
extern PWCHAR SxExtServiceName;

//
// The memory pool tag using in the extension.
//
extern ULONG  SxExtAllocationTag;

//
// The request ID used to identify OIDs initiated from this extension. 
//
extern ULONG  SxExtOidRequestId;


/*++

SxExtInitialize
  
Routine Description:
    This function is called from the SxBase Library during DriverEntry.
    An extension should allocate/initalize all global data in this function.
      
Arguments:
    DriverObject - Driver object.
    
Return Value:
    NDIS_STATUS_SUCCESS succeeds driver entry.
    
    NDIS_STATUS_*** fails driver entry.
   
--*/
NDIS_STATUS
SxExtInitialize(PDRIVER_OBJECT DriverObject);


/*++

SxExtUninitialize
  
Routine Description:
    This function is called from the SxBase Library during DriverUnload.
    An extension should free/reset all global data in this function.
      
Arguments:
    NULL
    
Return Value:
    VOID
   
--*/
VOID
SxExtUninitialize(PDRIVER_OBJECT DriverObject);


/*++

SxExtCreateSwitch
  
Routine Description:
    This function is called when an extension binds to a new switch.
    All switch specific data should be allocated during this function.
    OIDs cannot be sent from this function, and both the control
    and data paths are inactive.
      
Arguments:
    Switch - the Switch Object currently being created
    
    ExtensionContext -  Extension context specific to the switch being
                        createf. This context will be passed back to the
                        extension for all function calls in SxApi
    
Return Value:
    NDIS_STATUS_SUCCESS succeeds switch creation.
    
    NDIS_STATUS_RESOURCES fails switch creation because of insufficient
                          resources.
                          
    NDIS_STATUS_FAILURE fails switch creation.
   
--*/
NDIS_STATUS
SxExtCreateSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _Outptr_result_maybenull_ PNDIS_HANDLE *ExtensionContext
    );

    
/*++

SxExtDeleteSwitch
  
Routine Description:
    This function is called when an extension binds to a new switch.
    All switch specific data should be allocated/initialized during
    this function.
      
Arguments:
    Switch - the Switch being deleted
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch being deleted.
    
Return Value:
    VOID
   
--*/
VOID
SxExtDeleteSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
    );


/*++

SxExtRestartSwitch
  
Routine Description:
    This function is called to restart a switch from a paused state.
      
Arguments:
    Switch - the Switch being restarted
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch being restarted.
    
Return Value:
    NDIS_STATUS_SUCCESS succeeds switch restart.
    
    NDIS_STATUS_RESOURCES fails switch restart because of insufficient
                          resources.
                          
    NDIS_STATUS_FAILURE fails switch restart.
   
--*/
NDIS_STATUS
SxExtRestartSwitch(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext
    );


/*++

SxExtCreatePort
  
Routine Description:
    This function is called to create a new port on a switch.
      
Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    Port - the Port being created
    
Return Value:
    NDIS_STATUS_SUCCESS to succeed port creation
    
    NDIS_STATUS_*** to fail port creation
   
--*/  
NDIS_STATUS
SxExtCreatePort(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_PORT_PARAMETERS Port
    );


/*++

SxExtCreateNic
  
Routine Description:
    This function is called to create a new NIC to be connected
    to a switch.
    The extension may allocate context for this NIC, and traffic may
    start to flow from this NIC, but it may not be used as a destination
    until SxExtConnectNic has been called.
      
Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    Nic - the NIC being created
    
Return Value:
    NDIS_STATUS_SUCCESS to succeed NIC creation
    
    NDIS_STATUS_*** to fail NIC creation
   
--*/  
NDIS_STATUS
SxExtCreateNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
    );

    
/*++

SxExtConnectNic
  
Routine Description:
    This function is called to connect a NIC to a switch.
    After returning from this function the extension can use this NIC
    as a destination.
      
Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    Nic - the NIC being connected
    
Return Value:
    VOID
   
--*/  
VOID
SxExtConnectNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
    );

    
/*++

SxExtUpdateNic
  
Routine Description:
    This function is called to update an already created NIC.
      
Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    Nic - the NIC being updated
    
Return Value:
    VOID
   
--*/  
VOID
SxExtUpdateNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
    );

    
/*++

SxExtDisconnectNic
  
Routine Description:
    This function is called to disconnect a NIC from a switch.
    After returning from this function the extension cannot use
    this NIC as a destination.
      
Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    Nic - the NIC being disconnected
    
Return Value:
    VOID
   
--*/ 
VOID
SxExtDisconnectNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
    );

    
/*++

SxExtDeleteNic
  
Routine Description:
    This function is called to delete a NIC from a switch.
    No futher traffic/control will be recieved for this NIC.

Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    Nic - the NIC being deleted
    
Return Value:
    VOID
   
--*/
VOID
SxExtDeleteNic(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
    );


/*++

SxExtStartNetBufferListsIngress
  
Routine Description:
    This function is called upon the receipt on an NBL on ingress.
    The extension should call SxLibSendNetBufferListsIngress to continue
    the send of the NBL on ingress.
    The extension should call SxLibCompleteNetBufferListsIngress to
    drop the NBL.
    This function may also be call from egress to inject an NBL.

Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    NetBufferLists - the NBL to be sent
        
    SendFlags - the send flags from NDIS, equivalent to NDIS send flags for
                NdisFSendNetBufferLists
    
Return Value:
    VOID
   
--*/  
VOID
SxExtStartNetBufferListsIngress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG SendFlags
    );


/*++

SxExtStartNetBufferListsEgress
  
Routine Description:
    This function is called upon the receipt on an NBL on egress.
    The extension should call SxLibSendNetBufferListsEgress to continue
    the send of the NBL on egress.
    The extension should call SxLibSendNetBufferListsEgressComplete to
    drop the NBL.

Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    NetBufferLists - the NBL to be sent
        
    NumberOfNetBufferLists - the number of NBLs in NetBufferLists
    
    ReceiveFlags - the receive flags from NDIS, equivalent to NDIS receive flags for
                   NdisFIndicateReceiveNetBufferLists
    
Return Value:
    VOID
   
--*/     
VOID
SxExtStartNetBufferListsEgress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG NumberOfNetBufferLists,
    _In_ ULONG ReceiveFlags
    );

    
/*++

SxExtStartCompleteNetBufferListsEgress
  
Routine Description:
    This function is called upon the completion of an NBL on egress.
    The extension must call SxLibCompleteNetBufferListsEgress
    once it has finished processing the NBL.

Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    NetBufferLists - the NBL being completed
    
    ReturnFlags - the return flags from NDIS, equivalent to NDIS return flags for
                  NdisFReturnNetBufferLists
    
Return Value:
    VOID
   
--*/ 
VOID
SxExtStartCompleteNetBufferListsEgress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG ReturnFlags
    );

    
/*++

SxExtStartCompleteNetBufferListsIngress
  
Routine Description:
    This function is called upon the completion of an NBL on ingress.
    The extension must call SxLibCompleteNetBufferListsIngress
    once it has finished processing the NBL.
    
    If there are NBLs injected by this extension in NetBufferLists,
    the extension must NOT call SxLibCompleteNetBufferListsIngress, and
    instead call SxLibCompletedInjectedNetBufferLists with the number
    of injected NBLs completed.

Arguments:
    Switch - the Switch context
    
    ExtensionContext - The extension context allocated in SxExtCreateSwitch
                       for the switch
                       
    NetBufferLists - the NBL being completed
        
    SendCompleteFlags - the send complete flags from NDIS, equivalent to
                        NDIS send complete flags for
                        NdisFSendNetBufferListsComplete
    
Return Value:
    VOID
   
--*/ 
VOID
SxExtStartCompleteNetBufferListsIngress(
    _In_ PSX_SWITCH_OBJECT Switch,
    _In_ NDIS_HANDLE ExtensionContext,
    _In_ PNET_BUFFER_LIST NetBufferLists,
    _In_ ULONG SendCompleteFlags
    );
    
