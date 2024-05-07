#pragma once

#include "global_macros.hpp"
#include <algorithm>
#include <utility>
#include <chrono>
#include <format>
#include <string>
#include <typedefs.hpp>
#include <windows.h>


template<typename Return, typename ... Args>
inline Return engine_call(const unsigned long offset, Args... args)
{
	return (reinterpret_cast<Return(*)(Args...)>(offset))(args...);
}
inline std::string get_current_date() {
	SYSTEMTIME st;
	GetLocalTime(&st);


	return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

inline std::string base64_decode(const std::string& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4]{}, char_array_3[3]{};
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0x0F) << 4) + ((char_array_4[2] & 0x3C) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0x0F) << 4) + ((char_array_4[2] & 0x3C) >> 2);

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}


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

inline constexpr float ScaleByDistance(float dist)
{
	constexpr float d_max = 10000.0;
	constexpr float scale_max = 7.f;

	dist = std::max(0.0f, std::min(d_max, dist));

	const float scale = 2.f - scale_max * (dist / (d_max));

	return std::clamp(scale, 0.1f, 2.f);


}