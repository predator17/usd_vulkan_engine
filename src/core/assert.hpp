#pragma once

#include <string>
#include <sstream>

namespace Assert {

inline void assertTrue(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        std::ostringstream oss;
        oss << "Assertion failed: " << expr << " at " << file << ":" << line;
        throw std::runtime_error(oss.str());
    }
}

inline void assertFalse(bool condition, const char* expr, const char* file, int line) {
    if (condition) {
        std::ostringstream oss;
        oss << "Assertion failed: !" << expr << " at " << file << ":" << line;
        throw std::runtime_error(oss.str());
    }
}

} // namespace Assert

#define USD_TRUE(x) Assert::assertTrue((x), #x, __FILE__, __LINE__)
#define USD_FALSE(x) Assert::assertFalse((x), #x, __FILE__, __LINE__)
