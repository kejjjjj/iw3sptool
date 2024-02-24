#include "pch.hpp"
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
	__asm
	{
		lea eax, [esp + 28h];
		push eax;
		push ebx;
		call PM_OverBounce;
		add esp, 0x8;
		pop edi;
		pop esi;
		pop ebp;
		pop ebx;
		add esp, 98h;
		retn;
	}
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

	}
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
void PM_OverBounce(pmove_t* pm, pml_t* pml)
{
	vec3_t move{};

	move[0] = pm->ps->origin[0] - pml->previous_origin[0];
	move[1] = pm->ps->origin[1] - pml->previous_origin[1];
	move[2] = pm->ps->origin[2] - pml->previous_origin[2];

	float dot = move[2] * move[2] + move[1] * move[1] + move[0] * move[0];
	float dot_div_frametime = dot / (pml->frametime * pml->frametime);
	float dot_speed = pm->ps->velocity[2] * pm->ps->velocity[2] + pm->ps->velocity[1] * pm->ps->velocity[1] + pm->ps->velocity[0] * pm->ps->velocity[0];

	if (dot_speed * 0.25 > dot_div_frametime)
	{
		float inGameFramesPerSecond = 1.0f / pml->frametime;
		pm->ps->velocity[0] = inGameFramesPerSecond * move[0];
		pm->ps->velocity[1] = inGameFramesPerSecond * move[1];
		pm->ps->velocity[2] = inGameFramesPerSecond * move[2];
	}

	float clampedFrametime = std::clamp(pml->frametime, 0.f, 1.f);

	float diffX = pm->ps->velocity[0] - pm->ps->oldVelocity[0];
	float diffY = pm->ps->velocity[1] - pm->ps->oldVelocity[1];

	float frameX = clampedFrametime * diffX;
	float frameY = clampedFrametime * diffY;

	pm->ps->oldVelocity[0] = pm->ps->oldVelocity[0] + frameX;
	pm->ps->oldVelocity[1] = pm->ps->oldVelocity[1] + frameY;

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

	return;
}
float CG_GetPlayerHitboxHeight(playerState_s* ps)
{
	std::unordered_map<int, int> stance = { {60, 70}, {40, 50}, {11, 30} };
	float maxs = static_cast<float>(stance.find(int(ps->viewHeightTarget))->second);

	return maxs;
}