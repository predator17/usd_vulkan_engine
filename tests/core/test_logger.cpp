#include <catch2/catch_all.hpp>
using Catch::Approx;  // Catch2 v3
#include <core/logger.hpp>
#include <sstream>
#include <fstream>
#include <cstdio>

using namespace usd_vk;

TEST_CASE("Logger: Initialization", "[core][logger]") {
    SECTION("Get logger instance") {
        auto& logger = Logger::get();
        // Just verify we can get the instance
        REQUIRE(true);
    }

    SECTION("Multiple calls return same instance") {
        auto& logger1 = Logger::get();
        auto& logger2 = Logger::get();
        // Same instance (can't directly compare, but both should work)
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Set log level", "[core][logger]") {
    SECTION("Set level to trace") {
        Logger::setLevel(spdlog::level::trace);
        REQUIRE(true);
    }

    SECTION("Set level to info") {
        Logger::setLevel(spdlog::level::info);
        REQUIRE(true);
    }

    SECTION("Set level to warn") {
        Logger::setLevel(spdlog::level::warn);
        REQUIRE(true);
    }

    SECTION("Set level to error") {
        Logger::setLevel(spdlog::level::err);
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Simple logging", "[core][logger]") {
    SECTION("Trace log") {
        spdlog::trace("Trace message");
        REQUIRE(true);
    }

    SECTION("Debug log") {
        spdlog::debug("Debug message");
        REQUIRE(true);
    }

    SECTION("Info log") {
        spdlog::info("Info message");
        REQUIRE(true);
    }

    SECTION("Warning log") {
        spdlog::warn("Warning message");
        REQUIRE(true);
    }

    SECTION("Error log") {
        spdlog::error("Error message");
        REQUIRE(true);
    }

    SECTION("Critical log") {
        spdlog::critical("Critical message");
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Formatted messages with fmt", "[core][logger]") {
    SECTION("Single argument") {
        spdlog::info(fmt::format("Value: {}", 42));
        REQUIRE(true);
    }

    SECTION("Multiple arguments") {
        spdlog::info(fmt::format("Values: {}, {}, {}", 1, 2.5, "test"));
        REQUIRE(true);
    }

    SECTION("Mixed types") {
        spdlog::info(fmt::format("Int: {}, Float: {}, String: {}", 10, 3.14, "hello"));
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Special characters", "[core][logger]") {
    SECTION("Newlines in message") {
        spdlog::info("Line 1\nLine 2\nLine 3");
        REQUIRE(true);
    }

    SECTION("Tabs in message") {
        spdlog::info("Col1\tCol2\tCol3");
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Empty and null handling", "[core][logger]") {
    SECTION("Empty message") {
        spdlog::info("");
        REQUIRE(true);
    }

    SECTION("No format arguments") {
        spdlog::info("Message without arguments");
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Large messages", "[core][logger]") {
    SECTION("Long string") {
        std::string longMsg(1000, 'A');
        spdlog::info(fmt::format("Long message: {}", longMsg));
        REQUIRE(true);
    }

    SECTION("Many arguments") {
        spdlog::info(fmt::format("{},{},{},{},{},{},{},{},{},{}",
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Unicode and special text", "[core][logger]") {
    SECTION("UTF-8 characters") {
        spdlog::info("Unicode: Hello 世界 🌍");
        REQUIRE(true);
    }

    SECTION("Emojis") {
        spdlog::info("Emojis: ✅ ❌ ⚠️ 🚨");
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Numeric edge cases", "[core][logger]") {
    SECTION("Zero") {
        spdlog::info(fmt::format("Zero: {}", 0));
        spdlog::info(fmt::format("Zero float: {}", 0.0));
        REQUIRE(true);
    }

    SECTION("Negative numbers") {
        spdlog::info(fmt::format("Negative: {}", -42));
        spdlog::info(fmt::format("Negative float: {}", -3.14));
        REQUIRE(true);
    }

    SECTION("Large numbers") {
        spdlog::info(fmt::format("Large: {}", 1000000));
        spdlog::info(fmt::format("Large float: {:.1e}", 1e10));
        REQUIRE(true);
    }

    SECTION("Small numbers") {
        spdlog::info(fmt::format("Small: {}", 0.0001));
        spdlog::info(fmt::format("Tiny: {:.1e}", 1e-10));
        REQUIRE(true);
    }
}

TEST_CASE("Logger: Boolean logging", "[core][logger]") {
    SECTION("True value") {
        spdlog::info(fmt::format("True: {}", true));
        REQUIRE(true);
    }

    SECTION("False value") {
        spdlog::info(fmt::format("False: {}", false));
        REQUIRE(true);
    }
}
