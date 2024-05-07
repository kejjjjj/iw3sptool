#include "functions.hpp"
#include <random>
#include <timeapi.h>

DWORD Sys_MilliSeconds()
{
    return timeGetTime() - *(DWORD*)0x13E39A4; //sys_basetime
}

ivec2 GetCPos()
{
    tagPOINT p;
    GetCursorPos(&p);

    return { p.x, p.y };
}
bool MouseHovered(const ivec2& mins, const ivec2& maxs)
{
    tagPOINT p;
    GetCursorPos(&p);

    return
        p.x > mins.x && p.x < maxs.x
        && p.y > mins.y && p.y < maxs.y;
}
bool ValidNumber(const std::string_view& expr)
{
    size_t dot_count{ 0 }, dot_idx, index{ 0 };

    if (expr.size() == 1) {
        if (!std::isalnum(expr[0]) && expr[0] != '_')
            return false;
    }

    for (const auto& i : expr) {
        switch (i) {

        case '.': //has a decimal
            dot_count++;

            if (dot_count > 1)
                return false;
            dot_idx = index;
            break;
        case '-': //is a negative number

            if (index != 0)  //only the first character can be this
                return false;

            break;

        default:

            if (!std::isdigit(i))
                return false;
            break;
        }
        index++;

    }



    if (dot_count) {

        if (dot_idx == expr.size() - 1) //last character cannot be a dot
            return false;

        if (!std::isdigit(expr[dot_idx + 1])) //next character is not a number
            return false;
    }


    return true;
}
bool IsInteger(const std::string_view& expr)
{
    return ValidNumber(expr) && expr.find('.') == std::string_view::npos; //no decimal
}
bool IsHex(char c)
{
    return (c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F') || std::isdigit(c);
}

Pixel generateRainbowColor()
{
    const auto HSVtoRGB = [](float h, float s, float v) -> Pixel {
        int i = int(h * 6);
        float f = h * 6 - i;
        float p = v * (1 - s);
        float q = v * (1 - f * s);
        float t = v * (1 - (1 - f) * s);

        switch (i % 6) {
        case 0: return { uint8_t(v * 255), uint8_t(t * 255), uint8_t(p * 255), 255 };
        case 1: return { uint8_t(q * 255), uint8_t(v * 255), uint8_t(p * 255), 255 };
        case 2: return { uint8_t(p * 255), uint8_t(v * 255), uint8_t(t * 255), 255 };
        case 3: return { uint8_t(p * 255), uint8_t(q * 255), uint8_t(v * 255), 255 };
        case 4: return { uint8_t(t * 255), uint8_t(p * 255), uint8_t(v * 255), 255 };
        case 5: return { uint8_t(v * 255), uint8_t(p * 255), uint8_t(q * 255), 255 };
        default: return { 0, 0, 0, 255 };
        }
        };

    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    float hue = fmod(elapsed / 5000.0f, 1.0f);
    return HSVtoRGB(hue, 1.0f, 1.0f);
}

float random(const float range) { //0 -> HI
    std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution num{ 0.f, range };
    return num(mt);

}
float random(const float min, const float range) { //LO -> HI
    //std::random_device rd;
    static std::mt19937 mt;
    std::uniform_real_distribution num{ min, range };
    return num(mt);
}