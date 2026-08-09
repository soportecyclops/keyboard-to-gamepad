#include "app/Application.h"
#include "app/Constants.h"
#include "app/Version.h"
#include "core/RawInputManager.h"
#include "core/DeviceRouter.h"
#include "ui/RendererDX11.h"
#include "ui/MainWindow.h"
#include "ui/TrayIcon.h"
#include "utils/Logger.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <dbt.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ktg {

Application& Application::get() {
    static Application instance;
    return instance;
}

bool Application::initialize(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;

    Logger::init();
    Logger::info("========================================");
    Logger::info("  {} v{}", KTG_APP_NAME, KTG_VERSION_STRING);
    Logger::info("  Build: {} {}", __DATE__, __TIME__);
    Logger::info("========================================");

    if (!createWindow(nCmdShow)) {
        Logger::fatal("[Application] Failed to create main window");
        return false;
    }

    m_renderer = std::make_shared<RendererDX11>();
    if (!m_renderer->initialize(m_hwnd)) {
        Logger::fatal("[Application] Failed to initialize DX11 renderer");
        return false;
    }

    if (!initializeSubsystems()) {
        Logger::fatal("[Application] Failed to initialize core subsystems");
        return false;
    }

    m_ui = std::make_shared<MainWindow>();
    m_ui->initialize();

    m_tray = std::make_shared<TrayIcon>();
    if (!m_tray->create(m_hwnd, m_hInstance)) {
        Logger::warn("[Application] Tray icon creation failed (non-fatal)");
    }

    Logger::info("[Application] Initialization complete");
    return true;
}

bool Application::initializeSubsystems() {
    Logger::info("[Application] Initializing core subsystems...");

    // DeviceRouter primero (no depende de nada)
    m_deviceRouter = std::make_shared<DeviceRouter>();
    Logger::info("[Application] DeviceRouter created");

    // RawInputManager depende de DeviceRouter
    m_rawInput = std::make_shared<RawInputManager>();
    m_rawInput->setDeviceRouter(m_deviceRouter);

    if (!m_rawInput->initialize(m_hwnd)) {
        Logger::error("[Application] RawInputManager initialization failed");
        return false;
    }
    Logger::info("[Application] RawInputManager initialized");

    // Milestone 2: GamepadManager
    // m_gamepad = std::make_shared<GamepadManager>();
    // if (!m_gamepad->initialize()) { ... }

    Logger::info("[Application] Core subsystems ready");
    return true;
}

void Application::shutdown() {
    Logger::info("[Application] Shutting down...");

    if (m_tray) m_tray->destroy();
    if (m_ui) m_ui->shutdown();
    if (m_rawInput) m_rawInput->shutdown();
    if (m_renderer) m_renderer->shutdown();

    Logger::info("[Application] Shutdown complete");
    Logger::shutdown();
}

int Application::run() {
    MSG msg = {};
    while (m_running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!m_running) break;

        if (m_ui) m_ui->update();
        if (m_renderer) m_renderer->renderFrame();
    }
    return static_cast<int>(msg.wParam);
}

LRESULT Application::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (m_renderer && wParam != SIZE_MINIMIZED) {
            m_renderer->resize(static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)));
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE) {
            ShowWindow(hwnd, SW_HIDE);
            if (m_tray) m_tray->showBalloon(L"KeyboardToGamepad", L"La app sigue corriendo en la bandeja del sistema.");
            return 0;
        }
        break;

    case WM_INPUT: {
        if (m_rawInput) {
            m_rawInput->processWmInput(reinterpret_cast<HRAWINPUT>(lParam));
        }
        return 0;
    }

    case WM_INPUT_DEVICE_CHANGE: {
        if (m_rawInput) {
            m_rawInput->onDeviceChange();
        }
        return 0;
    }

    case WM_DEVICECHANGE: {
        // Dispositivos USB conectados/desconectados
        if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE) {
            if (m_rawInput) {
                Logger::info("[Application] WM_DEVICECHANGE: wParam=0x{:04X}", wParam);
                m_rawInput->onDeviceChange();
            }
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        Logger::info("[Application] WM_CLOSE received");
        m_running = false;
        return 0;

    case WM_TRAYICON:
        if (m_tray) m_tray->handleEvent(lParam);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Application::createWindow(int nCmdShow) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        return Application::get().handleMessage(hwnd, msg, wParam, lParam);
    };
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"KeyboardToGamepadClass";

    if (!RegisterClassExW(&wc)) {
        Logger::error("[Application] RegisterClassEx failed: error=0x{:08X}", GetLastError());
        return false;
    }

    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    std::wstring title = APP_NAME + L" v" + 
        std::to_wstring(KTG_VERSION_MAJOR) + L"." + 
        std::to_wstring(KTG_VERSION_MINOR) + L"." + 
        std::to_wstring(KTG_VERSION_PATCH);

    m_hwnd = CreateWindowExW(
        0,
        L"KeyboardToGamepadClass",
        title.c_str(),
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_hInstance, nullptr
    );

    if (!m_hwnd) {
        Logger::error("[Application] CreateWindowEx failed: error=0x{:08X}", GetLastError());
        return false;
    }

    // Registrar para notificaciones de dispositivos USB
    DEV_BROADCAST_DEVICEINTERFACE filter = {};
    filter.dbcc_size = sizeof(filter);
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid = GUID_DEVINTERFACE_HID;  // HID devices

    HDEVNOTIFY hDevNotify = RegisterDeviceNotificationW(m_hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (hDevNotify) {
        Logger::info("[Application] Registered for USB device notifications");
    } else {
        Logger::warn("[Application] RegisterDeviceNotification failed: error=0x{:08X}", GetLastError());
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    Logger::info("[Application] Main window created: {}x{}", WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

} // namespace ktg
