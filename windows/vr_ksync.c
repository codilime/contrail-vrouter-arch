#include "windows_mem.h"
#include "windows_devices.h"
#include "Ntstrsafe.h"
#include "vr_message.h"

#include "windows_ksync.h"

#include "vr_genetlink.h"

static ULONG KsyncAllocationTag = 'NYSK';

const WCHAR KsyncDeviceName[] = L"\\Device\\vrouterKsync";
const WCHAR KsyncDeviceSymLink[] = L"\\DosDevices\\vrouterKsync";

static PDEVICE_OBJECT KsyncDeviceObject = NULL;
static BOOLEAN KsyncSymlinkCreated = FALSE;

static struct ksync_device_context *
ksync_alloc_context()
{
    struct ksync_device_context *ctx = ExAllocatePoolWithTag(NonPagedPoolNx,
                                                             sizeof(*ctx),
                                                             KsyncAllocationTag);
    if (ctx == NULL)
        return NULL;

    ctx->responses = NULL;

    return ctx;
}

static void
ksync_attach_context_to_file_context(struct ksync_device_context *ctx, PIRP irp)
{
    PIO_STACK_LOCATION io_stack = IoGetCurrentIrpStackLocation(irp);
    PFILE_OBJECT file_obj = io_stack->FileObject;
    file_obj->FsContext = ctx;
}

static struct ksync_device_context *
ksync_get_context_from_file_context(PIRP irp)
{
    PIO_STACK_LOCATION io_stack = IoGetCurrentIrpStackLocation(irp);
    PFILE_OBJECT file_obj = io_stack->FileObject;
    return file_obj->FsContext;
}

static struct ksync_response *
KsyncResponseCreate()
{
    struct ksync_response *resp;

    resp = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct ksync_response), KsyncAllocationTag);
    if (resp != NULL) {
        // Zeroing whole struct sets type = 0 (DONE) and gives len = 0, which is a valid response.
        RtlZeroMemory(resp, sizeof(struct ksync_response));
    }

    return resp;
}

static void
KsyncResponseDelete(struct ksync_response *resp)
{
    ASSERT(resp != NULL);

    ExFreePoolWithTag(resp, KsyncAllocationTag);
}

static void
KsyncAppendResponse(struct ksync_device_context *ctx, struct ksync_response *resp)
{
    struct ksync_response *elem;
    struct ksync_response **iter = &(ctx->responses);

    while (*iter != NULL) {
        elem = *iter;
        iter = &(elem->next);
    }

    *iter = resp;
}

static struct ksync_response *
KsyncPopResponse(struct ksync_device_context *ctx)
{
    struct ksync_response *resp = ctx->responses;

    if (resp != NULL) {
        ctx->responses = resp->next;
        resp->next = NULL;
        return resp;
    } else {
        return NULL;
    }
}

static inline NTSTATUS
KsyncHandleRet(PIRP Irp, NTSTATUS ret_code)
{
    Irp->IoStatus.Status = ret_code;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return ret_code;
}

static inline NTSTATUS
KsyncHandleRetWithInfo(PIRP Irp, NTSTATUS ret_code, ULONG parameter)
{
    Irp->IoStatus.Status = ret_code;
    Irp->IoStatus.Information = parameter;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return ret_code;
}

NTSTATUS
KsyncDispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    struct ksync_device_context *ctx = ksync_alloc_context();
    if (ctx == NULL) {
        return KsyncHandleRet(Irp, STATUS_INSUFFICIENT_RESOURCES);
    }
    ksync_attach_context_to_file_context(ctx, Irp);

    return KsyncHandleRetWithInfo(Irp, STATUS_SUCCESS, FILE_OPENED);
}

NTSTATUS
KsyncDispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    struct ksync_device_context *ctx = ksync_get_context_from_file_context(Irp);
    ASSERTMSG("KSync device context was not set", ctx != NULL);

    ExFreePoolWithTag(ctx, KsyncAllocationTag);

    return KsyncHandleRet(Irp, STATUS_SUCCESS);
}

static NTSTATUS
KsyncHandleWrite(struct ksync_device_context *ctx, uint8_t *buffer, size_t buffer_size)
{
    struct vr_message request;
    struct vr_message *response;
    uint32_t multi_flag;
    int ret;

    /* Received buffer contains tightly packed Netlink headers, thus we can just
       increment appropriate headers */
    struct nlmsghdr   *nlh   = (struct nlmsghdr *)(buffer);
    struct genlmsghdr *genlh = (struct genlmsghdr *)(nlh + 1);
    struct nlattr     *nla   = (struct nlattr *)(genlh + 1);

    request.vr_message_buf = NLA_DATA(nla);
    request.vr_message_len = NLA_LEN(nla);

    ret = vr_message_request(&request);
    if (ret) {
        if (vr_send_response(ret)) {
            DbgPrint("%s: generating error response has failed\n", __func__);
            return STATUS_INVALID_PARAMETER;
        }
    }

    multi_flag = 0;
    while ((response = vr_message_dequeue_response())) {
        if (!multi_flag && !vr_response_queue_empty())
            multi_flag = NLM_F_MULTI;
        
        char *data = response->vr_message_buf - NETLINK_HEADER_LEN;
        size_t data_len = NLMSG_ALIGN(response->vr_message_len + NETLINK_HEADER_LEN);

        struct nlmsghdr *nlh_resp = (struct nlmsghdr *)(data);
        nlh_resp->nlmsg_len = data_len;
        nlh_resp->nlmsg_type = nlh->nlmsg_type;
        nlh_resp->nlmsg_flags = multi_flag;
        nlh_resp->nlmsg_seq = nlh->nlmsg_seq;
		nlh_resp->nlmsg_pid = 0;

        /* 'genlmsghdr' should be put directly after 'nlmsghdr', thus we can just
           increment previous header pointer */
        struct genlmsghdr *genlh_resp = (struct genlmsghdr *)(nlh_resp + 1);
        RtlCopyMemory(genlh_resp, genlh, sizeof(*genlh_resp));

        /* 'nlattr' should be put directly after 'genlmsghdr', thus we can just
           increment previous header pointer */
        struct nlattr *nla_resp = (struct nlattr *)(genlh_resp + 1);
        nla_resp->nla_len = response->vr_message_len;
        nla_resp->nla_type = NL_ATTR_VR_MESSAGE_PROTOCOL;

        struct ksync_response *ks_resp = KsyncResponseCreate();
        if (ks_resp != NULL) {
            ks_resp->message_len = data_len;
            RtlCopyMemory(ks_resp->buffer, data, ks_resp->message_len);
            KsyncAppendResponse(ctx, ks_resp);
        } else {
            DbgPrint("%s: ksync_response allocation failed\n", __func__);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        response->vr_message_buf = NULL;
        vr_message_free(response);
    }

    if (multi_flag) {
        struct ksync_response *ks_resp = KsyncResponseCreate();
        if (ks_resp == NULL) {
            DbgPrint("%s: ksync_response allocation failed\n", __func__);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        struct nlmsghdr *nlh_done = (struct nlmsghdr *)ks_resp->buffer;
        nlh_done->nlmsg_len = NLMSG_HDRLEN;
        nlh_done->nlmsg_type = NLMSG_DONE;
        nlh_done->nlmsg_flags = 0;
        nlh_done->nlmsg_seq = nlh->nlmsg_seq;
        nlh_done->nlmsg_pid = 0;

        ks_resp->message_len = NLMSG_HDRLEN;

        KsyncAppendResponse(ctx, ks_resp);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
KsyncDispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    PIO_STACK_LOCATION io_stack = IoGetCurrentIrpStackLocation(Irp);
    ASSERTMSG("could not get I/O parameters", io_stack != NULL);

    struct ksync_device_context *ctx = ksync_get_context_from_file_context(Irp);
    ASSERTMSG("ksync_device_context was not set", ctx != NULL);

    if (Irp->MdlAddress == NULL)
        return KsyncHandleRetWithInfo(Irp, STATUS_INVALID_PARAMETER, 0);

    PCHAR data_buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                                                     LowPagePriority | MdlMappingNoExecute);
    if (data_buffer == NULL)
        return KsyncHandleRetWithInfo(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);

    ULONG data_buffer_size = io_stack->Parameters.Write.Length;
    NTSTATUS status = KsyncHandleWrite(ctx, data_buffer, data_buffer_size);
    if (status == STATUS_SUCCESS)
        return KsyncHandleRetWithInfo(Irp, STATUS_SUCCESS, data_buffer_size);
    else
        return KsyncHandleRetWithInfo(Irp, status, 0);
}

NTSTATUS
KsyncDispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    struct ksync_device_context *ctx = ksync_get_context_from_file_context(Irp);
    ASSERTMSG("ksync_device_context was not set", ctx != NULL);

    if (Irp->MdlAddress == NULL)
        return KsyncHandleRetWithInfo(Irp, STATUS_INVALID_PARAMETER, 0);

    uint8_t *output_buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                                                          LowPagePriority | MdlMappingNoExecute);
    if (output_buffer != NULL) {
        struct ksync_response *resp = KsyncPopResponse(ctx);
        if (resp != NULL) {
            PIO_STACK_LOCATION io_stack = IoGetCurrentIrpStackLocation(Irp);
            ASSERTMSG("could not get I/O parameters", io_stack != NULL);

            size_t output_buffer_length = io_stack->Parameters.Read.Length;
            size_t resp_length = resp->message_len;
            ASSERTMSG("vRouter's response too big", resp_length <= output_buffer_length);

            RtlCopyMemory(output_buffer, resp->buffer, resp_length);
            KsyncResponseDelete(resp);
            return KsyncHandleRetWithInfo(Irp, STATUS_SUCCESS, resp_length);
        } else {
            // No messages in queue => cannot read anything
            return KsyncHandleRetWithInfo(Irp, STATUS_SUCCESS, 0);
        }
    } else {
        return KsyncHandleRetWithInfo(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
    }
}

NTSTATUS
KsyncDispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;
}

NTSTATUS
KsyncDispatchCleanup(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
KsyncCreateDevice(PDRIVER_OBJECT DriverObject)
{
    VR_DEVICE_DISPATCH_CALLBACKS Callbacks = {
        .create         = KsyncDispatchCreate,
        .close          = KsyncDispatchClose,
        .cleanup        = KsyncDispatchCleanup,
        .write          = KsyncDispatchWrite,
        .read           = KsyncDispatchRead,
        .device_control = KsyncDispatchDeviceControl,
    };

    return VRouterSetUpNamedPipeServer(DriverObject,
                                       KsyncDeviceName,
                                       KsyncDeviceSymLink,
                                       &Callbacks,
                                       &KsyncDeviceObject,
                                       &KsyncSymlinkCreated);
}

VOID
KsyncDestroyDevice(PDRIVER_OBJECT DriverObject)
{
    VRouterTearDownNamedPipeServer(DriverObject,
                                   KsyncDeviceSymLink,
                                   &KsyncDeviceObject,
                                   &KsyncSymlinkCreated);
}
