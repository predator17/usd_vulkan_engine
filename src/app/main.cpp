#include "application.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <csignal>

// Global signal handler for safe shutdown
Application* g_app = nullptr;

void signalHandler(int signal) {
    SPDLOG_INFO("Received signal {}, shutting down...", signal);
    if (g_app) {
        g_app->requestShutdown();
    }
}

int main(int argc, char* argv[]) {
    // Setup logging
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l] %v");

    SPDLOG_INFO("USD Vulkan Engine starting...");

    // Parse command line arguments
    std::string usdFile;
    if (argc > 1) {
        usdFile = argv[1];
    } else {
        // Use default test file
        usdFile = "tests/spinning_cube.usda";
    }

    // Create application
    Application app;
    g_app = &app;

    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Initialize
    if (!app.initialize("USD Vulkan Engine", 1280, 720)) {
        SPDLOG_ERROR("Failed to initialize application");
        return 1;
    }

    // Load USD scene
    if (!app.loadScene(usdFile)) {
        SPDLOG_ERROR("Failed to load USD file: {}", usdFile);
        SPDLOG_INFO("You can specify a USD file as command line argument");
        SPDLOG_INFO("Usage: {} <path/to/file.usda>", argv[0]);
        // Continue anyway - we can still render nothing
    }

    // Run main loop
    SPDLOG_INFO("Starting render loop...");
    SPDLOG_INFO("Controls:");
    SPDLOG_INFO("  ESC      - Exit");
    SPDLOG_INFO("  SPACE    - Pause/Play animation");
    SPDLOG_INFO("  LEFT     - Previous frame");
    SPDLOG_INFO("  RIGHT    - Next frame");

    app.run();

    // Cleanup
    SPDLOG_INFO("Shutting down...");
    g_app = nullptr;

    return 0;
}
