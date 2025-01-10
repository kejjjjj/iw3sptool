#pragma once

#include "cg/cg_local.hpp"

void Pmove(pmove_t* pm);
void PmoveSingleASM();
void PM_FoliageSnd(pmove_t* pm);

void PM_SprintFixASM();

void PM_OverBounce(pmove_t* pm, pml_t* pml);

float CG_GetPlayerHitboxHeight(playerState_s* ps);
