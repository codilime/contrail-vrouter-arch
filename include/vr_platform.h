#pragma once

#ifndef _WINDOWS

#ifndef _UTILS

#include <wdm.h>

typedef BOOLEAN bool;

#define true TRUE
#define false FALSE
#define htons(a) RtlUshortByteSwap(a)
#define ntohs(a) RtlUshortByteSwap(a)

#endif

#include <basetsd.h>

typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef INT64 int64_t;
typedef UINT64 uint64_t;

#define IFNAMSIZ 16
#define INET6_ADDRSTRLEN 46
#define AF_BRIDGE 7

#define __attribute__packed__open__ __pragma( pack( push, 1 ) )
#define __attribute__packed__close__ __pragma( pack( pop ) )
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) /* do nothing */

// TODO: Remove unused defines and structures
#define __attribute__(A) /* do nothing */

#define NLM_F_REQUEST           1       /* It is request message.       */
#define NLM_F_MULTI             2       /* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK               4       /* Reply with ack, with zero or error code */
#define NLM_F_ECHO              8       /* Echo this request            */
#define NLM_F_DUMP_INTR         16      /* Dump was inconsistent due to sequence change */
#define NLM_F_DUMP_FILTERED     32      /* Dump was filtered as requested */

struct nlattr {
	UINT16 nla_len;
	UINT16 nla_type;
};

struct nlmsghdr {
	UINT32           nlmsg_len;      /* Length of message including header */
	UINT16           nlmsg_type;     /* Message content */
	UINT16           nlmsg_flags;    /* Additional flags */
	UINT32           nlmsg_seq;      /* Sequence number */
	UINT32           nlmsg_pid;      /* Sending process port ID */
};

#define NLA_ALIGNTO 4
#define NLA_ALIGN(len) (((len)+NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN ((int)NLA_ALIGN(sizeof(struct nlattr)))

enum {
	CTRL_ATTR_UNSPEC,
	CTRL_ATTR_FAMILY_ID,
	CTRL_ATTR_FAMILY_NAME,
	CTRL_ATTR_VERSION,
	CTRL_ATTR_HDRSIZE,
	CTRL_ATTR_MAXATTR,
	CTRL_ATTR_OPS,
	CTRL_ATTR_MCAST_GROUPS,
	__CTRL_ATTR_MAX,
};

#else
#define __attribute__packed__open__ /* do nothing */
#define __attribute__packed__close__ __attribute__((__packed__))
#define __attribute__format__open__(...) /* do nothing */
#define __attribute__format__close__(...) __attribute__((format(__VA_ARGS__)))
#endif
