#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <string>
#include <algorithm>
#include <cmath>

namespace usd_vk {

class Color {
public:
    Color() = default;
    Color(float r, float g, float b, float a = 1.0f);
    explicit Color(const glm::vec3& rgb);
    explicit Color(const glm::vec4& rgba);

    static Color fromHex(uint32_t hex);
    static Color fromHex(const std::string& hex);
    static Color gray(float value);
    static Color red();
    static Color green();
    static Color blue();
    static Color white();
    static Color black();

    glm::vec3 toRGB() const { return glm::vec3(m_r, m_g, m_b); }
    glm::vec4 toRGBA() const { return glm::vec4(m_r, m_g, m_b, m_a); }
    uint32_t toHex() const;

    float getR() const { return m_r; }
    float getG() const { return m_g; }
    float getB() const { return m_b; }
    float getA() const { return m_a; }

    float luminance() const;
    Color toLinear() const;
    Color toSRGB() const;

    static Color lerp(const Color& a, const Color& b, float t);
    static Color clamp(const Color& c);

    friend Color operator+(const Color& a, const Color& b);
    friend Color operator*(const Color& c, float s);

private:
    float m_r = 0.0f, m_g = 0.0f, m_b = 0.0f, m_a = 1.0f;
};

inline Color operator+(const Color& a, const Color& b) {
    return Color(a.m_r + b.m_r, a.m_g + b.m_g, a.m_b + b.m_b, a.m_a + b.m_a);
}

inline Color operator*(const Color& c, float s) {
    return Color(c.m_r * s, c.m_g * s, c.m_b * s, c.m_a * s);
}

} // namespace usd_vk
