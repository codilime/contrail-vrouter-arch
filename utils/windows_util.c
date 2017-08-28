#include <assert.h>
#include <strsafe.h>
#include <stdbool.h>
#include "vr_defs.h"
#include "nl_util.h"

#define ETHER_ADDR_LEN	   (6)
#define ETHER_ADDR_STR_LEN (ETHER_ADDR_LEN * 3)

// nl_*_sendmsg and nl_*_recvmsg functions use sendmsg and recvmsg
// and they return -1 on error.
#define GLIBC_ERROR (-1)

const LPCTSTR KSYNC_PATH = TEXT("\\\\.\\vrouterKsync");

// TODO: JW-120 - Refactoring of vr_win_utils.c
struct ether_addr {
    u_char ether_addr_octet[ETHER_ADDR_LEN];
};

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

static DWORD
print_and_get_error_code()
{
    DWORD error = GetLastError();
    LPTSTR message = NULL;

    DWORD flags = (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS);
    DWORD lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    DWORD ret = FormatMessage(flags, NULL, error, lang_id, message, 0, NULL);

    if (ret != 0) {
        printf("Error: %s [%d]\r\n", message, error);
        LocalFree(message);
    } else {
        printf("Error: [%d]\r\n", error);
    }

    return error;
}

int
nl_sendmsg(struct nl_client *cl)
{
    DWORD written = 0;
    BOOL ret = WriteFile(cl->cl_win_pipe, cl->cl_buf, NL_MSG_DEFAULT_SIZE, &written, NULL);
    if (!ret) {
        print_and_get_error_code();
        return GLIBC_ERROR;
    }

    return written;
}

int
win_nl_client_recvmsg(struct nl_client *cl)
{
    DWORD read_bytes = 0;

    cl->cl_buf_offset = 0;
    cl->cl_recv_len = 0;

    BOOL ret = ReadFile(cl->cl_win_pipe, cl->cl_buf, NL_MSG_DEFAULT_SIZE, &read_bytes, NULL);
    if (!ret) {
        print_and_get_error_code();
        return GLIBC_ERROR;
    }

    cl->cl_recv_len = read_bytes;
    if (cl->cl_recv_len > cl->cl_buf_len)
        return -EOPNOTSUPP;

    return read_bytes;
}

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

char *
ether_ntoa(const struct ether_addr *addr)
{
    static char buffer[ETHER_ADDR_STR_LEN];

    memset(buffer, 0, sizeof(buffer));
    int ret = snprintf(buffer, sizeof(buffer), MAC_FORMAT, MAC_VALUE(addr->ether_addr_octet));
    assert(ret == ETHER_ADDR_STR_LEN - 1);  // ETHER_ADDR_STR_LEN includes '\0' byte

    return buffer;
}
