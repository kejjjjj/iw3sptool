#pragma once

#include "pch.hpp"

DWORD Sys_MilliSeconds();

ivec2 GetCPos();
bool MouseHovered(const ivec2& mins, const ivec2& maxs);
bool ValidNumber(const std::string_view& expr);
bool IsInteger(const std::string_view& expr);
bool IsHex(char c);


Pixel generateRainbowColor();

float random(const float range); //0 -> HI
float random(const float min, const float range); //LO -> HI
inline auto GetTime() { return std::chrono::system_clock::now(); }
template<typename t>
t TimeDifference(const std::chrono::time_point<std::chrono::system_clock>& old, const std::chrono::time_point<std::chrono::system_clock>& current)
{
	return (std::chrono::duration<t>(current - old)).count();
}