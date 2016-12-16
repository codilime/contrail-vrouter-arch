#include "precomp.h"

UCHAR SxExtMajorNdisVersion = NDIS_FILTER_MAJOR_VERSION;
UCHAR SxExtMinorNdisVersion = NDIS_FILTER_MINOR_VERSION;

PWCHAR SxExtFriendlyName = L"OpenContrail's vRouter forwarding extension";

PWCHAR SxExtUniqueName = L"{56553588-1538-4BE6-B8E0-CB46402DC205}";

PWCHAR SxExtServiceName = L"vRouter";

ULONG  SxExtAllocationTag = 'RVCO';
ULONG  SxExtOidRequestId = 'RVCO';

static char encoding_table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/' };
static int mod_table[] = { 0, 2, 1 };

char *base64_encode(const unsigned char *data, size_t input_length) {
	size_t output_length = 4 * ((input_length + 2) / 3);

	char *encoded_data = ExAllocatePoolWithTag(NonPagedPoolNx, output_length + 1, SxExtAllocationTag);
	if (encoded_data == NULL) return NULL;

	for (int i = 0, j = 0; i < input_length;) {

		UINT32 octet_a = i < input_length ? (unsigned char)data[i++] : 0;
		UINT32 octet_b = i < input_length ? (unsigned char)data[i++] : 0;
		UINT32 octet_c = i < input_length ? (unsigned char)data[i++] : 0;

		UINT32 triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[output_length - 1 - i] = '=';
	encoded_data[output_length] = 0;

	return encoded_data;
}

NDIS_STATUS
SxExtInitialize(PDRIVER_OBJECT DriverObject)
{
	DbgPrint("SxExtInitialize\r\n");
	UNREFERENCED_PARAMETER(DriverObject);

	return NDIS_STATUS_SUCCESS;
}

VOID
SxExtUninitialize(PDRIVER_OBJECT DriverObject)
{
	DbgPrint("SxExtUninitialize\r\n");
	UNREFERENCED_PARAMETER(DriverObject);
}

NDIS_STATUS
SxExtCreateSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_Outptr_result_maybenull_ PNDIS_HANDLE *ExtensionContext
)
{
	DbgPrint("SxExtCreateSwitch\r\n");
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);

	PNDIS_RW_LOCK_EX lock = NdisAllocateRWLock(Switch->NdisFilterHandle);

	*ExtensionContext = (NDIS_HANDLE)lock;

	return 0;
}

VOID
SxExtDeleteSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	DbgPrint("SxExtDeleteSwitch\r\n");
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);

	NdisFreeRWLock(ExtensionContext);
}

VOID
SxExtActivateSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	DbgPrint("SxExtActivateSwitch\r\n");
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
}

NDIS_STATUS
SxExtRestartSwitch(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext
)
{
	DbgPrint("SxExtRestartSwitch\r\n");
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
	DbgPrint("SxExtPauseSwitch\r\n");
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
	DbgPrint("SxExtCreatePort\r\n");
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
	DbgPrint("SxExtUpdatePort\r\n");
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);
}

NDIS_STATUS
SxExtCreateNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	DbgPrint("SxExtCreateNic\r\n");
	UNREFERENCED_PARAMETER(Switch);
	UNREFERENCED_PARAMETER(ExtensionContext);

	DbgPrint("NicFriendlyName: %S, NicName: %S, NicIndex: %u, NicState: %u, NicType: %u, PortId: %u, PermamentMacAddress: %s, CurrentMacAddress: %s, VMMacAddress: %s, VmName: %S",
		Nic->NicFriendlyName.String, Nic->NicName.String, Nic->NicIndex, Nic->NicState, Nic->NicType, Nic->PortId, Nic->PermanentMacAddress, Nic->CurrentMacAddress, Nic->VMMacAddress, Nic->VmName.String);

	return 0;
}

VOID
SxExtConnectNic(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNDIS_SWITCH_NIC_PARAMETERS Nic
)
{
	DbgPrint("SxExtConnectNic\r\n");
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
	DbgPrint("SxExtUpdateNic\r\n");
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
	DbgPrint("SxExtDisconnectNic\r\n");
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
	DbgPrint("SxExtDeleteNic\r\n");
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
	DbgPrint("SxExtTeardownPort\r\n");
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
	DbgPrint("SxExtDeletePort\r\n");
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
	DbgPrint("SxExtSaveNic\r\n");
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
	DbgPrint("SxExtSaveNicComplete\r\n");
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
	DbgPrint("SxExtNicRestore\r\n");
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
	DbgPrint("SxExtNicRestoreComplete\r\n");
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
	DbgPrint("SxExtAddSwitchProperty\r\n");
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
	DbgPrint("SxExtUpdateSwitchProperty\r\n");
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
	DbgPrint("SxExtDeleteSwitchProperty\r\n");
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
	DbgPrint("SxExtAddPortProperty\r\n");
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
	DbgPrint("SxExtUpdatePortProperty\r\n");
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
	DbgPrint("SxExtDeletePortProperty\r\n");
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
	DbgPrint("SxExtQuerySwitchFeatureStatus\r\n");
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
	DbgPrint("SxExtQueryPortFeatureStatus\r\n");
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
	DbgPrint("SxExtProcessNicRequest\r\n");
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
	DbgPrint("SxExtProcessNicRequestComplete\r\n");
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
	DbgPrint("SxExtProcessNicStatus\r\n");
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
	DbgPrint("SxExtStartNetBufferListsIngress\r\n");

	PNDIS_RW_LOCK_EX lock = (PNDIS_RW_LOCK_EX)ExtensionContext;
	LOCK_STATE_EX lockState;

	BOOLEAN sameSource;
	ULONG sendCompleteFlags = 0;
	BOOLEAN dispatch;

	PNDIS_SWITCH_FORWARDING_DETAIL_NET_BUFFER_LIST_INFO fwdDetail;

	PNET_BUFFER_LIST dropNbl = NULL;
	PNET_BUFFER_LIST extForwardedNbls = NULL;
	PNET_BUFFER_LIST nativeForwardedNbls = NULL;
	PNET_BUFFER_LIST *nextExtForwardNbl = &extForwardedNbls;
	PNET_BUFFER_LIST *nextNativeForwardedNbl = &nativeForwardedNbls;

	PNET_BUFFER_LIST curNbl = NULL, nextNbl = NULL;

	PMDL curMdl;
	PUINT8 curBuffer;

	dispatch = NDIS_TEST_SEND_FLAG(SendFlags, NDIS_SEND_FLAGS_DISPATCH_LEVEL);
	sameSource = NDIS_TEST_SEND_FLAG(SendFlags, NDIS_SEND_FLAGS_SWITCH_SINGLE_SOURCE);

	sendCompleteFlags |= (dispatch) ? NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL : 0;
	SendFlags |= NDIS_SEND_FLAGS_SWITCH_DESTINATION_GROUP;

	// Acquire the lock, now interfaces cannot disconnect, etc.
	NdisAcquireRWLockRead(lock, &lockState, dispatch);

	// Mark the flag is everything comes from a single source
	if (sameSource)
	{
		sendCompleteFlags |= NDIS_SEND_COMPLETE_FLAGS_SWITCH_SINGLE_SOURCE;
		DbgPrint("Same-source NBL\r\n");
	}
	else
	{
		DbgPrint("Not same-source NBL\r\n");
	}

	for (curNbl = NetBufferLists; curNbl != NULL; curNbl = nextNbl)
	{
		nextNbl = curNbl->Next;

		fwdDetail = NET_BUFFER_LIST_SWITCH_FORWARDING_DETAIL(curNbl);

		if (fwdDetail->NativeForwardingRequired)
		{
			DbgPrint("Native forwarded NBL\r\n");
			*nextNativeForwardedNbl = curNbl;
			nextNativeForwardedNbl = &(curNbl->Next);
		}
		else
		{
			DbgPrint("Non-native forewarded NBL\r\n");
			*nextExtForwardNbl = curNbl;
			nextExtForwardNbl = &(curNbl->Next);
		}
	}

	for (curNbl = extForwardedNbls; curNbl != NULL; curNbl = nextNbl)
	{
		nextNbl = curNbl->Next;
		curNbl->Next = NULL;

		//For now, assume everything important is in a single MDL
		curMdl = (NET_BUFFER_LIST_FIRST_NB(curNbl))->CurrentMdl;
		curBuffer = MmGetSystemAddressForMdlSafe(
			curMdl,
			LowPagePriority | MdlMappingNoExecute);
			
		char* str = base64_encode(curBuffer + NET_BUFFER_LIST_FIRST_NB(curNbl)->CurrentMdlOffset, NET_BUFFER_LIST_FIRST_NB(curNbl)->DataLength);
		DbgPrint("Packet data: %s\r\n", str);

		ExFreePoolWithTag(str, SxExtAllocationTag);
	}

	if (nativeForwardedNbls != NULL)
	{
		SxLibSendNetBufferListsIngress(Switch,
			nativeForwardedNbls,
			SendFlags,
			0);
	}

	if (extForwardedNbls != NULL)
	{
		SxLibSendNetBufferListsIngress(Switch,
			extForwardedNbls,
			SendFlags,
			0);
	}

	if (dropNbl != NULL)
	{
		SxLibCompleteNetBufferListsIngress(Switch,
			dropNbl,
			sendCompleteFlags);
	}

	// Release the lock, now interfaces can disconnect, etc.
	NdisReleaseRWLock(lock, &lockState);
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
	DbgPrint("SxExtStartNetBufferListsEgress\r\n");
	UNREFERENCED_PARAMETER(ExtensionContext);

	SxLibSendNetBufferListsEgress(Switch,
		NetBufferLists,
		NumberOfNetBufferLists,
		ReceiveFlags);
}

VOID
SxExtStartCompleteNetBufferListsEgress(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNET_BUFFER_LIST NetBufferLists,
	_In_ ULONG ReturnFlags
)
{
	DbgPrint("SxExtStartCompleteNetBufferListsEgress\r\n");
	UNREFERENCED_PARAMETER(ExtensionContext);

	SxLibCompleteNetBufferListsEgress(Switch,
		NetBufferLists,
		ReturnFlags);
}

VOID
SxExtStartCompleteNetBufferListsIngress(
	_In_ PSX_SWITCH_OBJECT Switch,
	_In_ NDIS_HANDLE ExtensionContext,
	_In_ PNET_BUFFER_LIST NetBufferLists,
	_In_ ULONG SendCompleteFlags
)
{
	DbgPrint("SxExtStartCompleteNetBufferListsIngress\r\n");
	UNREFERENCED_PARAMETER(ExtensionContext);

	SxLibCompleteNetBufferListsIngress(Switch,
		NetBufferLists,
		SendCompleteFlags);
}

