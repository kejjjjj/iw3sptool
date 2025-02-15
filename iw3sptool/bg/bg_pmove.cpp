
#include "bg_pmove.hpp"
#include "dvar/dvar.hpp"
#include "utils/functions.hpp"
#include "utils/hook.hpp"

#include <cmath>
#include <unordered_map>
#include <algorithm>

void Pmove(pmove_t* pm)
{
	playerState_s* ps = pm->ps;
	int pm_flags = pm->ps->pm_flags;

	if ((pm_flags & PMF_FROZEN) != 0)
	{
		pm->cmd.buttons &= 0x300u;
	}
	else
	{
		if ((pm_flags & PMF_RESPAWNED) != 0) {
			pm->cmd.rightmove = 0;
			pm->cmd.forwardmove = 0;
			pm->cmd.buttons &= 0x301u;
		}
	}

	int msec = 0;

	static dvar_s* pm_fixed = Dvar_FindMalleableVar("pm_fixed");
	static dvar_s* com_maxfps = Dvar_FindMalleableVar("com_maxfps");


	int cur_msec = 1000 / (com_maxfps->current.integer == 0 ? 1 : com_maxfps->current.integer);

	if (pm_fixed->current.enabled == false)
		cur_msec = 66;
	else
		pm->cmd.serverTime = ((pm->cmd.serverTime + (cur_msec < 2 ? 2 : cur_msec) - 1) / cur_msec) * cur_msec;

	int finalTime = pm->cmd.serverTime;

	if (finalTime < pm->ps->commandTime) {
		return; // should not happen
	}


	if (finalTime > ps->commandTime + 1000)
		ps->commandTime = finalTime - 1000;
	pm->numtouch = 0;

	while (pm->ps->commandTime != finalTime)
	{
		msec = finalTime - ps->commandTime;

		if (msec > cur_msec)
			msec = cur_msec;
				
		pm->cmd.serverTime = msec + ps->commandTime;
		engine_call<void>(0x5BCBB0, pm);
		memcpy(&pm->oldcmd, &pm->cmd, sizeof(pm->oldcmd));
	}
		
	
}
__declspec(naked) void PmoveSingleASM()
{
	constexpr static auto foliagesnd = 0x5BAD60;
	constexpr static auto jmpaddr = 0x5BD303;
	__asm
	{

		add esp, 14h;
		mov eax, ebx;
		call foliagesnd;

		push ebx;
		call PM_FoliageSnd;
		add esp, 4;
		jmp jmpaddr;
	}
}
void Sys_SnapVector(float* v) {

	int i{};
	float f{};

	f = *v;
	__asm fld f;
	__asm fistp i;
	*v = static_cast<float>(i);
	v++;
	f = *v;
	__asm fld f;
	__asm fistp i;
	*v = static_cast<float>(i);
	v++;
	f = *v;
	__asm fld f;
	__asm fistp i;
	*v = static_cast<float>(i);


}
void PM_FoliageSnd(pmove_t* pm)
{
	static dvar_s* pm_bounceFix = Dvar_FindMalleableVar("pm_bounceFix");
	static dvar_s* pm_multiplayer = Dvar_FindMalleableVar("pm_multiplayer");

	if (pm_bounceFix->current.enabled) {
		static float previousZ = pm->ps->jumpOriginZ;

		if (pm->ps->jumpOriginZ != NULL)
			previousZ = pm->ps->jumpOriginZ;

		pm->ps->pm_flags = pm->ps->pm_flags & 0xFFFFFE7F | PMF_JUMPING; //reset bouncing flags
		pm->ps->jumpOriginZ = previousZ;
	}

	if (pm_multiplayer->current.enabled)
		Sys_SnapVector(pm->ps->velocity);

}


void PM_OverBounce(pmove_t* pm, pml_t* pml)
{
	vec3_t move{};

	move[0] = pm->ps->origin[0] - pml->previous_origin[0];
	move[1] = pm->ps->origin[1] - pml->previous_origin[1];
	move[2] = pm->ps->origin[2] - pml->previous_origin[2];

	const auto dot = move[2] * move[2] + move[1] * move[1] + move[0] * move[0];
	const auto dot_div_frametime = dot / (pml->frametime * pml->frametime);
	const auto dot_speed = pm->ps->velocity[2] * pm->ps->velocity[2] + pm->ps->velocity[1] * pm->ps->velocity[1] + pm->ps->velocity[0] * pm->ps->velocity[0];

	if (dot_speed * 0.25 > dot_div_frametime)
	{
		const auto inGameFramesPerSecond = 1.0f / pml->frametime;
		pm->ps->velocity[0] = inGameFramesPerSecond * move[0];
		pm->ps->velocity[1] = inGameFramesPerSecond * move[1];
		pm->ps->velocity[2] = inGameFramesPerSecond * move[2];
	}

	const auto clampedFrametime = (1.f - pml->frametime < 0) ? 1.f : pml->frametime;

	const auto diffX = pm->ps->velocity[0] - pm->ps->oldVelocity[0];
	const auto diffY = pm->ps->velocity[1] - pm->ps->oldVelocity[1];

	const auto frameX = clampedFrametime * diffX;
	const auto frameY = clampedFrametime * diffY;

	pm->ps->oldVelocity[0] = pm->ps->oldVelocity[0] + frameX;
	pm->ps->oldVelocity[1] = pm->ps->oldVelocity[1] + frameY;

	return;
}
float CG_GetPlayerHitboxHeight(playerState_s* ps)
{
	std::unordered_map<int, int> stance = { {60, 70}, {40, 50}, {11, 30} };
	float maxs = static_cast<float>(stance.find(int(ps->viewHeightTarget))->second);

	return maxs;
}

void PM_SprintFix(playerState_s* ps, pmove_t* pm)
{
	static dvar_s* pm_fixed = Dvar_FindMalleableVar("pm_fixed");

	//fixes a sprint bug when you can't reach target fps with pm_fixed

	if (pm_fixed->current.enabled == false) {

		__asm {
			mov ecx, pm;
			mov eax, ps;
			mov esi, 0x5B7890;
			call esi;
		}

	}else
		ps->sprintState.sprintButtonUpRequired = 1;
}
__declspec(naked) void PM_SprintFixASM()
{
	__asm
	{
		push ebp;
		push edi;
		call PM_SprintFix;
		add esp, 0x8;
		pop edi;
		pop esi;
		pop ebp;
		pop ebx;
		retn;
	}
}