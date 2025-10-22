#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Automatic pattern generation from known memory values
// Useful for creating signatures that survive game updates

namespace BDO {
    namespace PatternGen {

        // Pattern generation options
        struct PatternOptions {
            bool useWildcards;          // Use wildcards for variable bytes
            int contextBefore;          // Bytes of context before the value
            int contextAfter;           // Bytes of context after the value
            bool includeRelativeOffsets; // Include relative jump/call offsets
            int minPatternLength;       // Minimum pattern length
            int maxPatternLength;       // Maximum pattern length
            
            PatternOptions()
                : useWildcards(true)
                , contextBefore(8)
                , contextAfter(8)
                , includeRelativeOffsets(true)
                , minPatternLength(8)
                , maxPatternLength(32)
            {}
        };

        // Generated pattern result
        struct GeneratedPattern {
            std::string pattern;        // Pattern bytes (e.g., "\x48\x8B\x05")
            std::string mask;           // Mask string (e.g., "xxx????xxx")
            std::string idaPattern;     // IDA-style pattern (e.g., "48 8B 05 ? ? ? ?")
            int offsetToValue;          // Offset from pattern start to the value
            std::string description;    // Human-readable description
            float uniqueness;           // Estimated uniqueness (0.0 - 1.0)
        };

        class PatternGenerator {
        private:
            PatternOptions options;
            
            // Check if byte is likely to be variable (part of an address/offset)
            bool IsLikelyVariable(const std::vector<BYTE>& memory, size_t index) {
                if (index + 3 >= memory.size()) return false;
                
                // Check if it looks like a pointer (high bytes are similar)
                DWORD value = *reinterpret_cast<const DWORD*>(&memory[index]);
                
                // Typical x64 address ranges
                if ((value & 0xFFFF0000) == 0x7FF00000 || // User-mode address
                    (value & 0xFFFF0000) == 0x14000000 || // Image base
                    (value & 0xFF000000) == 0x00000000) { // Low addresses (offsets)
                    return true;
                }
                
                return false;
            }
            
            // Convert byte array to IDA-style pattern
            std::string ToIdaPattern(const std::vector<BYTE>& bytes, const std::string& mask) {
                std::stringstream ss;
                for (size_t i = 0; i < bytes.size(); i++) {
                    if (i > 0) ss << " ";
                    
                    if (mask[i] == '?') {
                        ss << "?";
                    } else {
                        ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)bytes[i];
                    }
                }
                return ss.str();
            }
            
            // Estimate pattern uniqueness
            float EstimateUniqueness(const std::vector<BYTE>& pattern, const std::string& mask) {
                int fixedBytes = 0;
                int totalBytes = mask.length();
                
                for (char c : mask) {
                    if (c == 'x') fixedBytes++;
                }
                
                // More fixed bytes = more unique
                float ratio = (float)fixedBytes / totalBytes;
                
                // Bonus for longer patterns
                float lengthBonus = std::min(1.0f, totalBytes / 16.0f);
                
                return (ratio * 0.7f) + (lengthBonus * 0.3f);
            }
            
        public:
            PatternGenerator(const PatternOptions& opts = PatternOptions())
                : options(opts)
            {}
            
            // Generate pattern from memory region containing a known value
            GeneratedPattern GenerateFromMemory(HANDLE hProcess, PVOID address, SIZE_T valueSize) {
                GeneratedPattern result;
                
                // Calculate total size to read
                SIZE_T totalSize = options.contextBefore + valueSize + options.contextAfter;
                std::vector<BYTE> memory(totalSize);
                
                // Calculate start address
                PVOID startAddr = reinterpret_cast<PVOID>(
                    reinterpret_cast<ULONG_PTR>(address) - options.contextBefore
                );
                
                // Read memory
                SIZE_T bytesRead = 0;
                if (!ReadProcessMemory(hProcess, startAddr, memory.data(), totalSize, &bytesRead)) {
                    result.description = "Failed to read memory";
                    return result;
                }
                
                // Build pattern and mask
                std::vector<BYTE> patternBytes;
                std::string mask;
                
                for (size_t i = 0; i < memory.size(); i++) {
                    patternBytes.push_back(memory[i]);
                    
                    // Determine if this byte should be wildcarded
                    if (options.useWildcards && IsLikelyVariable(memory, i)) {
                        mask += "????"; // 4-byte wildcard for addresses
                        i += 3; // Skip next 3 bytes
                        patternBytes.push_back(memory[i - 2]);
                        patternBytes.push_back(memory[i - 1]);
                        patternBytes.push_back(memory[i]);
                    } else {
                        mask += "x";
                    }
                }
                
                // Convert to string pattern
                result.pattern = std::string(reinterpret_cast<const char*>(patternBytes.data()), patternBytes.size());
                result.mask = mask;
                result.idaPattern = ToIdaPattern(patternBytes, mask);
                result.offsetToValue = options.contextBefore;
                result.uniqueness = EstimateUniqueness(patternBytes, mask);
                result.description = "Auto-generated pattern";
                
                return result;
            }
            
            // Generate pattern from known instruction sequence
            GeneratedPattern GenerateFromInstructions(const std::vector<BYTE>& instructions) {
                GeneratedPattern result;
                
                std::string mask;
                for (size_t i = 0; i < instructions.size(); i++) {
                    BYTE byte = instructions[i];
                    
                    // Check for common x64 instruction patterns
                    if (i + 4 < instructions.size()) {
                        // MOV reg, [rip+offset] - 48 8B 05/0D/15/1D/25/2D/35/3D
                        if (byte == 0x48 && instructions[i + 1] == 0x8B) {
                            mask += "xx????"; // Keep opcode, wildcard offset
                            i += 5;
                            continue;
                        }
                        
                        // LEA reg, [rip+offset] - 48 8D 05/0D/15/1D/25/2D/35/3D
                        if (byte == 0x48 && instructions[i + 1] == 0x8D) {
                            mask += "xx????";
                            i += 5;
                            continue;
                        }
                        
                        // CALL relative - E8 ?? ?? ?? ??
                        if (byte == 0xE8) {
                            mask += "x????";
                            i += 4;
                            continue;
                        }
                        
                        // JMP relative - E9 ?? ?? ?? ??
                        if (byte == 0xE9) {
                            mask += "x????";
                            i += 4;
                            continue;
                        }
                    }
                    
                    // Default: keep byte as-is
                    mask += "x";
                }
                
                result.pattern = std::string(reinterpret_cast<const char*>(instructions.data()), instructions.size());
                result.mask = mask;
                result.idaPattern = ToIdaPattern(instructions, mask);
                result.offsetToValue = 0;
                result.uniqueness = EstimateUniqueness(instructions, mask);
                result.description = "Instruction-based pattern";
                
                return result;
            }
            
            // Generate multiple patterns with different options
            std::vector<GeneratedPattern> GenerateMultiple(HANDLE hProcess, PVOID address, SIZE_T valueSize) {
                std::vector<GeneratedPattern> patterns;
                
                // Try different context sizes
                std::vector<int> contextSizes = { 4, 8, 12, 16 };
                
                for (int contextSize : contextSizes) {
                    PatternOptions opts = options;
                    opts.contextBefore = contextSize;
                    opts.contextAfter = contextSize;
                    
                    PatternGenerator gen(opts);
                    auto pattern = gen.GenerateFromMemory(hProcess, address, valueSize);
                    
                    if (!pattern.pattern.empty()) {
                        pattern.description = "Context: " + std::to_string(contextSize) + " bytes";
                        patterns.push_back(pattern);
                    }
                }
                
                // Sort by uniqueness
                std::sort(patterns.begin(), patterns.end(), [](const GeneratedPattern& a, const GeneratedPattern& b) {
                    return a.uniqueness > b.uniqueness;
                });
                
                return patterns;
            }
            
            // Save pattern to file (C++ code format)
            static bool SaveToCppFile(const GeneratedPattern& pattern, const std::string& filename, const std::string& patternName) {
                std::ofstream file(filename);
                if (!file.is_open()) return false;
                
                file << "// Auto-generated pattern for " << patternName << "\n";
                file << "// " << pattern.description << "\n";
                file << "// Uniqueness: " << std::fixed << std::setprecision(2) << (pattern.uniqueness * 100) << "%\n\n";
                
                file << "const char* " << patternName << "_pattern = \"";
                for (size_t i = 0; i < pattern.pattern.length(); i++) {
                    file << "\\x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
                         << (int)(unsigned char)pattern.pattern[i];
                }
                file << "\";\n\n";
                
                file << "const char* " << patternName << "_mask = \"" << pattern.mask << "\";\n\n";
                
                file << "// IDA Pattern: " << pattern.idaPattern << "\n";
                file << "// Offset to value: " << pattern.offsetToValue << "\n";
                
                file.close();
                return true;
            }
            
            // Print pattern in various formats
            static void PrintPattern(const GeneratedPattern& pattern, const std::string& name = "Pattern") {
                std::cout << "\n=== " << name << " ===" << std::endl;
                std::cout << "Description: " << pattern.description << std::endl;
                std::cout << "Uniqueness:  " << std::fixed << std::setprecision(1) << (pattern.uniqueness * 100) << "%" << std::endl;
                std::cout << "Offset:      " << pattern.offsetToValue << std::endl;
                std::cout << "\nC++ Format:" << std::endl;
                std::cout << "  Pattern: \"";
                for (size_t i = 0; i < pattern.pattern.length(); i++) {
                    std::cout << "\\x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
                             << (int)(unsigned char)pattern.pattern[i];
                }
                std::cout << "\"" << std::endl;
                std::cout << "  Mask:    \"" << pattern.mask << "\"" << std::endl;
                std::cout << "\nIDA Format:" << std::endl;
                std::cout << "  " << pattern.idaPattern << std::endl;
                std::cout << std::endl;
            }
        };

    } // namespace PatternGen
} // namespace BDO

