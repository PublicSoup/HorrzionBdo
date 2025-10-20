#pragma once

#include <windows.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <variant>
#include <chrono>

// Advanced Memory Scanner with multi-threaded support and value type detection

enum class ValueType {
    Byte,           // 1 byte
    Word,           // 2 bytes
    Dword,          // 4 bytes
    Qword,          // 8 bytes
    Float,          // 4 bytes
    Double,         // 8 bytes
    String,         // Variable length
    ByteArray,      // Array of bytes
    Unknown         // Unknown type
};

enum class ScanType {
    ExactValue,     // Exact value match
    BiggerThan,     // Value bigger than
    SmallerThan,    // Value smaller than
    Between,        // Value between range
    Unknown,        // Unknown value scan
    Changed,        // Value changed
    Unchanged,      // Value unchanged
    Increased,      // Value increased
    Decreased,      // Value decreased
    IncreasedBy,    // Value increased by specific amount
    DecreasedBy     // Value decreased by specific amount
};

enum class ScanAlignment {
    Byte = 1,       // 1 byte alignment
    Word = 2,       // 2 byte alignment
    Dword = 4,      // 4 byte alignment
    Qword = 8,      // 8 byte alignment
    Default = 4     // Default 4 byte alignment
};

struct ScanResult {
    PVOID address;
    std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> value;
    ValueType type;
    SIZE_T regionSize;
    DWORD protection;
    std::chrono::steady_clock::time_point timestamp;
};

struct ScanFilter {
    bool executable = false;
    bool writable = true;
    bool mapped = false;
    bool privateMemory = true;
    DWORD minProtection = PAGE_READONLY;
    SIZE_T minSize = 0;
    SIZE_T maxSize = SIZE_MAX;
    std::vector<PVOID> includeAddresses;
    std::vector<PVOID> excludeAddresses;
};

struct ScanProgress {
    std::atomic<SIZE_T> scannedBytes{0};
    std::atomic<SIZE_T> totalBytes{0};
    std::atomic<SIZE_T> foundResults{0};
    std::atomic<bool> isRunning{false};
    std::atomic<bool> shouldStop{false};
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
};

class BDOMemoryScanner {
private:
    HANDLE hProcess;
    DWORD processId;
    std::vector<ScanResult> currentResults;
    std::vector<ScanResult> previousResults;
    std::map<std::string, std::vector<ScanResult>> savedScans;
    std::mutex scannerMutex;
    ScanProgress progress;
    
    // Thread management
    std::vector<std::thread> scanThreads;
    size_t threadCount;
    
    // Memory regions
    struct MemoryRegion {
        PVOID baseAddress;
        SIZE_T size;
        DWORD protection;
        DWORD state;
        DWORD type;
    };
    std::vector<MemoryRegion> memoryRegions;
    
    // Private scanning methods
    void ScanRegion(const MemoryRegion& region, ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value, ScanType scanType, ScanAlignment alignment);
    void ScanRegionUnknown(const MemoryRegion& region, ValueType type, ScanAlignment alignment);
    void ScanRegionCompare(const MemoryRegion& region, ScanType scanType, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value = {});
    
    // Value comparison
    template<typename T>
    bool CompareValue(T currentValue, T compareValue, ScanType scanType);
    bool CompareBytes(const BYTE* current, const BYTE* compare, SIZE_T size);
    
    // Memory reading
    bool ReadMemoryRegion(PVOID address, std::vector<BYTE>& buffer, SIZE_T size);
    template<typename T>
    bool ReadValue(PVOID address, T& value);
    bool ReadString(PVOID address, std::string& str, SIZE_T maxLength);
    bool ReadByteArray(PVOID address, std::vector<BYTE>& bytes, SIZE_T size);
    
    // Value type detection
    ValueType DetectValueType(const std::string& valueStr);
    ValueType DetectValueType(PVOID address, SIZE_T maxSize = 8);
    SIZE_T GetValueSize(ValueType type);
    
    // Memory region management
    void EnumerateMemoryRegions(const ScanFilter& filter);
    bool IsValidRegion(const MemoryRegion& region, const ScanFilter& filter);
    
    // Result management
    void AddResult(PVOID address, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value, ValueType type, const MemoryRegion& region);
    void FilterResults(const std::function<bool(const ScanResult&)>& predicate);
    void SortResults();
    
    // Multi-threading
    void DistributeWork(const std::vector<MemoryRegion>& regions);
    void WorkerThread(size_t threadId, const std::vector<MemoryRegion>& assignedRegions, ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value, ScanType scanType, ScanAlignment alignment);
    
public:
    BDOMemoryScanner(size_t threadCount = std::thread::hardware_concurrency());
    ~BDOMemoryScanner();
    
    // Initialization
    bool Initialize(DWORD processId);
    bool AttachToProcess(const std::wstring& processName);
    void Shutdown();
    
    // Basic scanning
    bool FirstScan(ValueType type, const std::string& valueStr, ScanType scanType = ScanType::ExactValue, ScanAlignment alignment = ScanAlignment::Default, const ScanFilter& filter = {});
    bool FirstScanUnknown(ValueType type, ScanAlignment alignment = ScanAlignment::Default, const ScanFilter& filter = {});
    bool NextScan(ScanType scanType, const std::string& valueStr = "");
    
    // Advanced scanning
    bool ScanForPattern(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask, const ScanFilter& filter = {});
    bool ScanForString(const std::string& str, bool caseSensitive = true, const ScanFilter& filter = {});
    bool ScanForArray(const std::vector<BYTE>& bytes, const ScanFilter& filter = {});
    bool ScanForPointer(PVOID targetAddress, const ScanFilter& filter = {});
    bool ScanForStructure(SIZE_T structureSize, const std::map<SIZE_T, std::variant<BYTE, WORD, DWORD, QWORD, float, double>>& knownValues, const ScanFilter& filter = {});
    
    // Memory snapshots and comparison
    bool CreateSnapshot(const std::string& name);
    bool CompareWithSnapshot(const std::string& name, ScanType compareType = ScanType::Changed);
    bool DeleteSnapshot(const std::string& name);
    std::vector<std::string> GetSnapshotNames();
    
    // Result management
    std::vector<ScanResult> GetResults();
    std::vector<ScanResult> GetResultsInRange(PVOID startAddress, PVOID endAddress);
    bool UpdateResultValues();
    bool FilterResultsByValue(ValueType type, const std::string& valueStr, ScanType scanType);
    bool FilterResultsByRegion(const ScanFilter& filter);
    bool RemoveResult(PVOID address);
    void ClearResults();
    
    // Value operations
    bool GetValue(PVOID address, ValueType type, std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value);
    bool SetValue(PVOID address, ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value);
    bool FreezeValue(PVOID address, ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value);
    bool UnfreezeValue(PVOID address);
    bool IsValueFrozen(PVOID address);
    
    // Memory structure analysis
    struct StructureInfo {
        SIZE_T size;
        std::map<SIZE_T, ValueType> memberTypes;
        std::map<SIZE_T, std::string> memberNames;
        std::map<SIZE_T, std::variant<BYTE, WORD, DWORD, QWORD, float, double>> memberValues;
    };
    bool AnalyzeStructure(PVOID address, SIZE_T maxSize, StructureInfo& info);
    bool FindPointerPath(PVOID sourceAddress, PVOID targetAddress, std::vector<SIZE_T>& offsets, SIZE_T maxDepth = 5);
    
    // Progress and control
    ScanProgress GetProgress();
    void StopScan();
    bool IsScanRunning();
    double GetScanSpeed(); // Bytes per second
    size_t GetResultCount();
    
    // Export/Import
    bool ExportResults(const std::string& filename);
    bool ImportResults(const std::string& filename);
    bool ExportAddressTable(const std::string& filename);
    
    // Utility
    std::string ValueToString(const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value);
    std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> StringToValue(const std::string& str, ValueType type);
    std::string GetValueTypeString(ValueType type);
    std::string GetScanTypeString(ScanType type);
    
private:
    // Frozen values management
    struct FrozenValue {
        PVOID address;
        ValueType type;
        std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> value;
        std::thread freezeThread;
        std::atomic<bool> shouldFreeze{true};
    };
    std::map<PVOID, std::unique_ptr<FrozenValue>> frozenValues;
    std::mutex frozenMutex;
    
    void FreezeThread(FrozenValue* frozen);
};

// Template implementations
template<typename T>
bool BDOMemoryScanner::CompareValue(T currentValue, T compareValue, ScanType scanType) {
    switch (scanType) {
        case ScanType::ExactValue:
            return currentValue == compareValue;
        case ScanType::BiggerThan:
            return currentValue > compareValue;
        case ScanType::SmallerThan:
            return currentValue < compareValue;
        case ScanType::Between:
            // For between, compareValue should be the lower bound
            // Upper bound should be passed separately
            return false; // Handled elsewhere
        default:
            return false;
    }
}

template<typename T>
bool BDOMemoryScanner::ReadValue(PVOID address, T& value) {
    SIZE_T bytesRead;
    return ReadProcessMemory(hProcess, address, &value, sizeof(T), &bytesRead) && bytesRead == sizeof(T);
}

// Helper class for memory value monitoring
class MemoryValueMonitor {
private:
    BDOMemoryScanner* scanner;
    std::vector<std::pair<PVOID, ValueType>> monitoredAddresses;
    std::map<PVOID, std::vector<std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>>> valueHistory;
    std::thread monitorThread;
    std::atomic<bool> isMonitoring{false};
    std::mutex monitorMutex;
    std::chrono::milliseconds updateInterval{100};
    
    void MonitorLoop();
    
public:
    MemoryValueMonitor(BDOMemoryScanner* scanner);
    ~MemoryValueMonitor();
    
    bool AddAddress(PVOID address, ValueType type);
    bool RemoveAddress(PVOID address);
    void ClearAddresses();
    
    bool StartMonitoring(std::chrono::milliseconds interval = std::chrono::milliseconds(100));
    void StopMonitoring();
    bool IsMonitoring();
    
    std::vector<std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>> GetValueHistory(PVOID address);
    std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> GetCurrentValue(PVOID address);
    std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> GetPreviousValue(PVOID address);
    
    // Analysis
    bool HasValueChanged(PVOID address);
    double GetValueChangeRate(PVOID address); // Changes per second
    std::pair<std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>, 
              std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>> GetMinMaxValues(PVOID address);
};
