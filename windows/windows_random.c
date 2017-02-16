#include "vr_os.h"
#include "vr_windows.h"

ULONG seed;
bool isSeedInitialized;

static void prepareSeed() {
    if (!isSeedInitialized) {
        seed = KeQueryPerformanceCounter(NULL).LowPart;
        isSeedInitialized = TRUE;
    }
}

// make sure prepareSeed has been run before using this function
static ULONG get_random_ulong() {
    const ULONG a = 1103515245UL, c = 12345UL;
    seed = a * seed + c;
    return seed;
}

void get_random_bytes(void *buf, int nbytes) {
    ULONG t;
    prepareSeed();
    while (nbytes > sizeof(ULONG)) {
        t = get_random_ulong();
        memcpy(buf, &t, sizeof(ULONG));
        nbytes -= sizeof(ULONG);
        buf = (PINT8)buf + sizeof(ULONG);
    }
    t = get_random_ulong();
    memcpy(buf, &t, nbytes);
}
