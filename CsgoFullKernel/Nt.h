#pragma once
#include <Ntifs.h>
#include <WinDef.h>
#include <wingdi.h>
#include <ntddmou.h>
#include "VkCodes.h"

extern "C" int _fltused = 0;


typedef struct Vector2
{
	float x, y;
};

typedef struct Vector3
{
	float x, y, z;
};

typedef struct _MAT4X4
{
	float c[4][4];
}MAT4X4, * PMAT4X4;

typedef struct _MAT3X4
{
	float c[3][4];
}MAT3X4, * PMAT3X4;



float targetWindowWidth = 1920;
float targetWindowHeight = 1080;
float targetWindowPosX = 0;
float targetWindowPosY = 0;


#define Print(fmt, ...) DbgPrint("[s11]: " fmt, ##__VA_ARGS__)
#define MAX_PATH 260


//this is the same in every windows version
ULONG processOffset = 0x220; //_KTHREAD->_KPROCESS* Process;
ULONG cidOffset = 0x478; //_ETHREAD->_CLIENT_ID Cid;  


typedef DWORD LFTYPE;

extern "C"
{
	NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS Process);
	NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(IN PEPROCESS Process);
	NTKERNELAPI PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);
	NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
	NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName(PVOID ImageBase, PCCH RoutineNam);
	NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(PUNICODE_STRING ObjectName,ULONG Attributes,PACCESS_STATE AccessState,ACCESS_MASK DesiredAccess,POBJECT_TYPE ObjectType, KPROCESSOR_MODE AccessMode, PVOID ParseContext, PVOID* Object);

	NTKERNELAPI PVOID __fastcall PsGetProcessImageFileName(PEPROCESS process);
	NTKERNELAPI PVOID PsGetThreadWin32Thread(PKTHREAD thread);
	NTKERNELAPI PVOID PsSetThreadWin32Thread(PKTHREAD thread, PVOID wantedValue, PVOID compareValue);
}


typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;


typedef struct _SYSTEM_MODULE_ENTRY {
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} SYSTEM_MODULE_ENTRY, * PSYSTEM_MODULE_ENTRY;

typedef struct _SYSTEM_MODULE_INFORMATION {
	ULONG Count;
	SYSTEM_MODULE_ENTRY Module[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION
{
	/* 0x0000 */ ULONG NextEntryOffset;
	/* 0x0004 */ ULONG NumberOfThreads;
	/* 0x0008 */ LARGE_INTEGER WorkingSetPrivateSize;
	/* 0x0010 */ ULONG HardFaultCount;
	/* 0x0014 */ ULONG NumberOfThreadsHighWatermark;
	/* 0x0018 */ ULONGLONG CycleTime;
	/* 0x0020 */ LARGE_INTEGER CreateTime;
	/* 0x0028 */ LARGE_INTEGER UserTime;
	/* 0x0030 */ LARGE_INTEGER KernelTime;
	/* 0x0038 */ UNICODE_STRING ImageName;
	/* 0x0048 */ KPRIORITY BasePriority;
	/* 0x004C */ ULONG Padding1;
	/* 0x0050 */ ULONGLONG UniqueProcessId;
	/* 0x0058 */ ULONGLONG InheritedFromUniqueProcessId;
	/* 0x0060 */ ULONG HandleCount;
	/* 0x0064 */ ULONG SessionId;
	/* 0x0068 */ ULONG_PTR UniqueProcessKey;
	/* 0x0070 */ SIZE_T PeakVirtualSize;
	/* 0x0078 */ SIZE_T VirtualSize;
	/* 0x0080 */ ULONG PageFaultCount;
	/* 0x0084 */ ULONG Padding2;
	/* 0x0088 */ SIZE_T PeakWorkingSetSize;
	/* 0x0090 */ SIZE_T WorkingSetSize;
	/* 0x0098 */ SIZE_T QuotaPeakPagedPoolUsage;
	/* 0x00A0 */ SIZE_T QuotaPagedPoolUsage;
	/* 0x00A8 */ SIZE_T QuotaPeakNonPagedPoolUsage;
	/* 0x00B0 */ SIZE_T QuotaNonPagedPoolUsage;
	/* 0x00B8 */ SIZE_T PagefileUsage;
	/* 0x00C0 */ SIZE_T PeakPagefileUsage;
	/* 0x00C8 */ SIZE_T PrivatePageCount;
	/* 0x00D0 */ LARGE_INTEGER ReadOperationCount;
	/* 0x00D8 */ LARGE_INTEGER WriteOperationCount;
	/* 0x00E0 */ LARGE_INTEGER OtherOperationCount;
	/* 0x00E8 */ LARGE_INTEGER ReadTransferCount;
	/* 0x00F0 */ LARGE_INTEGER WriteTransferCount;
	/* 0x00F8 */ LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION; /* size: 0x0100 */

C_ASSERT(sizeof(SYSTEM_PROCESS_INFORMATION) == 0x100);

PVOID NtUserGetForegroundWindowPtr = 0;

HWND NtUserGetForegroundWindow(void)
{
	auto fn = reinterpret_cast<HWND(*)(void)>(NtUserGetForegroundWindowPtr);
	return fn();
}

PVOID NtUserQueryWindowPtr = 0;

#define QUERY_FOREGROUNDWINDOW 7
#define QUERY_THREAD_PROCESS_ID 1
#define QUERY_THREAD_ID 2
#define QUERY_IS_WINDOW_HUNG 5 //this one could be wrong but it does something along those lines

//no fucking clue what the rest of the codes do. they query something from the wnd stuct which I have not worked with so /shrug

UINT64 NtUserQueryWindow(HWND window, int queryType)
{
	auto fn = reinterpret_cast<DWORD(*)(HWND, int)>(NtUserQueryWindowPtr);
	return fn(window, queryType);
}


PVOID NtUserGetDCPtr = 0;
PVOID NtGdiSelectBrushPtr = 0;
PVOID NtGdiPatBltPtr = 0;
PVOID NtUserReleaseDCPtr = 0;
PVOID NtGdiCreateSolidBrushPtr = 0;
PVOID NtGdiDeleteObjectAppPtr = 0;
PVOID NtGdiExtTextOutWPtr = 0;
PVOID NtGdiHfontCreatePtr = 0;
PVOID NtGdiSelectFontPtr = 0;

inline HDC NtUserGetDC(HWND hwnd)
{
	auto fn = reinterpret_cast<HDC(*)(HWND hwnd)>(NtUserGetDCPtr);
	return fn(hwnd);
}

inline HBRUSH NtGdiSelectBrush(HDC hdc, HBRUSH hbrush)
{
	auto fn = reinterpret_cast<HBRUSH(*)(HDC hdc, HBRUSH hbrush)>(NtGdiSelectBrushPtr);
	return fn(hdc, hbrush);
}

inline BOOL NtGdiPatBlt(HDC hdcDest, INT x, INT y, INT cx, INT cy, DWORD dwRop)
{
	auto fn = reinterpret_cast<BOOL(*)(HDC hdcDest, INT x, INT y, INT cx, INT cy, DWORD dwRop)>(NtGdiPatBltPtr);
	return fn(hdcDest, x, y, cx, cy, dwRop);
}

inline int NtUserReleaseDC(HDC hdc)
{
	auto fn = reinterpret_cast<int(*)(HDC hdc)>(NtUserReleaseDCPtr);
	return fn(hdc);
}

inline HBRUSH NtGdiCreateSolidBrush(COLORREF cr, HBRUSH hbr)
{
	auto fn = reinterpret_cast<HBRUSH(*)(COLORREF cr, HBRUSH hbr)>(NtGdiCreateSolidBrushPtr);
	return fn(cr, hbr);
}

inline BOOL NtGdiDeleteObjectApp(HANDLE hobj)
{
	auto fn = reinterpret_cast<BOOL(*)(HANDLE hobj)>(NtGdiDeleteObjectAppPtr);
	return fn(hobj);
}

inline BOOL NtGdiExtTextOutW(HDC hDC, INT XStart, INT YStart, UINT fuOptions, LPRECT UnsafeRect, LPWSTR UnsafeString, INT Count, LPINT UnsafeDx, DWORD dwCodePage)
{
	auto fn = reinterpret_cast<BOOL(*)(HDC hDC, INT XStart, INT YStart, UINT fuOptions, LPRECT UnsafeRect, LPWSTR UnsafeString, INT Count, LPINT UnsafeDx, DWORD dwCodePage)>(NtGdiExtTextOutWPtr);
	return fn(hDC, XStart, YStart, fuOptions, UnsafeRect, UnsafeString, Count, UnsafeDx, dwCodePage);
}

inline HFONT NtGdiHfontCreate(PENUMLOGFONTEXDVW pelfw, ULONG cjElfw, LFTYPE lft, FLONG fl, PVOID pvCliData)
{
	auto fn = reinterpret_cast<HFONT(*)(PENUMLOGFONTEXDVW pelfw, ULONG cjElfw, LFTYPE lft, FLONG fl, PVOID pvCliData)>(NtGdiHfontCreatePtr);
	return fn(pelfw, cjElfw, lft, fl, pvCliData);
}

inline HFONT NtGdiSelectFont(HDC hdc, HFONT hfont)
{
	auto fn = reinterpret_cast<HFONT(*)(HDC hdc, HFONT hfont)>(NtGdiSelectFontPtr);
	return fn(hdc, hfont);
}


typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef void(__stdcall* PPS_POST_PROCESS_INIT_ROUTINE)(void); // not exported

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID Reserved4[3];
	PVOID AtlThunkSListPtr;
	PVOID Reserved5;
	ULONG Reserved6;
	PVOID Reserved7;
	ULONG Reserved8;
	ULONG AtlThunkSListPtr32;
	PVOID Reserved9[45];
	BYTE Reserved10[96];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved11[128];
	PVOID Reserved12[1];
	ULONG SessionId;
} PEB, * PPEB;

typedef struct _PEB32 {
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG Mutant;
	ULONG ImageBaseAddress;
	ULONG Ldr;
	ULONG ProcessParameters;
	ULONG SubSystemData;
	ULONG ProcessHeap;
	ULONG FastPebLock;
	ULONG AtlThunkSListPtr;
	ULONG IFEOKey;
	ULONG CrossProcessFlags;
	ULONG UserSharedInfoPtr;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	ULONG ApiSetMap;
} PEB32, * PPEB32;

typedef struct _PEB_LDR_DATA32 {
	ULONG Length;
	UCHAR Initialized;
	ULONG SsHandle;
	LIST_ENTRY32 InLoadOrderModuleList;
	LIST_ENTRY32 InMemoryOrderModuleList;
	LIST_ENTRY32 InInitializationOrderModuleList;
} PEB_LDR_DATA32, * PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32 {
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderLinks;
	LIST_ENTRY32 InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY32 HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;  // in bytes
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;  // LDR_*
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
	//    PVOID			LoadedImports;
	//    // seems they are exist only on XP !!! PVOID
	//    EntryPointActivationContext;	// -same-
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

