/*++

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SxApi.h

Abstract:

    This file contains the API that must be implemented to
    create a switch extension using the SxBase library.


--*/

//
// The memory pool tag using in the extension.
//
extern ULONG  SxExtAllocationTag;

//
// The request ID used to identify OIDs initiated from this extension. 
//
extern ULONG  SxExtOidRequestId;


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
