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

typedef struct _KSYNC_RESPONSE   KSYNC_RESPONSE;
typedef struct _KSYNC_RESPONSE *PKSYNC_RESPONSE;
struct _KSYNC_RESPONSE {
    PKSYNC_RESPONSE next;
    size_t message_len;
    uint8_t buffer[NL_MSG_DEFAULT_SIZE];
};

typedef struct _KSYNC_DEVICE_CONTEXT   KSYNC_DEVICE_CONTEXT;
typedef struct _KSYNC_DEVICE_CONTEXT *PKSYNC_DEVICE_CONTEXT;
struct _KSYNC_DEVICE_CONTEXT {
    // Queue for responses generated by vRotuer
    PKSYNC_RESPONSE responses;

    size_t WrittenBytes;
    size_t WriteBufferSize;
    uint8_t WriteBuffer[NL_MSG_DEFAULT_SIZE];
};

#endif /* __WINDOWS_KSYNC_H__ */
