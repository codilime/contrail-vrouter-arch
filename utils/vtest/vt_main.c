/*
 * vt_main.c -- test main function
 *
 * Copyright (c) 2015, Juniper Networks, Inc.
 * All rights reserved
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <vtest.h>
#include <vt_main.h>
#include <vt_message.h>
#include <vt_process_xml.h>

#include <nl_util.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <net/if.h>
#include <vt_packet.h>
#endif /* _WINDOWS */

extern struct expect_vrouter expect_msg;
extern struct return_vrouter return_msg;

extern void vt_interface_req_process(void *s);
extern void vt_nexthop_req_process(void *s);
extern void vt_route_req_process(void *s);
extern void vt_response_process(void *s);
extern void vt_vrf_stats_req_process(void *s);
extern void vt_vrouter_ops_process(void *s);
extern void vt_vrf_assign_req_process(void *s);
extern void vt_flow_req_process(void *s);
extern void vt_vxlan_req_process(void *s);
extern void vt_drop_stats_req_process(void *s);
extern void vt_mpls_req_process(void *s);
extern void vt_mirror_req_process(void *s);
extern void vt_mem_stats_req_process(void *s);

struct vtest_module vt_modules[] = {
    {   .vt_name        =   "test_name",
        .vt_node        =   vt_test_name,
    },
    {
        .vt_name        =   "message",
        .vt_node        =   vt_message,
    },
#ifndef _WINDOWS
    {
        .vt_name        =   "packet",
        .vt_node        =   vt_packet,
    },
#endif /* _WINDOWS */
};

const size_t VTEST_NUM_MODULES = ARRAYSIZE(vt_modules);

void
vt_fill_nl_callbacks()
{
    nl_cb.vr_interface_req_process = vt_interface_req_process;
    nl_cb.vr_nexthop_req_process = vt_nexthop_req_process;
    nl_cb.vr_route_req_process = vt_route_req_process;
    nl_cb.vr_response_process = vt_response_process;
    nl_cb.vr_vrf_stats_req_process = vt_vrf_stats_req_process;
    nl_cb.vrouter_ops_process = vt_vrouter_ops_process;
    nl_cb.vr_vrf_assign_req_process = vt_vrf_assign_req_process;
    nl_cb.vr_flow_req_process = vt_flow_req_process;
    nl_cb.vr_vxlan_req_process = vt_vxlan_req_process;
    nl_cb.vr_drop_stats_req_process = vt_drop_stats_req_process;
    nl_cb.vr_mpls_req_process = vt_mpls_req_process;
    nl_cb.vr_mirror_req_process = vt_mirror_req_process;
    nl_cb.vr_mem_stats_req_process = vt_mem_stats_req_process;
}

static void
vt_dealloc_test(struct vtest *test) {

    vt_safe_free(test->vtest_name);
    vt_safe_free(test->vtest_error_module);
    int i = 0;

    for (i = 0; i < test->message_ptr_num; ++i) {
        vt_safe_free(test->messages.data[i].mem);
        vt_safe_free(test->messages.data[i].xml_data.element_expect_ptr);
    }

    for(i = 0; i < test->messages.expect_vrouter_msg->expected_ptr_num; ++i) {
        vt_safe_free(test->messages.expect_vrouter_msg->mem_expected_msg[i]);
    }

    return;
}

static int
vt_init(struct vtest *test)
{

    memset(test, 0, sizeof(struct vtest));
    memset(&expect_msg, 0, sizeof(expect_msg));
    memset(&return_msg, 0, sizeof(return_msg));

    test->messages.expect_vrouter_msg = &expect_msg;
    test->messages.return_vrouter_msg = &return_msg;
    test->message_ptr_num = 0;

    expect_msg.expected_ptr_num = test->message_ptr_num;
    return_msg.returned_ptr_num = test->message_ptr_num;

    test->vtest_name = calloc(VT_MAX_TEST_NAME_LEN, 1);
    if (!test->vtest_name) {
        return E_MAIN_ERR_ALLOC;
    }

    test->vtest_error_module = calloc(VT_MAX_TEST_MODULE_NAME_LEN, 1);
    if (!test->vtest_error_module) {
        return E_MAIN_ERR_ALLOC;
    }

    return E_MAIN_OK;

}

static void
vt_Usage(void)
{
    printf("Usage: %s <test xml description file>\n",
            VT_PROG_NAME);
    return;
}

int
main(int argc, char *argv[])
{
    int ret;
    unsigned int i;
    char *xml_file;
    unsigned int sock_proto = VR_NETLINK_PROTO_DEFAULT;

    struct stat stat_buf;
    struct vtest vtest;

    if (argc != 2) {
        vt_Usage();
        return E_MAIN_ERR_FARG;
    }

    xml_file = argv[1];
    ret = stat(xml_file, &stat_buf);
    if (ret) {
        perror(xml_file);
        return E_MAIN_ERR_XML;
    }
    ret = vt_init(&vtest);
    if (ret != E_MAIN_OK) {
        return ret;
    }

    vtest.vrouter_cl = vr_get_nl_client(sock_proto);
    if (!vtest.vrouter_cl) {
        fprintf(stderr, "Error registering NetLink client: %s (%d)\n",
                strerror(errno), errno);
        return E_MAIN_ERR_SOCK;
    }

    for (i = 0; i < VTEST_NUM_MODULES; i++) {
        if (vt_modules[i].vt_init) {
            ret = vt_modules[i].vt_init();
            if (ret != E_MAIN_OK) {
                fprintf(stderr, "%s: %s init failed\n", VT_PROG_NAME,
                        vt_modules[i].vt_name);
                return E_MAIN_ERR;
            }
        }
    }

    vt_parse_file(xml_file, &vtest);

    nl_free_client(vtest.vrouter_cl);
    vt_dealloc_test(&vtest);

    if (vtest.vtest_return == E_MAIN_TEST_FAIL) {
        fprintf(stderr, "Test failed\n");
        return EXIT_FAILURE;//E_MAIN_TEST_FAIL;

    } else if (vtest.vtest_return == E_MAIN_TEST_PASS) {
        fprintf(stdout, "Test passed\n");
        return EXIT_SUCCESS;//E_MAIN_TEST_PASS;

    } else {
        fprintf(stderr, "Test skipped\n");
        return 2;
    }

    return EXIT_SUCCESS;
}

