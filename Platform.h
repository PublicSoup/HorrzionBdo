#ifndef PLATFORM_H
#define PLATFORM_H

// Platform detection and compatibility layer
#ifdef _WIN32
    #include <windows.h>
#else
    // Linux/Unix compatibility definitions for Windows types
    #include <cstdint>
    #include <unistd.h>
    
    // Windows type definitions for Linux
    typedef void* HANDLE;
    typedef void* HWND;
    typedef unsigned short WORD;
    typedef short SHORT;
    typedef unsigned long DWORD;
    typedef long LONG;
    typedef void* LPVOID;
    typedef const void* LPCVOID;
    typedef long LPARAM;
    typedef uint64_t WPARAM;
    typedef size_t SIZE_T;
    
    // Virtual key codes
    #define VK_LBUTTON 0x01
    #define VK_RBUTTON 0x02
    #define VK_SHIFT   0x10
    #define VK_SPACE   0x20
    #define VK_ESCAPE  0x1B
    #define VK_BACK    0x08
    #define VK_TAB     0x09
    #define VK_MENU    0x12  // Alt key
    #define VK_NUMPAD1 0x61
    
    // Key event flags
    #define KEYEVENTF_KEYUP 0x0002
    
    // Window messages
    #define WM_KEYDOWN     0x0100
    #define WM_KEYUP       0x0101
    #define WM_LBUTTONDOWN 0x0201
    #define WM_LBUTTONUP   0x0202
    #define WM_MOUSEMOVE   0x0200
    #define MK_LBUTTON     0x0001
    
    // Process access rights
    #define PROCESS_VM_READ 0x0010
    #define PROCESS_QUERY_INFORMATION 0x0400
    
    // Structures
    struct POINT {
        LONG x;
        LONG y;
    };
    
    struct INPUT {
        DWORD type;
        union {
            struct {
                WORD wVk;
                WORD wScan;
                DWORD dwFlags;
            } ki;
        };
    };
    
    #define INPUT_KEYBOARD 1
    
    // Mock Windows API functions for Linux (simulation mode)
    inline bool GetCursorPos(POINT* point) {
        if (point) {
            point->x = 500;  // Default position
            point->y = 500;
        }
        return true;
    }
    
    inline bool SetCursorPos(int x, int y) {
        // Simulated - no actual mouse movement on Linux
        (void)x;
        (void)y;
        return true;
    }
    
    inline HWND FindWindowA(const char* className, const char* windowName) {
        // Simulated - always returns nullptr on Linux
        (void)className;
        (void)windowName;
        return nullptr;
    }
    
    inline bool SetForegroundWindow(HWND hWnd) {
        (void)hWnd;
        return false;
    }
    
    inline DWORD GetWindowThreadProcessId(HWND hWnd, DWORD* processId) {
        (void)hWnd;
        if (processId) *processId = 0;
        return 0;
    }
    
    inline HANDLE OpenProcess(DWORD access, bool inherit, DWORD processId) {
        (void)access;
        (void)inherit;
        (void)processId;
        return nullptr;
    }
    
    inline void CloseHandle(HANDLE handle) {
        (void)handle;
    }
    
    inline bool ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead) {
        (void)hProcess;
        (void)lpBaseAddress;
        (void)lpBuffer;
        (void)nSize;
        (void)lpNumberOfBytesRead;
        return false;
    }
    
    inline bool PostMessage(HWND hWnd, DWORD msg, WPARAM wParam, LPARAM lParam) {
        (void)hWnd;
        (void)msg;
        (void)wParam;
        (void)lParam;
        return false;
    }
    
    inline DWORD SendInput(DWORD nInputs, INPUT* pInputs, int cbSize) {
        (void)nInputs;
        (void)pInputs;
        (void)cbSize;
        return 0;
    }
    
    inline SHORT VkKeyScan(char ch) {
        // Simple mapping for demo
        if (ch >= 'a' && ch <= 'z') return ch - 'a' + 0x41;
        if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 0x41;
        if (ch >= '0' && ch <= '9') return ch - '0' + 0x30;
        return -1;
    }
    
    inline WORD LOBYTE(SHORT value) { return value & 0xFF; }
    inline WORD HIBYTE(SHORT value) { return (value >> 8) & 0xFF; }
    inline LPARAM MAKELPARAM(WORD low, WORD high) { return (high << 16) | low; }
    
    #define FALSE 0
    #define TRUE 1
#endif

#endif // PLATFORM_H