#include <strsafe.h>
#include <stdbool.h>
#include "nl_util.h"

#define ETHER_ADDR_LEN	   6
#define KSYNC_MAX_WRITE_COUNT (NL_MSG_DEFAULT_SIZE)

// nl_*_sendmsg and nl_*_recvmsg functions use sendmsg and recvmsg
// and they return -1 on error.
#define GLIBC_ERROR (-1)

const LPCTSTR KSYNC_PATH = TEXT("\\\\.\\vrouterKsync");

// TODO: JW-120 - Refactoring of vr_win_utils.c
struct ether_addr {
    u_char ether_addr_octet[ETHER_ADDR_LEN];
};

static DWORD
print_and_get_error_code()
{
    DWORD error = GetLastError();
    LPTSTR message = NULL;

    DWORD flags = (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS);
    DWORD lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    DWORD ret = FormatMessage(flags, NULL, error, lang_id, (LPTSTR)message, 0, NULL);

    if (ret != 0) {
        printf("Error: %ws [%d]\r\n", message, error);
        LocalFree(message);
    } else {
        printf("Error: [%d]\r\n", error);
    }

    return error;
}

int
win_setup_nl_client(struct nl_client *cl, unsigned int proto)
{
    UNREFERENCED_PARAMETER(proto);

    DWORD access_flags = GENERIC_READ | GENERIC_WRITE;
    DWORD attrs = OPEN_EXISTING;

    cl->cl_win_pipe = CreateFile(KSYNC_PATH, access_flags, 0, NULL, attrs, 0, NULL);
    if (cl->cl_win_pipe == INVALID_HANDLE_VALUE) {
        DWORD error = print_and_get_error_code();
        return error;
    }

    cl->cl_recvmsg = win_nl_client_recvmsg;

    return ERROR_SUCCESS;
}

int
win_nl_sendmsg(struct nl_client *cl)
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
    DWORD read = 0;

    cl->cl_buf_offset = 0;
    cl->cl_recv_len = 0;

    BOOL ret = ReadFile(cl->cl_win_pipe, cl->cl_buf, NL_MSG_DEFAULT_SIZE, &read, NULL);
    if (!ret) {
        print_and_get_error_code();
        return GLIBC_ERROR;
    }

    cl->cl_recv_len = read;
    if (cl->cl_recv_len > cl->cl_buf_len)
        return -EOPNOTSUPP;

    return read;
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
