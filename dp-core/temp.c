#include "vr_os.h"
#include "vrouter.h"

struct host_os *vrouter_host;

unsigned int vr_flow_entries = 0;
unsigned int vr_oflow_entries = 0;
unsigned int vr_flow_hold_limit = 0;
unsigned int vr_bridge_entries = 0;
unsigned int vr_bridge_oentries = 0;

const char *ContrailBuildInfo = "";

int vr_flow_init(struct vrouter *router) {
    return 0;
}

void vr_flow_exit(struct vrouter *router, bool soft_reset) {
}

int vr_fib_init(struct vrouter *router) {
    return 0;
}

void vr_fib_exit(struct vrouter *router, bool soft_reset) {
}

int vr_vxlan_init(struct vrouter *router) {
    return 0;
}

void vr_vxlan_exit(struct vrouter *router, bool soft_reset) {
}

int vr_stats_init(struct vrouter *router) {
    return 0;
}

void vr_stats_exit(struct vrouter *router, bool soft_reset) {
}

int vr_mpls_init(struct vrouter *router) {
    return 0;
}

void vr_mpls_exit(struct vrouter *router, bool soft_reset) {
}

int vr_qos_init(struct vrouter *router) {
    return 0;
}

void vr_qos_exit(struct vrouter *router, bool soft_reset) {
}

unsigned int vr_flow_table_used_oflow_entries(struct vrouter *router) {
    return 0;
}

unsigned int vr_flow_table_used_total_entries(struct vrouter *router) {
    return 0;
}

unsigned int vr_bridge_table_used_oflow_entries(struct vrouter *router) {
    return 0;
}

unsigned int vr_bridge_table_used_total_entries(struct vrouter *router) {
    return 0;
}

struct host_os * vrouter_get_host(void) {
    return vrouter_host;
}
