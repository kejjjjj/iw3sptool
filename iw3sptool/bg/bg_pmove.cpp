
#include "bg_pmove.hpp"
#include "dvar/dvar.hpp"
#include "utils/functions.hpp"
#include "com/com_channel.hpp"
#include "com/com_vector.hpp"
#include "cg/cg_offsets.hpp"
#include "cl/cl_main.hpp"

#include <cmath>
#include <unordered_map>
#include <print>

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
	int cur_msec = 66;
	int finalTime = pm->cmd.serverTime;

	if (finalTime < pm->ps->commandTime) {
		return; // should not happen
	}


	if (finalTime > ps->commandTime + 1000)
		ps->commandTime = finalTime - 1000;
	pm->numtouch = 0;

	TAS::CommandMeta meta{};

	bool isTasStrafebot = false;

	if (g_playback_script && g_playback_script->Active()) {
		if (const auto commandMeta = g_playback_script->FindCommandMeta(finalTime)) {
			meta = *commandMeta;
			isTasStrafebot = meta.strafebot;
		}
	}

	while (pm->ps->commandTime != finalTime)
	{
		msec = finalTime - ps->commandTime;

		if (msec > cur_msec)
			msec = cur_msec;
				
		pm->cmd.serverTime = msec + ps->commandTime;

		if (isTasStrafebot) {
			if (const auto optYaw = TAS::Player::GetOptYaw(pm, &pm->cmd, pm->ps, meta.targetFps)) {
				pm->cmd.angles[YAW] = ANGLE2SHORT(ps->viewangles[YAW] + *optYaw + ps->delta_angles[YAW]);
			}
		}

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
	 dvar_s* pm_bounceFix = Dvar_FindMalleableVar("pm_bounceFix");
	 dvar_s* pm_multiplayer = Dvar_FindMalleableVar("pm_multiplayer");

	if (pm_bounceFix->current.enabled) {
		 float previousZ = pm->ps->jumpOriginZ;

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
	 dvar_s* pm_fixed = Dvar_FindMalleableVar("pm_fixed");

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

 void PM_UpdatePlayerWalkingFlag_(pmove_t* pm) {
	__asm
	{
		mov ecx, pm;
		mov esi, 0x5BC240;
		call esi;

	}
}
 void PM_FoliageSnd_(pmove_t* pm) {
	__asm
	{
		mov eax, pm;
		mov esi, 0x5BAD60;
		call esi;

	}
}
inline int PM_InteruptWeaponWithProneMove_(playerState_s* ps) {
	int rval = 0;
	__asm
	{
		mov eax, ps;
		mov esi, 0x5BF9F0;
		call esi;
		mov rval, eax;
	}
	return rval;
}
 void PM_ExitAimDownSight_(playerState_s* ps) {
	__asm
	{
		mov eax, ps;
		mov esi, 0x05BF560;
		call esi;
	}
}
void PM_Weapon_(pmove_t* pm, pml_t* pml)
{
	__asm
	{
		lea eax, pml;
		push eax;
		mov eax, pm;
		mov esi, 0x5C29D0;
		call esi;
		add esp, 0x4;
	}
}
 void PM_CheckDuck_(pmove_t* pm, pml_t* pml)
{
	__asm
	{
		lea edx, pml;
		push edx;
		mov edi, pm;
		mov esi, 0x5B9E50;
		call esi;
		add esp, 0x4;
	}
}
 void Mantle_Move(playerState_s* ps, pml_t* pml)
{
	__asm
	{
		push pml;
		mov edi, ps;
		mov esi, 0x5B3860;
		call esi;
		add esp, 0x4;
	}
}
 void PM_UpdatePronePitch_(pmove_t* pm, pml_t* pml) {
	__asm
	{
		mov eax, pm;
		mov edi, pml;
		mov esi, 0x5BBF70;
		call esi;
	}
}
 void PM_UpdateViewAngles(playerState_s* ps, float msec, usercmd_s* cmd, char handler) {

	__asm
	{
		movzx eax, handler;
		mov esi, cmd;
		push eax;
		push esi;
		push msec;
		mov eax, ps;
		mov esi, 0x5BBE60;
		call esi;
		add esp, 0xC;
	}

}

int PM_GetSprintLeft(playerState_s* ps, int time)
{
	int timeleft = 0;
	constexpr DWORD f = 0x5B7750;
	__asm
	{
		push time;
		mov esi, ps;
		call f;
		add esp, 0x4;
		mov timeleft, eax;
	}
	return timeleft;
}
bool Mantle_FindMantleSurface(pmove_t* pm, trace_t* trace, float* normal, float* fwd)
{
	dvar_s* mantle_check_radius = Dvar_FindMalleableVar("mantle_check_radius");
	dvar_s* mantle_check_range = Dvar_FindMalleableVar("mantle_check_range");
	dvar_s* mantle_debug = Dvar_FindMalleableVar("mantle_debug");
	dvar_s* mantle_check_angle = Dvar_FindMalleableVar("mantle_check_angle");

	playerState_s* v7; // edi
	int v8; // ecx
	char result; // al
	float angles[3]; // [esp+18h] [ebp-3Ch] BYREF
	float end[3]; // [esp+24h] [ebp-30h] BYREF
	float maxs[3]; // [esp+30h] [ebp-24h] BYREF
	float mins[3]; // [esp+3Ch] [ebp-18h] BYREF
	float start[3]; // [esp+48h] [ebp-Ch] BYREF
	float mantle_range; // [esp+58h] [ebp+4h]
	float mins_v; // [esp+5Ch] [ebp+8h]
	float prone_mins; // [esp+5Ch] [ebp+8h]
	float tracec; // [esp+5Ch] [ebp+8h]
	float traced; // [esp+5Ch] [ebp+8h]

	mins_v = -mantle_check_radius->current.value;
	mins[0] = mins_v;
	mins[1] = mins_v;
	v7 = pm->ps;
	mins[2] = 0.0f;
	maxs[0] = mantle_check_radius->current.value;
	maxs[1] = mantle_check_radius->current.value;
	maxs[2] = 70.0f;
	prone_mins = 15.0f - mantle_check_radius->current.value;
	mantle_range = mantle_check_range->current.value + prone_mins;
	angles[0] = *fwd;
	angles[1] = fwd[1];
	angles[2] = 0.0f;
	VectorNormalize(angles);
	v8 = pm->handler;
	tracec = -prone_mins;
	start[0] = tracec * angles[0] + v7->origin[0];
	start[1] = angles[1] * tracec + v7->origin[1];
	start[2] = tracec * angles[2] + v7->origin[2];
	end[0] = angles[0] * mantle_range + v7->origin[0];
	end[1] = angles[1] * mantle_range + v7->origin[1];
	end[2] = angles[2] * mantle_range + v7->origin[2];

	pmoveHandler_t* handler = reinterpret_cast<pmoveHandler_t*>(0x06DA590);

	handler->trace(trace, start, mins, maxs, end, v7->clientNum, 0x1000000);

	if (trace->startsolid || trace->allsolid)
	{
		if (mantle_debug->current.enabled)
			Com_Printf(CON_CHANNEL_PLAYERWEAP, "%s\n", "Mantle Failed: Mantle brush is too thick");
		return 0;
	}
	if (1.0 == trace->fraction)
	{
		if (mantle_debug->current.enabled)
			Com_Printf(CON_CHANNEL_PLAYERWEAP, "%s\n", "Mantle Failed: No mantle surface found");
		return 0;
	}
	if ((trace->surfaceFlags & 0x6000000) != 0)
	{
		*normal = -trace->normal[0];
		normal[1] = -trace->normal[1];
		normal[2] = 0.0;
		if (VectorNormalize(normal) >= 0.00009999999747378752)
		{
			traced = normal[1] * angles[1] + *normal * angles[0] + normal[2] * angles[2];
			if (mantle_check_angle->current.value >= acos(traced) * 57.2957763671875)
			{
				//Com_Printf(CON_CHANNEL_SUBTITLE, "Mantle range!\n");
				result = 1;
			}
			else
			{
				Com_Printf(CON_CHANNEL_CONSOLEONLY, "Mantle Failed: Player is not facing mantle surface\n");
				result = 0;
			}
		}
		else
		{
			Com_Printf(CON_CHANNEL_CONSOLEONLY, "Mantle Failed: Mantle surface has vertical normal\n");
			result = 0;
		}
	}
	else
	{
		Com_Printf(CON_CHANNEL_CONSOLEONLY, "Mantle Failed: No mantle surface with MANTLE_ON or MANTLE_OVER found\n");
		result = 0;
	}

	return result;
}
bool Mantle_CheckLedge(MantleResults* r, pmove_t* pm, float height)
{
	char res{};
	__asm
	{
		//fld height;
		push height;
		mov ecx, pm;
		mov eax, r;
		mov esi, 0x5B2EB0;
		call esi;
		add esp, 0x4;
		mov res, al;
	}
	return res;

	//return res;
}
void Mantle_Check(pmove_t* pm, pml_t* pml) {
	//	return Mantle_Check_f(pm, pml);

		//__asm mov pm, edi;
		//__asm mov pml, eax;
	playerState_s* ps;
	float normal[3]; // [esp+8h] [ebp-6Ch] BYREF
	trace_t trace;
	MantleResults mantle;

	pm->ps->mantleState.flags &= 0xFFFFFFEF;

	 dvar_s* mantle_enable = Dvar_FindMalleableVar("mantle_enable");

	if (mantle_enable->current.enabled) {

		ps = pm->ps;
		ps->mantleState.flags &= 0xFFFFFFF7;

		if (ps->pm_type < PM_DEAD)
		{
			if ((ps->pm_flags & PMF_MANTLE) != 0)
			{

			}
			else if ((ps->eFlags & 0xC) != 0) {

			}
			else
			{
				bool result = Mantle_FindMantleSurface(pm, &trace, normal, pml->forward);

				if (result)
				{
					memset(&mantle, 0, sizeof(MantleResults));
					mantle.dir[0] = normal[0];
					mantle.dir[1] = normal[1];
					mantle.dir[2] = normal[2];
					mantle.startPos[0] = ps->origin[0];
					mantle.startPos[1] = ps->origin[1];
					mantle.startPos[2] = ps->origin[2];
					if ((trace.surfaceFlags & 0x4000000) != 0)
						mantle.flags |= 1u;

					result = Mantle_CheckLedge(&mantle, pm, 60.f);
					if (!result) {
						result = Mantle_CheckLedge(&mantle, pm, 40.f);
						if (!result)
							result = Mantle_CheckLedge(&mantle, pm, 20.f);
					}
					//if (result) {
					//	Com_Printf(CON_CHANNEL_SUBTITLE, "very good!\n");
					//}
				}

			}

		}

	}

	return;
}
void SimulatePmove(pmove_t* pm) {
	playerState_s* ps = pm->ps;

	if (ps->pm_flags & PMF_FROZEN)
	{
		pm->cmd.buttons &= 0x300;
		pm->cmd.rightmove = 0;
		pm->cmd.forwardmove = 0;
	}
	else if (ps->pm_flags & PMF_RESPAWNED)
	{
		pm->cmd.buttons &= 0x301;
		pm->cmd.rightmove = 0;
		pm->cmd.forwardmove = 0;
	}

	const int finalTime = pm->cmd.serverTime;

	if (finalTime < ps->commandTime)
		return;

	if (finalTime > ps->commandTime + 1000)
		ps->commandTime = finalTime - 1000;

	pm->numtouch = 0;

	TAS::CommandMeta meta{};

	bool isTasStrafebot = false;

	if (g_playback_script && g_playback_script->Active()) {
		if (const auto commandMeta = g_playback_script->FindCommandMeta(finalTime)) {
			meta = *commandMeta;
			isTasStrafebot = meta.strafebot;
		}
	}

	while (ps->commandTime != finalTime) {

		const int msec = std::min(finalTime - ps->commandTime, 66);
		pm->cmd.serverTime = ps->commandTime + msec;

		if (isTasStrafebot) {
			if (const auto optYaw = TAS::Player::GetOptYaw(pm, &pm->cmd, pm->ps, meta.targetFps)) {
				pm->cmd.angles[YAW] = ANGLE2SHORT(ps->viewangles[YAW] + *optYaw + ps->delta_angles[YAW]);
			}
		}

		SimulatePmoveSingle(pm);
		pm->oldcmd = pm->cmd;
	}
}
void SimulatePmoveSingle(pmove_t* pm) {
	
	pml_t pml{};
	playerState_s* ps = pm->ps;
	static dvar_s* mantle_enable = Dvar_FindMalleableVar("mantle_enable");

	if ((pm->ps->pm_flags & PMF_MELEE_CHARGE) != 0)
		pm->cmd.forwardmove = 127;

	if ((ps->pm_flags & PMF_RESPAWNED) != 0)
	{
		pm->cmd.buttons &= 0x1301u;
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		ps->velocity[0] = 0.0;
		ps->velocity[1] = 0.0;
		ps->velocity[2] = 0.0;
	}
	if (ps->weaponstate == WEAPON_FIRING)
	{
		int weaponIdx = ps->weapon;
		if (weaponIdx)
		{
			if (BG_WeaponNames[weaponIdx]->freezeMovementWhenFiring)
			{
				pm->cmd.buttons &= 0xFFFFFB3F;
				pm->cmd.forwardmove = 0;
				pm->cmd.rightmove = 0;
				ps->velocity[0] = 0.0;
				ps->velocity[1] = 0.0;
				ps->velocity[2] = 0.0;
			}
		}
	}

	ps->pm_flags &= 0xFFFFEFFF;
	if (ps->pm_type >= PM_DEAD)
		pm->tracemask &= 0xFDFF3FFF;

	if ((ps->pm_flags & PMF_PRONE) == 0 || BG_WeaponNames[ps->weapon]->overlayReticle && ps->fWeaponPosFrac > 0.0) {
		ps->pm_flags &= 0xFFFFFDFF;
	}

	if (PM_InteruptWeaponWithProneMove_(ps))
	{
		ps->pm_flags &= 0xFFFFFDFF;
		PM_ExitAimDownSight_(ps);
	}

	if ((ps->pm_flags & PMF_SIGHT_AIMING) != 0 && ps->viewHeightTarget == 11)
	{
		if (BG_WeaponNames[ps->weapon]->overlayReticle)
		{
			if (0.0f >= ps->fWeaponPosFrac)
			{
				pm->cmd.forwardmove = 0;
				pm->cmd.rightmove = 0;
			}
		}
		else
		{
			pm->cmd.forwardmove = 0;
			pm->cmd.rightmove = 0;
		}
	}
	ps->eFlags &= 0xFFFFFFBF;

	if ((ps->pm_flags & PMF_RESPAWNED) == 0)
	{
		if ((!ps->weaponstate || ps->weaponstate == 5) && ps->ammoclip[BG_WeaponNames[ps->weapon]->iClipIndex + 4] && (pm->cmd.buttons & 1) != 0)
			ps->eFlags |= EF_FIRING;
	}
	if (ps->pm_type < PM_DEAD && (pm->cmd.buttons & 0x101) == 0)
		ps->pm_flags &= 0xFFFFFBFF;

	pml.msec = pm->cmd.serverTime - pm->ps->commandTime;

	if (pml.msec >= 1) {
		if (pml.msec > 200)
			pml.msec = 200;
	} else {
		pml.msec = 1;
	}

	pml.frametime = (float)pml.msec / 1000.f;
	pm->ps->commandTime = pm->cmd.serverTime;

	pml.previous_origin[0] = ps->origin[0];
	pml.previous_origin[1] = ps->origin[1];
	pml.previous_origin[2] = ps->origin[2];
	pml.previous_velocity[0] = ps->velocity[0];
	pml.previous_velocity[1] = ps->velocity[1];
	pml.previous_velocity[2] = ps->velocity[2];

	PM_AdjustAimSpreadScale_(pm, &pml);
	PM_UpdateViewAngles(ps, static_cast<float>(pml.msec), &pm->cmd, pm->handler);

	AngleVectors(pm->ps->viewangles, pml.forward, pml.right, pml.up); //set viewangles


	char fwd = pm->cmd.forwardmove;
	if (fwd >= 0)
	{
		if (fwd > 0 || pm->cmd.rightmove)
			ps->pm_flags &= 0xFFFFFFDF;
	}
	else
	{
		ps->pm_flags |= PMF_BACKWARDS_RUN;
	}

	//if (pm->ps->weapon != seg.options.weapon) {
	//	ps->weaponstate = WEAPON_OFFHAND_START;
	//}

	//if (seg.options.viewangle_type == viewangle_type::STRAFEBOT) 
	//	rcmd.angles[YAW] = ANGLE2SHORT(pm->ps->viewangles[YAW]);

	if ((ps->pm_flags & PMF_MANTLE) == 0) {
		PM_UpdateAimDownSightFlag_(pm, &pml);
		PM_UpdateSprint_(pm);
		PM_UpdatePlayerWalkingFlag_(pm);
		PM_CheckDuck_(pm, &pml);
		PM_GroundTrace_(pm, &pml);
	}

	Mantle_Check(pm, &pml);

	if ((ps->pm_flags & PMF_MANTLE) != 0) {

		if ((ps->pm_flags & PMF_LADDER) != 0)
			ps->pm_flags = ps->pm_flags & 0xFFFFFFF7 | PMF_LADDER_FALL;
		ps->groundEntityNum = 2175;
		pml.groundPlane = 0;
		pml.walking = 0;

		PM_UpdateAimDownSightFlag_(pm, &pml);
		PM_UpdateSprint_(pm);
		PM_UpdatePlayerWalkingFlag_(pm);
		PM_CheckDuck_(pm, &pml);
		if (mantle_enable->current.enabled)
			Mantle_Move(ps, &pml);

		PM_Weapon_(pm, &pml);

	}
	else {



		PM_UpdatePronePitch_(pm, &pml);
		PM_DropTimers(ps, &pml);
		//if ( ps->pm_type >= PM_DEAD && pml.walking )
			//PM_DeadMove(ps);
		PM_CheckLadderMove_(pm, &pml);

		if ((ps->pm_flags & PMF_LADDER) != 0)
		{
			PM_LadderMove_(pm, &pml);
		}
		else if (pml.walking) {
			//if (seg.options.bhop)
			//	pm->cmd.buttons |= cmdEnums::jump;
			PM_WalkMove_f(pm, &pml);
		}
		else {
			PM_AirMove_f(pm, &pml);
		}
		PM_GroundTrace_(pm, &pml); //call groundtrace after 
		PM_Footsteps_(pm, &pml);
		PM_Weapon_(pm, &pml);
		PM_FoliageSnd_(pm);
		PM_OverBounce(pm, &pml);
		//Sys_SnapVector(pm->ps->velocity); //Sys_SnapVector | not called in singleplayer
	}

}