#pragma once

#include "vr_os.h"
#include <Ws2tcpip.h>
#include <basetsd.h>

typedef unsigned long   __kernel_size_t;

#define GENL_ID_CTRL 0x10

enum {
    CTRL_CMD_UNSPEC,
    CTRL_CMD_NEWFAMILY,
    CTRL_CMD_DELFAMILY,
    CTRL_CMD_GETFAMILY,
    CTRL_CMD_NEWOPS,
    CTRL_CMD_DELOPS,
    CTRL_CMD_GETOPS,
    CTRL_CMD_NEWMCAST_GRP,
    CTRL_CMD_DELMCAST_GRP,
    CTRL_CMD_GETMCAST_GRP, /* unused */
    __CTRL_CMD_MAX,
};


#define IFNAMSIZ 16
#define INET6_ADDRSTRLEN 46

#define IFNAMSIZ 16
#define AF_BRIDGE 7
// netlink doesn't exist on windows platform
#define AF_NETLINK 0
#define NETLINK_GENERIC 0
#define NLMSG_DONE  0x3


// TODO: Remove unused defines and structures
#define __attribute__(A) /* do nothing */

#define NLM_F_REQUEST           1       /* It is request message.       */
#define NLM_F_MULTI             2       /* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK               4       /* Reply with ack, with zero or error code */
#define NLM_F_ECHO              8       /* Echo this request            */
#define NLM_F_DUMP_INTR         16      /* Dump was inconsistent due to sequence change */
#define NLM_F_DUMP_FILTERED     32      /* Dump was filtered as requested */
#define VR_NAMED_PIPE_WINDOWS  100


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

struct iov_iter {
	int type;
	size_t iov_offset;
	size_t count;
	union {
		const struct iovec *iov;
		const struct kvec *kvec;
		const struct bio_vec *bvec;
		struct pipe_inode_info *pipe;
	};
	union {
		unsigned long nr_segs;
		int idx;
	};
};

struct genlmsghdr {
	UINT8    cmd;
	UINT8    version;
	UINT16   reserved;
};

#define NLA_ALIGNTO 4
#define NLMSG_ALIGNTO   4
#define NLA_ALIGN(len) (((len)+NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN ((int)NLA_ALIGN(sizeof(struct nlattr)))
#define NLMSG_ALIGN(len) ( ((len)+NLMSG_ALIGNTO-1) & ~(NLMSG_ALIGNTO-1) )
#define NLMSG_HDRLEN     ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define GENL_HDRLEN     NLMSG_ALIGN(sizeof(struct genlmsghdr))

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
