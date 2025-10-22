#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

// Memory encryption/decryption support for secure value storage
// Useful for protecting bot data from detection and reverse engineering

namespace BDO {
    namespace Crypto {

        // Simple XOR encryption (fast, but not cryptographically secure)
        class XORCipher {
        private:
            std::vector<BYTE> key;
            
        public:
            XORCipher(const std::string& keyString) {
                key.assign(keyString.begin(), keyString.end());
                if (key.empty()) {
                    key.push_back(0x42); // Default key
                }
            }
            
            void Encrypt(void* data, SIZE_T size) {
                BYTE* bytes = static_cast<BYTE*>(data);
                for (SIZE_T i = 0; i < size; i++) {
                    bytes[i] ^= key[i % key.size()];
                }
            }
            
            void Decrypt(void* data, SIZE_T size) {
                // XOR is symmetric
                Encrypt(data, size);
            }
        };

        // ROT cipher (simple rotation)
        class ROTCipher {
        private:
            BYTE rotation;
            
        public:
            ROTCipher(BYTE rot = 13) : rotation(rot) {}
            
            void Encrypt(void* data, SIZE_T size) {
                BYTE* bytes = static_cast<BYTE*>(data);
                for (SIZE_T i = 0; i < size; i++) {
                    bytes[i] = (bytes[i] + rotation) & 0xFF;
                }
            }
            
            void Decrypt(void* data, SIZE_T size) {
                BYTE* bytes = static_cast<BYTE*>(data);
                for (SIZE_T i = 0; i < size; i++) {
                    bytes[i] = (bytes[i] - rotation) & 0xFF;
                }
            }
        };

        // Multi-layer encryption (XOR + ROT + XOR)
        class MultiLayerCipher {
        private:
            XORCipher xor1;
            ROTCipher rot;
            XORCipher xor2;
            
        public:
            MultiLayerCipher(const std::string& key1, BYTE rotation, const std::string& key2)
                : xor1(key1), rot(rotation), xor2(key2)
            {}
            
            void Encrypt(void* data, SIZE_T size) {
                xor1.Encrypt(data, size);
                rot.Encrypt(data, size);
                xor2.Encrypt(data, size);
            }
            
            void Decrypt(void* data, SIZE_T size) {
                xor2.Decrypt(data, size);
                rot.Decrypt(data, size);
                xor1.Decrypt(data, size);
            }
        };

        // Encrypted value wrapper
        template<typename T>
        class EncryptedValue {
        private:
            std::vector<BYTE> encryptedData;
            XORCipher cipher;
            
        public:
            EncryptedValue(const std::string& key = "BDO_SECRET_KEY_2024")
                : cipher(key)
            {
                encryptedData.resize(sizeof(T));
            }
            
            // Set value (encrypts automatically)
            void Set(const T& value) {
                std::memcpy(encryptedData.data(), &value, sizeof(T));
                cipher.Encrypt(encryptedData.data(), sizeof(T));
            }
            
            // Get value (decrypts automatically)
            T Get() const {
                std::vector<BYTE> temp = encryptedData;
                cipher.Decrypt(temp.data(), sizeof(T));
                
                T value;
                std::memcpy(&value, temp.data(), sizeof(T));
                return value;
            }
            
            // Operators for convenience
            EncryptedValue& operator=(const T& value) {
                Set(value);
                return *this;
            }
            
            operator T() const {
                return Get();
            }
        };

        // Memory obfuscation helper
        class MemoryObfuscator {
        private:
            static BYTE GenerateRandomByte() {
                return static_cast<BYTE>(rand() % 256);
            }
            
        public:
            // Obfuscate memory region (fills with random data)
            static void Obfuscate(void* data, SIZE_T size) {
                BYTE* bytes = static_cast<BYTE*>(data);
                for (SIZE_T i = 0; i < size; i++) {
                    bytes[i] = GenerateRandomByte();
                }
            }
            
            // Secure zero (prevents compiler optimization)
            static void SecureZero(void* data, SIZE_T size) {
                volatile BYTE* bytes = static_cast<volatile BYTE*>(data);
                for (SIZE_T i = 0; i < size; i++) {
                    bytes[i] = 0;
                }
            }
            
            // XOR with random pattern
            static void RandomXOR(void* data, SIZE_T size) {
                BYTE* bytes = static_cast<BYTE*>(data);
                for (SIZE_T i = 0; i < size; i++) {
                    bytes[i] ^= GenerateRandomByte();
                }
            }
        };

        // Encrypted string (for storing sensitive strings)
        class EncryptedString {
        private:
            std::vector<BYTE> encryptedData;
            XORCipher cipher;
            
        public:
            EncryptedString(const std::string& key = "BDO_STRING_KEY")
                : cipher(key)
            {}
            
            void Set(const std::string& str) {
                encryptedData.assign(str.begin(), str.end());
                cipher.Encrypt(encryptedData.data(), encryptedData.size());
            }
            
            std::string Get() const {
                std::vector<BYTE> temp = encryptedData;
                cipher.Decrypt(temp.data(), temp.size());
                return std::string(temp.begin(), temp.end());
            }
            
            EncryptedString& operator=(const std::string& str) {
                Set(str);
                return *this;
            }
            
            operator std::string() const {
                return Get();
            }
        };

        // Checksum calculator (for integrity verification)
        class Checksum {
        public:
            // Simple CRC32
            static DWORD CRC32(const void* data, SIZE_T size) {
                const BYTE* bytes = static_cast<const BYTE*>(data);
                DWORD crc = 0xFFFFFFFF;
                
                for (SIZE_T i = 0; i < size; i++) {
                    crc ^= bytes[i];
                    for (int j = 0; j < 8; j++) {
                        crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
                    }
                }
                
                return ~crc;
            }
            
            // Simple sum
            static DWORD SimpleSum(const void* data, SIZE_T size) {
                const BYTE* bytes = static_cast<const BYTE*>(data);
                DWORD sum = 0;
                
                for (SIZE_T i = 0; i < size; i++) {
                    sum += bytes[i];
                }
                
                return sum;
            }
            
            // XOR checksum
            static BYTE XORChecksum(const void* data, SIZE_T size) {
                const BYTE* bytes = static_cast<const BYTE*>(data);
                BYTE checksum = 0;
                
                for (SIZE_T i = 0; i < size; i++) {
                    checksum ^= bytes[i];
                }
                
                return checksum;
            }
        };

        // Encrypted memory buffer
        class EncryptedBuffer {
        private:
            std::vector<BYTE> buffer;
            MultiLayerCipher cipher;
            DWORD checksum;
            
        public:
            EncryptedBuffer(const std::string& key1 = "KEY1", BYTE rot = 13, const std::string& key2 = "KEY2")
                : cipher(key1, rot, key2)
                , checksum(0)
            {}
            
            // Write data (encrypts and calculates checksum)
            bool Write(const void* data, SIZE_T size) {
                buffer.resize(size);
                std::memcpy(buffer.data(), data, size);
                
                checksum = Checksum::CRC32(data, size);
                cipher.Encrypt(buffer.data(), size);
                
                return true;
            }
            
            // Read data (decrypts and verifies checksum)
            bool Read(void* data, SIZE_T size) {
                if (buffer.size() != size) {
                    return false;
                }
                
                std::vector<BYTE> temp = buffer;
                cipher.Decrypt(temp.data(), size);
                
                // Verify checksum
                DWORD calculatedChecksum = Checksum::CRC32(temp.data(), size);
                if (calculatedChecksum != checksum) {
                    return false; // Data corrupted or tampered
                }
                
                std::memcpy(data, temp.data(), size);
                return true;
            }
            
            SIZE_T GetSize() const {
                return buffer.size();
            }
            
            bool IsValid() const {
                return !buffer.empty();
            }
        };

    } // namespace Crypto
} // namespace BDO

