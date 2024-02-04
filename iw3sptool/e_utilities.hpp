#pragma once

#include "pch.hpp"

cmd_function_s* Cmd_FindCommand(const char* name);
cmd_function_s* Cmd_AddCommand(const char* cmdname, void(__cdecl* function)());
cmd_function_s* Cmd_RemoveCommand(const char* cmdname);

unsigned short SL_GetStringOfSize(const char* str);

char* Scr_GetString(int string);

void Cbuf_AddText(const char* text);

inline void (*CG_Trace)(trace_t* result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mas) = (void(__cdecl*)(trace_t*, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int))0x4368A0;
float R_ScaleByDistance(float dist);

template<typename Return, typename ... Args>
inline Return engine_call(const uintptr_t offset, Args... args)
{
	return (reinterpret_cast<Return(*)(Args...)>(offset))(args...);
}