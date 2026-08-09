#include "core/RawInputManager.h"
#include "core/DeviceRouter.h"
#include "utils/Logger.h"

#include <vector>

namespace ktg {

bool RawInputManager::initialize(HWND hwnd) {
    m_hwnd = hwnd;

    RAWINPUTDEVICE rid[1] = {};
    rid[0].usUsagePage = 0x01;   // Generic Desktop
    rid[0].usUsage     = 0x06;   // Keyboard
    rid[0].dwFlags     = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY | RIDEV_NOLEGACY;
    rid[0].hwndTarget  = hwnd;

    if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE))) {
        DWORD err = GetLastError();
        Logger::error("[RawInputManager] RegisterRawInputDevices failed: error=0x{:08X}", err);
        return false;
    }

    m_initialized = true;
    Logger::info("[RawInputManager] Registered for keyboard Raw Input");

    // Enumerar dispositivos existentes
    enumerateDevices();

    return true;
}

void RawInputManager::shutdown() {
    if (!m_initialized) return;

    RAWINPUTDEVICE rid[1] = {};
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage     = 0x06;
    rid[0].dwFlags     = RIDEV_REMOVE;
    rid[0].hwndTarget  = nullptr;

    if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE))) {
        Logger::warn("[RawInputManager] Failed to unregister Raw Input: error=0x{:08X}", GetLastError());
    } else {
        Logger::info("[RawInputManager] Unregistered from Raw Input");
    }

    m_initialized = false;
}

void RawInputManager::processWmInput(HRAWINPUT hRawInput) {
    if (!m_router) return;

    UINT size = 0;
    UINT result = GetRawInputData(hRawInput, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    if (result == static_cast<UINT>(-1) || size == 0) {
        Logger::warn("[RawInputManager] GetRawInputData (size query) failed: error=0x{:08X}", GetLastError());
        return;
    }

    std::vector<BYTE> buffer(size);
    result = GetRawInputData(hRawInput, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));
    if (result == static_cast<UINT>(-1) || result != size) {
        Logger::warn("[RawInputManager] GetRawInputData (read) failed: expected={}, got={}, error=0x{:08X}",
                     size, result, GetLastError());
        return;
    }

    auto* raw = reinterpret_cast<const RAWINPUT*>(buffer.data());
    if (raw->header.dwType != RIM_TYPEKEYBOARD) {
        return;  // No es teclado, ignorar
    }

    const auto& kb = raw->data.keyboard;

    // Filtrar keys "fantasma" y repeats
    if (kb.VKey == 0xFF || kb.VKey == 0) {
        Logger::trace("[RawInputManager] Ignoring phantom key from device 0x{:X}",
                       reinterpret_cast<uintptr_t>(raw->header.hDevice));
        return;
    }

    bool pressed = !(kb.Flags & RI_KEY_BREAK);

    // Log de input a nivel trace (muy verboso, solo para debug)
    Logger::trace("[RawInputManager] hDevice=0x{:X} VK=0x{:04X} Flags=0x{:04X} {} ",
                  reinterpret_cast<uintptr_t>(raw->header.hDevice),
                  kb.VKey, kb.Flags,
                  pressed ? "DOWN" : "UP");

    // Enrutar al DeviceRouter
    m_router->onInputEvent(raw->header.hDevice, kb.VKey, pressed);
}

void RawInputManager::onDeviceChange() {
    Logger::info("[RawInputManager] WM_DEVICECHANGE received, refreshing devices...");
    if (m_router) {
        m_router->refreshDeviceList();
    }
}

void RawInputManager::enumerateDevices() {
    Logger::info("[RawInputManager] Enumerating existing devices...");
    if (m_router) {
        m_router->refreshDeviceList();
    }
}

} // namespace ktg
