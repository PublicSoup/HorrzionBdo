#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <chrono>
#include <json/json.h> // You may need to add nlohmann/json or similar

// Automatic offset updater for game patches
// Detects game version changes and updates memory offsets

namespace BDO {
    namespace OffsetUpdater {

        // Offset information
        struct OffsetInfo {
            std::string name;
            ULONG_PTR baseAddress;
            std::vector<ULONG_PTR> offsets;
            std::string pattern;
            std::string mask;
            int patternOffset;
            std::string gameVersion;
            std::chrono::system_clock::time_point lastUpdated;
            bool isValid;
        };

        // Game version info
        struct GameVersion {
            std::string version;
            std::string buildDate;
            DWORD fileSize;
            DWORD timestamp;
            std::string md5Hash;
            
            bool operator==(const GameVersion& other) const {
                return version == other.version && timestamp == other.timestamp;
            }
            
            bool operator!=(const GameVersion& other) const {
                return !(*this == other);
            }
        };

        class AutoOffsetUpdater {
        private:
            std::map<std::string, OffsetInfo> offsets;
            GameVersion currentVersion;
            GameVersion lastKnownVersion;
            std::string offsetDatabasePath;
            bool autoUpdate;
            
            // Get game version from executable
            GameVersion GetGameVersion(const std::wstring& exePath) {
                GameVersion version;
                
                // Get file size
                WIN32_FILE_ATTRIBUTE_DATA fileInfo;
                if (GetFileAttributesExW(exePath.c_str(), GetFileExInfoStandard, &fileInfo)) {
                    LARGE_INTEGER fileSize;
                    fileSize.HighPart = fileInfo.nFileSizeHigh;
                    fileSize.LowPart = fileInfo.nFileSizeLow;
                    version.fileSize = static_cast<DWORD>(fileSize.QuadPart);
                }
                
                // Get version info
                DWORD handle = 0;
                DWORD size = GetFileVersionInfoSizeW(exePath.c_str(), &handle);
                if (size > 0) {
                    std::vector<BYTE> buffer(size);
                    if (GetFileVersionInfoW(exePath.c_str(), handle, size, buffer.data())) {
                        VS_FIXEDFILEINFO* fileInfo = nullptr;
                        UINT len = 0;
                        if (VerQueryValueW(buffer.data(), L"\\", (LPVOID*)&fileInfo, &len)) {
                            std::stringstream ss;
                            ss << HIWORD(fileInfo->dwFileVersionMS) << "."
                               << LOWORD(fileInfo->dwFileVersionMS) << "."
                               << HIWORD(fileInfo->dwFileVersionLS) << "."
                               << LOWORD(fileInfo->dwFileVersionLS);
                            version.version = ss.str();
                            version.timestamp = fileInfo->dwFileDate;
                        }
                    }
                }
                
                return version;
            }
            
            // Scan for pattern and update offset
            bool ScanAndUpdateOffset(HANDLE hProcess, OffsetInfo& offset) {
                // This is a simplified version - you'd want to integrate with BDO_MemoryResolver
                // for actual pattern scanning
                
                // For now, just mark as needing manual update
                offset.isValid = false;
                return false;
            }
            
        public:
            AutoOffsetUpdater(const std::string& databasePath = "offsets.db", bool autoUpdateEnabled = true)
                : offsetDatabasePath(databasePath)
                , autoUpdate(autoUpdateEnabled)
            {}
            
            // Register an offset with its pattern
            void RegisterOffset(const std::string& name, const std::string& pattern, const std::string& mask, int patternOffset = 0) {
                OffsetInfo info;
                info.name = name;
                info.pattern = pattern;
                info.mask = mask;
                info.patternOffset = patternOffset;
                info.isValid = false;
                info.lastUpdated = std::chrono::system_clock::now();
                
                offsets[name] = info;
            }
            
            // Check if game version changed
            bool HasGameVersionChanged(const std::wstring& exePath) {
                currentVersion = GetGameVersion(exePath);
                
                if (lastKnownVersion.version.empty()) {
                    lastKnownVersion = currentVersion;
                    return false;
                }
                
                return currentVersion != lastKnownVersion;
            }
            
            // Update all offsets
            bool UpdateAllOffsets(HANDLE hProcess) {
                bool allSuccess = true;
                
                for (auto& [name, offset] : offsets) {
                    if (!ScanAndUpdateOffset(hProcess, offset)) {
                        allSuccess = false;
                    }
                }
                
                return allSuccess;
            }
            
            // Get offset by name
            OffsetInfo* GetOffset(const std::string& name) {
                auto it = offsets.find(name);
                if (it != offsets.end()) {
                    return &it->second;
                }
                return nullptr;
            }
            
            // Save offsets to database
            bool SaveDatabase() {
                std::ofstream file(offsetDatabasePath);
                if (!file.is_open()) return false;
                
                // Write version info
                file << "# BDO Offset Database\n";
                file << "# Game Version: " << currentVersion.version << "\n";
                file << "# Last Updated: " << std::time(nullptr) << "\n\n";
                
                // Write offsets
                for (const auto& [name, offset] : offsets) {
                    file << "[" << name << "]\n";
                    file << "BaseAddress=" << std::hex << offset.baseAddress << "\n";
                    file << "Pattern=" << offset.pattern << "\n";
                    file << "Mask=" << offset.mask << "\n";
                    file << "PatternOffset=" << std::dec << offset.patternOffset << "\n";
                    file << "Valid=" << (offset.isValid ? "1" : "0") << "\n\n";
                }
                
                file.close();
                return true;
            }
            
            // Load offsets from database
            bool LoadDatabase() {
                std::ifstream file(offsetDatabasePath);
                if (!file.is_open()) return false;
                
                std::string line;
                OffsetInfo* currentOffset = nullptr;
                
                while (std::getline(file, line)) {
                    // Skip comments and empty lines
                    if (line.empty() || line[0] == '#') continue;
                    
                    // Check for section header
                    if (line[0] == '[') {
                        std::string name = line.substr(1, line.find(']') - 1);
                        currentOffset = &offsets[name];
                        currentOffset->name = name;
                        continue;
                    }
                    
                    // Parse key=value pairs
                    if (currentOffset) {
                        size_t pos = line.find('=');
                        if (pos != std::string::npos) {
                            std::string key = line.substr(0, pos);
                            std::string value = line.substr(pos + 1);
                            
                            if (key == "BaseAddress") {
                                currentOffset->baseAddress = std::stoull(value, nullptr, 16);
                            } else if (key == "Pattern") {
                                currentOffset->pattern = value;
                            } else if (key == "Mask") {
                                currentOffset->mask = value;
                            } else if (key == "PatternOffset") {
                                currentOffset->patternOffset = std::stoi(value);
                            } else if (key == "Valid") {
                                currentOffset->isValid = (value == "1");
                            }
                        }
                    }
                }
                
                file.close();
                return true;
            }
            
            // Print offset status
            void PrintStatus() {
                std::cout << "\n=== Offset Database Status ===" << std::endl;
                std::cout << "Game Version: " << currentVersion.version << std::endl;
                std::cout << "Total Offsets: " << offsets.size() << std::endl;
                std::cout << "\nOffset Status:" << std::endl;
                std::cout << "------------------------------------------------------------" << std::endl;
                
                int validCount = 0;
                for (const auto& [name, offset] : offsets) {
                    std::cout << "  " << (offset.isValid ? "✓" : "✗") << " "
                             << std::setw(30) << std::left << name
                             << " 0x" << std::hex << offset.baseAddress << std::dec;
                    
                    if (!offset.isValid) {
                        std::cout << " (Needs Update)";
                    }
                    std::cout << std::endl;
                    
                    if (offset.isValid) validCount++;
                }
                
                std::cout << "\nValid: " << validCount << "/" << offsets.size() << std::endl;
            }
            
            // Export offsets to C++ header file
            bool ExportToCppHeader(const std::string& filename, const std::string& namespaceName = "BDO::Offsets") {
                std::ofstream file(filename);
                if (!file.is_open()) return false;
                
                file << "// Auto-generated offset header\n";
                file << "// Game Version: " << currentVersion.version << "\n";
                file << "// Generated: " << std::time(nullptr) << "\n\n";
                
                file << "#pragma once\n";
                file << "#include <windows.h>\n\n";
                
                file << "namespace " << namespaceName << " {\n\n";
                
                for (const auto& [name, offset] : offsets) {
                    file << "    // " << name << "\n";
                    file << "    constexpr ULONG_PTR " << name << " = 0x" 
                         << std::hex << offset.baseAddress << std::dec << ";\n\n";
                }
                
                file << "} // namespace " << namespaceName << "\n";
                
                file.close();
                return true;
            }
        };

    } // namespace OffsetUpdater
} // namespace BDO

