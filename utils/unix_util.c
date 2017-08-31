#include <sys/socket.h>
#if defined(__linux__)
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#elif defined(__FreeBSD__)
#include <net/ethernet.h>
#endif

#include "nl_util.h"

int
nl_socket(struct nl_client *cl, int domain, int type, int protocol)
{
    if (cl->cl_sock >= 0)
        return -EEXIST;

#if defined(__FreeBSD__)
    /*
    * Fake Contrail socket has only one protocol for handling
    * sandesh protocol, so zero must be passed as a parameter
    */
    domain = AF_VENDOR00;
    type = SOCK_DGRAM;
    protocol = 0;
#endif
    cl->cl_sock = socket(domain, type, protocol);
    if (cl->cl_sock < 0)
        return cl->cl_sock;

    cl->cl_sock_protocol = protocol;
    cl->cl_socket_domain = domain;
    cl->cl_socket_type = type;

    if (type == SOCK_STREAM) {
        cl->cl_recvmsg = nl_client_stream_recvmsg;
    }
    else {
        cl->cl_recvmsg = nl_client_datagram_recvmsg;
    }

    return cl->cl_sock;
}

int
nl_connect(struct nl_client *cl, uint32_t ip, uint16_t port)
{
    if (cl->cl_socket_domain == AF_NETLINK) {
        struct sockaddr_nl *sa = malloc(sizeof(struct sockaddr_nl));

        if (!sa)
            return -1;

        memset(sa, 0, sizeof(*sa));
        sa->nl_family = cl->cl_socket_domain;
        sa->nl_pid = cl->cl_id;
        cl->cl_sa = (struct sockaddr *)sa;
        cl->cl_sa_len = sizeof(*sa);

        return bind(cl->cl_sock, cl->cl_sa, cl->cl_sa_len);
    }

    if (cl->cl_socket_domain == AF_INET) {
        struct in_addr address;
        struct sockaddr_in *sa = malloc(sizeof(struct sockaddr_in));
        if (!sa)
            return -1;

        memset(sa, 0, sizeof(*sa));
        address.s_addr = htonl(ip);
        sa->sin_family = cl->cl_socket_domain;
        sa->sin_addr = address;
        sa->sin_port = htons(port);
        cl->cl_sa = (struct sockaddr *)sa;
        cl->cl_sa_len = sizeof(*sa);

        return connect(cl->cl_sock, cl->cl_sa, cl->cl_sa_len);
    }
    return 0;
}

int
nl_client_datagram_recvmsg(struct nl_client *cl)
{
    int ret;
    struct msghdr msg;
    struct iovec iov;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = cl->cl_sa;
    msg.msg_namelen = cl->cl_sa_len;

    iov.iov_base = (void *)(cl->cl_buf);
    iov.iov_len = cl->cl_buf_len;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    cl->cl_buf_offset = 0;

    ret = recvmsg(cl->cl_sock, &msg, MSG_DONTWAIT);
    if (ret < 0) {
        return ret;
    }

    cl->cl_recv_len = ret;
    if (cl->cl_recv_len > cl->cl_buf_len)
        return -EOPNOTSUPP;

    return ret;
}

int
nl_client_stream_recvmsg(struct nl_client *cl) {
    int ret;
    struct msghdr msg;
    struct iovec iov;

    memset(&msg, 0, sizeof(msg));

    msg.msg_name = cl->cl_sa;
    msg.msg_namelen = sizeof(cl->cl_sa_len);

    iov.iov_base = (void *)(cl->cl_buf);
    iov.iov_len = NLMSG_HDRLEN;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    cl->cl_buf_offset = 0;

    /* read netlink header and get the lenght of sandesh message */
    ret = recvmsg(cl->cl_sock, &msg, 0);
    if (ret < 0) {
        return ret;
    }
    struct nlmsghdr *nlh = (struct nlmsghdr *)(cl->cl_buf + cl->cl_buf_offset);
    uint32_t pending_length = nlh->nlmsg_len - NLMSG_HDRLEN;

    /* read sandesh message */
    iov.iov_base = (void *)(cl->cl_buf + NLMSG_HDRLEN);
    iov.iov_len = pending_length;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    ret = recvmsg(cl->cl_sock, &msg, 0);
    if (ret < 0) {
        return ret;
    }

    cl->cl_recv_len = nlh->nlmsg_len;
    if (cl->cl_recv_len > cl->cl_buf_len)
        return -EOPNOTSUPP;

    return ret;
}

int
nl_sendmsg(struct nl_client *cl)
{
    struct msghdr msg;
    struct iovec iov;

    memset(&msg, 0, sizeof(msg));
#if defined (__linux__)
    msg.msg_name = cl->cl_sa;
    msg.msg_namelen = cl->cl_sa_len;
#endif

    iov.iov_base = (void *)(cl->cl_buf);
    iov.iov_len = cl->cl_buf_offset;

    cl->cl_buf_offset = 0;

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    return sendmsg(cl->cl_sock, &msg, 0);
}

#if defined(__linux__)
int
nl_build_attr_linkinfo(struct nl_client *cl, struct vn_if *ifp)
{
    char *link_info_buf;
    int len;
    struct nlattr *nla = (struct nlattr *)
        ((char *)cl->cl_buf + cl->cl_buf_offset);

    len = NLA_HDRLEN + NLA_HDRLEN +
        NLA_ALIGN(strlen(ifp->if_kind) + 1);

    if (cl->cl_buf_offset + len > cl->cl_buf_len)
        return -ENOMEM;

    nla->nla_len = len;
    nla->nla_type = IFLA_LINKINFO;

    link_info_buf = (char *)nla + NLA_HDRLEN;
    nla = (struct nlattr *)link_info_buf;
    nla->nla_len = len - NLA_HDRLEN;
    nla->nla_type = IFLA_INFO_KIND;

    link_info_buf += NLA_HDRLEN;
    strcpy(link_info_buf, ifp->if_kind);

    cl->cl_buf_offset += len;

    return 0;
}

int
nl_build_attr_ifname(struct nl_client *cl, struct vn_if *ifp)
{
    char *if_name_buf;
    int len;
    struct nlattr *nla = (struct nlattr *)
        ((char *)cl->cl_buf + cl->cl_buf_offset);

    len = NLA_HDRLEN + NLA_ALIGN(strlen(ifp->if_name) + 1);
    if (cl->cl_buf_offset + len > cl->cl_buf_len)
        return -ENOMEM;

    nla->nla_len = len;
    nla->nla_type = IFLA_IFNAME;

    if_name_buf = (char *)nla + NLA_HDRLEN;
    strcpy(if_name_buf, ifp->if_name);

    cl->cl_buf_offset += nla->nla_len;

    return 0;
}

int
nl_build_mac_address(struct nl_client *cl, struct vn_if *ifp)
{
    int len;
    char *mac_buf;
    struct nlattr *nla = (struct nlattr *)
        ((char *)cl->cl_buf + cl->cl_buf_offset);

    len = NLA_HDRLEN + NLA_ALIGN(6);
    if (cl->cl_buf_offset + len > cl->cl_buf_len)
        return -ENOMEM;

    nla->nla_len = len;
    nla->nla_type = IFLA_ADDRESS;

    mac_buf = (char *)nla + NLA_HDRLEN;
    memcpy(mac_buf, ifp->if_mac, sizeof(ifp->if_mac));

    cl->cl_buf_offset += nla->nla_len;

    return 0;
}


int
nl_build_ifinfo(struct nl_client *cl, struct vn_if *ifp)
{
    struct ifinfomsg *ifi_msg = (struct ifinfomsg *)
        (cl->cl_buf + cl->cl_buf_offset);

    if (cl->cl_buf_offset + NLMSG_ALIGN(sizeof(*ifi_msg)) >
        cl->cl_buf_len)
        return -ENOMEM;

    memset(ifi_msg, 0, sizeof(struct ifinfomsg));
    cl->cl_buf_offset += NLMSG_ALIGN(sizeof(struct ifinfomsg));

    return 0;
}

int
nl_build_if_create_msg(struct nl_client *cl, struct vn_if *ifp, uint8_t ack)
{
    int ret;
    uint32_t flags;

    if (!cl->cl_buf || cl->cl_buf_offset || !ifp)
        return -EINVAL;

    flags = NLM_F_REQUEST | NLM_F_CREATE;
    if (ack) {
        flags |= NLM_F_ACK;
    }
    ret = nl_build_nlh(cl, RTM_NEWLINK, flags);
    if (ret)
        return ret;

    ret = nl_build_ifinfo(cl, ifp);
    if (ret)
        return ret;

    ret = nl_build_mac_address(cl, ifp);
    if (ret)
        return ret;

    ret = nl_build_attr_ifname(cl, ifp);
    if (ret)
        return ret;

    ret = nl_build_attr_linkinfo(cl, ifp);
    if (ret)
        return ret;

    cl->cl_msg_len = cl->cl_buf_offset;
    nl_update_nlh(cl);

    return 0;
}
#endif  /* __linux__ */
