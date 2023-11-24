

#include "pch.hpp"

void CG_DrawActive()
{
	decltype(auto) detour_func = find_hook(hookEnums_e::HOOK_DRAWACTIVE);


	char buff[256];

	auto ps = predictedPlayerState;
	if (!ps)
		return;

	if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		IWonderIfThisWorks();

	sprintf_s(buff, "%.6f\n%.6f\n%.6f\n%.6f\n%d", ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1], (int)(fvec2(ps->velocity[0], ps->velocity[1]).mag()));

	R_DrawTextWithEffects(buff, "fonts/normalfont", 0, 25, 0.3f, 0.4f, 0, vec4_t{ 1,1,1,0.7f }, 3, vec4_t{ 1,0,0,0 });

	return detour_func.cast_call<void(__cdecl*)()>();

}