#pragma once

#include "cg_local.hpp"

inline void (*CG_Trace)(trace_t* result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mas) = (void(__cdecl*)(trace_t*, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int))0x4368A0;
