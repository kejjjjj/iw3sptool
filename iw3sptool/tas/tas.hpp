#pragma once

#include <array>

#include "bg/bg_pmove.hpp"
#include "cg/cg_local.hpp"

#include <nlohmann/json.hpp> 

namespace TAS {

	using json = nlohmann::json;

	constexpr int32_t DefaultCommandRate = 250;
	constexpr int32_t MinCommandRate = 1;
	constexpr int32_t MaxCommandRate = 1000;
	constexpr float AngleToShort = 182.0444488525390625f;
	constexpr float ShortToAngle = 1.0f / AngleToShort;

	struct ButtonDefinition {
		std::string_view name;
		std::int32_t value;
	};

	constexpr ButtonDefinition ButtonMap[] = {
		{ "attack",         cmdEnums::fire },
		{ "melee",          cmdEnums::melee },
		{ "use",            cmdEnums::activate },
		{ "reload",         cmdEnums::reload },

		{ "lean_left",      cmdEnums::lean_left },
		{ "lean_right",     cmdEnums::lean_right },

		{ "prone",          cmdEnums::prone },
		{ "crouch",         cmdEnums::crouch },
		{ "jump",           cmdEnums::jump },

		{ "ads_toggle_on",  cmdEnums::ads_toggle_on },
		{ "ads_toggle_off", cmdEnums::ads_toggle_off },

		{ "crouch_hold",    cmdEnums::crouch_hold },
		{ "prone_hold",     cmdEnums::prone_hold },

		{ "steady_aim",     cmdEnums::steady_aim },
		{ "sprint",         cmdEnums::sprint },

		{ "frag",           cmdEnums::frag },
		{ "special_grenade",cmdEnums::special_grenade },

		{ "ads_hold",       cmdEnums::ads_hold },

		{ "no_input",       cmdEnums::no_input },
	};

	enum class AngleType { None, Absolute, Delta, Strafebot };
	struct AngleCommand {
		AngleType type = AngleType::None;
		std::array<float, 3> angles{};
	};

	struct CTasSegment {
		int32_t durationMs = 0;
		int buttons = 0;
		AngleCommand angle;
		int32_t commandRate = DefaultCommandRate;
		int8_t forwardmove = 0;
		int8_t rightmove = 0;
		std::string weapon;
		std::string offHand;
	};

	struct CompiledCommand {
		usercmd_s cmd{};
		int32_t time = 0;
		bool strafebot{};
		int32_t targetFps{125};
	};

	struct TasHeader
	{
		std::optional<std::array<float, 3>> startOrigin;
		std::optional<std::array<float, 3>> startViewangles;
	};

	struct PlaybackScript {
		TasHeader header;
		std::vector<CompiledCommand> m_commands;
	};

	struct Script { 
		TasHeader header;
		std::vector<CTasSegment> segments; 
	};
	struct CommandMeta {
		bool strafebot = false;
		int targetFps = 0;
	};

	std::vector<CompiledCommand> Compile(const Script& script);

    class Player
    {
		friend void ::Pmove(pmove_t* pm);
    public:
		Player(const PlaybackScript& script) : m_script(script) {}

        void Start();
        void Stop();
		void Predict();

        [[nodiscard]]
		bool Active() const { return m_active; }

        void Update();

		static std::optional<float> GetOptYaw(pmove_t* pm, const usercmd_s* cmd, const playerState_s* ps, int32_t fps);
		static void PredictPlayerState(playerState_s* ps, usercmd_s* cmd, const usercmd_s* oldCmd);
		std::vector<fvec3> m_predictedPositions;

		std::optional<CommandMeta> FindCommandMeta(int32_t serverTime) const {
			const auto it = m_commandMeta.find(serverTime);
			
			if (it == m_commandMeta.end())
				return std::nullopt;

			return it->second;
		}

    private:

		void InjectNext();
		static int32_t GetRealtime();
		usercmd_s BuildNextCommand(const CompiledCommand& compiled, const playerState_s* state);

    private:

		PlaybackScript m_script;

        size_t m_commandIndex = 0;
        int32_t m_baseServerTime = 0;
		int32_t m_startRealtime = 0;

		bool m_started = false;
        bool m_active = false;

		std::unordered_map<int32_t, CommandMeta> m_commandMeta;
	};

}