#include "json_helpers.hpp"

#include <fstream>

[[nodiscard]] std::array<float, 3> ParseVec3(const TAS::json& value) { 
    if (!value.is_array() || value.size() != 3) 
        throw std::runtime_error("\"viewangles\" must contain exactly 3 values"); 
    
    return { value[0].get<float>(), value[1].get<float>(), value[2].get<float>() }; 
}

[[nodiscard]] TAS::TasHeader TAS::ParseHeader(const json& value) {
	TasHeader header;
	if (value.contains("startOrigin")) {
		header.startOrigin = ParseVec3(value["startOrigin"]);
	}
	if (value.contains("startViewangles")) {
		header.startViewangles = ParseVec3(value["startViewangles"]);
	}
	return header;
}

[[nodiscard]]
int TAS::FindButton(std::string_view name)
{
    for (const auto& button : ButtonMap)
    {
        if (button.name == name)
            return button.value;
    }

    throw std::runtime_error(
        "Unknown TAS button: " + std::string(name));
}

[[nodiscard]]
int TAS::ParseButtons(const json& value)
{
    if (value.is_null())
        return 0;

    if (!value.is_array())
        throw std::runtime_error("\"buttons\" must be an array");

    int result = 0;

    for (const auto& button : value) {
        if (!button.is_string())
            throw std::runtime_error("TAS button must be a string");

        result |= FindButton(button.get<std::string>());
    }

    return result;
}

[[nodiscard]]
TAS::AngleCommand TAS::ParseAngle(const json& value) {
    AngleCommand result{};

    if (!value.is_object())
        throw std::runtime_error("\"angle\" must be an object");

    if (!value.contains("type"))
        throw std::runtime_error("\"angle\" requires a \"type\"");

    const auto type = value["type"].get<std::string>();

    if (type == "none")
        result.type = AngleType::None;
    else if (type == "absolute")
        result.type = AngleType::Absolute;
    else if (type == "delta")
        result.type = AngleType::Delta;
    else if(type == "strafebot")
		result.type = AngleType::Strafebot;
    else
        throw std::runtime_error("Unknown angle type: " + type);

    if (result.type == AngleType::Strafebot)
        return result;

    if (!value.contains("value"))
        throw std::runtime_error("\"angle\" requires a \"value\"");

    const auto& angles = value["value"];

    if (!angles.is_array() || angles.size() != 3)
        throw std::runtime_error("\"angle.value\" must contain exactly 3 values");

    result.angles =
    {
        angles[0].get<float>(),
        angles[1].get<float>(),
        angles[2].get<float>()
    };

    return result;
}

[[nodiscard]]
int8_t TAS::ParseMove(const json& value)
{
    const auto result = value.get<int>();

    if (result < -127 || result > 127)
        throw std::runtime_error(
            "Movement value must be between -127 and 127");

    return static_cast<int8_t>(result);
}

[[nodiscard]]
TAS::CTasSegment TAS::ParseSegment(const json& value)
{
    CTasSegment segment;

    if (!value.contains("duration"))
        throw std::runtime_error(
            "TAS segment is missing \"duration\"");

    segment.durationMs =
        value["duration"].get<int32_t>();

    if (segment.durationMs < 0)
        throw std::runtime_error("TAS segment duration cannot be negative");

    segment.buttons = ParseButtons(value.value("buttons", json::array()));

    if (value.contains("angle"))
        segment.angle = ParseAngle(value["angle"]);
    else
        segment.angle = {};

    segment.commandRate =
        value.value(
            "fps",
            DefaultCommandRate);

    if (segment.commandRate < MinCommandRate || segment.commandRate > MaxCommandRate)
    {
        throw std::runtime_error(
            "TAS fps must be between " +
            std::to_string(MinCommandRate) +
            " and " +
            std::to_string(MaxCommandRate));
    }

    segment.forwardmove = ParseMove(value.value("forwardmove", 0));
    segment.rightmove = ParseMove(value.value("rightmove", 0));

    segment.weapon = value.value("weapon", "none");
    segment.offHand = value.value("offhand", "none");

    return segment;
}

[[nodiscard]]
TAS::Script TAS::ParseScript(const json& root) {
    TAS::Script script;

    script.header = ParseHeader(root);

    if (!root.contains("segments"))
        throw std::runtime_error("TAS file is missing \"segments\"");

    if (!root["segments"].is_array())
        throw std::runtime_error("\"segments\" must be an array");

    for (const auto& value : root["segments"])
        script.segments.push_back(ParseSegment(value));

    return script;
}

#include <iostream>

[[nodiscard]]
TAS::Script TAS::LoadScript(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
        throw std::runtime_error("Could not open TAS file: " + filename);

    json root = json::parse(file, nullptr, true, true);

    return TAS::ParseScript(root);
}