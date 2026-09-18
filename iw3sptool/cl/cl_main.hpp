#pragma once

void CL_Disconnect();
void Map_Restart_f();
void Fast_Restart_f();

void CL_CreateNewCommands();
void LoadTasScript_f();
void StartTasScript_f();

#include "tas/tas.hpp"
inline std::optional<TAS::Player> g_playback_script;