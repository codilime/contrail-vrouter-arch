/*-
 * Copyright (c) 2014 Semihalf
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include "vrouter.h"
#include "vr_flow.h"

const WCHAR SectionName[] = L"\\BaseNamedObjects\\vRouter";

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
