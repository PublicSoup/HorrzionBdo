#pragma once

// RTCore64.sys configuration for BDO bypass
// This replaces the corrupted iqvw64e.sys data

// Note: We don't embed RTCore64.sys directly because:
// 1. It's a legitimate signed driver from MSI
// 2. It's better to use the installed version
// 3. Embedding would increase binary size unnecessarily

// RTCore64 driver information
#define RTCORE_SERVICE_NAME "RTCore64"
#define RTCORE_DISPLAY_NAME "Micro-Star MSI Afterburner"
#define RTCORE_DRIVER_NAME "RTCore64.sys"

// Download information if needed
const char* RTCORE_DOWNLOAD_URL = "https://www.msi.com/Landing/afterburner/graphics-cards";
const char* RTCORE_INSTALL_PATH = "C:\\Program Files (x86)\\MSI Afterburner\\RTCore64.sys";

// Alternative vulnerable drivers for fallback
struct VulnerableDriverInfo {
    const char* name;
    const char* filename;
    const char* deviceName;
    const char* downloadUrl;
    unsigned int ioctlRead;
    unsigned int ioctlWrite;
};

// List of alternative vulnerable drivers for BDO bypass
const VulnerableDriverInfo alternativeDrivers[] = {
    {
        "MSI RTCore64",
        "RTCore64.sys",
        "\\\\.\\RTCore64",
        "https://www.msi.com/Landing/afterburner/graphics-cards",
        0x80002048,
        0x8000204C
    },
    {
        "Gigabyte gdrv",
        "gdrv.sys",
        "\\\\.\\GIO",
        "https://www.gigabyte.com/Support/Utility",
        0xC3502808,
        0xC3502808
    },
    {
        "ASUS AsIO2",
        "AsIO2.sys",
        "\\\\.\\AsIO2",
        "https://www.asus.com/support/",
        0xA0402440,
        0xA0402444
    }
};

// Placeholder for compatibility (no embedded driver data)
unsigned char* vulnerable_driver_data = nullptr;
unsigned int vulnerable_driver_len = 0;