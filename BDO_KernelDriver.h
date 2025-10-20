#pragma once

#include <ntddk.h>
#include <windef.h>
#include <ntstrsafe.h>
#include <ntimage.h>

// BDO Kernel Driver for studying game internals and anticheat system
// This driver operates at kernel level to bypass user-mode protections

#define DEVICE_NAME L"\\Device\\BDOAnalyzer"
#define SYMBOLIC_LINK L"\\DosDevices\\BDOAnalyzer"
#define IOCTL_BASE 0x8000

// IOCTL codes for communication with user-mode
#define IOCTL_GET_PROCESS_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x01, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SCAN_MEMORY \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x02, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MONITOR_HOOKS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x03, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ANALYZE_ANTICHEAT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x04, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_MODULE_INFO \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x05, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_MONITOR_SYSCALLS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x06, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ANALYZE_MEMORY_PATTERNS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 0x07, METHOD_BUFFERED, FILE_ANY_ACCESS)

// BDO-specific constants
#define BDO_PROCESS_NAME L"BlackDesert64.exe"
#define BDO_PROCESS_NAME_32 L"BlackDesert.exe"
#define BDO_ANTICHEAT_MODULE L"XIGNCODE3.exe"
#define BDO_ANTICHEAT_MODULE_ALT L"nProtect GameGuard"

// Memory regions to monitor
typedef struct _MEMORY_REGION {
    PVOID BaseAddress;
    SIZE_T RegionSize;
    ULONG Protection;
    ULONG State;
    ULONG Type;
} MEMORY_REGION, *PMEMORY_REGION;

// Process information structure
typedef struct _PROCESS_INFO {
    ULONG ProcessId;
    ULONG ParentProcessId;
    ULONG64 ProcessHandle;
    UNICODE_STRING ProcessName;
    UNICODE_STRING CommandLine;
    PVOID BaseAddress;
    SIZE_T ImageSize;
    ULONG64 CreationTime;
    ULONG64 ExitTime;
    ULONG ExitStatus;
} PROCESS_INFO, *PPROCESS_INFO;

// Module information structure
typedef struct _MODULE_INFO {
    PVOID BaseAddress;
    SIZE_T Size;
    UNICODE_STRING ModuleName;
    UNICODE_STRING FullPath;
    ULONG LoadCount;
    ULONG Flags;
    PVOID EntryPoint;
} MODULE_INFO, *PMODULE_INFO;

// Hook detection structure
typedef struct _HOOK_INFO {
    PVOID OriginalAddress;
    PVOID HookAddress;
    SIZE_T HookSize;
    UCHAR OriginalBytes[16];
    UCHAR HookBytes[16];
    UNICODE_STRING ModuleName;
    ULONG HookType; // 0=Inline, 1=IAT, 2=EAT, 3=SSDT, etc.
} HOOK_INFO, *PHOOK_INFO;

// Anticheat analysis structure
typedef struct _ANTICHEAT_ANALYSIS {
    BOOLEAN IsPresent;
    UNICODE_STRING AnticheatName;
    PVOID AnticheatBase;
    SIZE_T AnticheatSize;
    ULONG DetectionMethods;
    ULONG HookCount;
    ULONG DriverCount;
    BOOLEAN KernelModeProtection;
    BOOLEAN UserModeProtection;
    BOOLEAN MemoryProtection;
    BOOLEAN ProcessProtection;
    BOOLEAN NetworkMonitoring;
    ULONG64 LastScanTime;
    ULONG ScanFrequency;
} ANTICHEAT_ANALYSIS, *PANTICHEAT_ANALYSIS;

// Memory pattern structure
typedef struct _MEMORY_PATTERN {
    PVOID Address;
    SIZE_T Size;
    UCHAR Pattern[256];
    SIZE_T PatternSize;
    UNICODE_STRING Description;
    BOOLEAN IsExecutable;
    BOOLEAN IsWritable;
    BOOLEAN IsReadable;
} MEMORY_PATTERN, *PMEMORY_PATTERN;

// Syscall monitoring structure
typedef struct _SYSCALL_INFO {
    ULONG SyscallNumber;
    PVOID SyscallAddress;
    PVOID HookAddress;
    UCHAR OriginalBytes[16];
    UCHAR HookBytes[16];
    ULONG64 CallCount;
    ULONG64 LastCallTime;
    BOOLEAN IsHooked;
} SYSCALL_INFO, *PSYSCALL_INFO;

// Request/Response structures for IOCTL communication
typedef struct _SCAN_MEMORY_REQUEST {
    ULONG ProcessId;
    PVOID StartAddress;
    SIZE_T Size;
    UCHAR Pattern[256];
    SIZE_T PatternSize;
    UCHAR Mask[256];
} SCAN_MEMORY_REQUEST, *PSCAN_MEMORY_REQUEST;

typedef struct _SCAN_MEMORY_RESPONSE {
    ULONG ResultCount;
    PVOID Addresses[100]; // Maximum 100 results
} SCAN_MEMORY_RESPONSE, *PSCAN_MEMORY_RESPONSE;

typedef struct _MONITOR_HOOKS_REQUEST {
    ULONG ProcessId;
    BOOLEAN EnableMonitoring;
    ULONG HookTypes; // Bitmask of hook types to monitor
} MONITOR_HOOKS_REQUEST, *PMONITOR_HOOKS_REQUEST;

typedef struct _MONITOR_HOOKS_RESPONSE {
    ULONG HookCount;
    HOOK_INFO Hooks[50]; // Maximum 50 hooks
} MONITOR_HOOKS_RESPONSE, *PMONITOR_HOOKS_RESPONSE;

typedef struct _ANALYZE_ANTICHEAT_REQUEST {
    ULONG ProcessId;
    BOOLEAN DeepAnalysis;
    BOOLEAN MonitorNetwork;
    BOOLEAN MonitorMemory;
} ANALYZE_ANTICHEAT_REQUEST, *PANALYZE_ANTICHEAT_REQUEST;

typedef struct _ANALYZE_ANTICHEAT_RESPONSE {
    ANTICHEAT_ANALYSIS Analysis;
    ULONG HookCount;
    HOOK_INFO Hooks[20];
    ULONG DriverCount;
    UNICODE_STRING Drivers[10];
} ANALYZE_ANTICHEAT_RESPONSE, *PANALYZE_ANTICHEAT_RESPONSE;

// BDO-specific memory offsets (these change with game updates)
typedef struct _BDO_OFFSETS {
    // Player data offsets
    PVOID PlayerBase;
    PVOID PlayerHealth;
    PVOID PlayerMana;
    PVOID PlayerStamina;
    PVOID PlayerPosition;
    PVOID PlayerRotation;
    PVOID PlayerLevel;
    PVOID PlayerSilver;
    
    // Combat system offsets
    PVOID CombatState;
    PVOID SkillCooldowns;
    PVOID TargetInfo;
    PVOID DamageInfo;
    
    // Inventory system offsets
    PVOID InventoryBase;
    PVOID ItemSlots;
    PVOID WeightInfo;
    
    // UI system offsets
    PVOID UIBase;
    PVOID ChatSystem;
    PVOID MenuSystem;
    
    // Network system offsets
    PVOID NetworkBase;
    PVOID PacketHandler;
    PVOID SendFunction;
    PVOID RecvFunction;
} BDO_OFFSETS, *PBDO_OFFSETS;

// Function prototypes
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS CreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// Helper functions
NTSTATUS GetProcessInfo(ULONG ProcessId, PPROCESS_INFO ProcessInfo);
NTSTATUS ScanMemoryPattern(ULONG ProcessId, PVOID StartAddress, SIZE_T Size, 
                          PUCHAR Pattern, SIZE_T PatternSize, PUCHAR Mask, 
                          PVOID* Results, PULONG ResultCount);
NTSTATUS DetectHooks(ULONG ProcessId, PHOOK_INFO Hooks, PULONG HookCount);
NTSTATUS AnalyzeAnticheat(ULONG ProcessId, PANTICHEAT_ANALYSIS Analysis);
NTSTATUS GetModuleInfo(ULONG ProcessId, PUNICODE_STRING ModuleName, PMODULE_INFO ModuleInfo);
NTSTATUS MonitorSyscalls(ULONG ProcessId, PSYSCALL_INFO Syscalls, PULONG SyscallCount);
NTSTATUS AnalyzeMemoryPatterns(ULONG ProcessId, PMEMORY_PATTERN Patterns, PULONG PatternCount);

// BDO-specific functions
NTSTATUS FindBDOProcess(PULONG ProcessId);
NTSTATUS AnalyzeBDOInternals(ULONG ProcessId, PBDO_OFFSETS Offsets);
NTSTATUS DetectBDOAnticheat(ULONG ProcessId, PANTICHEAT_ANALYSIS Analysis);
NTSTATUS MonitorBDOBehavior(ULONG ProcessId);
NTSTATUS AnalyzeBDOCombat(ULONG ProcessId);
NTSTATUS AnalyzeBDONetwork(ULONG ProcessId);

// Utility functions
BOOLEAN IsValidProcess(ULONG ProcessId);
BOOLEAN IsBDOProcess(ULONG ProcessId);
BOOLEAN IsAnticheatProcess(ULONG ProcessId);
PVOID GetProcessBaseAddress(ULONG ProcessId);
SIZE_T GetProcessImageSize(ULONG ProcessId);
NTSTATUS ReadProcessMemory(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size);
NTSTATUS WriteProcessMemory(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size);

// Pattern scanning functions
NTSTATUS FindPattern(PVOID BaseAddress, SIZE_T Size, PUCHAR Pattern, SIZE_T PatternSize, PUCHAR Mask);
NTSTATUS FindPatternInProcess(ULONG ProcessId, PVOID BaseAddress, SIZE_T Size, 
                             PUCHAR Pattern, SIZE_T PatternSize, PUCHAR Mask, PVOID* Result);

// Hook detection functions
BOOLEAN IsInlineHook(PVOID Address);
BOOLEAN IsIATHook(PVOID Address);
BOOLEAN IsEATHook(PVOID Address);
BOOLEAN IsSSDTHook(ULONG SyscallNumber);
NTSTATUS RestoreOriginalBytes(PVOID Address, PUCHAR OriginalBytes, SIZE_T Size);

// Anticheat detection functions
BOOLEAN DetectXigncode3(ULONG ProcessId);
BOOLEAN DetectGameGuard(ULONG ProcessId);
BOOLEAN DetectBattlEye(ULONG ProcessId);
BOOLEAN DetectEasyAntiCheat(ULONG ProcessId);
NTSTATUS AnalyzeAnticheatBehavior(ULONG ProcessId, PANTICHEAT_ANALYSIS Analysis);

// Memory protection functions
NTSTATUS DisableMemoryProtection(ULONG ProcessId);
NTSTATUS EnableMemoryProtection(ULONG ProcessId);
NTSTATUS BypassASLR(ULONG ProcessId);
NTSTATUS BypassDEP(ULONG ProcessId);

// Debug and logging functions
VOID LogMessage(PWCHAR Format, ...);
VOID LogError(NTSTATUS Status, PWCHAR Function);
VOID LogProcessInfo(ULONG ProcessId);
VOID LogModuleInfo(ULONG ProcessId, PUNICODE_STRING ModuleName);

// Global variables
PDEVICE_OBJECT g_DeviceObject = NULL;
UNICODE_STRING g_DeviceName;
UNICODE_STRING g_SymbolicLink;
BOOLEAN g_DriverLoaded = FALSE;

// BDO-specific global variables
ULONG g_BDOProcessId = 0;
BOOLEAN g_BDOFound = FALSE;
BDO_OFFSETS g_BDOOffsets = { 0 };
ANTICHEAT_ANALYSIS g_AnticheatAnalysis = { 0 };

// Hook monitoring
HOOK_INFO g_DetectedHooks[100];
ULONG g_HookCount = 0;
BOOLEAN g_HookMonitoringEnabled = FALSE;

// Syscall monitoring
SYSCALL_INFO g_SyscallInfo[256];
ULONG g_SyscallCount = 0;
BOOLEAN g_SyscallMonitoringEnabled = FALSE;

// Memory pattern analysis
MEMORY_PATTERN g_MemoryPatterns[50];
ULONG g_PatternCount = 0;
BOOLEAN g_PatternAnalysisEnabled = FALSE;
