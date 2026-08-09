#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace ktg {

class RendererDX11 {
public:
    RendererDX11() = default;
    ~RendererDX11() { shutdown(); }

    bool initialize(HWND hwnd);
    void shutdown();
    void resize(UINT width, UINT height);
    void renderFrame();

    ID3D11Device* getDevice() const { return m_device; }
    ID3D11DeviceContext* getContext() const { return m_context; }

private:
    bool createDevice();
    void createRenderTarget();
    void cleanupRenderTarget();

    HWND m_hwnd = nullptr;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
    UINT m_width = 0;
    UINT m_height = 0;
};

} // namespace ktg
