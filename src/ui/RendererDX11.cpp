#include "ui/RendererDX11.h"
#include "utils/Logger.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace ktg {

bool RendererDX11::initialize(HWND hwnd) {
    m_hwnd = hwnd;

    if (!createDevice()) {
        Logger::error("Failed to create D3D11 device");
        return false;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_device, m_context);

    RECT rc;
    GetClientRect(hwnd, &rc);
    resize(static_cast<UINT>(rc.right - rc.left), static_cast<UINT>(rc.bottom - rc.top));

    Logger::info("DX11 renderer initialized");
    return true;
}

void RendererDX11::shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanupRenderTarget();
    if (m_swapChain) { m_swapChain->Release(); m_swapChain = nullptr; }
    if (m_context) { m_context->Release(); m_context = nullptr; }
    if (m_device) { m_device->Release(); m_device = nullptr; }
}

void RendererDX11::resize(UINT width, UINT height) {
    if (m_width == width && m_height == height) return;
    m_width = width;
    m_height = height;

    cleanupRenderTarget();
    m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    createRenderTarget();
}

void RendererDX11::renderFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // UI se renderiza externamente via MainWindow::update()

    ImGui::Render();

    const float clearColor[4] = {0.1f, 0.1f, 0.12f, 1.0f};
    m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    m_context->ClearRenderTargetView(m_renderTargetView, clearColor);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_swapChain->Present(1, 0);
}

bool RendererDX11::createDevice() {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevelArray, 2, D3D11_SDK_VERSION,
        &sd, &m_swapChain, &m_device, &featureLevel, &m_context
    );

    return SUCCEEDED(hr);
}

void RendererDX11::createRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (backBuffer) {
        m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
        backBuffer->Release();
    }
}

void RendererDX11::cleanupRenderTarget() {
    if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = nullptr; }
}

} // namespace ktg
