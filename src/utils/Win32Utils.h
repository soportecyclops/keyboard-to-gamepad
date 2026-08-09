#pragma once
#include <string>
#include <windows.h>

namespace ktg {

// Obtiene ruta a %APPDATA% o %LOCALAPPDATA%
std::wstring getAppDataPath(bool roaming = false);

// Obtiene ruta a %PROGRAMDATA%
std::wstring getProgramDataPath();

// Verifica si ViGEmBus está instalado (servicio existe)
bool isViGEmBusInstalled();

// Obtiene info de dispositivo Raw Input (VID, PID, nombre)
struct DeviceInfo {
    std::wstring name;
    std::wstring hardwareId;  // VID_XXXX&PID_XXXX
    std::wstring serial;
};
DeviceInfo getRawInputDeviceInfo(HANDLE hDevice);

} // namespace ktg
