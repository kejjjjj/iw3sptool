#pragma once

#include "cg_local.hpp"

inline DxGlobals* dx = (DxGlobals*)(0x01623F84);
inline CInput* input = (CInput*)(0x869534 - sizeof(usercmd_s) * 64);
//inline Movement* move = (Movement*)(0x815EF8);
inline playerState_s* ps_loc = (playerState_s*)(0xE0DA00);
inline playerState_s* predictedPlayerState = (playerState_s*)(0x714BA8);
inline clipMap_t* cm = (clipMap_t*)(0xF788C8);
inline CmdArgs* cmd_args = (CmdArgs*)(0xF789E8);
inline materialCommands_t* tess = reinterpret_cast<materialCommands_t*>(0x18CB540);
inline r_global_permanent_t* rgp = reinterpret_cast<r_global_permanent_t*>(0x1621E00);
inline GfxCmdBufState* gfxCmdBufState = reinterpret_cast<GfxCmdBufState*>(0x1D37F70);
inline DpvsGlob* dpvsGlob = reinterpret_cast<DpvsGlob*>(0x189A328);
inline gentity_s* gentities = reinterpret_cast<gentity_s*>(0xC81418);
//inline refdef_s* refdef = (refdef_s*)(0x720080 - 8);
inline WinMouseVars_t* s_wmv = (WinMouseVars_t*)(0x13E06F0);
inline level_locals_t* level = reinterpret_cast<level_locals_t*>(0xE18E18);
inline WeaponDef** BG_WeaponNames = reinterpret_cast<WeaponDef**>(0x14227A8);
inline scrMemTreePub_t* gScrMemTreePub = reinterpret_cast<scrMemTreePub_t*>(0xFDBBC4);
inline int* bg_lastParsedWeaponIndex = reinterpret_cast<int*>(0x1E20A04);
inline ent_field_t* ent_fields = reinterpret_cast<ent_field_t*>(0x67FFB8);
inline cg_s* cgs = reinterpret_cast<cg_s*>(0x6FA590);
inline GfxWorld* gfxWorld = reinterpret_cast<GfxWorld*>(0x189A048);

inline clientUIActive_t* clientUI = reinterpret_cast<clientUIActive_t*>(0x85BD70);

inline const char** g_entityBeginParsePoint = reinterpret_cast<const char**>(0xC1FEE8);
inline const char** g_entityEndParsePoint = reinterpret_cast<const char**>(0xC1FEEC);