#include "vr_os.h"
#include "vr_windows.h"

LARGE_INTEGER seed;
bool isSeedInitialized;

// Do not use it, use get_random_bytes instead
static ULONG get_random_ulong() {
    return 0;// RtlRandomEx(&seed.LowPart);
}

void get_random_bytes(void *buf, int nbytes) {
    ULONG t;
    if (!isSeedInitialized) {
        seed = KeQueryPerformanceCounter(NULL);
        isSeedInitialized = TRUE;
    }
    while (nbytes > sizeof(ULONG)) {
        t = get_random_ulong();
        memcpy(buf, &t, sizeof(ULONG));
        nbytes -= sizeof(ULONG);
        buf = (PINT8)buf + sizeof(ULONG);
    }
    t = get_random_ulong();
    memcpy(buf, &t, nbytes);
}
