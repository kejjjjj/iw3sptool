#pragma once

#include "cg/cg_offsets.hpp"

cmd_function_s* Cmd_FindCommand(const char* name);
cmd_function_s* Cmd_AddCommand(const char* cmdname, void(__cdecl* function)());
cmd_function_s* Cmd_RemoveCommand(const char* cmdname);

void Cbuf_AddText(const char* text);