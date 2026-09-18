#pragma once

#include "tas.hpp"

namespace TAS {

	[[nodiscard]] TasHeader ParseHeader(const json& value);
	[[nodiscard]] int FindButton(std::string_view name);
	[[nodiscard]] int ParseButtons(const json& value);
	TAS::AngleCommand ParseAngle(const json& value);
	[[nodiscard]] int8_t ParseMove(const json& value);
	[[nodiscard]] CTasSegment ParseSegment(const json& value);
	[[nodiscard]] Script ParseScript(const json& root);
	[[nodiscard]] Script LoadScript(const std::string& filename);
}