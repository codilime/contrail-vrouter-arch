#pragma once

#include "vr_os.h"
#include "vr_windows.h"

extern NTSTATUS KsyncCreateDevice(PDRIVER_OBJECT DriverObject);
extern VOID KsyncDestroyDevice(PDRIVER_OBJECT DriverObject);
