

#include "pch.hpp"

void CG_DrawActive()
{
	decltype(auto) detour_func = find_hook(hookEnums_e::HOOK_DRAWACTIVE);
	static dvar_s* cm_entityInfo = Dvar_FindMalleableVar("cm_entityInfo");
	static dvar_s* cm_showCollisionDist = Dvar_FindMalleableVar("cm_showCollisionDist");
	static dvar_s* pm_coordinates = Dvar_FindMalleableVar("pm_coordinates");


	auto ps = predictedPlayerState;

	OnDvarsModified();

	if (pm_coordinates->current.enabled) {
		char buff[256];
		sprintf_s(buff, 
			"x:     %.6f\n"
			"y:     %.6f\n"
			"z:     %.6f\n"
			"yaw: %.6f", ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1]);
		R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, 40, 0.3f, 0.4f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });
	}
	if (cm_entityInfo->current.enabled) {
		
		for (auto& a : gameEntities::getInstance())
			a->render2d(cm_showCollisionDist->current.value);

	}
	//
	//trace_t trace;

	//fvec3 org = predictedPlayerState->origin;
	//org.z += CG_GetPlayerHitboxHeight(predictedPlayerState);

	//fvec3 end = fvec3(predictedPlayerState->viewangles).toforward() * 9999 + org;
	//CG_Trace(&trace, org, vec3_t{ -1,-1,-1 }, vec3_t{ 1,1,1 }, end, predictedPlayerState->clientNum, MASK_PLAYERSOLID);

	//char buff[256];
	//sprintf_s(buff, "%.6f\n%.6f\n%.6f", trace.normal[0], trace.normal[1], trace.normal[2]);
	//R_DrawTextWithEffects(buff, "fonts/bigdevFont", 200, 40, 0.4f, 0.5f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });

	return detour_func.cast_call<void(__cdecl*)()>();

}