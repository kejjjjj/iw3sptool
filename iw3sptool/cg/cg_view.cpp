#include "cg_trace.hpp"
#include "cg_view.hpp"
#include "cg_local.hpp"
#include "cg_offsets.hpp"
#include "cm/cm_renderer.hpp"
#include "dvar/dvar.hpp"
#include "utils/hook.hpp"
#include "bg/bg_pmove.hpp"
#include "cm/cm_brush.hpp"
#include "com/com_vector.hpp"
#include "global_macros.hpp"
#include "r/rb_endscene.hpp"

void CG_OffsetFirstPersonView(cg_s* _cgs)
{
	if (Dvar_FindMalleableVar("cg_thirdPerson")->current.enabled)
		return;

	return hooktable::find<void, cg_s*>(HOOK_PREFIX(__func__))->call(_cgs);

}
__declspec(naked) void CG_OffsetThirdPersonViewASM()
{
	static const DWORD perturb_camera = 0x42A340;
	static const DWORD _jmp = 0x42E1D0;
	__asm
	{
		call CG_OffsetThirdPersonView;
		mov edi, 0x6FA590;
		call perturb_camera;
		pop edi;
		pop esi;
		add esp, 0xC;
		jmp _jmp;

	}
}
void CG_OffsetThirdPersonView()
{
	if (Dvar_FindMalleableVar("cg_thirdPerson")->current.enabled == false)
		return;

	fvec3 focusAngles;
	fvec3 focusPoint;
	fvec3 view;
	fvec3 forward, right;
	static fvec3 mins = { -4, -4, -4 };
	static fvec3 maxs = { 4, 4, 4 };
	trace_t trace;
	
	cgs->refdef.vieworg[2] += cgs->predictedPlayerState.viewHeightCurrent;
	VectorCopy(cgs->refdefViewAngles, focusAngles);

	//if (focusAngles[PITCH] > 45) {
	//	focusAngles[PITCH] = 45;        // don't go too far overhead
	//}

	forward = focusAngles.toforward();

	VectorMA(cgs->refdef.vieworg, 512, forward, focusPoint);

	VectorCopy(cgs->refdef.vieworg, view);

	view[2] += 8;
	cgs->refdefViewAngles[PITCH] *= 0.5;

	right = focusAngles.toright();

	float cg_thirdPersonAngle = Dvar_FindMalleableVar("cg_thirdPersonAngle")->current.value;
	float cg_thirdPersonRange = Dvar_FindMalleableVar("cg_thirdPersonRange")->current.value;

	float forwardScale = cos(cg_thirdPersonAngle/ 180.f * M_PI);
	float sideScale = sin(cg_thirdPersonAngle / 180.f * M_PI);
	VectorMA(view, -cg_thirdPersonRange * forwardScale, forward, view);
	VectorMA(view, -cg_thirdPersonRange * sideScale, right, view);

	CG_Trace(&trace, cgs->refdef.vieworg, mins, maxs, view, cgs->predictedPlayerState.clientNum, 2065);
	
	if (trace.fraction != 1.f) {
		fvec3 hitpos = fvec3(cgs->refdef.vieworg) + ((view - fvec3(cgs->refdef.vieworg)) * trace.fraction);
		VectorCopy(hitpos, view);
		view[2] += (1.0f - trace.fraction) * 32.f;

		CG_Trace(&trace, cgs->refdef.vieworg, mins, maxs, view, cgs->predictedPlayerState.clientNum, 2065);
		hitpos = fvec3(cgs->refdef.vieworg) + ((view - fvec3(cgs->refdef.vieworg)) * trace.fraction);
		VectorCopy(hitpos, view);

	}

	VectorCopy(view, cgs->refdef.vieworg);

	VectorSubtract(focusPoint, cgs->refdef.vieworg, focusPoint);
	float focusDist = sqrt(focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1]);
	if (focusDist < 1) {
		focusDist = 1;  // should never happen
	}
	cgs->refdefViewAngles[PITCH] = -180 / M_PI * atan2(focusPoint[2], focusDist);
	cgs->refdefViewAngles[YAW] -= cg_thirdPersonAngle;

	AnglesToAxis(cgs->refdefViewAngles, cgs->refdef.viewaxis);
}

void RB_RenderPlayerHitboxes()
{
	if (Dvar_FindMalleableVar("cg_thirdPerson")->current.enabled == false)
		return;

	std::unordered_map<int, int> stance = { {60, 70}, {40, 50}, {11, 30} };

	fvec3 o = cgs->predictedPlayerState.origin;

	fvec3 mins = { o.x - 14.f, o.y - 14.f, o.z };
	fvec3 maxs = { o.x + 14.f, o.y + 14.f, o.z + CG_GetPlayerHitboxHeight(&cgs->predictedPlayerState) };

	auto v = CM_CreateHitbox(mins, maxs);

	CM_DrawCollisionEdges(v, vec4_t{1,1,0,0.7f}, true);

}

void CG_UpdateViewWeaponAnim()
{

	if (*(bool*)0x85BC80)
		return;

	return hooktable::find<void>(HOOK_PREFIX(__func__))->call();

}