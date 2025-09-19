#include "cg/cg_offsets.hpp"
#include "cl/cl_keymove.hpp"
#include "cm/cm_entity.hpp"
#include "cm/cm_renderer.hpp"
#include "com/com_channel.hpp"
#include "dvar/dvar.hpp"
#include "r_debug.hpp"
#include "r_drawtools.hpp"
#include "r_active.hpp"
#include "utils/hook.hpp"

#include <iostream>

volatile int CGDebugData::tessVerts{};
volatile int CGDebugData::tessIndices{};

static void CG_DrawCoordinates(float& y)
{
	auto ps = predictedPlayerState;

	char buff[256];
	sprintf_s(buff,
		"x:       %.6f\n"
		"y:       %.6f\n"
		"z:       %.6f\n"
		"yaw:   %.6f\n"
		"pitch: %.6f", ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1], ps->viewangles[0]);
	R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, y, 0.3f, 0.4f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });

	y += 50.f;
}
static void CG_DrawVelocity(float& y)
{
	auto ps = predictedPlayerState;

	char buff[256];
	sprintf_s(buff,
		"x:     %.1f\n"
		"y:     %.1f\n"
		"z:     %.1f\n"
		"xy:   %.1f\n",
		std::fabsf(ps->velocity[0]), std::fabsf(ps->velocity[1]), std::fabsf(ps->velocity[2]), fvec2(ps->velocity).mag());

	R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, y, 0.3f, 0.4f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });

	y += 40.f;
}

static void CG_DrawDebug(float& y)
{

	char buff[256];
	sprintf_s(buff,
		"verts:   %i\n"
		"indices: %i\n",
		CGDebugData::tessVerts, CGDebugData::tessIndices);

	R_DrawTextWithEffects(buff, "fonts/bigdevFont", 0, y, 0.3f, 0.4f, 0, vec4_t{ 1,0.753f,0.796f,0.7f }, 3, vec4_t{ 1,0,0,0 });

	y += 40.f;
}

void CG_DrawActive()
{
	static dvar_s* cm_entityInfo = Dvar_FindMalleableVar("cm_entityInfo");
	static dvar_s* cm_showCollisionDist = Dvar_FindMalleableVar("cm_showCollisionDist");
	static dvar_s* pm_coordinates = Dvar_FindMalleableVar("pm_coordinates");
	static dvar_s* pm_velocity = Dvar_FindMalleableVar("pm_velocity");
	static dvar_s* pm_debug = Dvar_FindMalleableVar("pm_debug");

	OnDvarsModified();

	float y = 40.f;

	if (pm_coordinates->current.enabled) {
		CG_DrawCoordinates(y);
	}

	if (pm_velocity->current.enabled) {
		CG_DrawVelocity(y);
	}

	if (pm_debug->current.enabled) {
		CG_DrawDebug(y);
	}
	if (cm_entityInfo->current.integer) {
		
		CGentities::ForEach([](GentityPtr_t& ptr) {
			ptr->CG_Render2D(cm_showCollisionDist->current.value, static_cast<entity_info_type>(cm_entityInfo->current.integer)); 
		});

	}


	return hooktable::find<void>(HOOK_PREFIX(__func__))->call();

}

