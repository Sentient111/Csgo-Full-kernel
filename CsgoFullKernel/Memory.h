#pragma once
#include "Nt.h"

PEPROCESS targetApplication = nullptr;
ULONG pid = NULL;
UINT64 clientBase = NULL;
UINT64 engineBase = NULL;

template<typename t>
t ReadMemory(UINT64 addr)
{
	t buffer;

	SIZE_T copiedBytes = 0;
	MmCopyVirtualMemory(targetApplication, (PVOID)addr, PsGetCurrentProcess(), &buffer, sizeof(t), KernelMode, &copiedBytes);

	return buffer;
}


template<typename t>
NTSTATUS ReadVirtual(PEPROCESS process, uintptr_t addr, t* buffer)
{
	if (!process || !addr || !buffer)
		return STATUS_INVALID_PARAMETER;

	SIZE_T copiedBytes = 0;
	return MmCopyVirtualMemory(process, (PVOID)addr, PsGetCurrentProcess(), buffer, sizeof(t), KernelMode, &copiedBytes);
}

template<typename t>
NTSTATUS WriteVirtual(PEPROCESS process, uintptr_t addr, t* buffer)
{
	if (!process || !addr || !buffer)
		return STATUS_INVALID_PARAMETER;

	SIZE_T copiedBytes = 0;
	return MmCopyVirtualMemory(PsGetCurrentProcess(), buffer, process, (PVOID)addr, sizeof(t), KernelMode, &copiedBytes);
}



