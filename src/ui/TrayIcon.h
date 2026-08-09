#pragma once
#include <windows.h>

#define WM_TRAYICON (WM_USER + 1)

namespace ktg {

class TrayIcon {
public:
    TrayIcon() = default;
    ~TrayIcon() { destroy(); }

    bool create(HWND hwnd, HINSTANCE hInstance);
    void destroy();
    void handleEvent(LPARAM lParam);
    void showBalloon(const wchar_t* title, const wchar_t* text);

private:
    HWND m_hwnd = nullptr;
    NOTIFYICONDATAW m_nid = {};
    bool m_created = false;
};

} // namespace ktg
