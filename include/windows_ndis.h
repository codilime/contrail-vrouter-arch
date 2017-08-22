#pragma once

#include "vr_windows.h"

NDIS_STATUS
vr_intialize_vrouter(
    PSX_SWITCH_OBJECT Switch,
    PNDIS_HANDLE *ExtensionContext
);

NDIS_STATUS
UpdateNics(PNDIS_SWITCH_NIC_PARAMETERS Nic, BOOLEAN connect);