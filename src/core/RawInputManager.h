#pragma once
#include <windows.h>
#include <functional>
#include <vector>
#include <memory>

namespace ktg {

class DeviceRouter;
struct InputEvent;

class RawInputManager {
public:
    RawInputManager() = default;
    ~RawInputManager() = default;

    bool initialize(HWND hwnd);
    void shutdown();

    // Inyectar el router para enrutar eventos
    void setDeviceRouter(std::shared_ptr<DeviceRouter> router) { m_router = router; }

    // Procesar un mensaje WM_INPUT (llamado desde WndProc)
    void processWmInput(HRAWINPUT hRawInput);

    // Procesar WM_DEVICECHANGE (conexión/desconexión)
    void onDeviceChange();

    // Enumerar dispositivos al inicio
    void enumerateDevices();

private:
    HWND m_hwnd = nullptr;
    std::shared_ptr<DeviceRouter> m_router;
    bool m_initialized = false;
};

} // namespace ktg
