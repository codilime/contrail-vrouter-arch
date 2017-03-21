#include <strsafe.h>
#include <stdbool.h>
#include "nl_util.h"
#include <stdint.h> // portable: uint64_t   MSVC: __int64 
#include <stdio.h> 
#include <Windows.h>
#define vRouterKsync "\\\\.\\vrouterKsync"
#define ETHER_ADDR_LEN	   6
#define WIN32_LEAN_AND_MEAN

static char ether_ntoa_data[18];

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}



// TODO: JW-120 - Refactoring of vr_win_utils.c
struct ether_addr {
    u_char ether_addr_octet[ETHER_ADDR_LEN];
};

static inline int
xdigit(char c) {
    if ('0' <= c && c <= '9') {
        return (int)(c - '0');
    }

    if ('a' <= c && c <= 'f') {
        return (int)(10 + c - 'a');
    }

    if ('A' <= c && c <= 'F') {
        return (int)(10 + c - 'A');
    }
    return -1;
}

int
inet_aton(const char *cp, struct in_addr *addr)
{
    return inet_pton(AF_INET, cp, addr);
}

char *ether_ntoa(struct ether_addr *n)
{
    int i;

    i = sprintf(ether_ntoa_data, "%02x:%02x:%02x:%02x:%02x:%02x", n->ether_addr_octet[0], n->ether_addr_octet[1],
                                                  n->ether_addr_octet[2], n->ether_addr_octet[3],
                                                  n->ether_addr_octet[4], n->ether_addr_octet[5]);
    if (i < 0)
        return (NULL);
    return &ether_ntoa_data;
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

LPSTR GetError()
{
    LPSTR errorText = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errorText,
        0,
        NULL);

    return errorText;
}

int
nl_client_stream_recvmsg(struct nl_client *cl) 
{
    DWORD dwRead = 0;
    char buffer[NL_MSG_DEFAULT_SIZE];

    cl->cl_buf_offset = 0;
    cl->cl_recv_len = 0;

    if (hPipe != INVALID_HANDLE_VALUE)
    {
        if (ReadFile(hPipe, buffer, NL_MSG_DEFAULT_SIZE, &dwRead, NULL)) {
            RtlCopyMemory(cl->cl_buf, buffer, dwRead);
        }
        else {
            DWORD dw = GetLastError();

            if (!FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer,
                0, NULL)) {

                printf("Format message failed with 0x%x\n", GetLastError());
                ExitProcess(dw);
            }

            printf("Error: Problem with ksync communicaton: %s\n", buffer);

            ExitProcess(dw);
        }
    }

    return dwRead;
}

// TODO: JW-120 - Refactoring of vr_win_utils.c
int
nl_sendmsg(struct nl_client *cl)
{
    DWORD dwWritten;
    struct nlmsghdr *nlh = (struct nlmsghdr *)cl->cl_buf;
    int d =  cl->cl_buf_offset;
    int r = -1;

    hPipe = CreateFile(TEXT(vRouterKsync),

        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

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
