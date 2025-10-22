#pragma once
#include <Windows.h>

// Physical memory access using RTCore64
// This bypasses ALL memory protection including XignCode's kernel-level blocks

class PhysicalMemoryReader {
public:
    PhysicalMemoryReader();
    ~PhysicalMemoryReader();
    
    // Main function: Read virtual address from target process using physical memory
    bool ReadVirtualMemory(DWORD processId, ULONG64 virtualAddress, PVOID buffer, SIZE_T size);
    
    // Get CR3 (page directory base) for a process
    bool GetProcessCR3(DWORD processId, ULONG64& cr3);
    
    // Translate virtual address to physical address
    bool VirtualToPhysical(ULONG64 virtualAddress, ULONG64 cr3, ULONG64& physicalAddress);
    
    // Read physical memory directly
    bool ReadPhysicalMemory(ULONG64 physicalAddress, PVOID buffer, SIZE_T size);
    
private:
    // Helper: Read physical memory in DWORD chunks
    bool ReadPhysicalDWORD(ULONG64 physicalAddress, DWORD& value);
    
    // Helper: Get DirectoryTableBase from EPROCESS
    bool GetDirectoryTableBase(DWORD processId, ULONG64& dtb);
};

extern PhysicalMemoryReader g_PhysicalMemory;

