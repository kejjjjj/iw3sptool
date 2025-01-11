#pragma once

struct cg_s;

void CG_OffsetFirstPersonView(cg_s* cgs);
void CG_OffsetThirdPersonViewASM();
void CG_OffsetThirdPersonView();

void CG_UpdateViewWeaponAnim();

void RB_RenderPlayerHitboxes();