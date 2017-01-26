struct host_os *vrouter_host;

int vr_perfr = 0;    /* GRO */
int vr_perfs = 0;    /* segmentation in software */

int vr_from_vm_mss_adj = 0; /* adjust TCP MSS on packets from VM */
int vr_to_vm_mss_adj = 0;   /* adjust TCP MSS on packet sent to VM */
