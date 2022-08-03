#pragma once

PEPROCESS csrssProc = nullptr;
PVOID gafAsyncKeyStatePtr = 0;


BYTE keystateBitmap[256 * 2 / 8];
BYTE keystateRecentBitmap[256 / 8];


NTSTATUS InitKeyMap()
{
    PVOID win32kBase = (PVOID)GetKernelModuleBase("win32kbase.sys");
    gafAsyncKeyStatePtr = RtlFindExportedRoutineByName(win32kBase, "gafAsyncKeyState");
    if (!gafAsyncKeyStatePtr)
    {
        Print("Failed to get gafAsyncKeyState\n");
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS status = GetProcByName("csrss.exe", &csrssProc, 1);
    if (!NT_SUCCESS(status))
    {
        Print("Failed to get csrss process %x\n", status);
        return status;
    }

    if (!csrssProc)
    {
        Print("Invalid csrss process\n");
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

bool KeyDown(BYTE vk) 
{
	return keystateBitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2;
}

bool WasKeyDown(BYTE vk) 
{
	bool result = keystateRecentBitmap[vk / 8] & 1 << vk % 8;
	keystateRecentBitmap[vk / 8] &= ~(1 << vk % 8);
	return result;
}

bool UpdateKeyMap() 
{
    SpoofWin32Thread();

    auto prevKeystateBitmap = keystateBitmap;
    SIZE_T size = 0;
    NTSTATUS status = MmCopyVirtualMemory(csrssProc, gafAsyncKeyStatePtr, PsGetCurrentProcess(), &keystateBitmap, sizeof(keystateBitmap), KernelMode, &size);
    if (!NT_SUCCESS(status))
    {
        Print("Failed to copy bitmap %x\n", status);
        UnspoofWin32Thread();
        return false;
    }

    for (auto vk = 0u; vk < 256; ++vk) 
    {
        if ((keystateBitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2) &&
            !(prevKeystateBitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2))
            keystateRecentBitmap[vk / 8] |= 1 << vk % 8;
    }

    UnspoofWin32Thread();
    return true;
}