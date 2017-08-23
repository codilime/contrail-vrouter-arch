#include "precomp.h"

#include <ndis.h>
#include <netiodef.h>
#include <intsafe.h>
#include <ntintsafe.h>

#include "vr_windows.h"
#include "windows_ndis.h"

NDIS_HANDLE SxDriverHandle = NULL;
NDIS_HANDLE SxDriverObject;

NDIS_SPIN_LOCK SxExtensionListLock;
LIST_ENTRY SxExtensionList;

// Function declarations
FILTER_ATTACH vr_win_attach;
FILTER_DETACH vr_win_detach;

FILTER_PAUSE vr_win_pause;
FILTER_RESTART vr_win_restart;

FILTER_SEND_NET_BUFFER_LISTS vr_win_send_net_buffer_lists;
FILTER_RECEIVE_NET_BUFFER_LISTS vr_win_receive_net_buffer_lists;

void
vr_win_uninitialize(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("%s: Unloading the vRouter Kernel Module\r\n", __func__);

    NdisFDeregisterFilterDriver(SxDriverHandle);
    NdisFreeSpinLock(&SxExtensionListLock);
}

NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath)
{
    NDIS_FILTER_DRIVER_CHARACTERISTICS fChars;
    NDIS_STRING serviceName;
    NDIS_STATUS status;

    UNREFERENCED_PARAMETER(RegistryPath);
    
    DbgPrint("%s: Loading vRouter Kernel Module\r\n", __func__);

    RtlInitUnicodeString(&serviceName, SxExtServiceName);
    RtlInitUnicodeString(&SxExtensionFriendlyName, SxExtFriendlyName);
    RtlInitUnicodeString(&SxExtensionGuid, SxExtUniqueName);
    SxDriverObject = DriverObject;

    NdisZeroMemory(&fChars, sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS));
    fChars.Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
    fChars.Header.Size = sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS);
    fChars.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_2;
    fChars.MajorNdisVersion = SxExtMajorNdisVersion;
    fChars.MinorNdisVersion = SxExtMinorNdisVersion;
    fChars.MajorDriverVersion = 1;
    fChars.MinorDriverVersion = 0;
    fChars.Flags = 0;
    fChars.FriendlyName = SxExtensionFriendlyName;
    fChars.UniqueName = SxExtensionGuid;
    fChars.ServiceName = serviceName;
    
    fChars.AttachHandler = vr_win_attach;
    fChars.DetachHandler = vr_win_detach;
    fChars.PauseHandler = vr_win_pause;
    fChars.RestartHandler = vr_win_restart;

    fChars.SendNetBufferListsHandler = vr_win_send_net_buffer_lists;
    fChars.SendNetBufferListsCompleteHandler = SxNdisSendNetBufferListsComplete;
    fChars.ReceiveNetBufferListsHandler = vr_win_receive_net_buffer_lists;
    fChars.ReturnNetBufferListsHandler = SxNdisReturnNetBufferLists;
    
    fChars.OidRequestHandler = SxNdisOidRequest;
    fChars.OidRequestCompleteHandler = SxNdisOidRequestComplete;
    
    NdisAllocateSpinLock(&SxExtensionListLock);
    InitializeListHead(&SxExtensionList);

    DriverObject->DriverUnload = vr_win_uninitialize;

    status = NdisFRegisterFilterDriver(DriverObject,
                                       (NDIS_HANDLE)SxDriverObject,
                                       &fChars,
                                       &SxDriverHandle);

    return status;
}

NDIS_STATUS
vr_win_attach(
    NDIS_HANDLE NdisFilterHandle,
    NDIS_HANDLE SxDriverContext,
    PNDIS_FILTER_ATTACH_PARAMETERS AttachParameters
    )
{
    NDIS_STATUS status;
    NDIS_FILTER_ATTRIBUTES sxAttributes;
    ULONG switchObjectSize;
    NDIS_SWITCH_CONTEXT switchContext;
    NDIS_SWITCH_OPTIONAL_HANDLERS switchHandler;
    PSX_SWITCH_OBJECT switchObject;
    
    UNREFERENCED_PARAMETER(SxDriverContext);

    DbgPrint("%s: Attaching the driver\r\n", __func__);

    status = NDIS_STATUS_SUCCESS;
    switchObject = NULL;

    ASSERT(SxDriverContext == (NDIS_HANDLE)SxDriverObject);

    if (AttachParameters->MiniportMediaType != NdisMedium802_3)
    {
        status = NDIS_STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    switchHandler.Header.Type = NDIS_OBJECT_TYPE_SWITCH_OPTIONAL_HANDLERS;
    switchHandler.Header.Size = NDIS_SIZEOF_SWITCH_OPTIONAL_HANDLERS_REVISION_1;
    switchHandler.Header.Revision = NDIS_SWITCH_OPTIONAL_HANDLERS_REVISION_1;

    status = NdisFGetOptionalSwitchHandlers(NdisFilterHandle,
                                            &switchContext,
                                            &switchHandler);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Extension is running in non-switch environment.\r\n", __func__);
        goto Cleanup;
    }

    switchObjectSize = sizeof(SX_SWITCH_OBJECT);
    switchObject = ExAllocatePoolWithTag(NonPagedPoolNx,
                                         switchObjectSize,
                                         SxExtAllocationTag);

    if (switchObject == NULL)
    {
        status = NDIS_STATUS_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory(switchObject, switchObjectSize);

    // Initialize NDIS related information.
    switchObject->NdisFilterHandle = NdisFilterHandle;
    switchObject->NdisSwitchContext = switchContext;
    RtlCopyMemory(&switchObject->NdisSwitchHandlers,
                  &switchHandler,
                  sizeof(NDIS_SWITCH_OPTIONAL_HANDLERS));

    // Let the extension create its own context.
    status = vr_intialize_vrouter(switchObject,
                               &(switchObject->ExtensionContext));

    if (status != NDIS_STATUS_SUCCESS)
    {
        goto Cleanup;
    }

    NdisZeroMemory(&sxAttributes, sizeof(NDIS_FILTER_ATTRIBUTES));
    sxAttributes.Header.Revision = NDIS_FILTER_ATTRIBUTES_REVISION_1;
    sxAttributes.Header.Size = sizeof(NDIS_FILTER_ATTRIBUTES);
    sxAttributes.Header.Type = NDIS_OBJECT_TYPE_FILTER_ATTRIBUTES;
    sxAttributes.Flags = 0;

    NDIS_DECLARE_FILTER_MODULE_CONTEXT(SX_SWITCH_OBJECT);
    status = NdisFSetAttributes(NdisFilterHandle, switchObject, &sxAttributes);

    if (status != NDIS_STATUS_SUCCESS)
    {
        DbgPrint("%s: Failed to set attributes.\n", __func__);
        goto Cleanup;
    }

    switchObject->ControlFlowState = SxSwitchAttached;
    switchObject->DataFlowState = SxSwitchPaused;
    
    NdisAcquireSpinLock(&SxExtensionListLock);
    InsertHeadList(&SxExtensionList, &switchObject->Link);
    NdisReleaseSpinLock(&SxExtensionListLock);
    
Cleanup:

    if (status != NDIS_STATUS_SUCCESS)
    {
        if (switchObject != NULL)
        {
            ExFreePoolWithTag(switchObject, SxExtAllocationTag);
        }
    }

    DbgPrint("%s: status %x\n", __func__, status);

    return status;
}

void
vr_win_detach(
    NDIS_HANDLE FilterModuleContext
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    DbgPrint("%s: Detaching the driver\r\n", __func__);

    // The extension must be in paused state.
    NT_ASSERT(switchObject->DataFlowState == SxSwitchPaused);
    switchObject->ControlFlowState = SxSwitchDetached;

    KeMemoryBarrier();

    while(switchObject->PendingOidCount > 0)
    {
        NdisMSleep(1000);
    }

    SxExtDeleteSwitch(switchObject, switchObject->ExtensionContext);

    NdisAcquireSpinLock(&SxExtensionListLock);
    RemoveEntryList(&switchObject->Link);
    NdisReleaseSpinLock(&SxExtensionListLock);
    
    ExFreePoolWithTag(switchObject, SxExtAllocationTag);

    // Alway return success.
    DEBUGP(DL_TRACE, ("<===SxDetach Successfully\n"));

    return;
}

NDIS_STATUS
vr_win_pause(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_FILTER_PAUSE_PARAMETERS PauseParameters
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)(FilterModuleContext);

    UNREFERENCED_PARAMETER(PauseParameters);

    DbgPrint("%s: Pausing the switch\r\n");

    // Set the flag that the filter is going to pause.
    NT_ASSERT(switchObject->DataFlowState == SxSwitchRunning);
    switchObject->DataFlowState = SxSwitchPaused;

    KeMemoryBarrier();

    while(switchObject->PendingInjectedNblCount > 0)
    {
        NdisMSleep(1000);
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
vr_win_restart(
    NDIS_HANDLE FilterModuleContext,
    PNDIS_FILTER_RESTART_PARAMETERS RestartParameters
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(RestartParameters);

    DbgPrint("%s: Restarting Switch\r\n", __func__);
           
    struct vr_switch_context *ctx = (struct vr_switch_context *)switchObject->ExtensionContext;

    PNDIS_SWITCH_NIC_ARRAY array;
    SxLibGetNicArrayUnsafe(switchObject, &array);

    for (unsigned i = 0; i < array->NumElements; i++){
        PNDIS_SWITCH_NIC_PARAMETERS element = NDIS_SWITCH_NIC_AT_ARRAY_INDEX(array, i);
        UpdateNics(element, TRUE);
    }

    ExFreePoolWithTag(array, SxExtAllocationTag);

    ctx->restart = FALSE;

    if (status != NDIS_STATUS_SUCCESS)
    {
        status = NDIS_STATUS_RESOURCES;
        goto Cleanup;
    }

    ASSERT(switchObject->DataFlowState == SxSwitchPaused);

    switchObject->DataFlowState = SxSwitchRunning;
   

Cleanup:
    return status;
}

void
vr_win_send_net_buffer_lists(
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

void
vr_win_receive_net_buffer_lists(
    NDIS_HANDLE FilterModuleContext,
    PNET_BUFFER_LIST NetBufferLists,
    NDIS_PORT_NUMBER PortNumber,
    ULONG NumberOfNetBufferLists,
    ULONG ReceiveFlags
    )
{
    PSX_SWITCH_OBJECT switchObject = (PSX_SWITCH_OBJECT)FilterModuleContext;

    UNREFERENCED_PARAMETER(PortNumber);

    DbgPrint("%s: Receive NBL\r\n", __func__);
    SxLibSendNetBufferListsEgress(switchObject,
        NetBufferLists,
        NumberOfNetBufferLists,
        ReceiveFlags);
}
