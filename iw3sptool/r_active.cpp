

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
		sprintf_s(buff, "%.6f\n%.6f\n%.6f\n%.6f", ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1]);
		R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, 40, 0.4f, 0.5f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });
	}
	if (cm_entityInfo->current.enabled) {
		
		for (auto& a : gameEntities::getInstance())
			a->render2d(cm_showCollisionDist->current.value);

	}
		


	return detour_func.cast_call<void(__cdecl*)()>();

}