/*
 * windows_ksync.h -- definitions used in KSync handling on Windows
 *
 * Copyright (c) 2017 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __WINDOWS_KSYNC_H__
#define __WINDOWS_KSYNC_H__

#define NL_RESP_DEFAULT_SIZE        512
#define NL_MSG_DEFAULT_SIZE         4096

#define NL_MSG_TYPE_ERROR           0
#define NL_MSG_TYPE_DONE            1
#define NL_MSG_TYPE_GEN_CTRL        2
#define NL_MSG_TYPE_FMLY            3

#define VR_NETLINK_PROTO_DEFAULT    0xFFFFFFFF

#define GENL_FAMILY_NAME_LEN            16

#define NLA_DATA(nla)                   ((char *)nla + NLA_HDRLEN)
#define NLA_LEN(nla)                    (nla->nla_len - NLA_HDRLEN)
#define GENLMSG_DATA(buf)               ((char *)buf + GENL_HDRLEN)

enum ksync_response_type {
    KSYNC_RESPONSE_DONE, // ending marker for MULTIPLE messages
    KSYNC_RESPONSE_MULTIPLE, // receiver should expect multiple messages, last one with type DONE
    KSYNC_RESPONSE_SINGLE, // receiver should expect only this message
};

struct ksync_response_header {
    enum ksync_response_type type;
    ULONG len;
    UINT32 seq;
};

struct ksync_response {
    struct ksync_response *next;
    struct ksync_response_header header;
    unsigned char buffer[NL_MSG_DEFAULT_SIZE];
};

// TODO: JW-261 Implement multiple queues to handle multiple processes
//       opening a pipe
struct ksync_device_context {
    PVOID user_virtual_address;
    struct ksync_response *responses; // responses queue
};

// Returns true if response header contains a response, parseable by sandesh.
#define IS_KSYNC_RESPONSE_VALID(hdr) ((hdr)->type == KSYNC_RESPONSE_SINGLE || (hdr)->type == KSYNC_RESPONSE_MULTIPLE)

#define SIOCTL_TYPE 40000
#define IOCTL_SIOCTL_METHOD_OUT_DIRECT \
CTL_CODE( SIOCTL_TYPE, 0x901, METHOD_OUT_DIRECT , FILE_ANY_ACCESS )

struct mem_wrapper
{
    PVOID       pBuffer;
};

#endif /* __WINDOWS_KSYNC_H__ */
