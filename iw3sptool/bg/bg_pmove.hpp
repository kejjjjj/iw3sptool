#pragma once

#include "cg/cg_local.hpp"

void Pmove(pmove_t* pm);
void PmoveSingleASM();
void PM_FoliageSnd(pmove_t* pm);

void PM_SprintFixASM();

void PM_OverBounce(pmove_t* pm, pml_t* pml);

float CG_GetPlayerHitboxHeight(playerState_s* ps);

static void(*PM_GroundTrace_)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5B95F0);
static void(__cdecl* PM_CheckLadderMove_)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5BC290);
static void(__cdecl* PM_LadderMove_)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5BC570);
static void(__cdecl* PM_AdjustAimSpreadScale_)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5C0CF0);

static void(*PM_UpdateSprint_)(pmove_t* pm) = ((void(__cdecl*)(pmove_t*))0x5B79F0);
//static void(*PM_UpdatePlayerWalkingFlag_)(pmove_t* pm) = ((void(__cdecl*)(pmove_t*))0x5BC240);

static void(__fastcall* PM_DropTimers)(playerState_s* ps, pml_t* pml) = ((void(__fastcall*)(playerState_s*, pml_t*))0x5BAED0);

static void(*PM_UpdateAimDownSightFlag_)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5BF480);
static void(*PM_Footsteps_)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5BABD0);
static void(*PM_WalkMove_f)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5B8940);
static void(*PM_AirMove_f)(pmove_t* pm, pml_t* pml) = ((void(__cdecl*)(pmove_t*, pml_t*))0x5B8820);

void PM_UpdatePlayerWalkingFlag_(pmove_t* pm);
void PM_FoliageSnd_(pmove_t* pm);
int PM_InteruptWeaponWithProneMove_(playerState_s* ps);
void PM_ExitAimDownSight_(playerState_s* ps);
void PM_Weapon_(pmove_t* pm, pml_t* pml);
void PM_CheckDuck_(pmove_t* pm, pml_t* pml);
void Mantle_Move(playerState_s* ps, pml_t* pml);
void PM_UpdatePronePitch_(pmove_t* pm, pml_t* pml);
void PM_UpdateViewAngles(playerState_s* ps, float msec, usercmd_s* cmd, char handler);
void PM_BeginWeaponChange_(int weapon, playerState_s* ps, bool quick);
int PM_GetSprintLeft(playerState_s* ps, int time);

inline void (*PmoveSingle_fnc)(pmove_t* pm) = (void(__cdecl*)(pmove_t*))0x5BCBB0;
void Mantle_Check(pmove_t* pm, pml_t* pml);

void SimulatePmove(pmove_t* pm);
void SimulatePmoveSingle(pmove_t* pm);
