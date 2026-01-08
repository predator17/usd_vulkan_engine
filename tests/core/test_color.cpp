#include <catch2/catch.hpp>
#include <core/color.hpp>
#include <cmath>

using namespace usd_vk;

TEST_CASE("Color: Default construction", "[core][color]") {
    Color color;

    SECTION("Default is black") {
        REQUIRE(color.getR() == 0.0f);
        REQUIRE(color.getG() == 0.0f);
        REQUIRE(color.getB() == 0.0f);
        REQUIRE(color.getA() == 1.0f);  // Default alpha is 1.0
    }

    SECTION("Initial center is zero") {
        REQUIRE(color.toRGB() == glm::vec3(0.0f));
    }

    SECTION("Initial radius is zero") {
        REQUIRE(color.toRGBA() == glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

TEST_CASE("Color: RGB construction", "[core][color]") {
    Color color(0.5f, 0.75f, 1.0f);

    REQUIRE(color.getR() == 0.5f);
    REQUIRE(color.getG() == 0.75f);
    REQUIRE(color.getB() == 1.0f);
    REQUIRE(color.getA() == 1.0f);  // Default alpha
}

TEST_CASE("Color: RGBA construction", "[core][color]") {
    Color color(0.2f, 0.4f, 0.6f, 0.8f);

    REQUIRE(color.getR() == 0.2f);
    REQUIRE(color.getG() == 0.4f);
    REQUIRE(color.getB() == 0.6f);
    REQUIRE(color.getA() == 0.8f);
}

TEST_CASE("Color: Grayscale construction", "[core][color]") {
    Color color = Color::gray(0.5f);

    REQUIRE(color.getR() == 0.5f);
    REQUIRE(color.getG() == 0.5f);
    REQUIRE(color.getB() == 0.5f);
    REQUIRE(color.getA() == 1.0f);
}

TEST_CASE("Color: From hex uint32_t", "[core][color]") {
    SECTION("Opaque red") {
        Color color = Color::fromHex(0xFF0000FF);
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Opaque green") {
        Color color = Color::fromHex(0x00FF00FF);
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(1.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Opaque blue") {
        Color color = Color::fromHex(0x0000FFFF);
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(1.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Semi-transparent white") {
        Color color = Color::fromHex(0xFFFFFF80);
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(1.0f));
        REQUIRE(color.getB() == Approx(1.0f));
        REQUIRE(color.getA() == Approx(0.5f).margin(0.01f));  // 0x80 / 255 ≈ 0.5
    }

    SECTION("Black") {
        Color color = Color::fromHex(0x000000FF);
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Gray") {
        Color color = Color::fromHex(0x808080FF);
        REQUIRE(color.getR() == Approx(0.5f).margin(0.01f));
        REQUIRE(color.getG() == Approx(0.5f).margin(0.01f));
        REQUIRE(color.getB() == Approx(0.5f).margin(0.01f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Yellow (RGB mix)") {
        Color color = Color::fromHex(0xFFFF00FF);
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(1.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }
}

TEST_CASE("Color: From hex string", "[core][color]") {
    SECTION("With # prefix, 6 digits") {
        Color color = Color::fromHex("#FF0000");
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Without # prefix, 6 digits") {
        Color color = Color::fromHex("00FF00");
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(1.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("With # prefix, 8 digits with alpha") {
        Color color = Color::fromHex("#FF000080");
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(0.5f).margin(0.01f));
    }

    SECTION("Without # prefix, 8 digits with alpha") {
        Color color = Color::fromHex("000000FF");
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Lowercase hex string") {
        Color color = Color::fromHex("#ff00ff");
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(1.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }

    SECTION("Mixed case hex string") {
        Color color = Color::fromHex("#Ff00Ff");
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(1.0f));
        REQUIRE(color.getA() == Approx(1.0f));
    }
}

TEST_CASE("Color: To hex", "[core][color]") {
    SECTION("Red to hex") {
        Color color(1.0f, 0.0f, 0.0f, 1.0f);
        REQUIRE(color.toHex() == 0xFF0000FF);
    }

    SECTION("Green to hex") {
        Color color(0.0f, 1.0f, 0.0f, 1.0f);
        REQUIRE(color.toHex() == 0x00FF00FF);
    }

    SECTION("Blue to hex") {
        Color color(0.0f, 0.0f, 1.0f, 1.0f);
        REQUIRE(color.toHex() == 0x0000FFFF);
    }

    SECTION("White to hex") {
        Color color(1.0f, 1.0f, 1.0f, 1.0f);
        REQUIRE(color.toHex() == 0xFFFFFFFF);
    }

    SECTION("Black to hex") {
        Color color(0.0f, 0.0f, 0.0f, 1.0f);
        REQUIRE(color.toHex() == 0x000000FF);
    }

    SECTION("Semi-transparent red to hex") {
        Color color(1.0f, 0.0f, 0.0f, 0.5f);
        uint32_t expected = 0xFF000080;  // Alpha is 0x80 (128/255 ≈ 0.5)
        REQUIRE(color.toHex() == expected);
    }

    SECTION("Gray to hex") {
        Color color(0.5f, 0.5f, 0.5f, 1.0f);
        // 0.5 * 255 = 127.5 -> 127 or 128
        uint32_t hex = color.toHex();
        uint8_t grayVal = (hex >> 24) & 0xFF;
        REQUIRE(grayVal >= 127);
        REQUIRE(grayVal <= 128);
    }
}

TEST_CASE("Color: Luminance (Rec. 709)", "[core][color]") {
    SECTION("Black luminance") {
        Color color(0.0f, 0.0f, 0.0f);
        REQUIRE(color.luminance() == Approx(0.0f));
    }

    SECTION("White luminance") {
        Color color(1.0f, 1.0f, 1.0f);
        REQUIRE(color.luminance() == Approx(1.0f));
    }

    SECTION("Pure red luminance") {
        Color color(1.0f, 0.0f, 0.0f);
        // Rec. 709: R * 0.2126
        REQUIRE(color.luminance() == Approx(0.2126f));
    }

    SECTION("Pure green luminance") {
        Color color(0.0f, 1.0f, 0.0f);
        // Rec. 709: G * 0.7152
        REQUIRE(color.luminance() == Approx(0.7152f));
    }

    SECTION("Pure blue luminance") {
        Color color(0.0f, 0.0f, 1.0f);
        // Rec. 709: B * 0.0722
        REQUIRE(color.luminance() == Approx(0.0722f));
    }

    SECTION("Gray luminance") {
        Color color(0.5f, 0.5f, 0.5f);
        float expected = 0.5f * (0.2126f + 0.7152f + 0.0722f);
        REQUIRE(color.luminance() == Approx(expected));
    }

    SECTION("Yellow luminance (red + green)") {
        Color color(1.0f, 1.0f, 0.0f);
        REQUIRE(color.luminance() == Approx(0.2126f + 0.7152f));
    }

    SECTION("Cyan luminance (green + blue)") {
        Color color(0.0f, 1.0f, 1.0f);
        REQUIRE(color.luminance() == Approx(0.7152f + 0.0722f));
    }
}

TEST_CASE("Color: Linear to sRGB conversion", "[core][color]") {
    SECTION("Zero remains zero") {
        Color linear(0.0f, 0.0f, 0.0f);
        Color srgb = linear.toSRGB();
        REQUIRE(srgb.getR() == Approx(0.0f));
        REQUIRE(srgb.getG() == Approx(0.0f));
        REQUIRE(srgb.getB() == Approx(0.0f));
    }

    SECTION("One remains one") {
        Color linear(1.0f, 1.0f, 1.0f);
        Color srgb = linear.toSRGB();
        REQUIRE(srgb.getR() == Approx(1.0f));
        REQUIRE(srgb.getG() == Approx(1.0f));
        REQUIRE(srgb.getB() == Approx(1.0f));
    }

    SECTION("Middle gray (0.5 linear) is darker in sRGB") {
        Color linear(0.5f, 0.5f, 0.5f);
        Color srgb = linear.toSRGB();
        // sRGB gamma curve: 0.5 linear ≈ 0.735 sRGB (for linear portion)
        // or for pow portion: 0.5^(1/2.4) ≈ 0.735
        REQUIRE(srgb.getR() == Approx(0.7354f).margin(0.01f));
        REQUIRE(srgb.getG() == Approx(0.7354f).margin(0.01f));
        REQUIRE(srgb.getB() == Approx(0.7354f).margin(0.01f));
    }

    SECTION("Low values (linear portion of curve)") {
        Color linear(0.01f, 0.01f, 0.01f);
        Color srgb = linear.toSRGB();
        // GLM's convertLinearToSRGB uses the power portion for most values
        // 0.01 linear -> approximately 0.10 sRGB
        REQUIRE(srgb.getR() == Approx(0.10f).margin(0.01f));
    }

    SECTION("Alpha is preserved") {
        Color linear(0.5f, 0.5f, 0.5f, 0.75f);
        Color srgb = linear.toSRGB();
        REQUIRE(srgb.getA() == Approx(0.75f));
    }
}

TEST_CASE("Color: sRGB to linear conversion", "[core][color]") {
    SECTION("Zero remains zero") {
        Color srgb(0.0f, 0.0f, 0.0f);
        Color linear = srgb.toLinear();
        REQUIRE(linear.getR() == Approx(0.0f));
        REQUIRE(linear.getG() == Approx(0.0f));
        REQUIRE(linear.getB() == Approx(0.0f));
    }

    SECTION("One remains one") {
        Color srgb(1.0f, 1.0f, 1.0f);
        Color linear = srgb.toLinear();
        REQUIRE(linear.getR() == Approx(1.0f));
        REQUIRE(linear.getG() == Approx(1.0f));
        REQUIRE(linear.getB() == Approx(1.0f));
    }

    SECTION("Middle gray (0.5 sRGB) is lighter in linear") {
        Color srgb(0.5f, 0.5f, 0.5f);
        Color linear = srgb.toLinear();
        // 0.5 sRGB ^ 2.4 ≈ 0.215 linear
        REQUIRE(linear.getR() == Approx(0.215f).margin(0.01f));
        REQUIRE(linear.getG() == Approx(0.215f).margin(0.01f));
        REQUIRE(linear.getB() == Approx(0.215f).margin(0.01f));
    }

    SECTION("Alpha is preserved") {
        Color srgb(0.5f, 0.5f, 0.5f, 0.75f);
        Color linear = srgb.toLinear();
        REQUIRE(linear.getA() == Approx(0.75f));
    }
}

TEST_CASE("Color: Lerp (linear interpolation)", "[core][color]") {
    SECTION("Lerp at t=0 returns first color") {
        Color c1(1.0f, 0.0f, 0.0f);
        Color c2(0.0f, 1.0f, 0.0f);
        Color result = Color::lerp(c1, c2, 0.0f);
        REQUIRE(result.getR() == Approx(1.0f));
        REQUIRE(result.getG() == Approx(0.0f));
        REQUIRE(result.getB() == Approx(0.0f));
    }

    SECTION("Lerp at t=1 returns second color") {
        Color c1(1.0f, 0.0f, 0.0f);
        Color c2(0.0f, 1.0f, 0.0f);
        Color result = Color::lerp(c1, c2, 1.0f);
        REQUIRE(result.getR() == Approx(0.0f));
        REQUIRE(result.getG() == Approx(1.0f));
        REQUIRE(result.getB() == Approx(0.0f));
    }

    SECTION("Lerp at t=0.5 returns average") {
        Color c1(1.0f, 0.0f, 0.0f);
        Color c2(0.0f, 1.0f, 0.0f);
        Color result = Color::lerp(c1, c2, 0.5f);
        REQUIRE(result.getR() == Approx(0.5f));
        REQUIRE(result.getG() == Approx(0.5f));
        REQUIRE(result.getB() == Approx(0.0f));
    }

    SECTION("Lerp with different colors") {
        Color c1(0.2f, 0.4f, 0.6f);
        Color c2(0.8f, 0.6f, 0.4f);
        Color result = Color::lerp(c1, c2, 0.25f);
        REQUIRE(result.getR() == Approx(0.35f));  // 0.2 + 0.25*(0.8-0.2) = 0.35
        REQUIRE(result.getG() == Approx(0.45f));  // 0.4 + 0.25*(0.6-0.4) = 0.45
        REQUIRE(result.getB() == Approx(0.55f));  // 0.6 + 0.25*(0.4-0.6) = 0.55
    }

    SECTION("Lerp extrapolates for t > 1 (values are clamped to 1.0)") {
        Color c1(0.0f, 0.0f, 0.0f);
        Color c2(1.0f, 1.0f, 1.0f);
        Color result = Color::lerp(c1, c2, 2.0f);
        // lerp gives 2.0, but constructor clamps to 1.0
        REQUIRE(result.getR() == Approx(1.0f));
        REQUIRE(result.getG() == Approx(1.0f));
        REQUIRE(result.getB() == Approx(1.0f));
    }

    SECTION("Lerp extrapolates for t < 0 (values are clamped to 0.0)") {
        Color c1(0.5f, 0.5f, 0.5f);
        Color c2(1.0f, 1.0f, 1.0f);
        Color result = Color::lerp(c1, c2, -0.5f);
        // lerp gives 0.25, which is >= 0 so no clamping
        REQUIRE(result.getR() == Approx(0.25f));
        REQUIRE(result.getG() == Approx(0.25f));
        REQUIRE(result.getB() == Approx(0.25f));
    }

    SECTION("Lerp with alpha") {
        Color c1(1.0f, 0.0f, 0.0f, 1.0f);
        Color c2(0.0f, 0.0f, 0.0f, 0.0f);
        Color result = Color::lerp(c1, c2, 0.5f);
        REQUIRE(result.getR() == Approx(0.5f));
        REQUIRE(result.getG() == Approx(0.0f));
        REQUIRE(result.getB() == Approx(0.0f));
        REQUIRE(result.getA() == Approx(0.5f));
    }
}

TEST_CASE("Color: Clamping", "[core][color]") {
    SECTION("Values are clamped to [0, 1] on construction") {
        // Color constructor should clamp values
        // This tests implicit behavior
        Color c1(-0.5f, 1.5f, 0.5f);
        REQUIRE(c1.getR() >= 0.0f);
        REQUIRE(c1.getG() <= 1.0f);
    }
}

TEST_CASE("Color: Named colors", "[core][color]") {
    SECTION("Red") {
        Color color = Color::red();
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
    }

    SECTION("Green") {
        Color color = Color::green();
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(1.0f));
        REQUIRE(color.getB() == Approx(0.0f));
    }

    SECTION("Blue") {
        Color color = Color::blue();
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(1.0f));
    }

    SECTION("White") {
        Color color = Color::white();
        REQUIRE(color.getR() == Approx(1.0f));
        REQUIRE(color.getG() == Approx(1.0f));
        REQUIRE(color.getB() == Approx(1.0f));
    }

    SECTION("Black") {
        Color color = Color::black();
        REQUIRE(color.getR() == Approx(0.0f));
        REQUIRE(color.getG() == Approx(0.0f));
        REQUIRE(color.getB() == Approx(0.0f));
    }
}
