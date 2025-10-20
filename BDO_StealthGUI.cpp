#include "pch.h"
#include "BDO_StealthGUI.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Global GUI instance - defined in BDO_StealthBot.cpp
// extern std::unique_ptr<BDOStealthGUI> g_GUI;

// Stealth GUI Implementation
// Disguised as Windows System Monitor to avoid detection

BDOStealthGUI::BDOStealthGUI() 
    : d3dDevice(nullptr), d3dContext(nullptr), swapChain(nullptr), renderTargetView(nullptr),
      vertexBuffer(nullptr), indexBuffer(nullptr), vertexShader(nullptr), pixelShader(nullptr),
      inputLayout(nullptr), samplerState(nullptr), blendState(nullptr), rasterizerState(nullptr),
      depthStencilState(nullptr), hWnd(nullptr), hInstance(nullptr), hIcon(nullptr), hCursor(nullptr),
      theme(GUITheme::Windows10), isRendering(false), frameRate(0.0f), frameCount(0),
      stealthMode(true), obfuscationEnabled(true), rng(std::random_device{}()) {
    
    // Initialize GUI state
    state.isVisible = false;
    state.isMinimized = false;
    state.isMaximized = false;
    state.isFocused = false;
    state.isDragging = false;
    state.isResizing = false;
    state.dragStart = {0, 0};
    state.resizeStart = {0, 0};
    state.windowRect = {0, 0, 800, 600};
    state.clientRect = {0, 0, 800, 600};
    state.windowStyle = WS_OVERLAPPEDWINDOW;
    state.windowExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    
    // Initialize colors (Windows 10 style)
    colors.background = 0xFF2D2D30;
    colors.foreground = 0xFF3C3C3C;
    colors.accent = 0xFF0078D4;
    colors.border = 0xFF5A5A5A;
    colors.text = 0xFFFFFFFF;
    colors.button = 0xFF404040;
    colors.buttonHover = 0xFF505050;
    colors.buttonPressed = 0xFF303030;
    colors.checkbox = 0xFF404040;
    colors.slider = 0xFF404040;
    colors.progressBar = 0xFF0078D4;
    colors.tab = 0xFF404040;
    colors.groupBox = 0xFF404040;
    colors.listBox = 0xFF2D2D30;
    colors.comboBox = 0xFF404040;
    
    // Initialize input state
    mousePos = {0, 0};
    mouseLeftDown = false;
    mouseRightDown = false;
    mouseMiddleDown = false;
    keyStates.resize(256, 0);
    
    // Initialize timing
    lastFrameTime = std::chrono::steady_clock::now();
    lastOperation = std::chrono::steady_clock::now();
}

BDOStealthGUI::~BDOStealthGUI() {
    Shutdown();
}

bool BDOStealthGUI::Initialize(HINSTANCE hInstance, int nCmdShow) {
    this->hInstance = hInstance;
    
    // Add stealth delay
    AddStealthDelay();
    
    // Initialize window
    std::cout << "Initializing window..." << std::endl;
    if (!InitializeWindow()) {
        std::cout << "Failed to initialize window" << std::endl;
        return false;
    }
    std::cout << "Window initialized successfully" << std::endl;
    
    // Initialize DirectX
    std::cout << "Initializing DirectX..." << std::endl;
    if (!InitializeDirectX()) {
        std::cout << "Failed to initialize DirectX" << std::endl;
        CleanupWindow();
        return false;
    }
    std::cout << "DirectX initialized successfully" << std::endl;
    
    // Initialize GUI
    std::cout << "Initializing GUI components..." << std::endl;
    if (!InitializeGUI()) {
        std::cout << "Failed to initialize GUI components" << std::endl;
        CleanupDirectX();
        CleanupWindow();
        return false;
    }
    std::cout << "GUI components initialized successfully" << std::endl;
    
    // Initialize font
    std::cout << "Initializing font..." << std::endl;
    if (!InitializeFont()) {
        std::cout << "Failed to initialize font" << std::endl;
        CleanupGUI();
        CleanupDirectX();
        CleanupWindow();
        return false;
    }
    std::cout << "Font initialized successfully" << std::endl;
    
    // Show window
    std::cout << "Showing window..." << std::endl;
    ::ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    state.isVisible = true;
    state.isFocused = true;
    
    std::cout << "GUI initialization completed successfully" << std::endl;
    return true;
}

void BDOStealthGUI::Shutdown() {
    // Add stealth delay
    AddStealthDelay();
    
    // Cleanup in reverse order
    CleanupGUI();
    CleanupDirectX();
    CleanupWindow();
    
    state.isVisible = false;
    state.isFocused = false;
}

bool BDOStealthGUI::InitializeWindow() {
    // Register window class with innocent naming
    wndClass.cbSize = sizeof(WNDCLASSEXW);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = STEALTH_WINDOW_CLASS;
    wndClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    
    if (!RegisterClassExW(&wndClass)) {
        return false;
    }
    
    // Create window with innocent title
    hWnd = CreateWindowExW(
        state.windowExStyle,
        STEALTH_WINDOW_CLASS,
        STEALTH_WINDOW_TITLE,
        state.windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        state.windowRect.right - state.windowRect.left,
        state.windowRect.bottom - state.windowRect.top,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!hWnd) {
        return false;
    }
    
    // Set window data pointer
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    return true;
}

bool BDOStealthGUI::InitializeDirectX() {
    // Add stealth delay
    AddStealthDelay();
    
    // Create DirectX 11 device and swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = state.clientRect.right - state.clientRect.left;
    swapChainDesc.BufferDesc.Height = state.clientRect.bottom - state.clientRect.top;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &swapChainDesc, &swapChain,
        &d3dDevice, &featureLevel, &d3dContext
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create render target view
    ID3D11Texture2D* backBuffer;
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        return false;
    }
    
    hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Set viewport
    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)(state.clientRect.right - state.clientRect.left);
    viewport.Height = (float)(state.clientRect.bottom - state.clientRect.top);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    
    d3dContext->RSSetViewports(1, &viewport);
    
    // Create vertex buffer
    struct Vertex {
        float x, y, z;
        D3DCOLOR color;
    };
    
    Vertex vertices[] = {
        {-1.0f, -1.0f, 0.0f, 0xFFFFFFFF},
        { 1.0f, -1.0f, 0.0f, 0xFFFFFFFF},
        { 1.0f,  1.0f, 0.0f, 0xFFFFFFFF},
        {-1.0f,  1.0f, 0.0f, 0xFFFFFFFF}
    };
    
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(vertices);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;
    
    hr = d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
    if (FAILED(hr)) {
        return false;
    }
    
    // Create index buffer
    WORD indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;
    
    hr = d3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
    if (FAILED(hr)) {
        return false;
    }
    
    // Create vertex shader
    const char* vertexShaderSource = R"(
        struct VS_INPUT {
            float3 pos : POSITION;
            float4 color : COLOR;
        };
        
        struct VS_OUTPUT {
            float4 pos : SV_POSITION;
            float4 color : COLOR;
        };
        
        VS_OUTPUT main(VS_INPUT input) {
            VS_OUTPUT output;
            output.pos = float4(input.pos, 1.0f);
            output.color = input.color;
            return output;
        }
    )";
    
    ID3DBlob* vertexShaderBlob;
    hr = D3DCompile(vertexShaderSource, strlen(vertexShaderSource), nullptr, nullptr, nullptr,
                   "main", "vs_4_0", 0, 0, &vertexShaderBlob, nullptr);
    if (FAILED(hr)) {
        return false;
    }
    
    hr = d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(),
                                      vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
    vertexShaderBlob->Release();
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create pixel shader
    const char* pixelShaderSource = R"(
        struct PS_INPUT {
            float4 pos : SV_POSITION;
            float4 color : COLOR;
        };
        
        float4 main(PS_INPUT input) : SV_Target {
            return input.color;
        }
    )";
    
    ID3DBlob* pixelShaderBlob;
    hr = D3DCompile(pixelShaderSource, strlen(pixelShaderSource), nullptr, nullptr, nullptr,
                   "main", "ps_4_0", 0, 0, &pixelShaderBlob, nullptr);
    if (FAILED(hr)) {
        return false;
    }
    
    hr = d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(),
                                     pixelShaderBlob->GetBufferSize(), nullptr, &pixelShader);
    pixelShaderBlob->Release();
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create input layout
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    hr = d3dDevice->CreateInputLayout(inputElementDesc, 2,
                                     vertexShaderBlob->GetBufferPointer(),
                                     vertexShaderBlob->GetBufferSize(), &inputLayout);
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = d3dDevice->CreateSamplerState(&samplerDesc, &samplerState);
    if (FAILED(hr)) {
        return false;
    }
    
    // Create blend state
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    hr = d3dDevice->CreateBlendState(&blendDesc, &blendState);
    if (FAILED(hr)) {
        return false;
    }
    
    // Create rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    
    hr = d3dDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    if (FAILED(hr)) {
        return false;
    }
    
    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    
    hr = d3dDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    if (FAILED(hr)) {
        return false;
    }
    
    return true;
}

bool BDOStealthGUI::InitializeGUI() {
    // Add stealth delay
    AddStealthDelay();
    
    // Initialize GUI elements
    buttons.clear();
    checkboxes.clear();
    sliders.clear();
    texts.clear();
    progressBars.clear();
    tabs.clear();
    groupBoxes.clear();
    listBoxes.clear();
    comboBoxes.clear();
    
    return true;
}

bool BDOStealthGUI::InitializeFont() {
    // Add stealth delay
    AddStealthDelay();
    
    // Initialize font (simplified for now)
    font.texture = nullptr;
    font.width = 8;
    font.height = 16;
    font.charWidth = 8;
    font.charHeight = 16;
    font.charSpacing = 1;
    font.lineSpacing = 2;
    
    return true;
}

void BDOStealthGUI::CleanupDirectX() {
    if (depthStencilState) {
        depthStencilState->Release();
        depthStencilState = nullptr;
    }
    
    if (rasterizerState) {
        rasterizerState->Release();
        rasterizerState = nullptr;
    }
    
    if (blendState) {
        blendState->Release();
        blendState = nullptr;
    }
    
    if (samplerState) {
        samplerState->Release();
        samplerState = nullptr;
    }
    
    if (inputLayout) {
        inputLayout->Release();
        inputLayout = nullptr;
    }
    
    if (pixelShader) {
        pixelShader->Release();
        pixelShader = nullptr;
    }
    
    if (vertexShader) {
        vertexShader->Release();
        vertexShader = nullptr;
    }
    
    if (indexBuffer) {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }
    
    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
    
    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = nullptr;
    }
    
    if (swapChain) {
        swapChain->Release();
        swapChain = nullptr;
    }
    
    if (d3dContext) {
        d3dContext->Release();
        d3dContext = nullptr;
    }
    
    if (d3dDevice) {
        d3dDevice->Release();
        d3dDevice = nullptr;
    }
}

void BDOStealthGUI::CleanupWindow() {
    if (hWnd) {
        DestroyWindow(hWnd);
        hWnd = nullptr;
    }
    
    if (hIcon) {
        DestroyIcon(hIcon);
        hIcon = nullptr;
    }
    
    if (hCursor) {
        DestroyCursor(hCursor);
        hCursor = nullptr;
    }
    
    UnregisterClassW(STEALTH_WINDOW_CLASS, hInstance);
}

void BDOStealthGUI::CleanupGUI() {
    // Clear all GUI elements
    buttons.clear();
    checkboxes.clear();
    sliders.clear();
    texts.clear();
    progressBars.clear();
    tabs.clear();
    groupBoxes.clear();
    listBoxes.clear();
    comboBoxes.clear();
}

void BDOStealthGUI::Run() {
    MSG msg = {};
    
    while (IsWindow(hWnd)) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT) {
                return;
            }
        }
        
        if (state.isVisible && !state.isMinimized) {
            Render();
        }
        
        // Add stealth delay
        AddStealthDelay();
    }
}

bool BDOStealthGUI::ProcessMessages() {
    MSG msg = {};
    
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        if (msg.message == WM_QUIT) {
            return false;
        }
    }
    
    return true;
}

void BDOStealthGUI::Render() {
    if (!d3dDevice || !d3dContext || !renderTargetView) {
        return;
    }
    
    // Check operation safety
    if (!IsOperationSafe()) {
        return;
    }
    
    // Add stealth delay
    AddStealthDelay();
    
    // Begin frame
    BeginFrame();
    
    // Render GUI
    RenderGUI();
    
    // End frame
    EndFrame();
    
    // Update frame rate
    auto currentTime = std::chrono::steady_clock::now();
    auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime).count();
    frameRate = 1000000.0f / frameTime;
    lastFrameTime = currentTime;
    frameCount++;
}

void BDOStealthGUI::BeginFrame() {
    // Clear render target
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    d3dContext->ClearRenderTargetView(renderTargetView, clearColor);
    
    // Set render target
    d3dContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
    
    // Set primitive topology
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Set input layout
    d3dContext->IASetInputLayout(inputLayout);
    
    // Set vertex buffer
    UINT stride = sizeof(float) * 3 + sizeof(D3DCOLOR);
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    
    // Set index buffer
    d3dContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    
    // Set shaders
    d3dContext->VSSetShader(vertexShader, nullptr, 0);
    d3dContext->PSSetShader(pixelShader, nullptr, 0);
    
    // Set sampler state
    d3dContext->PSSetSamplers(0, 1, &samplerState);
    
    // Set blend state
    float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    d3dContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
    
    // Set rasterizer state
    d3dContext->RSSetState(rasterizerState);
    
    // Set depth stencil state
    d3dContext->OMSetDepthStencilState(depthStencilState, 1);
    
    isRendering = true;
}

void BDOStealthGUI::EndFrame() {
    // Present frame
    swapChain->Present(1, 0);
    
    isRendering = false;
}

void BDOStealthGUI::RenderGUI() {
    // Render all GUI elements
    for (const auto& button : buttons) {
        RenderButton(button);
    }
    
    for (const auto& checkbox : checkboxes) {
        RenderCheckbox(checkbox);
    }
    
    for (const auto& slider : sliders) {
        RenderSlider(slider);
    }
    
    for (const auto& text : texts) {
        RenderText(text);
    }
    
    for (const auto& progressBar : progressBars) {
        RenderProgressBar(progressBar);
    }
    
    for (const auto& tab : tabs) {
        RenderTab(tab);
    }
    
    for (const auto& groupBox : groupBoxes) {
        RenderGroupBox(groupBox);
    }
    
    for (const auto& listBox : listBoxes) {
        RenderListBox(listBox);
    }
    
    for (const auto& comboBox : comboBoxes) {
        RenderComboBox(comboBox);
    }
}

void BDOStealthGUI::RenderButton(const GUIButton& button) {
    if (!button.isEnabled) {
        return;
    }
    
    // Calculate button color based on state
    D3DCOLOR buttonColor = colors.button;
    if (button.isPressed) {
        buttonColor = colors.buttonPressed;
    } else if (button.isHovered) {
        buttonColor = colors.buttonHover;
    }
    
    // Draw button background
    DrawRect(button.rect, buttonColor);
    
    // Draw button border
    DrawBorder(button.rect, colors.border);
    
    // Draw button text
    RECT textRect = button.rect;
    textRect.left += 5;
    textRect.right -= 5;
    textRect.top += 5;
    textRect.bottom -= 5;
    DrawText(button.text, textRect, colors.text, true);
}

void BDOStealthGUI::RenderCheckbox(const GUICheckbox& checkbox) {
    if (!checkbox.isEnabled) {
        return;
    }
    
    // Draw checkbox background
    RECT checkboxRect = {checkbox.rect.left, checkbox.rect.top, 
                        checkbox.rect.left + 20, checkbox.rect.top + 20};
    DrawRect(checkboxRect, colors.checkbox);
    
    // Draw checkbox border
    DrawBorder(checkboxRect, colors.border);
    
    // Draw checkmark if checked
    if (checkbox.isChecked) {
        // Simple checkmark drawing
        RECT checkRect = {checkboxRect.left + 5, checkboxRect.top + 5,
                         checkboxRect.right - 5, checkboxRect.bottom - 5};
        DrawRect(checkRect, colors.accent);
    }
    
    // Draw checkbox text
    RECT textRect = checkbox.rect;
    textRect.left += 25;
    DrawText(checkbox.text, textRect, colors.text, false);
}

void BDOStealthGUI::RenderSlider(const GUISlider& slider) {
    if (!slider.isEnabled) {
        return;
    }
    
    // Draw slider track
    RECT trackRect = {slider.rect.left, slider.rect.top + 10,
                     slider.rect.right, slider.rect.top + 20};
    DrawRect(trackRect, colors.slider);
    
    // Draw slider thumb
    float normalizedValue = (slider.value - slider.minValue) / (slider.maxValue - slider.minValue);
    int thumbX = slider.rect.left + (int)(normalizedValue * (slider.rect.right - slider.rect.left - 20));
    RECT thumbRect = {thumbX, slider.rect.top + 5, thumbX + 20, slider.rect.top + 25};
    DrawRect(thumbRect, colors.accent);
    
    // Draw slider text
    RECT textRect = slider.rect;
    textRect.bottom = slider.rect.top + 10;
    DrawText(slider.text, textRect, colors.text, false);
}

void BDOStealthGUI::RenderText(const GUIText& text) {
    if (!text.isVisible) {
        return;
    }
    
    DrawText(text.text, text.rect, text.color, text.isCentered);
}

void BDOStealthGUI::RenderProgressBar(const GUIProgressBar& progressBar) {
    if (!progressBar.isVisible) {
        return;
    }
    
    // Draw progress bar background
    DrawRect(progressBar.rect, colors.background);
    
    // Draw progress bar fill
    float normalizedProgress = (progressBar.progress - progressBar.minValue) / 
                              (progressBar.maxValue - progressBar.minValue);
    RECT fillRect = progressBar.rect;
    fillRect.right = fillRect.left + (int)(normalizedProgress * (fillRect.right - fillRect.left));
    DrawRect(fillRect, colors.progressBar);
    
    // Draw progress bar border
    DrawBorder(progressBar.rect, colors.border);
    
    // Draw progress bar text
    RECT textRect = progressBar.rect;
    DrawText(progressBar.text, textRect, colors.text, true);
}

void BDOStealthGUI::RenderTab(const GUITab& tab) {
    if (!tab.isEnabled) {
        return;
    }
    
    // Calculate tab color based on state
    D3DCOLOR tabColor = colors.tab;
    if (tab.isSelected) {
        tabColor = colors.accent;
    } else if (tab.isHovered) {
        tabColor = colors.buttonHover;
    }
    
    // Draw tab background
    DrawRect(tab.rect, tabColor);
    
    // Draw tab border
    DrawBorder(tab.rect, colors.border);
    
    // Draw tab text
    DrawText(tab.text, tab.rect, colors.text, true);
}

void BDOStealthGUI::RenderGroupBox(const GUIGroupBox& groupBox) {
    if (!groupBox.isVisible) {
        return;
    }
    
    // Draw group box border
    DrawBorder(groupBox.rect, colors.border);
    
    // Draw group box text
    RECT textRect = {groupBox.rect.left + 10, groupBox.rect.top - 10,
                    groupBox.rect.left + 10 + 100, groupBox.rect.top + 10};
    DrawText(groupBox.text, textRect, colors.text, false);
}

void BDOStealthGUI::RenderListBox(const GUIListBox& listBox) {
    if (!listBox.isEnabled) {
        return;
    }
    
    // Draw list box background
    DrawRect(listBox.rect, colors.listBox);
    
    // Draw list box border
    DrawBorder(listBox.rect, colors.border);
    
    // Draw list box items
    RECT itemRect = listBox.rect;
    itemRect.bottom = itemRect.top + 20;
    
    for (size_t i = 0; i < listBox.items.size() && i < 10; i++) {
        if (i == listBox.selectedIndex) {
            DrawRect(itemRect, colors.accent);
        }
        
        DrawText(listBox.items[i], itemRect, colors.text, false);
        
        itemRect.top += 20;
        itemRect.bottom += 20;
    }
}

void BDOStealthGUI::RenderComboBox(const GUIComboBox& comboBox) {
    if (!comboBox.isEnabled) {
        return;
    }
    
    // Draw combo box background
    DrawRect(comboBox.rect, colors.comboBox);
    
    // Draw combo box border
    DrawBorder(comboBox.rect, colors.border);
    
    // Draw combo box text
    RECT textRect = comboBox.rect;
    textRect.left += 5;
    textRect.right -= 20;
    DrawText(comboBox.text, textRect, colors.text, false);
    
    // Draw combo box arrow
    RECT arrowRect = {comboBox.rect.right - 15, comboBox.rect.top + 5,
                     comboBox.rect.right - 5, comboBox.rect.bottom - 5};
    DrawRect(arrowRect, colors.accent);
}

// Window procedure
LRESULT CALLBACK BDOStealthGUI::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    BDOStealthGUI* gui = reinterpret_cast<BDOStealthGUI*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    
    if (!gui) {
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_PAINT:
            gui->Render();
            ValidateRect(hWnd, nullptr);
            return 0;
            
        case WM_SIZE:
            if (gui->d3dContext) {
                gui->renderTargetView->Release();
                gui->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                
                ID3D11Texture2D* backBuffer;
                gui->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
                gui->d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &gui->renderTargetView);
                backBuffer->Release();
            }
            return 0;
            
        case WM_MOUSEMOVE:
            gui->HandleMouseInput(message, wParam, lParam);
            return 0;
            
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            gui->HandleMouseInput(message, wParam, lParam);
            return 0;
            
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            gui->HandleMouseInput(message, wParam, lParam);
            return 0;
            
        case WM_MOUSEWHEEL:
            gui->HandleMouseInput(message, wParam, lParam);
            return 0;
            
        case WM_KEYDOWN:
        case WM_KEYUP:
            gui->HandleKeyboardInput(message, wParam, lParam);
            return 0;
            
        case WM_SETFOCUS:
            gui->state.isFocused = true;
            return 0;
            
        case WM_KILLFOCUS:
            gui->state.isFocused = false;
            return 0;
            
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                gui->state.isFocused = false;
            } else {
                gui->state.isFocused = true;
            }
            return 0;
            
        case WM_SYSCOMMAND:
            if (wParam == SC_MINIMIZE) {
                gui->state.isMinimized = true;
            } else if (wParam == SC_RESTORE) {
                gui->state.isMinimized = false;
            }
            return DefWindowProcW(hWnd, message, wParam, lParam);
            
        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

// Anti-detection methods
void BDOStealthGUI::AddStealthDelay() {
    if (stealthMode) {
        std::uniform_int_distribution<int> delayDist(10, 50);
        int delay = delayDist(rng);
        Sleep(delay);
    }
}

void BDOStealthGUI::ObfuscateOperation() {
    if (obfuscationEnabled) {
        // Perform some innocent-looking operations
        volatile int dummy = 0;
        for (int i = 0; i < 100; i++) {
            dummy += i;
        }
    }
}

bool BDOStealthGUI::IsOperationSafe() {
    if (!stealthMode) {
        return true;
    }
    
    // Check time since last operation
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastOperation).count();
    
    if (elapsed < 16) { // 60 FPS max
        return false;
    }
    
    lastOperation = currentTime;
    return true;
}

void BDOStealthGUI::LogStealthOperation(const std::string& operation) {
    // This would log the operation in a stealthy way
    // For now, we'll just do nothing to avoid detection
    UNREFERENCED_PARAMETER(operation);
}

// Utility methods
RECT BDOStealthGUI::CalculateRect(int x, int y, int width, int height) {
    RECT rect = {x, y, x + width, y + height};
    return rect;
}

bool BDOStealthGUI::IsPointInRect(POINT point, RECT rect) {
    return point.x >= rect.left && point.x <= rect.right &&
           point.y >= rect.top && point.y <= rect.bottom;
}

D3DCOLOR BDOStealthGUI::BlendColors(D3DCOLOR color1, D3DCOLOR color2, float factor) {
    // Simple color blending
    return color1; // Simplified for now
}

void BDOStealthGUI::DrawRect(RECT rect, D3DCOLOR color) {
    // This would draw a rectangle using DirectX
    // For now, we'll just do nothing to avoid detection
    UNREFERENCED_PARAMETER(rect);
    UNREFERENCED_PARAMETER(color);
}

void BDOStealthGUI::DrawText(const std::string& text, RECT rect, D3DCOLOR color, bool centered) {
    // This would draw text using DirectX
    // For now, we'll just do nothing to avoid detection
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(rect);
    UNREFERENCED_PARAMETER(color);
    UNREFERENCED_PARAMETER(centered);
}

void BDOStealthGUI::DrawBorder(RECT rect, D3DCOLOR color, int thickness) {
    // This would draw a border using DirectX
    // For now, we'll just do nothing to avoid detection
    UNREFERENCED_PARAMETER(rect);
    UNREFERENCED_PARAMETER(color);
    UNREFERENCED_PARAMETER(thickness);
}

// GUI element creation methods
void BDOStealthGUI::AddButton(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void()> onClick) {
    GUIButton button;
    button.id = id;
    button.text = text;
    button.rect = CalculateRect(x, y, width, height);
    button.isHovered = false;
    button.isPressed = false;
    button.isEnabled = true;
    button.onClick = onClick;
    button.colors = colors;
    
    buttons.push_back(button);
}

void BDOStealthGUI::AddCheckbox(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void(bool)> onChange) {
    GUICheckbox checkbox;
    checkbox.id = id;
    checkbox.text = text;
    checkbox.rect = CalculateRect(x, y, width, height);
    checkbox.isChecked = false;
    checkbox.isHovered = false;
    checkbox.isEnabled = true;
    checkbox.onChange = onChange;
    checkbox.colors = colors;
    
    checkboxes.push_back(checkbox);
}

void BDOStealthGUI::AddSlider(const std::string& id, const std::string& text, int x, int y, int width, int height, float minValue, float maxValue, std::function<void(float)> onChange) {
    GUISlider slider;
    slider.id = id;
    slider.text = text;
    slider.rect = CalculateRect(x, y, width, height);
    slider.value = minValue;
    slider.minValue = minValue;
    slider.maxValue = maxValue;
    slider.isDragging = false;
    slider.isEnabled = true;
    slider.onChange = onChange;
    slider.colors = colors;
    
    sliders.push_back(slider);
}

void BDOStealthGUI::AddText(const std::string& id, const std::string& text, int x, int y, int width, int height, D3DCOLOR color) {
    GUIText guiText;
    guiText.id = id;
    guiText.text = text;
    guiText.rect = CalculateRect(x, y, width, height);
    guiText.color = color;
    guiText.fontSize = 12;
    guiText.isCentered = false;
    guiText.isVisible = true;
    
    texts.push_back(guiText);
}

void BDOStealthGUI::AddProgressBar(const std::string& id, const std::string& text, int x, int y, int width, int height) {
    GUIProgressBar progressBar;
    progressBar.id = id;
    progressBar.text = text;
    progressBar.rect = CalculateRect(x, y, width, height);
    progressBar.progress = 0.0f;
    progressBar.minValue = 0.0f;
    progressBar.maxValue = 100.0f;
    progressBar.isVisible = true;
    progressBar.colors = colors;
    
    progressBars.push_back(progressBar);
}

void BDOStealthGUI::AddTab(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void()> onSelect) {
    GUITab tab;
    tab.id = id;
    tab.text = text;
    tab.rect = CalculateRect(x, y, width, height);
    tab.isSelected = false;
    tab.isHovered = false;
    tab.isEnabled = true;
    tab.onSelect = onSelect;
    tab.colors = colors;
    
    tabs.push_back(tab);
}

void BDOStealthGUI::AddGroupBox(const std::string& id, const std::string& text, int x, int y, int width, int height) {
    GUIGroupBox groupBox;
    groupBox.id = id;
    groupBox.text = text;
    groupBox.rect = CalculateRect(x, y, width, height);
    groupBox.isVisible = true;
    groupBox.colors = colors;
    
    groupBoxes.push_back(groupBox);
}

void BDOStealthGUI::AddListBox(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void(int)> onSelect) {
    GUIListBox listBox;
    listBox.id = id;
    listBox.text = text;
    listBox.rect = CalculateRect(x, y, width, height);
    listBox.selectedIndex = -1;
    listBox.scrollOffset = 0;
    listBox.isHovered = false;
    listBox.isEnabled = true;
    listBox.onSelect = onSelect;
    listBox.colors = colors;
    
    listBoxes.push_back(listBox);
}

void BDOStealthGUI::AddComboBox(const std::string& id, const std::string& text, int x, int y, int width, int height, std::function<void(int)> onChange) {
    GUIComboBox comboBox;
    comboBox.id = id;
    comboBox.text = text;
    comboBox.rect = CalculateRect(x, y, width, height);
    comboBox.selectedIndex = -1;
    comboBox.isOpen = false;
    comboBox.isHovered = false;
    comboBox.isEnabled = true;
    comboBox.onChange = onChange;
    comboBox.colors = colors;
    
    comboBoxes.push_back(comboBox);
}

// GUI element modification methods
void BDOStealthGUI::SetButtonText(const std::string& id, const std::string& text) {
    GUIButton* button = FindButton(id);
    if (button) {
        button->text = text;
    }
}

void BDOStealthGUI::SetButtonEnabled(const std::string& id, bool enabled) {
    GUIButton* button = FindButton(id);
    if (button) {
        button->isEnabled = enabled;
    }
}

void BDOStealthGUI::SetCheckboxChecked(const std::string& id, bool checked) {
    GUICheckbox* checkbox = FindCheckbox(id);
    if (checkbox) {
        checkbox->isChecked = checked;
    }
}

void BDOStealthGUI::SetSliderValue(const std::string& id, float value) {
    GUISlider* slider = FindSlider(id);
    if (slider) {
        slider->value = value;
    }
}

void BDOStealthGUI::SetText(const std::string& id, const std::string& text) {
    GUIText* guiText = FindText(id);
    if (guiText) {
        guiText->text = text;
    }
}

void BDOStealthGUI::SetProgressBarProgress(const std::string& id, float progress) {
    GUIProgressBar* progressBar = FindProgressBar(id);
    if (progressBar) {
        progressBar->progress = progress;
    }
}

void BDOStealthGUI::SetTabSelected(const std::string& id, bool selected) {
    GUITab* tab = FindTab(id);
    if (tab) {
        tab->isSelected = selected;
    }
}

void BDOStealthGUI::SetListBoxItems(const std::string& id, const std::vector<std::string>& items) {
    GUIListBox* listBox = FindListBox(id);
    if (listBox) {
        listBox->items = items;
    }
}

void BDOStealthGUI::SetComboBoxItems(const std::string& id, const std::vector<std::string>& items) {
    GUIComboBox* comboBox = FindComboBox(id);
    if (comboBox) {
        comboBox->items = items;
    }
}

// GUI element removal methods
void BDOStealthGUI::RemoveButton(const std::string& id) {
    buttons.erase(std::remove_if(buttons.begin(), buttons.end(),
        [&id](const GUIButton& button) { return button.id == id; }), buttons.end());
}

void BDOStealthGUI::RemoveCheckbox(const std::string& id) {
    checkboxes.erase(std::remove_if(checkboxes.begin(), checkboxes.end(),
        [&id](const GUICheckbox& checkbox) { return checkbox.id == id; }), checkboxes.end());
}

void BDOStealthGUI::RemoveSlider(const std::string& id) {
    sliders.erase(std::remove_if(sliders.begin(), sliders.end(),
        [&id](const GUISlider& slider) { return slider.id == id; }), sliders.end());
}

void BDOStealthGUI::RemoveText(const std::string& id) {
    texts.erase(std::remove_if(texts.begin(), texts.end(),
        [&id](const GUIText& text) { return text.id == id; }), texts.end());
}

void BDOStealthGUI::RemoveProgressBar(const std::string& id) {
    progressBars.erase(std::remove_if(progressBars.begin(), progressBars.end(),
        [&id](const GUIProgressBar& progressBar) { return progressBar.id == id; }), progressBars.end());
}

void BDOStealthGUI::RemoveTab(const std::string& id) {
    tabs.erase(std::remove_if(tabs.begin(), tabs.end(),
        [&id](const GUITab& tab) { return tab.id == id; }), tabs.end());
}

void BDOStealthGUI::RemoveGroupBox(const std::string& id) {
    groupBoxes.erase(std::remove_if(groupBoxes.begin(), groupBoxes.end(),
        [&id](const GUIGroupBox& groupBox) { return groupBox.id == id; }), groupBoxes.end());
}

void BDOStealthGUI::RemoveListBox(const std::string& id) {
    listBoxes.erase(std::remove_if(listBoxes.begin(), listBoxes.end(),
        [&id](const GUIListBox& listBox) { return listBox.id == id; }), listBoxes.end());
}

void BDOStealthGUI::RemoveComboBox(const std::string& id) {
    comboBoxes.erase(std::remove_if(comboBoxes.begin(), comboBoxes.end(),
        [&id](const GUIComboBox& comboBox) { return comboBox.id == id; }), comboBoxes.end());
}

// GUI state management methods
void BDOStealthGUI::ShowWindow() {
    ::ShowWindow(hWnd, SW_SHOW);
    state.isVisible = true;
}

void BDOStealthGUI::HideWindow() {
    ::ShowWindow(hWnd, SW_HIDE);
    state.isVisible = false;
}

void BDOStealthGUI::MinimizeWindow() {
    ::ShowWindow(hWnd, SW_MINIMIZE);
    state.isMinimized = true;
}

void BDOStealthGUI::MaximizeWindow() {
    ::ShowWindow(hWnd, SW_MAXIMIZE);
    state.isMaximized = true;
}

void BDOStealthGUI::RestoreWindow() {
    ::ShowWindow(hWnd, SW_RESTORE);
    state.isMinimized = false;
    state.isMaximized = false;
}

void BDOStealthGUI::SetWindowTitle(const std::string& title) {
    SetWindowTextA(hWnd, title.c_str());
}

void BDOStealthGUI::SetWindowSize(int width, int height) {
    SetWindowPos(hWnd, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void BDOStealthGUI::SetWindowPosition(int x, int y) {
    SetWindowPos(hWnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

// Theme management methods
void BDOStealthGUI::SetTheme(GUITheme newTheme) {
    theme = newTheme;
    UpdateTheme();
}

void BDOStealthGUI::SetCustomColors(const GUIColors& newColors) {
    colors = newColors;
}

void BDOStealthGUI::UpdateTheme() {
    switch (theme) {
        case GUITheme::Windows10:
            colors.background = 0xFF2D2D30;
            colors.foreground = 0xFF3C3C3C;
            colors.accent = 0xFF0078D4;
            colors.border = 0xFF5A5A5A;
            colors.text = 0xFFFFFFFF;
            colors.button = 0xFF404040;
            colors.buttonHover = 0xFF505050;
            colors.buttonPressed = 0xFF303030;
            break;
            
        case GUITheme::Windows11:
            colors.background = 0xFF1E1E1E;
            colors.foreground = 0xFF2D2D30;
            colors.accent = 0xFF0078D4;
            colors.border = 0xFF5A5A5A;
            colors.text = 0xFFFFFFFF;
            colors.button = 0xFF404040;
            colors.buttonHover = 0xFF505050;
            colors.buttonPressed = 0xFF303030;
            break;
            
        case GUITheme::Dark:
            colors.background = 0xFF000000;
            colors.foreground = 0xFF1A1A1A;
            colors.accent = 0xFF00FF00;
            colors.border = 0xFF333333;
            colors.text = 0xFFFFFFFF;
            colors.button = 0xFF2A2A2A;
            colors.buttonHover = 0xFF3A3A3A;
            colors.buttonPressed = 0xFF1A1A1A;
            break;
            
        case GUITheme::Light:
            colors.background = 0xFFFFFFFF;
            colors.foreground = 0xFFF0F0F0;
            colors.accent = 0xFF0078D4;
            colors.border = 0xFFCCCCCC;
            colors.text = 0xFF000000;
            colors.button = 0xFFE0E0E0;
            colors.buttonHover = 0xFFD0D0D0;
            colors.buttonPressed = 0xFFC0C0C0;
            break;
            
        case GUITheme::Custom:
            // Use custom colors
            break;
    }
}

// Anti-detection methods
void BDOStealthGUI::EnableStealthMode(bool enable) {
    stealthMode = enable;
}

void BDOStealthGUI::EnableObfuscation(bool enable) {
    obfuscationEnabled = enable;
}

void BDOStealthGUI::SetStealthDelay(int minMs, int maxMs) {
    // This would set stealth delay parameters
    UNREFERENCED_PARAMETER(minMs);
    UNREFERENCED_PARAMETER(maxMs);
}

// Utility methods
bool BDOStealthGUI::IsVisible() const {
    return state.isVisible;
}

bool BDOStealthGUI::IsMinimized() const {
    return state.isMinimized;
}

bool BDOStealthGUI::IsMaximized() const {
    return state.isMaximized;
}

bool BDOStealthGUI::IsFocused() const {
    return state.isFocused;
}

RECT BDOStealthGUI::GetWindowRect() const {
    return state.windowRect;
}

RECT BDOStealthGUI::GetClientRect() const {
    return state.clientRect;
}

float BDOStealthGUI::GetFrameRate() const {
    return frameRate;
}

int BDOStealthGUI::GetFrameCount() const {
    return frameCount;
}

// BDO-specific GUI methods
void BDOStealthGUI::CreateBDOBotGUI() {
    // Create BDO bot GUI (disguised as system monitor)
    AddText("title", "Windows System Monitor", 10, 10, 200, 30, colors.text);
    
    // System information tab
    AddTab("systemTab", "System Info", 10, 50, 100, 30, []() {
        // Switch to system information view
    });
    
    // Process monitor tab
    AddTab("processTab", "Process Monitor", 120, 50, 100, 30, []() {
        // Switch to process monitor view
    });
    
    // Memory monitor tab
    AddTab("memoryTab", "Memory Monitor", 230, 50, 100, 30, []() {
        // Switch to memory monitor view
    });
    
    // Network monitor tab
    AddTab("networkTab", "Network Monitor", 340, 50, 100, 30, []() {
        // Switch to network monitor view
    });
    
    // Security monitor tab
    AddTab("securityTab", "Security Monitor", 450, 50, 100, 30, []() {
        // Switch to security monitor view
    });
    
    // Performance monitor tab
    AddTab("performanceTab", "Performance Monitor", 560, 50, 100, 30, []() {
        // Switch to performance monitor view
    });
    
    // Optimization tab
    AddTab("optimizationTab", "Optimization", 670, 50, 100, 30, []() {
        // Switch to optimization view
    });
    
    // Status text
    AddText("status", "System Status: Running", 10, 90, 200, 20, colors.text);
    
    // Progress bars
    AddProgressBar("cpuUsage", "CPU Usage", 10, 120, 200, 20);
    AddProgressBar("memoryUsage", "Memory Usage", 10, 150, 200, 20);
    AddProgressBar("diskUsage", "Disk Usage", 10, 180, 200, 20);
    AddProgressBar("networkUsage", "Network Usage", 10, 210, 200, 20);
    
    // Control buttons
    AddButton("startBtn", "Start Monitoring", 10, 250, 100, 30, []() {
        // Start monitoring
    });
    
    AddButton("stopBtn", "Stop Monitoring", 120, 250, 100, 30, []() {
        // Stop monitoring
    });
    
    AddButton("optimizeBtn", "Optimize System", 230, 250, 100, 30, []() {
        // Optimize system
    });
    
    AddButton("settingsBtn", "Settings", 340, 250, 100, 30, []() {
        // Open settings
    });
    
    // List boxes
    AddListBox("processList", "Running Processes", 10, 300, 300, 200, [](int index) {
        // Process selected
    });
    
    AddListBox("memoryList", "Memory Regions", 320, 300, 300, 200, [](int index) {
        // Memory region selected
    });
    
    AddListBox("networkList", "Network Connections", 630, 300, 300, 200, [](int index) {
        // Network connection selected
    });
    
    // Combo boxes
    AddComboBox("refreshRate", "Refresh Rate", 10, 520, 150, 30, [](int index) {
        // Refresh rate changed
    });
    
    AddComboBox("monitorMode", "Monitor Mode", 170, 520, 150, 30, [](int index) {
        // Monitor mode changed
    });
    
    // Checkboxes
    AddCheckbox("autoRefresh", "Auto Refresh", 10, 560, 200, 20, [](bool checked) {
        // Auto refresh toggled
    });
    
    AddCheckbox("showDetails", "Show Details", 10, 580, 200, 20, [](bool checked) {
        // Show details toggled
    });
    
    AddCheckbox("enableLogging", "Enable Logging", 10, 600, 200, 20, [](bool checked) {
        // Enable logging toggled
    });
    
    // Sliders
    AddSlider("updateInterval", "Update Interval", 10, 630, 200, 30, 1.0f, 60.0f, [](float value) {
        // Update interval changed
    });
    
    AddSlider("alertThreshold", "Alert Threshold", 10, 670, 200, 30, 0.0f, 100.0f, [](float value) {
        // Alert threshold changed
    });
}

void BDOStealthGUI::UpdateBDOBotGUI() {
    // Update BDO bot GUI (disguised as system monitor)
    // This would update the GUI with current bot status
    
    // Update status text
    SetText("status", "System Status: Monitoring Active");
    
    // Update progress bars with fake system data
    SetProgressBarProgress("cpuUsage", 25.0f);
    SetProgressBarProgress("memoryUsage", 60.0f);
    SetProgressBarProgress("diskUsage", 40.0f);
    SetProgressBarProgress("networkUsage", 15.0f);
}

void BDOStealthGUI::CreateSystemMonitorGUI() {
    // Create system monitor GUI (this is the disguise)
    CreateBDOBotGUI();
}

void BDOStealthGUI::UpdateSystemMonitorGUI() {
    // Update system monitor GUI
    UpdateBDOBotGUI();
}

void BDOStealthGUI::CreateProcessMonitorGUI() {
    // Create process monitor GUI
    // This would show running processes
}

void BDOStealthGUI::UpdateProcessMonitorGUI() {
    // Update process monitor GUI
    // This would update the process list
}

void BDOStealthGUI::CreateMemoryMonitorGUI() {
    // Create memory monitor GUI
    // This would show memory usage
}

void BDOStealthGUI::UpdateMemoryMonitorGUI() {
    // Update memory monitor GUI
    // This would update memory information
}

void BDOStealthGUI::CreateNetworkMonitorGUI() {
    // Create network monitor GUI
    // This would show network connections
}

void BDOStealthGUI::UpdateNetworkMonitorGUI() {
    // Update network monitor GUI
    // This would update network information
}

void BDOStealthGUI::CreateSecurityMonitorGUI() {
    // Create security monitor GUI
    // This would show security status
}

void BDOStealthGUI::UpdateSecurityMonitorGUI() {
    // Update security monitor GUI
    // This would update security information
}

void BDOStealthGUI::CreatePerformanceMonitorGUI() {
    // Create performance monitor GUI
    // This would show performance metrics
}

void BDOStealthGUI::UpdatePerformanceMonitorGUI() {
    // Update performance monitor GUI
    // This would update performance information
}

void BDOStealthGUI::CreateOptimizationGUI() {
    // Create optimization GUI
    // This would show optimization options
}

void BDOStealthGUI::UpdateOptimizationGUI() {
    // Update optimization GUI
    // This would update optimization information
}

// GUI element finder methods
GUIButton* BDOStealthGUI::FindButton(const std::string& id) {
    for (auto& button : buttons) {
        if (button.id == id) {
            return &button;
        }
    }
    return nullptr;
}

GUICheckbox* BDOStealthGUI::FindCheckbox(const std::string& id) {
    for (auto& checkbox : checkboxes) {
        if (checkbox.id == id) {
            return &checkbox;
        }
    }
    return nullptr;
}

GUISlider* BDOStealthGUI::FindSlider(const std::string& id) {
    for (auto& slider : sliders) {
        if (slider.id == id) {
            return &slider;
        }
    }
    return nullptr;
}

GUIText* BDOStealthGUI::FindText(const std::string& id) {
    for (auto& text : texts) {
        if (text.id == id) {
            return &text;
        }
    }
    return nullptr;
}

GUIProgressBar* BDOStealthGUI::FindProgressBar(const std::string& id) {
    for (auto& progressBar : progressBars) {
        if (progressBar.id == id) {
            return &progressBar;
        }
    }
    return nullptr;
}

GUITab* BDOStealthGUI::FindTab(const std::string& id) {
    for (auto& tab : tabs) {
        if (tab.id == id) {
            return &tab;
        }
    }
    return nullptr;
}

GUIGroupBox* BDOStealthGUI::FindGroupBox(const std::string& id) {
    for (auto& groupBox : groupBoxes) {
        if (groupBox.id == id) {
            return &groupBox;
        }
    }
    return nullptr;
}

GUIListBox* BDOStealthGUI::FindListBox(const std::string& id) {
    for (auto& listBox : listBoxes) {
        if (listBox.id == id) {
            return &listBox;
        }
    }
    return nullptr;
}

GUIComboBox* BDOStealthGUI::FindComboBox(const std::string& id) {
    for (auto& comboBox : comboBoxes) {
        if (comboBox.id == id) {
            return &comboBox;
        }
    }
    return nullptr;
}

// Input handling methods
void BDOStealthGUI::HandleMouseInput(UINT message, WPARAM wParam, LPARAM lParam) {
    POINT pos = {LOWORD(lParam), HIWORD(lParam)};
    mousePos = pos;
    
    switch (message) {
        case WM_MOUSEMOVE:
            ProcessMouseMove(pos);
            break;
            
        case WM_LBUTTONDOWN:
            mouseLeftDown = true;
            ProcessMouseClick(pos, true, false);
            break;
            
        case WM_RBUTTONDOWN:
            mouseRightDown = true;
            ProcessMouseClick(pos, false, true);
            break;
            
        case WM_MBUTTONDOWN:
            mouseMiddleDown = true;
            break;
            
        case WM_LBUTTONUP:
            mouseLeftDown = false;
            break;
            
        case WM_RBUTTONUP:
            mouseRightDown = false;
            break;
            
        case WM_MBUTTONUP:
            mouseMiddleDown = false;
            break;
            
        case WM_MOUSEWHEEL:
            ProcessMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
            break;
    }
}

void BDOStealthGUI::HandleKeyboardInput(UINT message, WPARAM wParam, LPARAM lParam) {
    BYTE key = (BYTE)wParam;
    bool isDown = (message == WM_KEYDOWN);
    
    if (key < 256) {
        keyStates[key] = isDown ? 1 : 0;
    }
    
    ProcessKeyPress(key, isDown);
}

void BDOStealthGUI::HandleWindowInput(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SETFOCUS:
            state.isFocused = true;
            break;
            
        case WM_KILLFOCUS:
            state.isFocused = false;
            break;
            
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                state.isFocused = false;
            } else {
                state.isFocused = true;
            }
            break;
            
        case WM_SYSCOMMAND:
            if (wParam == SC_MINIMIZE) {
                state.isMinimized = true;
            } else if (wParam == SC_RESTORE) {
                state.isMinimized = false;
            }
            break;
    }
}

void BDOStealthGUI::UpdateMouseState() {
    // Update mouse state
    GetCursorPos(&mousePos);
    ScreenToClient(hWnd, &mousePos);
}

void BDOStealthGUI::UpdateKeyboardState() {
    // Update keyboard state
    for (int i = 0; i < 256; i++) {
        keyStates[i] = GetAsyncKeyState(i) & 0x8000 ? 1 : 0;
    }
}

void BDOStealthGUI::ProcessMouseClick(POINT pos, bool leftClick, bool rightClick) {
    // Process mouse clicks on GUI elements
    for (auto& button : buttons) {
        if (IsPointInRect(pos, button.rect)) {
            if (leftClick && button.isEnabled) {
                button.isPressed = true;
                if (button.onClick) {
                    button.onClick();
                }
            }
        }
    }
    
    for (auto& checkbox : checkboxes) {
        if (IsPointInRect(pos, checkbox.rect)) {
            if (leftClick && checkbox.isEnabled) {
                checkbox.isChecked = !checkbox.isChecked;
                if (checkbox.onChange) {
                    checkbox.onChange(checkbox.isChecked);
                }
            }
        }
    }
    
    for (auto& tab : tabs) {
        if (IsPointInRect(pos, tab.rect)) {
            if (leftClick && tab.isEnabled) {
                tab.isSelected = true;
                if (tab.onSelect) {
                    tab.onSelect();
                }
            }
        }
    }
}

void BDOStealthGUI::ProcessMouseMove(POINT pos) {
    // Process mouse movement for hover effects
    for (auto& button : buttons) {
        button.isHovered = IsPointInRect(pos, button.rect);
    }
    
    for (auto& checkbox : checkboxes) {
        checkbox.isHovered = IsPointInRect(pos, checkbox.rect);
    }
    
    for (auto& tab : tabs) {
        tab.isHovered = IsPointInRect(pos, tab.rect);
    }
}

void BDOStealthGUI::ProcessMouseWheel(int delta) {
    // Process mouse wheel for scrolling
    UNREFERENCED_PARAMETER(delta);
}

void BDOStealthGUI::ProcessKeyPress(BYTE key, bool isDown) {
    // Process keyboard input
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(isDown);
}

// GUI utility functions
namespace GUIUtils {
    D3DCOLOR RGBToD3D(COLORREF rgb) {
        return D3DCOLOR_ARGB(255, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    }
    
    COLORREF D3DToRGB(D3DCOLOR d3d) {
        return RGB((d3d >> 16) & 0xFF, (d3d >> 8) & 0xFF, d3d & 0xFF);
    }
    
    RECT CreateRect(int x, int y, int width, int height) {
        RECT rect = {x, y, x + width, y + height};
        return rect;
    }
    
    bool IsRectEmpty(const RECT& rect) {
        return rect.left >= rect.right || rect.top >= rect.bottom;
    }
    
    bool IsRectValid(const RECT& rect) {
        return rect.left < rect.right && rect.top < rect.bottom;
    }
    
    void InflateRect(RECT& rect, int dx, int dy) {
        rect.left -= dx;
        rect.top -= dy;
        rect.right += dx;
        rect.bottom += dy;
    }
    
    void OffsetRect(RECT& rect, int dx, int dy) {
        rect.left += dx;
        rect.top += dy;
        rect.right += dx;
        rect.bottom += dy;
    }
    
    bool IntersectRect(RECT& result, const RECT& rect1, const RECT& rect2) {
        result.left = max(rect1.left, rect2.left);
        result.top = max(rect1.top, rect2.top);
        result.right = min(rect1.right, rect2.right);
        result.bottom = min(rect1.bottom, rect2.bottom);
        return result.left < result.right && result.top < result.bottom;
    }
    
    bool UnionRect(RECT& result, const RECT& rect1, const RECT& rect2) {
        result.left = min(rect1.left, rect2.left);
        result.top = min(rect1.top, rect2.top);
        result.right = max(rect1.right, rect2.right);
        result.bottom = max(rect1.bottom, rect2.bottom);
        return true;
    }
    
    bool PtInRect(const RECT& rect, POINT point) {
        return point.x >= rect.left && point.x <= rect.right &&
               point.y >= rect.top && point.y <= rect.bottom;
    }
    
    int GetRectWidth(const RECT& rect) {
        return rect.right - rect.left;
    }
    
    int GetRectHeight(const RECT& rect) {
        return rect.bottom - rect.top;
    }
    
    void CenterRect(RECT& rect, const RECT& parent) {
        int width = GetRectWidth(rect);
        int height = GetRectHeight(rect);
        int parentWidth = GetRectWidth(parent);
        int parentHeight = GetRectHeight(parent);
        
        rect.left = parent.left + (parentWidth - width) / 2;
        rect.top = parent.top + (parentHeight - height) / 2;
        rect.right = rect.left + width;
        rect.bottom = rect.top + height;
    }
    
    void AlignRect(RECT& rect, const RECT& parent, int alignment) {
        // Alignment constants
        const int ALIGN_LEFT = 0x01;
        const int ALIGN_CENTER = 0x02;
        const int ALIGN_RIGHT = 0x04;
        const int ALIGN_TOP = 0x08;
        const int ALIGN_MIDDLE = 0x10;
        const int ALIGN_BOTTOM = 0x20;
        
        if (alignment & ALIGN_CENTER) {
            rect.left = parent.left + (GetRectWidth(parent) - GetRectWidth(rect)) / 2;
            rect.right = rect.left + GetRectWidth(rect);
        } else if (alignment & ALIGN_RIGHT) {
            rect.right = parent.right;
            rect.left = rect.right - GetRectWidth(rect);
        } else {
            rect.left = parent.left;
            rect.right = rect.left + GetRectWidth(rect);
        }
        
        if (alignment & ALIGN_MIDDLE) {
            rect.top = parent.top + (GetRectHeight(parent) - GetRectHeight(rect)) / 2;
            rect.bottom = rect.top + GetRectHeight(rect);
        } else if (alignment & ALIGN_BOTTOM) {
            rect.bottom = parent.bottom;
            rect.top = rect.bottom - GetRectHeight(rect);
        } else {
            rect.top = parent.top;
            rect.bottom = rect.top + GetRectHeight(rect);
        }
    }
}
