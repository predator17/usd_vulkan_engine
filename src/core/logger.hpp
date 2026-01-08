#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <string>

namespace usd_vk {

class Logger {
public:
    static void init();
    static void shutdown();

    static std::shared_ptr<spdlog::logger>& get();
    static void setLevel(spdlog::level::level_enum level);

    // Direct wrappers that avoid template format string issues
    static void trace(const std::string& msg) { get()->trace(msg); }
    static void debug(const std::string& msg) { get()->debug(msg); }
    static void info(const std::string& msg) { get()->info(msg); }
    static void warn(const std::string& msg) { get()->warn(msg); }
    static void error(const std::string& msg) { get()->error(msg); }
    static void critical(const std::string& msg) { get()->critical(msg); }

    // Template versions for formatted logging
    template<typename... Args>
    static void trace(const char* fmt, Args&&... args) {
        get()->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void debug(const char* fmt, Args&&... args) {
        get()->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(const char* fmt, Args&&... args) {
        get()->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(const char* fmt, Args&&... args) {
        get()->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(const char* fmt, Args&&... args) {
        get()->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(const char* fmt, Args&&... args) {
        get()->critical(fmt, std::forward<Args>(args)...);
    }
};

} // namespace usd_vk
