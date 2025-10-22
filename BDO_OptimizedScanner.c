/*
 * BDO_OptimizedScanner.c
 * Optimized pattern scanning for kernel driver
 * Uses Boyer-Moore-like optimization for faster scanning
 */

#include <ntddk.h>

#define POOL_TAG_SCAN 'SCNB'

// Bad character table for Boyer-Moore optimization
typedef struct _PATTERN_SCANNER {
    PUCHAR Pattern;
    PUCHAR Mask;
    SIZE_T PatternLength;
    SIZE_T BadCharTable[256];
} PATTERN_SCANNER, *PPATTERN_SCANNER;

//=============================================================================
// Build Bad Character Table for Boyer-Moore
//=============================================================================

VOID BuildBadCharTable(PPATTERN_SCANNER Scanner) {
    // Initialize all characters to pattern length (max skip)
    for (int i = 0; i < 256; i++) {
        Scanner->BadCharTable[i] = Scanner->PatternLength;
    }
    
    // Set skip distance for characters in pattern
    for (SIZE_T i = 0; i < Scanner->PatternLength - 1; i++) {
        if (Scanner->Mask[i] == 0xFF) {
            Scanner->BadCharTable[Scanner->Pattern[i]] = Scanner->PatternLength - 1 - i;
        }
    }
}

//=============================================================================
// Fast Pattern Match (Boyer-Moore inspired)
//=============================================================================

BOOLEAN FastPatternMatch(PUCHAR Data, PPATTERN_SCANNER Scanner) {
    for (SIZE_T i = 0; i < Scanner->PatternLength; i++) {
        if (Scanner->Mask[i] == 0xFF) {
            if (Data[i] != Scanner->Pattern[i]) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

//=============================================================================
// Optimized Pattern Scan
//=============================================================================

NTSTATUS OptimizedPatternScan(
    PEPROCESS Process,
    ULONG64 StartAddress,
    ULONG64 EndAddress,
    PUCHAR Pattern,
    PUCHAR Mask,
    SIZE_T PatternLength,
    PULONG64 ResultAddress
) {
    NTSTATUS status;
    PUCHAR scanBuffer;
    SIZE_T chunkSize = 0x100000; // 1MB chunks (larger = faster)
    PATTERN_SCANNER scanner;
    
    *ResultAddress = 0;
    
    // Setup scanner
    scanner.Pattern = Pattern;
    scanner.Mask = Mask;
    scanner.PatternLength = PatternLength;
    BuildBadCharTable(&scanner);
    
    // Allocate scan buffer
    scanBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, chunkSize, POOL_TAG_SCAN);
    if (!scanBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // Scan memory in chunks
    for (ULONG64 currentAddress = StartAddress; 
         currentAddress < EndAddress; 
         currentAddress += (chunkSize - PatternLength + 1)) {
        
        SIZE_T readSize = (SIZE_T)min(chunkSize, EndAddress - currentAddress);
        
        // Read chunk using MmCopyVirtualMemory
        SIZE_T bytesRead = 0;
        
        __try {
            status = MmCopyVirtualMemory(
                Process,
                (PVOID)currentAddress,
                PsGetCurrentProcess(),
                scanBuffer,
                readSize,
                KernelMode,
                &bytesRead
            );
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            continue; // Skip inaccessible regions
        }
        
        if (!NT_SUCCESS(status) || bytesRead == 0) {
            continue;
        }
        
        // Boyer-Moore-like search
        SIZE_T i = 0;
        while (i <= bytesRead - PatternLength) {
            if (FastPatternMatch(&scanBuffer[i], &scanner)) {
                // Found match!
                *ResultAddress = currentAddress + i;
                ExFreePoolWithTag(scanBuffer, POOL_TAG_SCAN);
                return STATUS_SUCCESS;
            }
            
            // Skip ahead using bad character table
            BYTE lastChar = scanBuffer[i + PatternLength - 1];
            SIZE_T skip = scanner.BadCharTable[lastChar];
            
            if (skip == 0) skip = 1; // Minimum skip
            i += skip;
        }
    }
    
    ExFreePoolWithTag(scanBuffer, POOL_TAG_SCAN);
    return STATUS_NOT_FOUND;
}

//=============================================================================
// Multi-Pattern Scanner (Scan for Multiple Patterns at Once)
//=============================================================================

typedef struct _MULTI_PATTERN_RESULT {
    ULONG PatternIndex;
    ULONG64 Address;
} MULTI_PATTERN_RESULT, *PMULTI_PATTERN_RESULT;

NTSTATUS ScanMultiplePatterns(
    PEPROCESS Process,
    ULONG64 StartAddress,
    ULONG64 EndAddress,
    PPATTERN_SCANNER* Scanners,
    ULONG ScannerCount,
    PMULTI_PATTERN_RESULT Results,
    PULONG ResultCount
) {
    PUCHAR scanBuffer;
    SIZE_T chunkSize = 0x100000; // 1MB
    ULONG foundCount = 0;
    
    *ResultCount = 0;
    
    // Allocate scan buffer
    scanBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, chunkSize, POOL_TAG_SCAN);
    if (!scanBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // Scan once, test all patterns
    for (ULONG64 currentAddress = StartAddress; 
         currentAddress < EndAddress; 
         currentAddress += (chunkSize - 256)) { // Max pattern length = 256
        
        SIZE_T readSize = (SIZE_T)min(chunkSize, EndAddress - currentAddress);
        SIZE_T bytesRead = 0;
        
        __try {
            MmCopyVirtualMemory(
                Process,
                (PVOID)currentAddress,
                PsGetCurrentProcess(),
                scanBuffer,
                readSize,
                KernelMode,
                &bytesRead
            );
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            continue;
        }
        
        if (bytesRead == 0) continue;
        
        // Test all patterns
        for (ULONG p = 0; p < ScannerCount; p++) {
            for (SIZE_T i = 0; i <= bytesRead - Scanners[p]->PatternLength; i++) {
                if (FastPatternMatch(&scanBuffer[i], Scanners[p])) {
                    Results[foundCount].PatternIndex = p;
                    Results[foundCount].Address = currentAddress + i;
                    foundCount++;
                    
                    if (foundCount >= *ResultCount) {
                        // Results buffer full
                        ExFreePoolWithTag(scanBuffer, POOL_TAG_SCAN);
                        *ResultCount = foundCount;
                        return STATUS_SUCCESS;
                    }
                }
            }
        }
    }
    
    ExFreePoolWithTag(scanBuffer, POOL_TAG_SCAN);
    *ResultCount = foundCount;
    
    return foundCount > 0 ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

//=============================================================================
// Signature String Parser ("48 8B 0D ?? ?? ?? ??")
//=============================================================================

BOOLEAN ParseSignatureString(
    const char* SigStr,
    PUCHAR Pattern,
    PUCHAR Mask,
    PSIZE_T Length
) {
    SIZE_T len = 0;
    const char* p = SigStr;
    
    while (*p && len < 256) {
        // Skip whitespace
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        
        // Check for wildcard
        if (*p == '?' && *(p+1) == '?') {
            Pattern[len] = 0x00;
            Mask[len] = 0x00;
            p += 2;
            len++;
        }
        // Parse hex byte
        else if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'F') || (*p >= 'a' && *p <= 'f')) {
            char hex[3] = {*p, *(p+1), 0};
            ULONG value;
            
            RtlCharToInteger(hex, 16, &value);
            
            Pattern[len] = (UCHAR)value;
            Mask[len] = 0xFF;
            p += 2;
            len++;
        }
        else {
            // Invalid character
            return FALSE;
        }
    }
    
    *Length = len;
    return len > 0;
}

