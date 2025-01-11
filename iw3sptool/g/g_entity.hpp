#pragma once

#include <unordered_set>
#include <string>
#include "cg/cg_local.hpp"


void G_DiscoverGentities(level_locals_t* l, const std::unordered_set<std::string>& filters);

[[nodiscard]] bool G_RepopulateEntities();

void G_FreeEntity(gentity_s* gent);
void G_FreeEntityASM();
void G_FreeEntityASM2();

void G_Spawn(gentity_s* gent);
void G_SpawnASM();
void G_SpawnASM2();

void G_LoadGame_f();