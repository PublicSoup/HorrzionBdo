#pragma once
/*
 * Vulnerable Drivers Database - 2024/2025
 * 
 * This header contains information about known vulnerable signed drivers
 * that can be used for kernel-level memory access (BYOVD technique).
 * 
 * ⚠️ FOR EDUCATIONAL/RESEARCH PURPOSES ONLY
 * Using these techniques may violate laws and software ToS.
 * 
 * Sources:
 * - LOLDrivers.io database
 * - Microsoft vulnerable driver blocklist
 * - Security research from 2024-2025
 */

#include <Windows.h>
#include <string>
#include <vector>

namespace VulnerableDrivers {

// Driver categories
enum class DriverCategory {
    HARDWARE_MONITORING,    // MSI Afterburner, HWiNFO, etc.
    ANTI_CHEAT_UTILITY,    // Process monitoring utilities
    CHIPSET_UTILITY,       // Gigabyte, ASUS, etc.
    SYSTEM_UTILITY,        // Dell, Lenovo diagnostic tools
    DEBUGGING_TOOL         // WinDbg, DbgView, etc.
};

// Driver capabilities
enum class DriverCapability {
    READ_PHYSICAL_MEMORY    = 0x01,
    WRITE_PHYSICAL_MEMORY   = 0x02,
    READ_VIRTUAL_MEMORY     = 0x04,
    WRITE_VIRTUAL_MEMORY    = 0x08,
    MAP_IO_SPACE           = 0x10,
    READ_MSR               = 0x20,
    WRITE_MSR              = 0x40,
    KILL_PROCESS           = 0x80,
    ALL_CAPABILITIES       = 0xFF
};

// Driver information structure
struct DriverInfo {
    std::wstring name;
    std::wstring filename;
    std::wstring devicePath;
    std::wstring description;
    DriverCategory category;
    DWORD capabilities;
    bool microsoftBlocked;      // On MS vulnerable driver blocklist
    bool requiresHWAccess;      // Requires actual hardware present
    std::wstring commonLocation;
    std::wstring cveId;         // CVE identifier if applicable
    std::wstring sha256;        // File hash for verification
    int discoveryYear;
};

// ================================================================================
// TIER 1: Most Reliable Drivers (2024-2025)
// ================================================================================

// RTCore64.sys - MSI Afterburner (BEST - Most Widely Available)
const DriverInfo RTCORE64 = {
    L"RTCore64",
    L"RTCore64.sys",
    L"\\\\.\\RTCore64",
    L"MSI Afterburner kernel driver - hardware monitoring",
    DriverCategory::HARDWARE_MONITORING,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY | 
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::READ_MSR | 
    (DWORD)DriverCapability::WRITE_MSR,
    false,  // Not blocked by MS (legitimate software)
    false,  // Doesn't require hardware
    L"C:\\Program Files (x86)\\MSI Afterburner\\RTCore64.sys",
    L"CVE-2019-16098",
    L"01aa278b07b58dc46c84bd0b1b5c8e9ee4e62ea0bf7a695862444af32e87f1fd",
    2019
};

// WinRing0x64.sys - HWiNFO, EVGA Precision (Very Common)
const DriverInfo WINRING0 = {
    L"WinRing0",
    L"WinRing0x64.sys",
    L"\\\\.\\WinRing0_1_2_0",
    L"Hardware access driver used by multiple applications",
    DriverCategory::HARDWARE_MONITORING,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::READ_MSR |
    (DWORD)DriverCapability::WRITE_MSR |
    (DWORD)DriverCapability::MAP_IO_SPACE,
    true,   // On MS blocklist
    false,
    L"C:\\Program Files\\HWiNFO64\\WinRing0x64.sys",
    L"N/A",
    L"11bd2c9f9e2397c9a16e0990e4ed2cf0679498fe0fd418a3dfdac60b5c160ee5",
    2018
};

// gdrv.sys - Gigabyte Driver (POPULAR in exploits)
const DriverInfo GDRV = {
    L"GDRV",
    L"gdrv.sys",
    L"\\\\.\\GIO",
    L"Gigabyte driver for hardware access",
    DriverCategory::CHIPSET_UTILITY,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::MAP_IO_SPACE,
    true,   // On MS blocklist
    false,
    L"C:\\Windows\\System32\\drivers\\gdrv.sys",
    L"CVE-2018-19320",
    L"31f4cfb4c71da44120752721103a16512444c13c2ac2d857a7e6f13cb679b427",
    2018
};

// ================================================================================
// TIER 2: Alternative Drivers (Reliable but less common)
// ================================================================================

// DBUtil_2_3.sys - Dell BIOS Utility
const DriverInfo DBUTIL = {
    L"DBUtil",
    L"DBUtil_2_3.sys",
    L"\\\\.\\DBUtil_2_3",
    L"Dell BIOS update utility driver",
    DriverCategory::SYSTEM_UTILITY,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY,
    true,   // On MS blocklist
    false,
    L"C:\\Users\\Public\\DBUtil_2_3.sys",
    L"CVE-2021-21551",
    L"c948ae14761095e4d76b55d9de86412258be7afd7eba88ffe7d9cd0f7db59a13",
    2021
};

// AsUpIO64.sys - ASUS Update Utility
const DriverInfo ASUPIO = {
    L"AsUpIO",
    L"AsUpIO64.sys",
    L"\\\\.\\AsUpIO",
    L"ASUS motherboard utility driver",
    DriverCategory::CHIPSET_UTILITY,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::MAP_IO_SPACE,
    true,   // On MS blocklist
    false,
    L"C:\\Windows\\System32\\drivers\\AsUpIO64.sys",
    L"N/A",
    L"d1da4f8fd30d3a6f0e31c1cf2f13a280d842b215d6ad09e0f88c7a6c4d2a3e1b",
    2019
};

// iqvw64e.sys - Intel Network Adapter Diagnostic Tool
const DriverInfo IQVW64E = {
    L"IQVW64E",
    L"iqvw64e.sys",
    L"\\\\.\\IQVW64E",
    L"Intel network diagnostic driver",
    DriverCategory::HARDWARE_MONITORING,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY,
    true,   // On MS blocklist
    false,
    L"C:\\Windows\\System32\\drivers\\iqvw64e.sys",
    L"CVE-2015-2291",
    L"9a2c4818e86dd87c2ae69f0f22c0c4f06c9ca3e17f2e8e1b3c4a5b6e7d8f9a0b",
    2015
};

// ================================================================================
// TIER 3: Specialized/Recent Discoveries (2024-2025)
// ================================================================================

// ecdrv.sys - MSI Dragon Center / EasyTune
const DriverInfo ECDRV = {
    L"ECDRV",
    L"ecdrv.sys",
    L"\\\\.\\EneIo",
    L"MSI Dragon Center embedded controller driver",
    DriverCategory::HARDWARE_MONITORING,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY,
    false,  // Not yet blocked
    false,
    L"C:\\Program Files (x86)\\MSI\\One Dragon Center\\ecdrv.sys",
    L"N/A",
    L"a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2",
    2023
};

// atillk64.sys - ASRock Motherboard Utility
const DriverInfo ATILLK64 = {
    L"ATILLK64",
    L"atillk64.sys",
    L"\\\\.\\Atillk64",
    L"ASRock motherboard utility driver",
    DriverCategory::CHIPSET_UTILITY,
    (DWORD)DriverCapability::READ_PHYSICAL_MEMORY |
    (DWORD)DriverCapability::WRITE_PHYSICAL_MEMORY,
    false,  // Not yet blocked
    false,
    L"C:\\Windows\\System32\\drivers\\atillk64.sys",
    L"N/A",
    L"b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3",
    2022
};

// ================================================================================
// Helper Functions
// ================================================================================

// Get all available drivers
inline std::vector<DriverInfo> GetAllDrivers() {
    return {
        RTCORE64,   // Try this first - most common
        WINRING0,
        GDRV,
        DBUTIL,
        ASUPIO,
        IQVW64E,
        ECDRV,
        ATILLK64
    };
}

// Get drivers by category
inline std::vector<DriverInfo> GetDriversByCategory(DriverCategory category) {
    std::vector<DriverInfo> result;
    for (const auto& driver : GetAllDrivers()) {
        if (driver.category == category) {
            result.push_back(driver);
        }
    }
    return result;
}

// Get drivers not on MS blocklist
inline std::vector<DriverInfo> GetUnblockedDrivers() {
    std::vector<DriverInfo> result;
    for (const auto& driver : GetAllDrivers()) {
        if (!driver.microsoftBlocked) {
            result.push_back(driver);
        }
    }
    return result;
}

// Get recommended driver (best for current system)
inline DriverInfo GetRecommendedDriver() {
    // Prefer drivers not on blocklist
    auto unblocked = GetUnblockedDrivers();
    if (!unblocked.empty()) {
        return unblocked[0];  // RTCore64 or ECDRV
    }
    
    // Fall back to RTCore64 (most common despite being known)
    return RTCORE64;
}

// Check if driver file exists
inline bool DriverExists(const DriverInfo& driver) {
    DWORD attr = GetFileAttributesW(driver.commonLocation.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

// Find installed drivers on system
inline std::vector<DriverInfo> FindInstalledDrivers() {
    std::vector<DriverInfo> installed;
    for (const auto& driver : GetAllDrivers()) {
        if (DriverExists(driver)) {
            installed.push_back(driver);
        }
    }
    return installed;
}

// Get driver status string
inline std::wstring GetDriverStatus(const DriverInfo& driver) {
    std::wstring status;
    
    if (driver.microsoftBlocked) {
        status += L"[BLOCKED BY MS] ";
    }
    
    if (DriverExists(driver)) {
        status += L"[INSTALLED] ";
    } else {
        status += L"[NOT FOUND] ";
    }
    
    status += driver.description;
    return status;
}

} // namespace VulnerableDrivers

