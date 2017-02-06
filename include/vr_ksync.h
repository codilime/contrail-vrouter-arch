#pragma once

#include "vr_os.h"
#include "vr_windows.h"

NTSTATUS
CreateDevice(PDRIVER_OBJECT DriverObject);

VOID
DestroyDevice(PDRIVER_OBJECT DriverObject);

