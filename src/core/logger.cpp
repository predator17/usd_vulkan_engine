#include "logger.hpp"

namespace usd_vk {

namespace {
std::shared_ptr<spdlog::logger> s_logger;
}

void Logger::init() {
    try {
        s_logger = spdlog::stdout_color_mt("usd_vk");
        s_logger->set_level(spdlog::level::info);
        s_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        spdlog::register_logger(s_logger);
        spdlog::set_default_logger(s_logger);
        s_logger->info("Logger initialized");
    } catch (const spdlog::spdlog_ex& ex) {
        std::fprintf(stderr, "Log init failed: %s\n", ex.what());
    }
}

void Logger::shutdown() {
    if (s_logger) {
        s_logger->info("Logger shutting down");
        s_logger.reset();
    }
}

std::shared_ptr<spdlog::logger>& Logger::get() {
    return s_logger;
}

void Logger::setLevel(spdlog::level::level_enum level) {
    if (s_logger) {
        s_logger->set_level(level);
    }
}

} // namespace usd_vk
