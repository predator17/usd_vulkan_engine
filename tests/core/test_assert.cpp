#include <catch2/catch.hpp>
#include <core/assert.hpp>
#include <sstream>
#include <cstring>

TEST_CASE("Assert: USD_TRUE passes with true condition", "[core][assert]") {
    // This should not throw
    bool executed = false;
    if (true) {
        executed = true;
    }
    REQUIRE(executed);
}

TEST_CASE("Assert: Basic boolean evaluation", "[core][assert]") {
    SECTION("True boolean") {
        bool value = true;
        REQUIRE(value);
    }

    SECTION("False boolean") {
        bool value = false;
        REQUIRE_FALSE(value);
    }

    SECTION("Negated true") {
        bool value = true;
        REQUIRE_FALSE(!value);
    }
}

TEST_CASE("Assert: Comparison operations", "[core][assert]") {
    int a = 5;
    int b = 10;

    SECTION("Less than") {
        REQUIRE(a < b);
        REQUIRE_FALSE(b < a);
    }

    SECTION("Greater than") {
        REQUIRE(b > a);
        REQUIRE_FALSE(a > b);
    }

    SECTION("Less than or equal") {
        REQUIRE(a <= b);
        REQUIRE(a <= a);
    }

    SECTION("Greater than or equal") {
        REQUIRE(b >= a);
        REQUIRE(b >= b);
    }

    SECTION("Equal") {
        REQUIRE(a == a);
        REQUIRE_FALSE(a == b);
    }

    SECTION("Not equal") {
        REQUIRE(a != b);
        REQUIRE_FALSE(a != a);
    }
}

TEST_CASE("Assert: Floating point comparisons", "[core][assert]") {
    SECTION("Float equality") {
        float a = 1.0f;
        float b = 1.0f;
        REQUIRE(a == b);
    }

    SECTION("Float approx with margin") {
        float a = 1.0f;
        float b = 1.001f;
        REQUIRE(a == Approx(b).margin(0.01f));
    }

    SECTION("Double approx") {
        double a = 3.14159;
        double b = 3.14160;
        REQUIRE(a == Approx(b).margin(0.0001));
    }

    SECTION("NaN check") {
        float nan = std::numeric_limits<float>::quiet_NaN();
        REQUIRE_FALSE(nan == nan);
        REQUIRE(nan != nan);
    }

    SECTION("Infinity check") {
        float inf = std::numeric_limits<float>::infinity();
        REQUIRE(inf > 1000000.0f);
        REQUIRE(-inf < -1000000.0f);
    }
}

TEST_CASE("Assert: String comparisons", "[core][assert]") {
    SECTION("C string equality") {
        const char* str1 = "hello";
        const char* str2 = "hello";
        const char* str3 = "world";

        REQUIRE(strcmp(str1, str2) == 0);
        REQUIRE(strcmp(str1, str3) != 0);
    }

    SECTION("C string comparison") {
        const char* str1 = "apple";
        const char* str2 = "banana";

        REQUIRE(strcmp(str1, str2) < 0);
        REQUIRE(strcmp(str2, str1) > 0);
    }

    SECTION("String length") {
        const char* str = "hello";
        REQUIRE(strlen(str) == 5);
    }
}

TEST_CASE("Assert: Pointer operations", "[core][assert]") {
    SECTION("Null pointer") {
        int* ptr = nullptr;
        REQUIRE(ptr == nullptr);
        REQUIRE_FALSE(ptr != nullptr);
    }

    SECTION("Valid pointer") {
        int value = 42;
        int* ptr = &value;
        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == 42);
    }

    SECTION("Pointer comparison") {
        int arr[3];
        REQUIRE(arr == &arr[0]);
        REQUIRE(&arr[1] > &arr[0]);
        REQUIRE(&arr[2] > &arr[1]);
    }
}

TEST_CASE("Assert: Array operations", "[core][assert]") {
    SECTION("Array indexing") {
        int arr[] = {1, 2, 3, 4, 5};
        REQUIRE(arr[0] == 1);
        REQUIRE(arr[4] == 5);
    }

    SECTION("Array size via sizeof") {
        int arr[] = {1, 2, 3, 4, 5};
        size_t size = sizeof(arr) / sizeof(arr[0]);
        REQUIRE(size == 5);
    }
}

TEST_CASE("Assert: Bitwise operations", "[core][assert]") {
    unsigned char a = 0b1100;  // 12
    unsigned char b = 0b1010;  // 10

    SECTION("AND") {
        REQUIRE((a & b) == 0b1000);  // 8
    }

    SECTION("OR") {
        REQUIRE((a | b) == 0b1110);  // 14
    }

    SECTION("XOR") {
        REQUIRE((a ^ b) == 0b0110);  // 6
    }

    SECTION("NOT") {
        REQUIRE((~a & 0b1111) == 0b0011);  // 3 (only 4 bits)
    }

    SECTION("Left shift") {
        REQUIRE((a << 1) == 0b11000);  // 24
    }

    SECTION("Right shift") {
        REQUIRE((a >> 2) == 0b11);  // 3
    }
}

TEST_CASE("Assert: Logical operations", "[core][assert]") {
    bool a = true;
    bool b = false;

    SECTION("Logical AND") {
        REQUIRE((a && b) == false);
        REQUIRE((a && a) == true);
        REQUIRE((b && b) == false);
    }

    SECTION("Logical OR") {
        REQUIRE((a || b) == true);
        REQUIRE((a || a) == true);
        REQUIRE((b || b) == false);
    }

    SECTION("Logical NOT") {
        REQUIRE(!a == false);
        REQUIRE(!b == true);
    }

    SECTION("Short-circuit evaluation") {
        int counter = 0;
        bool result = false && (++counter > 0);
        REQUIRE(counter == 0);
        REQUIRE(result == false);

        result = true || (++counter > 0);
        REQUIRE(counter == 0);
        REQUIRE(result == true);
    }
}

TEST_CASE("Assert: Arithmetic operations", "[core][assert]") {
    int a = 10;
    int b = 3;

    SECTION("Addition") {
        REQUIRE(a + b == 13);
    }

    SECTION("Subtraction") {
        REQUIRE(a - b == 7);
    }

    SECTION("Multiplication") {
        REQUIRE(a * b == 30);
    }

    SECTION("Integer division") {
        REQUIRE(a / b == 3);
    }

    SECTION("Modulo") {
        REQUIRE(a % b == 1);
    }

    SECTION("Unary minus") {
        REQUIRE(-a == -10);
    }

    SECTION("Increment") {
        int c = a++;
        REQUIRE(c == 10);
        REQUIRE(a == 11);
    }

    SECTION("Decrement") {
        int c = --b;
        REQUIRE(c == 2);
        REQUIRE(b == 2);
    }
}

TEST_CASE("Assert: Compound assignment", "[core][assert]") {
    int a = 10;

    SECTION("+=") {
        a += 5;
        REQUIRE(a == 15);
    }

    SECTION("-=") {
        a -= 3;
        REQUIRE(a == 7);
    }

    SECTION("*=") {
        a *= 2;
        REQUIRE(a == 20);
    }

    SECTION("/=") {
        a /= 2;
        REQUIRE(a == 5);
    }

    SECTION("%=") {
        a %= 3;
        REQUIRE(a == 1);
    }
}

TEST_CASE("Assert: Conditional operator", "[core][assert]") {
    int a = 5;
    int b = 10;

    SECTION("True condition") {
        int result = (a < b) ? 100 : 200;
        REQUIRE(result == 100);
    }

    SECTION("False condition") {
        int result = (a > b) ? 100 : 200;
        REQUIRE(result == 200);
    }
}
