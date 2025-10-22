#pragma once
#include <windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// Enhanced GUI interface for memory scanner
// Modern, user-friendly interface for all scanning operations

namespace BDO {
    namespace GUI {

        // Scan result for display
        struct ScanResultDisplay {
            PVOID address;
            std::string value;
            std::string type;
            bool selected;
            bool frozen;
            std::string description;
        };

        // GUI theme colors
        struct Theme {
            ImVec4 background;
            ImVec4 titleBar;
            ImVec4 button;
            ImVec4 buttonHovered;
            ImVec4 buttonActive;
            ImVec4 text;
            ImVec4 accent;
            
            static Theme Dark() {
                Theme theme;
                theme.background = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
                theme.titleBar = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
                theme.button = ImVec4(0.2f, 0.5f, 0.8f, 1.0f);
                theme.buttonHovered = ImVec4(0.3f, 0.6f, 0.9f, 1.0f);
                theme.buttonActive = ImVec4(0.1f, 0.4f, 0.7f, 1.0f);
                theme.text = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                theme.accent = ImVec4(0.2f, 0.8f, 0.4f, 1.0f);
                return theme;
            }
            
            static Theme Light() {
                Theme theme;
                theme.background = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
                theme.titleBar = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
                theme.button = ImVec4(0.3f, 0.6f, 0.9f, 1.0f);
                theme.buttonHovered = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
                theme.buttonActive = ImVec4(0.2f, 0.5f, 0.8f, 1.0f);
                theme.text = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                theme.accent = ImVec4(0.2f, 0.7f, 0.3f, 1.0f);
                return theme;
            }
        };

        class MemoryScannerGUI {
        private:
            // DirectX 11 resources
            ID3D11Device* d3dDevice;
            ID3D11DeviceContext* d3dContext;
            IDXGISwapChain* swapChain;
            ID3D11RenderTargetView* renderTargetView;
            
            // Window handle
            HWND hwnd;
            bool running;
            
            // GUI state
            std::vector<ScanResultDisplay> scanResults;
            char searchValue[256];
            int selectedValueType;
            int selectedScanType;
            bool showSettings;
            bool showAbout;
            Theme currentTheme;
            
            // Callbacks
            std::function<void(const std::string&, int, int)> onScanCallback;
            std::function<void(PVOID)> onAddressSelectedCallback;
            std::function<void(PVOID, bool)> onFreezeCallback;
            
            // Initialize DirectX
            bool InitDirectX() {
                // Create swap chain
                DXGI_SWAP_CHAIN_DESC sd;
                ZeroMemory(&sd, sizeof(sd));
                sd.BufferCount = 2;
                sd.BufferDesc.Width = 0;
                sd.BufferDesc.Height = 0;
                sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                sd.BufferDesc.RefreshRate.Numerator = 60;
                sd.BufferDesc.RefreshRate.Denominator = 1;
                sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                sd.OutputWindow = hwnd;
                sd.SampleDesc.Count = 1;
                sd.SampleDesc.Quality = 0;
                sd.Windowed = TRUE;
                sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
                
                UINT createDeviceFlags = 0;
                D3D_FEATURE_LEVEL featureLevel;
                const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
                
                if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, 
                    featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &d3dDevice, &featureLevel, &d3dContext) != S_OK) {
                    return false;
                }
                
                // Create render target
                ID3D11Texture2D* pBackBuffer;
                swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
                d3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetView);
                pBackBuffer->Release();
                
                return true;
            }
            
            // Apply theme
            void ApplyTheme(const Theme& theme) {
                ImGuiStyle& style = ImGui::GetStyle();
                
                style.Colors[ImGuiCol_WindowBg] = theme.background;
                style.Colors[ImGuiCol_TitleBg] = theme.titleBar;
                style.Colors[ImGuiCol_TitleBgActive] = theme.titleBar;
                style.Colors[ImGuiCol_Button] = theme.button;
                style.Colors[ImGuiCol_ButtonHovered] = theme.buttonHovered;
                style.Colors[ImGuiCol_ButtonActive] = theme.buttonActive;
                style.Colors[ImGuiCol_Text] = theme.text;
                style.Colors[ImGuiCol_CheckMark] = theme.accent;
                style.Colors[ImGuiCol_SliderGrab] = theme.accent;
                style.Colors[ImGuiCol_SliderGrabActive] = theme.accent;
            }
            
        public:
            MemoryScannerGUI()
                : d3dDevice(nullptr)
                , d3dContext(nullptr)
                , swapChain(nullptr)
                , renderTargetView(nullptr)
                , hwnd(nullptr)
                , running(false)
                , selectedValueType(0)
                , selectedScanType(0)
                , showSettings(false)
                , showAbout(false)
                , currentTheme(Theme::Dark())
            {
                memset(searchValue, 0, sizeof(searchValue));
            }
            
            ~MemoryScannerGUI() {
                Shutdown();
            }
            
            // Initialize GUI
            bool Initialize(HWND window) {
                hwnd = window;
                
                if (!InitDirectX()) {
                    return false;
                }
                
                // Setup ImGui
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                
                // Setup style
                ImGui::StyleColorsDark();
                ApplyTheme(currentTheme);
                
                // Setup Platform/Renderer backends
                ImGui_ImplWin32_Init(hwnd);
                ImGui_ImplDX11_Init(d3dDevice, d3dContext);
                
                running = true;
                return true;
            }
            
            // Shutdown GUI
            void Shutdown() {
                if (running) {
                    ImGui_ImplDX11_Shutdown();
                    ImGui_ImplWin32_Shutdown();
                    ImGui::DestroyContext();
                    
                    if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
                    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
                    if (d3dContext) { d3dContext->Release(); d3dContext = nullptr; }
                    if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
                    
                    running = false;
                }
            }
            
            // Render one frame
            void Render() {
                // Start ImGui frame
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                
                // Main window
                RenderMainWindow();
                
                // Settings window
                if (showSettings) {
                    RenderSettingsWindow();
                }
                
                // About window
                if (showAbout) {
                    RenderAboutWindow();
                }
                
                // Rendering
                ImGui::Render();
                const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
                d3dContext->OMSetRenderTargets(1, &renderTargetView, NULL);
                d3dContext->ClearRenderTargetView(renderTargetView, clear_color);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                
                swapChain->Present(1, 0);
            }
            
            // Render main scanner window
            void RenderMainWindow() {
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
                
                ImGui::Begin("BDO Memory Scanner", &running, ImGuiWindowFlags_MenuBar);
                
                // Menu bar
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Save Results")) { /* TODO */ }
                        if (ImGui::MenuItem("Load Results")) { /* TODO */ }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Exit")) { running = false; }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("View")) {
                        if (ImGui::MenuItem("Settings")) { showSettings = !showSettings; }
                        if (ImGui::MenuItem("About")) { showAbout = !showAbout; }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                
                // Search controls
                ImGui::Text("Search Value:");
                ImGui::SameLine();
                ImGui::InputText("##searchvalue", searchValue, sizeof(searchValue));
                
                ImGui::Text("Value Type:");
                ImGui::SameLine();
                const char* valueTypes[] = { "Byte", "Word", "Dword", "Qword", "Float", "Double", "String" };
                ImGui::Combo("##valuetype", &selectedValueType, valueTypes, IM_ARRAYSIZE(valueTypes));
                
                ImGui::Text("Scan Type:");
                ImGui::SameLine();
                const char* scanTypes[] = { "Exact Value", "Bigger Than", "Smaller Than", "Changed", "Unchanged", "Increased", "Decreased" };
                ImGui::Combo("##scantype", &selectedScanType, scanTypes, IM_ARRAYSIZE(scanTypes));
                
                // Scan buttons
                if (ImGui::Button("First Scan", ImVec2(120, 30))) {
                    if (onScanCallback) {
                        onScanCallback(searchValue, selectedValueType, selectedScanType);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Next Scan", ImVec2(120, 30))) {
                    if (onScanCallback) {
                        onScanCallback(searchValue, selectedValueType, selectedScanType);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("New Scan", ImVec2(120, 30))) {
                    scanResults.clear();
                }
                
                ImGui::Separator();
                
                // Results table
                ImGui::Text("Results: %d", (int)scanResults.size());
                
                ImGui::BeginChild("Results", ImVec2(0, 0), true);
                
                if (ImGui::BeginTable("ResultsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
                    ImGui::TableSetupColumn("Address");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Frozen");
                    ImGui::TableSetupColumn("Description");
                    ImGui::TableHeadersRow();
                    
                    for (auto& result : scanResults) {
                        ImGui::TableNextRow();
                        
                        // Address
                        ImGui::TableNextColumn();
                        char addrStr[32];
                        sprintf_s(addrStr, "%p", result.address);
                        if (ImGui::Selectable(addrStr, result.selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            result.selected = !result.selected;
                            if (onAddressSelectedCallback) {
                                onAddressSelectedCallback(result.address);
                            }
                        }
                        
                        // Value
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", result.value.c_str());
                        
                        // Type
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", result.type.c_str());
                        
                        // Frozen
                        ImGui::TableNextColumn();
                        if (ImGui::Checkbox(("##frozen" + std::to_string((ULONG_PTR)result.address)).c_str(), &result.frozen)) {
                            if (onFreezeCallback) {
                                onFreezeCallback(result.address, result.frozen);
                            }
                        }
                        
                        // Description
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", result.description.c_str());
                    }
                    
                    ImGui::EndTable();
                }
                
                ImGui::EndChild();
                
                ImGui::End();
            }
            
            // Render settings window
            void RenderSettingsWindow() {
                ImGui::Begin("Settings", &showSettings);
                
                ImGui::Text("Theme:");
                if (ImGui::RadioButton("Dark", &currentTheme == Theme::Dark())) {
                    currentTheme = Theme::Dark();
                    ApplyTheme(currentTheme);
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Light", &currentTheme == Theme::Light())) {
                    currentTheme = Theme::Light();
                    ApplyTheme(currentTheme);
                }
                
                ImGui::Separator();
                ImGui::Text("Scan Options:");
                // Add more settings here
                
                ImGui::End();
            }
            
            // Render about window
            void RenderAboutWindow() {
                ImGui::Begin("About", &showAbout);
                
                ImGui::Text("BDO Memory Scanner");
                ImGui::Text("Version 1.0");
                ImGui::Separator();
                ImGui::Text("Advanced memory scanning tool for BDO");
                ImGui::Text("Features:");
                ImGui::BulletText("Multi-threaded scanning");
                ImGui::BulletText("Pattern generation");
                ImGui::BulletText("Value freezing");
                ImGui::BulletText("Automatic offset updating");
                
                ImGui::End();
            }
            
            // Add scan result
            void AddScanResult(PVOID address, const std::string& value, const std::string& type) {
                ScanResultDisplay result;
                result.address = address;
                result.value = value;
                result.type = type;
                result.selected = false;
                result.frozen = false;
                scanResults.push_back(result);
            }
            
            // Clear results
            void ClearResults() {
                scanResults.clear();
            }
            
            // Set callbacks
            void SetScanCallback(std::function<void(const std::string&, int, int)> callback) {
                onScanCallback = callback;
            }
            
            void SetAddressSelectedCallback(std::function<void(PVOID)> callback) {
                onAddressSelectedCallback = callback;
            }
            
            void SetFreezeCallback(std::function<void(PVOID, bool)> callback) {
                onFreezeCallback = callback;
            }
            
            // Check if running
            bool IsRunning() const {
                return running;
            }
        };

    } // namespace GUI
} // namespace BDO

