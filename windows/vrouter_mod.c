#include <ndis.h>
#include <netiodef.h>
#include <intsafe.h>
#include <ntintsafe.h>

#include "SxBase\SxBase.h"
#include "SxBase\SxApi.h"
#include "SxBase\SxLibrary.h"

UCHAR  SxExtMajorNdisVersion;

UCHAR  SxExtMinorNdisVersion;

PWCHAR SxExtFriendlyName;

PWCHAR SxExtUniqueName;

PWCHAR SxExtServiceName;

ULONG  SxExtAllocationTag;

ULONG  SxExtOidRequestId;

NDIS_STATUS
SxExtInitialize(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	return 0;
}

VOID
SxExtUninitialize(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
}

NDIS_STATUS
SxExtCreateSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_Outptr_result_maybenull_ PNDIS_HANDLE *ExtensionContext
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);

	return 0;
}

VOID
SxExtDeleteSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
}

VOID
SxExtActivateSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
}

NDIS_STATUS
SxExtRestartSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);

	return 0;
}

VOID
SxExtPauseSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
}

NDIS_STATUS
SxExtCreatePort(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Port);

	return 0;
}

VOID
SxExtUpdatePort(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Port);
}

NDIS_STATUS
SxExtCreateNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Nic);

	return 0;
}

VOID
SxExtConnectNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Nic);
}

VOID
SxExtUpdateNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Nic);
}

VOID
SxExtDisconnectNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Nic);
}

VOID
SxExtDeleteNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Nic);
}

VOID
SxExtTeardownPort(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Port);
}

VOID
SxExtDeletePort(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PARAMETERS Port
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(Port);
}

NDIS_STATUS
SxExtSaveNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_Inout_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState,
	_Out_ PULONG BytesWritten,
	_Out_ PULONG BytesNeeded
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SaveState);
	UNREFERENCED_PARAMETER(BytesWritten);
	UNREFERENCED_PARAMETER(BytesNeeded);

	return 0;
}

VOID
SxExtSaveNicComplete(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SaveState);
}

NDIS_STATUS
SxExtNicRestore(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState,
	_Out_ PULONG BytesRestored
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SaveState);
	UNREFERENCED_PARAMETER(BytesRestored);

	return 0;
}

VOID
SxExtNicRestoreComplete(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_SAVE_STATE SaveState
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SaveState);
}

NDIS_STATUS
SxExtAddSwitchProperty(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PROPERTY_PARAMETERS SwitchProperty
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SwitchProperty);

	return 0;
}

NDIS_STATUS
SxExtUpdateSwitchProperty(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PROPERTY_PARAMETERS SwitchProperty
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SwitchProperty);

	return 0;
}

BOOLEAN
SxExtDeleteSwitchProperty(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PROPERTY_DELETE_PARAMETERS SwitchProperty
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SwitchProperty);

	return 0;
}

NDIS_STATUS
SxExtAddPortProperty(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PROPERTY_PARAMETERS PortProperty
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(PortProperty);

	return 0;
}

NDIS_STATUS
SxExtUpdatePortProperty(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PROPERTY_PARAMETERS PortProperty
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(PortProperty);

	return 0;
}

BOOLEAN
SxExtDeletePortProperty(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_PORT_PROPERTY_DELETE_PARAMETERS PortProperty
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(PortProperty);

	return 0;
}

BOOLEAN
SxExtQuerySwitchFeatureStatus(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_Inout_ PNDIS_SWITCH_FEATURE_STATUS_PARAMETERS SwitchFeatureStatus,
	_Inout_ PULONG BytesNeeded
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(SwitchFeatureStatus);
	UNREFERENCED_PARAMETER(BytesNeeded);

	return 0;
}

BOOLEAN
SxExtQueryPortFeatureStatus(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_Inout_ PNDIS_SWITCH_PORT_FEATURE_STATUS_PARAMETERS PortFeatureStatus,
	_Inout_ PULONG BytesNeeded
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(PortFeatureStatus);
	UNREFERENCED_PARAMETER(BytesNeeded);

	return 0;
}

NDIS_STATUS
SxExtProcessNicRequest(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_Inout_ PNDIS_OID_REQUEST OidRequest,
	_Inout_ PNDIS_SWITCH_PORT_ID SourcePortId,
	_Inout_ PNDIS_SWITCH_NIC_INDEX SourceNicIndex,
	_Inout_ PNDIS_SWITCH_PORT_ID DestinationPortId,
	_Inout_ PNDIS_SWITCH_NIC_INDEX DestinationNicIndex
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(OidRequest);
	UNREFERENCED_PARAMETER(SourcePortId);
	UNREFERENCED_PARAMETER(SourceNicIndex);
	UNREFERENCED_PARAMETER(DestinationPortId);
	UNREFERENCED_PARAMETER(DestinationNicIndex);

	return 0;
}

NDIS_STATUS
SxExtProcessNicRequestComplete(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_Inout_ PNDIS_OID_REQUEST OidRequest,
	_In_ NDIS_SWITCH_PORT_ID SourcePortId,
	_In_ NDIS_SWITCH_NIC_INDEX SourceNicIndex,
	_In_ NDIS_SWITCH_PORT_ID DestinationPortId,
	_In_ NDIS_SWITCH_NIC_INDEX DestinationNicIndex,
	_In_ NDIS_STATUS Status
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(OidRequest);
	UNREFERENCED_PARAMETER(SourcePortId);
	UNREFERENCED_PARAMETER(SourceNicIndex);
	UNREFERENCED_PARAMETER(DestinationPortId);
	UNREFERENCED_PARAMETER(DestinationNicIndex);
	UNREFERENCED_PARAMETER(Status);

	return 0;
}

NDIS_STATUS
SxExtProcessNicStatus(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_STATUS_INDICATION StatusIndication,
	_In_ NDIS_SWITCH_PORT_ID SourcePortId,
	_In_ NDIS_SWITCH_NIC_INDEX SourceNicIndex
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(StatusIndication);
	UNREFERENCED_PARAMETER(SourcePortId);
	UNREFERENCED_PARAMETER(SourceNicIndex);

	return 0;
}

VOID
SxExtStartNetBufferListsIngress(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNET_BUFFER_LIST NetBufferLists,
	_In_ ULONG SendFlags
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(NetBufferLists);
	UNREFERENCED_PARAMETER(SendFlags);
}

VOID
SxExtStartNetBufferListsEgress(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNET_BUFFER_LIST NetBufferLists,
	_In_ ULONG NumberOfNetBufferLists,
	_In_ ULONG ReceiveFlags
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(NetBufferLists);
	UNREFERENCED_PARAMETER(NumberOfNetBufferLists);
	UNREFERENCED_PARAMETER(ReceiveFlags);
}

VOID
SxExtStartCompleteNetBufferListsEgress(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNET_BUFFER_LIST NetBufferLists,
	_In_ ULONG ReturnFlags
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(NetBufferLists);
	UNREFERENCED_PARAMETER(ReturnFlags);
}

VOID
SxExtStartCompleteNetBufferListsIngress(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNET_BUFFER_LIST NetBufferLists,
	_In_ ULONG SendCompleteFlags
)
{
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
	UNREFERENCED_PARAMETER(NetBufferLists);
	UNREFERENCED_PARAMETER(SendCompleteFlags);
}

