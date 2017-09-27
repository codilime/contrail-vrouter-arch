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

struct ksync_response {
    struct ksync_response *next;
    size_t message_len;
    unsigned char buffer[NL_MSG_DEFAULT_SIZE];
};

struct ksync_device_context {
    // Queue for responses generated by vRotuer
    struct ksync_response *responses;

    // Address usable by user space program to access shared memory
    void *user_virtual_address;
};

/* TODO(sodar): Refactor */
#define SIOCTL_TYPE 40000
#define IOCTL_SIOCTL_METHOD_OUT_DIRECT \
CTL_CODE( SIOCTL_TYPE, 0x901, METHOD_OUT_DIRECT , FILE_ANY_ACCESS )

#endif /* __WINDOWS_KSYNC_H__ */
