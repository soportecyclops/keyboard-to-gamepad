#include "utils/Logger.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ktg {

std::mutex Logger::s_mutex;
LogLevel Logger::s_minLevel = LogLevel::Debug;

static std::wofstream g_logFile;
static std::wstring g_logPath;

void Logger::init() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        g_logPath = std::wstring(path) + L"\\KeyboardToGamepad\\logs";
        CreateDirectoryW(g_logPath.c_str(), nullptr);
        g_logPath += L"\\app.log";
    } else {
        g_logPath = L"app.log";
    }

    g_logFile.open(g_logPath, std::ios::out | std::ios::app);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    std::wstringstream wss;
    wss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");

    std::lock_guard<std::mutex> lock(s_mutex);
    if (g_logFile.is_open()) {
        g_logFile << L"\n================================================================================\n";
        g_logFile << L"  KeyboardToGamepad Session Started: " << wss.str() << L"\n";
        g_logFile << L"================================================================================\n";
        g_logFile.flush();
    }
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (g_logFile.is_open()) {
        g_logFile << L"================================================================================\n";
        g_logFile << L"  Session Ended\n";
        g_logFile << L"================================================================================\n\n";
        g_logFile.close();
    }
}

void Logger::setMinLevel(LogLevel level) {
    s_minLevel = level;
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < s_minLevel) return;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm;
    localtime_s(&tm, &time);

    std::stringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    ss << " [" << levelToString(level) << "] " << msg;

    // OutputDebugString para Visual Studio
    OutputDebugStringA((ss.str() + "\n").c_str());

    // Consola si existe
    #ifdef _DEBUG
    std::cout << ss.str() << std::endl;
    #endif

    // Archivo de log
    std::lock_guard<std::mutex> lock(s_mutex);
    if (g_logFile.is_open()) {
        std::wstring wmsg(ss.str.begin(), ss.str.end());
        g_logFile << wmsg << L"\n";
        g_logFile.flush();
    }
}

} // namespace ktg
