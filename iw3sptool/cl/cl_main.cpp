#include <cg/cg_offsets.hpp>
#include <cg/cg_local.hpp>
#include "cm/cm_brush.hpp"
#include <utils/functions.hpp>
#include "utils/hook.hpp"
#include "com/com_channel.hpp"
#include "cl_main.hpp"
#include "dvar/dvar.hpp"

void CL_Disconnect()
{

	CClipMap::ClearThreadSafe();
	CGentities::ClearThreadSafe();

	return ((void(*)())0x444F10)();

}
void Map_Restart_f()
{

	CClipMap::ClearThreadSafe();
	CGentities::ClearThreadSafe();

	return engine_call<void>(0x005C4E30, 1);

}
void Fast_Restart_f()
{

	CClipMap::ClearThreadSafe();
	CGentities::ClearThreadSafe();

	return engine_call<void>(0x005C4E30, 0);

}

#include "tas/tas.hpp"
#include "tas/json_helpers.hpp"
#include "fs_utilities.hpp"

void LoadTasScript_f()
{
	int num_args = cmd_args->argc[cmd_args->nesting];
	if (num_args < 2) {
		Com_Printf(CON_CHANNEL_CONSOLEONLY, "Usage: loadscript <filename>\n");
		return;
	}

	const char* filename = *(cmd_args->argv[cmd_args->nesting] + 1);
	try {

		TAS::Script script = TAS::LoadScript(fs::exe_path() + "\\iw3sptool\\tas\\" + filename + ".json");
		std::vector<TAS::CompiledCommand> commands = TAS::Compile(script);

		g_playback_script = TAS::Player{ TAS::PlaybackScript{ std::move(script.header), std::move(commands) } };
		g_playback_script->Predict();
	}
	catch (const std::exception& e) {
		Com_Printf("^1Error loading script: %s\n", e.what());
	}
}
void StartTasScript_f() {
	if (!g_playback_script || g_playback_script->Active()) {
		Com_Printf("No active TAS script to start.\n");
		return;
	}

	g_playback_script->Start();
}
#include "cg/cg_angles.hpp"
void CL_CreateNewCommands() {

	if (g_playback_script && g_playback_script->Active()) {
		g_playback_script->Update();
		return;
	}

	hooktable::find<void>(HOOK_PREFIX(__func__))->call();

	std::int32_t& cmdNumber = *reinterpret_cast<std::int32_t*>(0x869534);
	usercmd_s* cmds = reinterpret_cast<usercmd_s*>(8816436);
	auto& cmd = cmds[(cmdNumber)&CMD_MASK];

	if (const auto strafebot = Dvar_FindMalleableVar("tas_strafebot"); strafebot && strafebot->current.enabled) {
		pmove_t* pm = (pmove_t*)0x7FDE88;
		if (const auto optYaw = TAS::Player::GetOptYaw(pm, &cmd, predictedPlayerState, 1000 / *reinterpret_cast<int32_t*>(0x815ECC))) {
			CL_SetPlayerYaw(&cmd, predictedPlayerState->delta_angles, predictedPlayerState->viewangles[YAW] + *optYaw);
		}
	}

}