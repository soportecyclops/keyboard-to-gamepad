#include "utils/Win32Utils.h"
#include <windows.h>
#include <shlobj.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <hidsdi.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

namespace ktg {

std::wstring getAppDataPath(bool roaming) {
    wchar_t path[MAX_PATH];
    int csidl = roaming ? CSIDL_APPDATA : CSIDL_LOCAL_APPDATA;
    if (SUCCEEDED(SHGetFolderPathW(nullptr, csidl, nullptr, 0, path))) {
        return std::wstring(path);
    }
    return L"";
}

std::wstring getProgramDataPath() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, path))) {
        return std::wstring(path);
    }
    return L"";
}

bool isViGEmBusInstalled() {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) return false;

    SC_HANDLE service = OpenServiceW(scm, L"ViGEmBus", SERVICE_QUERY_STATUS);
    bool installed = (service != nullptr);

    if (service) CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return installed;
}

DeviceInfo getRawInputDeviceInfo(HANDLE hDevice) {
    DeviceInfo info;

    UINT size = 0;
    GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, nullptr, &size);
    if (size > 0) {
        std::wstring name(size, L'\0');
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, name.data(), &size) > 0) {
            info.name = name;
        }
    }

    RID_DEVICE_INFO rdi = {};
    rdi.cbSize = sizeof(RID_DEVICE_INFO);
    UINT cbSize = sizeof(RID_DEVICE_INFO);
    if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &cbSize) > 0) {
        if (rdi.dwType == RIM_TYPEKEYBOARD) {
            wchar_t buf[128];
            swprintf_s(buf, L"VID_%04X&PID_%04X", 
                       rdi.keyboard.dwVendorId, rdi.keyboard.dwProductId);
            info.hardwareId = buf;
        }
    }

    return info;
}

} // namespace ktg
