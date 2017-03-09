#include "vrouter.h"
#include "vr_flow.h"

const WCHAR SectionName[] = L"\\BaseNamedObjects\\vRouter";
extern void *vr_flow_table;
extern void *vr_oflow_table;

HANDLE Section;

void
MemoryExit(void)
{
    NTSTATUS status = STATUS_SUCCESS;
    status = ZwClose(Section);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed closing a section, error code: %lx\r\n", status);
    }
}

VOID
UnmapSectionAddress(void)
{
    NTSTATUS status = STATUS_SUCCESS;
    status = ZwUnmapViewOfSection(Section, vr_flow_table);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed closing a section, error code: %lx\r\n", status);
    }
}

VOID
SetSectionAddress(void)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t flow_table_size;
    UNICODE_STRING _SectionName;
    PVOID BaseAddress = NULL;
    SIZE_T ViewSize = 0;

    RtlInitUnicodeString(&_SectionName, SectionName);

    flow_table_size = VR_FLOW_TABLE_SIZE + VR_OFLOW_TABLE_SIZE;

    status = ZwMapViewOfSection(Section, ZwCurrentProcess(), &BaseAddress, 0, flow_table_size, NULL, &ViewSize, ViewShare, 0, PAGE_READWRITE);
    if (status != STATUS_SUCCESS)
    {
        DbgPrint("Failed creating a mapping, error code: %lx\r\n", status);
    }

    vr_flow_table = BaseAddress;
    vr_oflow_table = (char *)BaseAddress + VR_FLOW_TABLE_SIZE;
}


PVOID
MemoryInit(void)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNICODE_STRING _SectionName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Attributes = OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK | OBJ_OPENIF;;
    LARGE_INTEGER MaxSize;

    RtlInitUnicodeString(&_SectionName, SectionName);

    InitializeObjectAttributes(&ObjectAttributes, &_SectionName, Attributes, NULL, NULL);

    if (!vr_oflow_entries)
        vr_oflow_entries = ((vr_flow_entries / 5) + 1023) & ~1023;

    size_t flow_table_size = VR_FLOW_TABLE_SIZE + VR_OFLOW_TABLE_SIZE;

    MaxSize.QuadPart = flow_table_size;

    status = ZwCreateSection(&Section, SECTION_ALL_ACCESS, &ObjectAttributes, &MaxSize, PAGE_READWRITE, SEC_COMMIT, NULL);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed creating a section, error code: %lx\r\n", status);
        DbgPrint("Falling back to opening an existing section...\r\n");
        status = ZwOpenSection(&Section, SECTION_ALL_ACCESS, &ObjectAttributes);
        if (status != STATUS_SUCCESS)
        {
            DbgPrint("Failed open a section, error code: %lx\r\n", status);
            return 0;
        }
    }

    return 0;
}
