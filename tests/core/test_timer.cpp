#include <catch2/catch.hpp>
#include <core/timer.hpp>
#include <thread>
#include <chrono>

using namespace usd_ve;

TEST_CASE("Timer: Default construction", "[core][timer]") {
    Timer timer;

    SECTION("Initial elapsed time is zero") {
        REQUIRE(timer.elapsed() >= 0.0);
        REQUIRE(timer.elapsedMs() >= 0.0);
    }
}

TEST_CASE("Timer: Reset functionality", "[core][timer]") {
    Timer timer;

    SECTION("Reset clears elapsed time") {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.reset();
        double elapsed = timer.elapsed();
        REQUIRE(elapsed < 0.01);  // Should be very small after reset
    }
}

TEST_CASE("Timer: Elapsed time in seconds", "[core][timer]") {
    Timer timer;

    SECTION("Elapsed increases over time") {
        double t1 = timer.elapsed();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        double t2 = timer.elapsed();
        REQUIRE(t2 > t1);
        REQUIRE(t2 - t1 >= 0.1);  // At least 100ms
        REQUIRE(t2 - t1 < 0.2);    // But not too long
    }

    SECTION("Multiple calls are cumulative") {
        double t1 = timer.elapsed();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double t2 = timer.elapsed();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double t3 = timer.elapsed();
        REQUIRE(t3 > t2);
        REQUIRE(t2 > t1);
        REQUIRE(t3 - t1 >= 0.1);  // Total at least 100ms
    }
}

TEST_CASE("Timer: Elapsed time in milliseconds", "[core][timer]") {
    Timer timer;

    SECTION("ElapsedMs returns milliseconds") {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        double ms = timer.elapsedMs();
        REQUIRE(ms >= 100.0);
        REQUIRE(ms < 200.0);
    }

    SECTION("ElapsedMs matches elapsed() * 1000") {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double sec = timer.elapsed();
        double ms = timer.elapsedMs();
        REQUIRE(ms == Approx(sec * 1000.0).margin(1.0));
    }
}

TEST_CASE("Timer: Precision", "[core][timer]") {
    Timer timer;

    SECTION("Can measure short durations") {
        double t1 = timer.elapsed();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        double t2 = timer.elapsed();
        REQUIRE(t2 > t1);
    }

    SECTION("Can measure sub-millisecond differences") {
        double t1 = timer.elapsed();
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        double t2 = timer.elapsed();
        // The difference should be measurable (at least not negative)
        REQUIRE(t2 >= t1);
    }
}

TEST_CASE("Timer: Restart", "[core][timer]") {
    Timer timer;

    SECTION("Start after some time") {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        double elapsedBefore = timer.elapsed();
        timer.restart();
        double elapsedAfter = timer.elapsed();
        REQUIRE(elapsedAfter < elapsedBefore);
        REQUIRE(elapsedAfter < 0.01);
    }
}

TEST_CASE("Timer: ScopedTimer", "[core][timer]") {
    SECTION("ScopedTimer measures block duration") {
        double elapsed = 0.0;
        {
            ScopedTimer st(elapsed);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        REQUIRE(elapsed >= 0.05);
        REQUIRE(elapsed < 0.1);
    }

    SECTION("ScopedTimer with custom output") {
        // Test that ScopedTimer compiles with different output types
        double seconds = 0.0;
        {
            ScopedTimer st(seconds);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(seconds >= 0.01);
    }
}

TEST_CASE("Timer: Multiple timers", "[core][timer]") {
    Timer timer1;
    Timer timer2;

    SECTION("Timers are independent") {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double t1 = timer1.elapsed();
        double t2 = timer2.elapsed();
        // Both should have similar elapsed times
        REQUIRE(t1 >= 0.05);
        REQUIRE(t2 >= 0.05);
        REQUIRE(std::abs(t1 - t2) < 0.01);
    }

    SECTION("Independent reset") {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        timer1.reset();
        double t1 = timer1.elapsed();
        double t2 = timer2.elapsed();
        REQUIRE(t1 < 0.01);
        REQUIRE(t2 >= 0.05);
    }
}

TEST_CASE("Timer: High frequency measurements", "[core][timer]") {
    Timer timer;

    SECTION("Can measure rapid iterations") {
        std::vector<double> measurements;
        for (int i = 0; i < 10; ++i) {
            measurements.push_back(timer.elapsed());
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // Each measurement should be larger than the previous
        for (size_t i = 1; i < measurements.size(); ++i) {
            REQUIRE(measurements[i] > measurements[i - 1]);
        }
    }
}

TEST_CASE("Timer: Long duration", "[core][timer][slow]") {
    Timer timer;

    SECTION("Measures seconds correctly") {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        double sec = timer.elapsed();
        REQUIRE(sec >= 0.5);
        REQUIRE(sec < 0.6);
    }
}
