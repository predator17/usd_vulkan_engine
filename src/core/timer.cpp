#include "timer.hpp"
#include <cstdio>

namespace usd_ve {

Timer::Timer() {
    start();
}

void Timer::start() {
    m_start = std::chrono::high_resolution_clock::now();
    m_running = true;
}

void Timer::stop() {
    m_end = std::chrono::high_resolution_clock::now();
    m_running = false;
}

void Timer::reset() {
    m_start = std::chrono::high_resolution_clock::now();
    m_running = true;
}

void Timer::restart() {
    reset();
}

double Timer::elapsed() const {
    auto end = m_running ? std::chrono::high_resolution_clock::now() : m_end;
    return std::chrono::duration<double>(end - m_start).count();
}

double Timer::elapsedMs() const {
    return elapsed() * 1000.0;
}

ScopedTimer::ScopedTimer(const char* name)
    : m_name(name) {
    m_timer.start();
}

ScopedTimer::ScopedTimer(double& output)
    : m_output(&output) {
    m_timer.start();
}

ScopedTimer::~ScopedTimer() {
    m_timer.stop();
    if (m_output) {
        *m_output = m_timer.elapsed();
    } else if (m_name) {
        std::printf("%s: %.3f ms\n", m_name, m_timer.elapsedMs());
    }
}

} // namespace usd_ve
