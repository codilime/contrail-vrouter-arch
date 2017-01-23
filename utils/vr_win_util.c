#include <stdbool.h>
#include "nl_util.h"

#define ETHER_ADDR_LEN	   6

// TODO: JW-120 - Refactoring of vr_win_utils.c
struct ether_addr {
	u_char ether_addr_octet[ETHER_ADDR_LEN];
};


static inline int
xdigit(char c) {
	unsigned d;
	d = (unsigned)(c - '0');
	if (d < 10) return (int)d;
	d = (unsigned)(c - 'a');
	if (d < 6) return (int)(10 + d);
	d = (unsigned)(c - 'A');
	if (d < 6) return (int)(10 + d);
	return -1;
}

int
inet_aton(const char *cp, struct in_addr *addr)
{
	return inet_pton(AF_INET, cp, addr);
}


struct ether_addr *
	ether_aton_r(const char *asc, struct ether_addr * addr)
{
	int i, val0, val1;
	for (i = 0; i < ETHER_ADDR_LEN; ++i) {
		val0 = xdigit(*asc);
		asc++;
		if (val0 < 0)
			return NULL;

		val1 = xdigit(*asc);
		asc++;
		if (val1 < 0)
			return NULL;

		addr->ether_addr_octet[i] = (u_int8_t)((val0 << 4) + val1);

		if (i < ETHER_ADDR_LEN - 1) {
			if (*asc != ':')
				return NULL;
			asc++;
		}
	}
	if (*asc != '\0')
		return NULL;
	return addr;
}

/*
* Convert Ethernet address in the standard hex-digits-and-colons to binary
* representation.
* Re-entrant version (GNU extensions)
*/

struct ether_addr *
	ether_aton(const char *asc)
{
	static struct ether_addr addr;
	return ether_aton_r(asc, &addr);
}
HANDLE hPipe;

// TODO: JW-120 - Refactoring of vr_win_utils.cl
/* It is only temporary mock*/
int
nl_client_datagram_recvmsg(struct nl_client *cl)
{
    return nl_client_stream_recvmsg(cl);
}

// TODO: JW-120 - Refactoring of vr_win_utils.cl
int
nl_client_stream_recvmsg(struct nl_client *cl) 
{
    DWORD dwWritten;
    DWORD dwRead;
    void *rr;
    struct nlmsghdr *nlh = (struct nlmsghdr *)cl->cl_buf;
    int d = cl->cl_buf_offset;
    char buffer[4096];
    int r = 0;
    while (hPipe != INVALID_HANDLE_VALUE)
    {
            while (ReadFile(hPipe, buffer, 4096, &dwRead, NULL) != FALSE)
            {
                struct nlmsghdr *nlh = (struct nlmsghdr *)buffer;
                /* do something with data in buffer */
                printf("%d\n", nlh->nlmsg_len);
                printf("%d\n", nlh->nlmsg_type);
                printf("%d\n", nlh->nlmsg_flags);
                printf("----\n");
                break;
            }
            break;
    }
    cl->cl_recv_len = 4096;
    return cl->cl_recv_len;
}

// TODO: JW-120 - Refactoring of vr_win_utils.c
int
nl_sendmsg(struct nl_client *cl)
{
	DWORD dwWritten;
	void *rr;
	struct nlmsghdr *nlh = (struct nlmsghdr *)cl->cl_buf;
	int d =  cl->cl_buf_offset;
	hPipe = CreateFile(TEXT("\\\\.\\pipe\\Pipe"),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	int r;
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		r = WriteFile(hPipe,
			cl->cl_buf,
			4096,   // = length of string + terminating '\0' !!!
			&dwWritten,
			NULL);

	}

	return r;
}
