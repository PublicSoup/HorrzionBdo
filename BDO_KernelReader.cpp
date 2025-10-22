/*
 * BDO Kernel Memory Reader using RTCore64
 * Reads player health directly from kernel level
 */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>

// MinGW compatibility
int StrCmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? (*s1 + 32) : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? (*s2 + 32) : *s2;
        if (c1 != c2) return c1 - c2;
        s1++; s2++;
    }
    return *s1 - *s2;
}

#define IOCTL_READ_MEMORY 0x80002048

struct RTCore64Request {
    unsigned long long Address;
    unsigned long Size;
    unsigned long Value;
    unsigned long Padding;
};

// Find process ID by name
DWORD GetProcessId(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(snapshot, &pe32)) {
        do {
            if (StrCmp(pe32.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    return 0;
}

// Read memory using RTCore64 (physical memory)
bool ReadMemoryKernel(HANDLE hDriver, unsigned long long address, void* buffer, size_t size) {
    if (size != 4) {
        printf("Note: RTCore64 reads 4 bytes at a time\n");
        size = 4;
    }
    
    RTCore64Request request = {0};
    request.Address = address;
    request.Size = 4;
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_READ_MEMORY,
        &request,
        sizeof(request),
        &request,
        sizeof(request),
        &bytesReturned,
        NULL
    );
    
    if (result) {
        memcpy(buffer, &request.Value, size);
        return true;
    }
    
    return false;
}

// Read using standard ReadProcessMemory (for comparison)
bool ReadMemoryStandard(DWORD processId, unsigned long long address, void* buffer, size_t size) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) return false;
    
    SIZE_T bytesRead = 0;
    BOOL result = ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead);
    
    CloseHandle(hProcess);
    return result && (bytesRead == size);
}

int main() {
    printf("===============================================\n");
    printf("  BDO Kernel Memory Reader (RTCore64)\n");
    printf("===============================================\n\n");
    
    // Open RTCore64 driver
    printf("[*] Opening RTCore64 driver...\n");
    HANDLE hDriver = CreateFileA(
        "\\\\.\\RTCore64",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        printf("[-] Failed to open RTCore64!\n");
        printf("    Make sure MSI Afterburner is running\n");
        printf("    Or run START_RTCORE64.bat as admin\n");
        return 1;
    }
    
    printf("[+] RTCore64 opened successfully!\n\n");
    
    // Find BDO process
    printf("[*] Looking for BlackDesert64.exe...\n");
    DWORD pid = GetProcessId("BlackDesert64.exe");
    
    if (pid == 0) {
        printf("[-] BlackDesert64.exe not running\n");
        printf("    Start BDO first, then run this program\n");
        CloseHandle(hDriver);
        return 1;
    }
    
    printf("[+] Found BlackDesert64.exe (PID: %lu)\n\n", pid);
    
    // Example: Read memory (you need to find addresses with Cheat Engine first)
    printf("===============================================\n");
    printf("  INSTRUCTIONS:\n");
    printf("===============================================\n");
    printf("1. Use Cheat Engine to find your health address\n");
    printf("2. Edit this code and put the address below\n");
    printf("3. Recompile and run\n");
    printf("4. You'll see your health read via kernel!\n\n");
    
    // EXAMPLE ADDRESS - Replace with your actual address from Cheat Engine!
    unsigned long long healthAddress = 0x7FF600000000;  // CHANGE THIS!
    
    printf("NOTE: The address 0x%llX is just an example\n", healthAddress);
    printf("      Find your real address with Cheat Engine!\n\n");
    
    // Try reading with standard method (for comparison)
    printf("[*] Testing standard ReadProcessMemory...\n");
    float healthStandard = 0;
    if (ReadMemoryStandard(pid, healthAddress, &healthStandard, sizeof(float))) {
        printf("[+] Standard read: %.2f\n", healthStandard);
    } else {
        printf("[-] Standard read failed (anti-cheat may block this)\n");
    }
    
    printf("\n[*] Testing kernel ReadMemory via RTCore64...\n");
    unsigned long healthKernel = 0;
    if (ReadMemoryKernel(hDriver, healthAddress, &healthKernel, 4)) {
        printf("[+] Kernel read successful!\n");
        printf("    Raw value: 0x%08lX\n", healthKernel);
        
        // Interpret as float
        float* fValue = (float*)&healthKernel;
        printf("    As float: %.2f\n", *fValue);
    } else {
        printf("[-] Kernel read failed\n");
    }
    
    printf("\n===============================================\n");
    printf("  NEXT STEPS:\n");
    printf("===============================================\n");
    printf("1. Open Cheat Engine\n");
    printf("2. Attach to BlackDesert64.exe\n");
    printf("3. Scan for your health (Float, 4 bytes)\n");
    printf("4. Take damage and scan again\n");
    printf("5. When you find it, copy the address\n");
    printf("6. Replace healthAddress in this code\n");
    printf("7. Recompile: g++ BDO_KernelReader.cpp -o BDO_KernelReader.exe\n");
    printf("8. Run again to see your health!\n");
    printf("===============================================\n\n");
    
    CloseHandle(hDriver);
    
    printf("Press Enter to exit...\n");
    getchar();
    
    return 0;
}

