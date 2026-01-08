#include "color.hpp"
#include <sstream>
#include <iomanip>
#include <charconv>
#include <algorithm>
#include <cmath>

namespace usd_ve {

Color::Color(float r, float g, float b, float a)
    : m_r(std::clamp(r, 0.0f, 1.0f)),
      m_g(std::clamp(g, 0.0f, 1.0f)),
      m_b(std::clamp(b, 0.0f, 1.0f)),
      m_a(std::clamp(a, 0.0f, 1.0f)) {}

Color::Color(const glm::vec3& rgb)
    : m_r(std::clamp(rgb.r, 0.0f, 1.0f)),
      m_g(std::clamp(rgb.g, 0.0f, 1.0f)),
      m_b(std::clamp(rgb.b, 0.0f, 1.0f)),
      m_a(1.0f) {}

Color::Color(const glm::vec4& rgba)
    : m_r(std::clamp(rgba.r, 0.0f, 1.0f)),
      m_g(std::clamp(rgba.g, 0.0f, 1.0f)),
      m_b(std::clamp(rgba.b, 0.0f, 1.0f)),
      m_a(std::clamp(rgba.a, 0.0f, 1.0f)) {}

Color Color::fromHex(uint32_t hex) {
    float r = ((hex >> 24) & 0xFF) / 255.0f;
    float g = ((hex >> 16) & 0xFF) / 255.0f;
    float b = ((hex >> 8) & 0xFF) / 255.0f;
    float a = (hex & 0xFF) / 255.0f;
    return Color(r, g, b, a);
}

Color Color::fromHex(const std::string& hex) {
    uint32_t value = 0;
    if (hex.empty()) return Color();

    std::string h = hex;
    if (h[0] == '#') h = h.substr(1);

    if (h.length() == 6) {
        h += "FF";
    }

    std::from_chars(h.data(), h.data() + h.size(), value, 16);
    return fromHex(value);
}

uint32_t Color::toHex() const {
    uint32_t r = static_cast<uint32_t>(std::round(std::clamp(m_r, 0.0f, 1.0f) * 255.0f));
    uint32_t g = static_cast<uint32_t>(std::round(std::clamp(m_g, 0.0f, 1.0f) * 255.0f));
    uint32_t b = static_cast<uint32_t>(std::round(std::clamp(m_b, 0.0f, 1.0f) * 255.0f));
    uint32_t a = static_cast<uint32_t>(std::round(std::clamp(m_a, 0.0f, 1.0f) * 255.0f));
    return (r << 24) | (g << 16) | (b << 8) | a;
}

float Color::luminance() const {
    // Rec. 709 luminance coefficients
    return 0.2126f * m_r + 0.7152f * m_g + 0.0722f * m_b;
}

Color Color::toLinear() const {
    glm::vec3 linear = glm::convertSRGBToLinear(toRGB());
    return Color(linear.r, linear.g, linear.b, m_a);
}

Color Color::toSRGB() const {
    glm::vec3 srgb = glm::convertLinearToSRGB(toRGB());
    return Color(srgb.r, srgb.g, srgb.b, m_a);
}

Color Color::lerp(const Color& a, const Color& b, float t) {
    return Color(
        a.m_r + (b.m_r - a.m_r) * t,
        a.m_g + (b.m_g - a.m_g) * t,
        a.m_b + (b.m_b - a.m_b) * t,
        a.m_a + (b.m_a - a.m_a) * t
    );
}

Color Color::clamp(const Color& c) {
    return Color(
        std::clamp(c.m_r, 0.0f, 1.0f),
        std::clamp(c.m_g, 0.0f, 1.0f),
        std::clamp(c.m_b, 0.0f, 1.0f),
        std::clamp(c.m_a, 0.0f, 1.0f)
    );
}

Color Color::gray(float value) {
    return Color(value, value, value);
}

Color Color::red() {
    return Color(1.0f, 0.0f, 0.0f);
}

Color Color::green() {
    return Color(0.0f, 1.0f, 0.0f);
}

Color Color::blue() {
    return Color(0.0f, 0.0f, 1.0f);
}

Color Color::white() {
    return Color(1.0f, 1.0f, 1.0f);
}

Color Color::black() {
    return Color(0.0f, 0.0f, 0.0f);
}

} // namespace usd_ve
