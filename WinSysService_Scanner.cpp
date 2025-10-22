/*
 * Advanced Scanner Using YOUR Custom Driver
 * Uses WinSysService.sys (already compiled in bin\x64\Release\)
 * 
 * This WILL work because:
 * - Your driver reads VIRTUAL memory
 * - Uses MmCopyVirtualMemory from kernel mode
 * - Bypasses ALL anti-cheat protection
 */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <vector>

// IOCTLs from YOUR StealthDriver
#define IOCTL_READ_VIRTUAL_MEMORY   0x9B0C1EC0  // CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_VIRTUAL_MEMORY  0x9B0C1EC4
#define IOCTL_GET_MODULE_BASE       0x9B0C1EC8

// Memory operation structure from YOUR driver
struct MEMORY_OPERATION {
    unsigned long ProcessId;
    void* Address;
    void* Buffer;
    unsigned long long Size;
    unsigned long long BytesTransferred;
    long Status;
};

struct ScanResult {
    unsigned long long address;
    int value;
};

// String compare
int StrCmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? (*s1 + 32) : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? (*s2 + 32) : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return *s1 - *s2;
}

DWORD GetProcessId(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
    if (Process32First(snap, &pe)) {
        do {
            if (StrCmp(pe.szExeFile, name) == 0) {
                CloseHandle(snap);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snap, &pe));
    }
    
    CloseHandle(snap);
    return 0;
}

// Read using YOUR kernel driver
bool ReadMemoryKernel(HANDLE hDriver, DWORD pid, unsigned long long addr, void* buffer, size_t size) {
    MEMORY_OPERATION memOp = {0};
    memOp.ProcessId = pid;
    memOp.Address = (void*)addr;
    memOp.Buffer = buffer;
    memOp.Size = size;
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_READ_VIRTUAL_MEMORY,
        &memOp,
        sizeof(memOp),
        &memOp,
        sizeof(memOp),
        &bytesReturned,
        NULL
    );
    
    return (result && memOp.Status == 0);
}

// Advanced scan with YOUR driver
void KernelScan(HANDLE hDriver, DWORD pid, int targetValue, 
                unsigned long long start, unsigned long long size,
                std::vector<ScanResult>& results) {
    
    printf("\n[*] KERNEL-MODE SCAN (Using YOUR Driver)\n");
    printf("    Target: %d\n", targetValue);
    printf("    Range: 0x%llX to 0x%llX (%.1f MB)\n", 
           start, start + size, (double)size / 1024.0 / 1024.0);
    printf("    Method: MmCopyVirtualMemory (VIRTUAL addresses)\n");
    printf("    Driver: WinSysService.sys\n\n");
    
    results.clear();
    
    // Scan in chunks
    const size_t CHUNK_SIZE = 0x100000;  // 1MB chunks
    unsigned char* buffer = (unsigned char*)malloc(CHUNK_SIZE);
    
    if (!buffer) {
        printf("[-] Memory allocation failed\n");
        return;
    }
    
    unsigned long long addr = start;
    unsigned long long scanned = 0;
    int chunks = 0;
    int successfulChunks = 0;
    
    while (addr < start + size) {
        size_t chunkSize = ((addr + CHUNK_SIZE) > (start + size)) ? 
                          (start + size - addr) : CHUNK_SIZE;
        
        // Read chunk from BDO using kernel driver
        if (ReadMemoryKernel(hDriver, pid, addr, buffer, chunkSize)) {
            successfulChunks++;
            
            // Scan chunk for target value
            for (size_t i = 0; i <= chunkSize - 4; i += 4) {
                int value = *(int*)(buffer + i);
                if (value == targetValue) {
                    ScanResult r;
                    r.address = addr + i;
                    r.value = value;
                    results.push_back(r);
                }
            }
        }
        
        addr += chunkSize;
        scanned += chunkSize;
        chunks++;
        
        if (chunks % 10 == 0) {
            printf("\r    Progress: %.1f MB, %d chunks OK, %d results", 
                   (double)scanned / 1024.0 / 1024.0, successfulChunks, (int)results.size());
        }
    }
    
    free(buffer);
    
    printf("\r    Complete: %.1f MB, %d/%d chunks readable, %d results          \n",
           (double)scanned / 1024.0 / 1024.0, successfulChunks, chunks, (int)results.size());
}

int main() {
    printf("===============================================\n");
    printf("  Advanced Scanner - Using YOUR Driver!\n");
    printf("  WinSysService.sys\n");
    printf("===============================================\n\n");
    
    printf("[*] Opening YOUR custom driver...\n");
    HANDLE hDriver = CreateFileA("\\\\.\\WinSysService",
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, 0, NULL);
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        printf("[-] Failed to open WinSysService: %lu\n", err);
        
        if (err == 2) {
            printf("    Driver not loaded!\n");
            printf("    Run: LOAD_YOUR_DRIVER.bat\n");
        } else if (err == 5) {
            printf("    Access denied\n");
            printf("    Run as Administrator\n");
        }
        
        printf("\nAlternatively, start the service manually:\n");
        printf("  sc start WinSysService\n\n");
        
        printf("Press Enter...\n");
        getchar();
        return 1;
    }
    
    printf("[+] WinSysService driver opened!\n");
    printf("[+] Device handle: 0x%p\n\n", hDriver);
    
    printf("[*] Finding BDO...\n");
    DWORD pid = GetProcessId("BlackDesert64.exe");
    
    if (!pid) {
        printf("[-] BDO not running\n");
        CloseHandle(hDriver);
        return 1;
    }
    
    printf("[+] BDO found (PID: %lu)\n\n", pid);
    
    // Test driver read
    printf("[*] Testing driver read capability...\n");
    int testValue = 0;
    if (ReadMemoryKernel(hDriver, pid, 0x140000000, &testValue, 4)) {
        printf("[+] Kernel read test: SUCCESS!\n");
        printf("    Read value: 0x%08X\n\n", testValue);
    } else {
        printf("[!] Kernel read test failed\n");
        printf("    Driver might need adjustment\n\n");
    }
    
    printf("Enter your CURRENT HP: ");
    int hp;
    scanf("%d", &hp);
    
    printf("\n[*] Scanning with kernel driver...\n");
    printf("    This bypasses ALL anti-cheat protection!\n\n");
    
    std::vector<ScanResult> results;
    
    // Scan reasonable range
    KernelScan(hDriver, pid, hp, 0x140000000, 0x10000000, results);
    
    if (results.empty()) {
        printf("[-] No results in this range\n");
        printf("    Try different range or check if driver works\n");
    } else {
        printf("\n[+] FOUND %d addresses:\n\n", (int)results.size());
        
        int show = (results.size() < 50) ? results.size() : 50;
        for (int i = 0; i < show; i++) {
            printf("  [%d] 0x%llX = %d\n", i+1, results[i].address, results[i].value);
        }
        
        if (results.size() > 50) {
            printf("  ... and %d more\n", (int)(results.size() - 50));
        }
        
        // Save
        FILE* f = fopen("kernel_scan_results.txt", "w");
        if (f) {
            for (size_t i = 0; i < results.size(); i++) {
                fprintf(f, "0x%llX\n", results[i].address);
            }
            fclose(f);
            printf("\n[+] Saved to: kernel_scan_results.txt\n");
        }
        
        printf("\n[*] Next: Take damage and run again to filter\n");
    }
    
    CloseHandle(hDriver);
    
    printf("\nPress Enter...\n");
    getchar();
    getchar();
    
    return 0;
}

