#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <chrono>
#include <random>

// Define D3DCOLOR if not already defined
#ifndef D3DCOLOR
typedef DWORD D3DCOLOR;
#endif

// Define D3DCOLOR_ARGB macro if not already defined
#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

// Stealth GUI System - Disguised as Windows System Monitor
// This GUI looks like a legitimate Windows system monitoring tool

// Disguised window class and title
#define STEALTH_WINDOW_CLASS L"WinSysMonitor"
#define STEALTH_WINDOW_TITLE L"Windows System Monitor"
#define STEALTH_ICON_ID 101

// GUI Themes
enum class GUITheme {
    Windows10,
    Windows11,
    Dark,
    Light,
    Custom
};

// GUI Elements
enum class GUIElement {
    Window,
    Button,
    Checkbox,
    Slider,
    Text,
    ProgressBar,
    Tab,
    GroupBox,
    ListBox,
    ComboBox
};

// GUI State
struct GUIState {
    bool isVisible;
    bool isMinimized;
    bool isMaximized;
    bool isFocused;
    bool isDragging;
    bool isResizing;
    POINT dragStart;
    POINT resizeStart;
    RECT windowRect;
    RECT clientRect;
    DWORD windowStyle;
    DWORD windowExStyle;
};

// GUI Colors (Windows 10/11 style)
struct GUIColors {
    D3DCOLOR background;
    D3DCOLOR foreground;
    D3DCOLOR accent;
    D3DCOLOR border;
    D3DCOLOR text;
    D3DCOLOR button;
    D3DCOLOR buttonHover;
    D3DCOLOR buttonPressed;
    D3DCOLOR checkbox;
    D3DCOLOR slider;
    D3DCOLOR progressBar;
    D3DCOLOR tab;
    D3DCOLOR groupBox;
    D3DCOLOR listBox;
    D3DCOLOR comboBox;
};

// GUI Font
struct GUIFont {
    ID3D11ShaderResourceView* texture;
    int width;
    int height;
    int charWidth;
    int charHeight;
    int charSpacing;
    int lineSpacing;
};

// GUI Button
struct GUIButton {
    std::string id;
    std::string text;
    RECT rect;
    bool isHovered;
    bool isPressed;
    bool isEnabled;
    std::function<void()> onClick;
    GUIColors colors;
};

// GUI Checkbox
struct GUICheckbox {
    std::string id;
    std::string text;
    RECT rect;
    bool isChecked;
    bool isHovered;
    bool isEnabled;
    std::function<void(bool)> onChange;
    GUIColors colors;
};

// GUI Slider
struct GUISlider {
    std::string id;
    std::string text;
    RECT rect;
    float value;
    float minValue;
    float maxValue;
    bool isDragging;
    bool isEnabled;
    std::function<void(float)> onChange;
    GUIColors colors;
};

// GUI Text
struct GUIText {
    std::string id;
    std::string text;
    RECT rect;
    D3DCOLOR color;
    int fontSize;
    bool isCentered;
    bool isVisible;
};

// GUI Progress Bar
struct GUIProgressBar {
    std::string id;
    std::string text;
    RECT rect;
    float progress;
    float minValue;
    float maxValue;
    bool isVisible;
    GUIColors colors;
};

// GUI Tab
struct GUITab {
    std::string id;
    std::string text;
    RECT rect;
    bool isSelected;
    bool isHovered;
    bool isEnabled;
    std::function<void()> onSelect;
    GUIColors colors;
};

// GUI Group Box
struct GUIGroupBox {
    std::string id;
    std::string text;
    RECT rect;
    std::vector<std::string> children;
    bool isVisible;
    GUIColors colors;
};

// GUI List Box
struct GUIListBox {
    std::string id;
    std::string text;
    RECT rect;
    std::vector<std::string> items;
    int selectedIndex;
    int scrollOffset;
    bool isHovered;
    bool isEnabled;
    std::function<void(int)> onSelect;
    GUIColors colors;
};

// GUI Combo Box
struct GUIComboBox {
    std::string id;
    std::string text;
    RECT rect;
    std::vector<std::string> items;
    int selectedIndex;
    bool isOpen;
    bool isHovered;
    bool isEnabled;
    std::function<void(int)> onChange;
    GUIColors colors;
};

// Main GUI Class
class BDOStealthGUI {
private:
    // DirectX 11 objects
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11SamplerState* samplerState;
    ID3D11BlendState* blendState;
    ID3D11RasterizerState* rasterizerState;
    ID3D11DepthStencilState* depthStencilState;

    // Window objects
    HWND hWnd;
    WNDCLASSEXW wndClass;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;

    // GUI state
    GUIState state;
    GUITheme theme;
    GUIColors colors;
    GUIFont font;

    // GUI elements
    std::vector<GUIButton> buttons;
    std::vector<GUICheckbox> checkboxes;
    std::vector<GUISlider> sliders;
    std::vector<GUIText> texts;
    std::vector<GUIProgressBar> progressBars;
    std::vector<GUITab> tabs;
    std::vector<GUIGroupBox> groupBoxes;
    std::vector<GUIListBox> listBoxes;
    std::vector<GUIComboBox> comboBoxes;

    // Input state
    POINT mousePos;
    bool mouseLeftDown;
    bool mouseRightDown;
    bool mouseMiddleDown;
    std::vector<BYTE> keyStates;

    // Rendering
    bool isRendering;
    std::chrono::steady_clock::time_point lastFrameTime;
    float frameRate;
    int frameCount;

    // Anti-detection
    bool stealthMode;
    bool obfuscationEnabled;
    std::mt19937 rng;
    std::chrono::steady_clock::time_point lastOperation;

    // Private methods
    bool InitializeDirectX();
    bool InitializeWindow();
    bool InitializeGUI();
    bool InitializeFont();
    void CleanupDirectX();
    void CleanupWindow();
    void CleanupGUI();

    // Rendering methods
    void BeginFrame();
    void EndFrame();
    void RenderGUI();
    void RenderButton(const GUIButton& button);
    void RenderCheckbox(const GUICheckbox& checkbox);
    void RenderSlider(const GUISlider& slider);
    void RenderText(const GUIText& text);
    void RenderProgressBar(const GUIProgressBar& progressBar);
    void RenderTab(const GUITab& tab);
    void RenderGroupBox(const GUIGroupBox& groupBox);
    void RenderListBox(const GUIListBox& listBox);
    void RenderComboBox(const GUIComboBox& comboBox);

    // Input handling
    void HandleMouseInput(UINT message, WPARAM wParam, LPARAM lParam);
    void HandleKeyboardInput(UINT message, WPARAM wParam, LPARAM lParam);
    void HandleWindowInput(UINT message, WPARAM wParam, LPARAM lParam);

    // GUI interaction
    void UpdateMouseState();
    void UpdateKeyboardState();
    void ProcessMouseClick(POINT pos, bool leftClick, bool rightClick);
    void ProcessMouseMove(POINT pos);
    void ProcessMouseWheel(int delta);
    void ProcessKeyPress(BYTE key, bool isDown);

    // GUI element management
    GUIButton* FindButton(const std::string& id);
    GUICheckbox* FindCheckbox(const std::string& id);
    GUISlider* FindSlider(const std::string& id);
    GUIText* FindText(const std::string& id);
    GUIProgressBar* FindProgressBar(const std::string& id);
    GUITab* FindTab(const std::string& id);
    GUIGroupBox* FindGroupBox(const std::string& id);
    GUIListBox* FindListBox(const std::string& id);
    GUIComboBox* FindComboBox(const std::string& id);

    // Anti-detection methods
    void AddStealthDelay();
    void ObfuscateOperation();
    bool IsOperationSafe();
    void LogStealthOperation(const std::string& operation);

    // Utility methods
    RECT CalculateRect(int x, int y, int width, int height);
    bool IsPointInRect(POINT point, RECT rect);
    D3DCOLOR BlendColors(D3DCOLOR color1, D3DCOLOR color2, float factor);
    void DrawRect(RECT rect, D3DCOLOR color);
    void DrawText(const std::string& text, RECT rect, D3DCOLOR color, bool centered = false);
    void DrawBorder(RECT rect, D3DCOLOR color, int thickness = 1);

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    BDOStealthGUI();
    ~BDOStealthGUI();

    // Initialization
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Shutdown();

    // Main loop
    void Run();
    bool ProcessMessages();
    void Render();

    // GUI element creation
    void AddButton(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void()> onClick);
    void AddCheckbox(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void(bool)> onChange);
    void AddSlider(const std::string& id, const std::string& text, int x, int y, int width, int height, float minValue, float maxValue, std::function<void(float)> onChange);
    void AddText(const std::string& id, const std::string& text, int x, int y, int width, int height, D3DCOLOR color = 0xFFFFFFFF);
    void AddProgressBar(const std::string& id, const std::string& text, int x, int y, int width, int height);
    void AddTab(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void()> onSelect);
    void AddGroupBox(const std::string& id, const std::string& text, int x, int y, int width, int height);
    void AddListBox(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void(int)> onSelect);
    void AddComboBox(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void(int)> onChange);

    // GUI element modification
    void SetButtonText(const std::string& id, const std::string& text);
    void SetButtonEnabled(const std::string& id, bool enabled);
    void SetCheckboxChecked(const std::string& id, bool checked);
    void SetSliderValue(const std::string& id, float value);
    void SetText(const std::string& id, const std::string& text);
    void SetProgressBarProgress(const std::string& id, float progress);
    void SetTabSelected(const std::string& id, bool selected);
    void SetListBoxItems(const std::string& id, const std::vector<std::string>& items);
    void SetComboBoxItems(const std::string& id, const std::vector<std::string>& items);

    // GUI element removal
    void RemoveButton(const std::string& id);
    void RemoveCheckbox(const std::string& id);
    void RemoveSlider(const std::string& id);
    void RemoveText(const std::string& id);
    void RemoveProgressBar(const std::string& id);
    void RemoveTab(const std::string& id);
    void RemoveGroupBox(const std::string& id);
    void RemoveListBox(const std::string& id);
    void RemoveComboBox(const std::string& id);

    // GUI state management
    void ShowWindow();
    void HideWindow();
    void MinimizeWindow();
    void MaximizeWindow();
    void RestoreWindow();
    void SetWindowTitle(const std::string& title);
    void SetWindowSize(int width, int height);
    void SetWindowPosition(int x, int y);

    // Theme management
    void SetTheme(GUITheme newTheme);
    void SetCustomColors(const GUIColors& newColors);
    void UpdateTheme();

    // Anti-detection
    void EnableStealthMode(bool enable);
    void EnableObfuscation(bool enable);
    void SetStealthDelay(int minMs, int maxMs);

    // Utility
    bool IsVisible() const;
    bool IsMinimized() const;
    bool IsMaximized() const;
    bool IsFocused() const;
    RECT GetWindowRect() const;
    RECT GetClientRect() const;
    float GetFrameRate() const;
    int GetFrameCount() const;

    // BDO-specific GUI methods
    void CreateBDOBotGUI();
    void UpdateBDOBotGUI();
    void CreateSystemMonitorGUI();
    void UpdateSystemMonitorGUI();
    void CreateProcessMonitorGUI();
    void UpdateProcessMonitorGUI();
    void CreateMemoryMonitorGUI();
    void UpdateMemoryMonitorGUI();
    void CreateNetworkMonitorGUI();
    void UpdateNetworkMonitorGUI();
    void CreateSecurityMonitorGUI();
    void UpdateSecurityMonitorGUI();
    void CreatePerformanceMonitorGUI();
    void UpdatePerformanceMonitorGUI();
    void CreateOptimizationGUI();
    void UpdateOptimizationGUI();
};

// Global GUI instance
extern std::unique_ptr<BDOStealthGUI> g_GUI;

// GUI utility functions
namespace GUIUtils {
    D3DCOLOR RGBToD3D(COLORREF rgb);
    COLORREF D3DToRGB(D3DCOLOR d3d);
    RECT CreateRect(int x, int y, int width, int height);
    bool IsRectEmpty(const RECT& rect);
    bool IsRectValid(const RECT& rect);
    void InflateRect(RECT& rect, int dx, int dy);
    void OffsetRect(RECT& rect, int dx, int dy);
    bool IntersectRect(RECT& result, const RECT& rect1, const RECT& rect2);
    bool UnionRect(RECT& result, const RECT& rect1, const RECT& rect2);
    bool PtInRect(const RECT& rect, POINT point);
    int GetRectWidth(const RECT& rect);
    int GetRectHeight(const RECT& rect);
    void CenterRect(RECT& rect, const RECT& parent);
    void AlignRect(RECT& rect, const RECT& parent, int alignment);
}

// GUI color constants
namespace GUIColorConstants {
    const D3DCOLOR WHITE = 0xFFFFFFFF;
    const D3DCOLOR BLACK = 0xFF000000;
    const D3DCOLOR RED = 0xFF0000FF;
    const D3DCOLOR GREEN = 0xFF00FF00;
    const D3DCOLOR BLUE = 0xFFFF0000;
    const D3DCOLOR YELLOW = 0xFF00FFFF;
    const D3DCOLOR CYAN = 0xFFFFFF00;
    const D3DCOLOR MAGENTA = 0xFFFF00FF;
    const D3DCOLOR GRAY = 0xFF808080;
    const D3DCOLOR LIGHT_GRAY = 0xFFC0C0C0;
    const D3DCOLOR DARK_GRAY = 0xFF404040;
    const D3DCOLOR TRANSPARENT_COLOR = 0x00000000;
};
