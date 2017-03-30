#include <strsafe.h>
#include <stdbool.h>
#include "nl_util.h"

#define ETHER_ADDR_LEN	   6

#define KSYNC_MAX_WRITE_COUNT 4096
const WCHAR *KSYNC_PATH = L"\\\\.\\vrouterKsync";

// Handle for name pipe used by Ksync
HANDLE hPipe;

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

// TODO: JW-120 - Refactoring of vr_win_utils.cl
/* It is only temporary mock*/
int
nl_client_datagram_recvmsg(struct nl_client *cl)
{
    return nl_client_stream_recvmsg(cl);
}

DWORD
get_error_message(DWORD message_id, LPTSTR *message)
{
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD lang_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    return FormatMessage(flags, NULL, message_id, lang_id, (LPTSTR)message, 0, NULL);
}

void
print_last_error_message()
{
    DWORD error = GetLastError();
    LPTSTR message = NULL;
    if (get_error_message(error, &message) != 0) {
        printf("Error: %ws [%d]\r\n", message, error);
        LocalFree(message);
    } else {
        printf("Error: [%d]\r\n", error);
    }
}

int
nl_client_stream_recvmsg(struct nl_client *cl) 
{
    DWORD dwRead = 0;
    char buffer[NL_MSG_DEFAULT_SIZE];

    cl->cl_buf_offset = 0;
    cl->cl_recv_len = 0;

    if (hPipe != INVALID_HANDLE_VALUE) {
        if (ReadFile(hPipe, buffer, NL_MSG_DEFAULT_SIZE, &dwRead, NULL)) {
            memcpy(cl->cl_buf, buffer, dwRead);
        } else {
            print_last_error_message();
            ExitProcess(EXIT_FAILURE);
        }
    }
    return dwRead;
}

// TODO: JW-120 - Refactoring of vr_win_utils.c
int
nl_sendmsg(struct nl_client *cl)
{
    DWORD dwWritten = 0;
    int status = -1;

    DWORD access_flags = GENERIC_READ | GENERIC_WRITE;
    DWORD attrs = OPEN_EXISTING;

    hPipe = CreateFile(KSYNC_PATH, access_flags, 0, NULL, attrs, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE) {
        status = WriteFile(hPipe, cl->cl_buf, KSYNC_MAX_WRITE_COUNT, &dwWritten, NULL);
        if (status == 0) {
            goto handle_error;
        }
    } else {
        goto handle_error;
    }

    return status;

handle_error:
    print_last_error_message();
    return -1;
}
