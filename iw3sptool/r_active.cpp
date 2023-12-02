

#include "pch.hpp"

void CG_DrawActive()
{
	decltype(auto) detour_func = find_hook(hookEnums_e::HOOK_DRAWACTIVE);



	auto ps = predictedPlayerState;

	OnDvarsModified();
	//char buff[256];
	//sprintf_s(buff, "%.6f\n%.6f\n%.6f\n%.6f\n%d\n%.1f", ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1], (int)(fvec2(ps->velocity[0], ps->velocity[1]).mag()), cgs->predictedPlayerState.viewHeightCurrent);
	
	//decltype(auto) ents = gameEntities::getInstance();

	//sprintf_s(buff, "active entities: %i (%i)", ents.spawned_entities, Sys_MilliSeconds());
	//
	//R_DrawTextWithEffects(buff, "fonts/bigdevFont", 25, 350, 0.5f, 0.6f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });



	return detour_func.cast_call<void(__cdecl*)()>();

}