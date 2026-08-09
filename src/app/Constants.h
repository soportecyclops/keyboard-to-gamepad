#pragma once
#include <string>

namespace ktg {

inline constexpr int MAX_PLAYERS = 4;
inline constexpr int MAX_VKEY = 256;

inline const std::wstring APP_NAME = L"KeyboardToGamepad";
inline const std::wstring CONFIG_DIR = L"KeyboardToGamepad";
inline const std::wstring PROFILES_SUBDIR = L"profiles";

// DirectX
inline constexpr int WINDOW_WIDTH = 1024;
inline constexpr int WINDOW_HEIGHT = 768;

} // namespace ktg
