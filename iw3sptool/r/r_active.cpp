#include "cg/cg_offsets.hpp"
#include "cl/cl_keymove.hpp"
#include "cm/cm_entity.hpp"
#include "cm/cm_renderer.hpp"
#include "com/com_channel.hpp"
#include "dvar/dvar.hpp"
#include "r_debug.hpp"
#include "r_drawtools.hpp"
#include "utils/hook.hpp"

#include <iostream>

void CG_DrawActive()
{
	static dvar_s* cm_entityInfo = Dvar_FindMalleableVar("cm_entityInfo");
	static dvar_s* cm_showCollisionDist = Dvar_FindMalleableVar("cm_showCollisionDist");
	static dvar_s* pm_coordinates = Dvar_FindMalleableVar("pm_coordinates");
	static dvar_s* pm_velocity = Dvar_FindMalleableVar("pm_velocity");

	auto ps = predictedPlayerState;

	OnDvarsModified();

	float y = 40.f;

	if (pm_coordinates->current.enabled) {
		char buff[256];
		sprintf_s(buff, 
			"x:     %.6f\n"
			"y:     %.6f\n"
			"z:     %.6f\n"
			"yaw: %.6f", ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1]);
		R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, y, 0.3f, 0.4f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });

		y += 40.f;
	}

	if (pm_velocity->current.enabled) {
		char buff[256];
		sprintf_s(buff,
			"x:     %.1f\n"
			"y:     %.1f\n"
			"z:     %.1f\n"
			"xy:   %.1f\n", 
			std::fabsf(ps->velocity[0]), std::fabsf(ps->velocity[1]), std::fabsf(ps->velocity[2]), fvec2(ps->velocity).mag());

		R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, y, 0.3f, 0.4f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });
	}
	if (cm_entityInfo->current.integer) {
		
		CGentities::ForEach([](GentityPtr_t& ptr) {
			ptr->CG_Render2D(cm_showCollisionDist->current.value, static_cast<entity_info_type>(cm_entityInfo->current.integer)); 
		});

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

	return hooktable::find<void>(HOOK_PREFIX(__func__))->call();

}

static void __Test();
__declspec(naked) void R_AddWorldSurfacesPortalWalk()
{
	__asm
	{
		call __Test;
		add esp, 0Ch;
		pop edi;
		pop esi;
		pop ebp;
		pop ebx;
		pop ecx;
		retn;
	}
}

void __Test()
{

	//static vec3_t mins = { 0, 0, 0 };
	//static vec3_t maxs = { 10, 10, 10 };
	//static vec4_t c = { 0, 1, 1, 1 };

	//R_AddDebugLine(mins, maxs, c);
	//R_AddDebugBox(mins, maxs, c);

	//std::vector<fvec3> pts = { { -100.f, 0.f, -100.f }, { 0.f, 0.f, 0.f }, { 100.f, 0.f, -100.f } };

	//R_AddDebugPolygon(pts, vec4_t{ 1,1,1,0.5f });

	//CM_ShowCollision(nullptr);
}
