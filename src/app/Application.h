#pragma once
#include <memory>
#include <windows.h>

namespace ktg {

class RawInputManager;
class DeviceRouter;
class GamepadManager;
class RendererDX11;
class MainWindow;
class TrayIcon;

class Application {
public:
    static Application& get();

    bool initialize(HINSTANCE hInstance, int nCmdShow);
    void shutdown();
    int run();

    // WndProc delega aquí
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND getHwnd() const { return m_hwnd; }
    HINSTANCE getInstance() const { return m_hInstance; }

    bool isRunning() const { return m_running; }
    void requestQuit() { m_running = false; }

    // Acceso a subsistemas para UI
    std::shared_ptr<DeviceRouter> getDeviceRouter() const { return m_deviceRouter; }
    std::shared_ptr<RawInputManager> getRawInputManager() const { return m_rawInput; }

private:
    Application() = default;
    ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool createWindow(int nCmdShow);
    bool initializeSubsystems();

    HINSTANCE m_hInstance = nullptr;
    HWND m_hwnd = nullptr;
    bool m_running = true;

    // Subsistemas
    std::shared_ptr<RendererDX11> m_renderer;
    std::shared_ptr<MainWindow> m_ui;
    std::shared_ptr<TrayIcon> m_tray;
    std::shared_ptr<RawInputManager> m_rawInput;
    std::shared_ptr<DeviceRouter> m_deviceRouter;
    // std::shared_ptr<GamepadManager> m_gamepad;  // Milestone 2
};

} // namespace ktg
