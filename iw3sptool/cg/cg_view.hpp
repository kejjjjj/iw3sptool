#pragma once

#include "cg/cg_offsets.hpp"

void CG_OffsetFirstPersonView(cg_s* cgs);
void CG_OffsetThirdPersonViewASM();
void CG_OffsetThirdPersonView();

void CG_UpdateViewWeaponAnim();

void RB_RenderPlayerHitboxes();