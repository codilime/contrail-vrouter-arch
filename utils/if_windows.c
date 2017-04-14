#include <stdio.h>

// TODO: JW-351
unsigned int
if_nametoindex(const char *ifname)
{
    printf("%s: Not supported on Windows\n", __func__);
    return -1;
}

// TODO: JW-351
char *
if_indextoname(unsigned int ifindex, char *ifname)
{
    printf("%s: Not supported on Windows\n", __func__);
    return NULL;
}