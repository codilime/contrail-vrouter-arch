#pragma once

NTSTATUS
memory_init(void);

PVOID
memory_exit(void);

int
set_section_address(void);

VOID
unmap_section_address(void);