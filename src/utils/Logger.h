#pragma once
#include <string>
#include <format>
#include <mutex>

namespace ktg {

enum class LogLevel {
    Trace, Debug, Info, Warn, Error, Fatal
};

class Logger {
public:
    static void init();
    static void shutdown();

    static void setMinLevel(LogLevel level);

    template<typename... Args>
    static void trace(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::Trace, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void debug(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void info(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void warn(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::Warn, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void error(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static void fatal(const std::format_string<Args...>& fmt, Args&&... args) {
        log(LogLevel::Fatal, std::format(fmt, std::forward<Args>(args)...));
    }

private:
    static void log(LogLevel level, const std::string& msg);
    static const char* levelToString(LogLevel level);

    static std::mutex s_mutex;
    static LogLevel s_minLevel;
};

} // namespace ktg
