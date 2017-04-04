#pragma once

#include "vr_os.h"
#include "vr_windows.h"

NTSTATUS KsyncCreateDevice(PDRIVER_OBJECT DriverObject);
VOID KsyncDestroyDevice(PDRIVER_OBJECT DriverObject);
