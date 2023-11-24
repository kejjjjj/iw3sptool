#pragma once

#include "pch.hpp"

cmd_function_s* Cmd_FindCommand(const char* name);
cmd_function_s* Cmd_AddCommand(const char* cmdname, void(__cdecl* function)());
cmd_function_s* Cmd_RemoveCommand(const char* cmdname);

unsigned short SL_GetStringOfSize(const char* str);


void Cbuf_AddText(const char* text);