#include "ui/TrayIcon.h"
#include "app/Application.h"
#include "utils/Logger.h"

namespace ktg {

bool TrayIcon::create(HWND hwnd, HINSTANCE hInstance) {
    m_hwnd = hwnd;

    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(m_nid.szTip, L"KeyboardToGamepad");

    if (!Shell_NotifyIconW(NIM_ADD, &m_nid)) {
        Logger::error("Failed to create tray icon");
        return false;
    }

    m_created = true;
    Logger::info("Tray icon created");
    return true;
}

void TrayIcon::destroy() {
    if (m_created) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_created = false;
    }
}

void TrayIcon::handleEvent(LPARAM lParam) {
    if (lParam == WM_LBUTTONUP) {
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
    } else if (lParam == WM_RBUTTONUP) {
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, 1, L"Mostrar");
        AppendMenuW(hMenu, MF_STRING, 2, L"Salir");

        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(m_hwnd);

        int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, 
                                  pt.x, pt.y, 0, m_hwnd, nullptr);
        DestroyMenu(hMenu);

        if (cmd == 1) {
            ShowWindow(m_hwnd, SW_SHOW);
        } else if (cmd == 2) {
            Application::get().requestQuit();
        }
    }
}

void TrayIcon::showBalloon(const wchar_t* title, const wchar_t* text) {
    if (!m_created) return;
    m_nid.uFlags |= NIF_INFO;
    wcscpy_s(m_nid.szInfoTitle, title);
    wcscpy_s(m_nid.szInfo, text);
    m_nid.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
    m_nid.uFlags &= ~NIF_INFO;
}

} // namespace ktg
