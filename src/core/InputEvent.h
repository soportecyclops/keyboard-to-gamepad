#pragma once
#include <windows.h>

namespace ktg {

struct InputEvent {
    HANDLE hDevice;      // Handle único del dispositivo Raw Input
    USHORT vKey;         // Virtual-Key Code
    bool pressed;        // true = down, false = up
    DWORD timestamp;     // GetTickCount()
};

} // namespace ktg
