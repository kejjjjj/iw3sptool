#pragma once

#include <cg/cg_local.hpp>


void Com_Printf		 ($B87C0110D100A68234FECCEB9075A41E channel, const char* msg, ...);
void Com_Printf(const char* msg, ...);
void Com_PrintWarning($B87C0110D100A68234FECCEB9075A41E channel, const char* msg, ...);
void Com_PrintError	 ($B87C0110D100A68234FECCEB9075A41E channel, const char* msg, ...);
void Com_Error		 (errorParm_t channel, const char* msg, ...);