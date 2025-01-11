#pragma once

void G_SpawnEntitiesFromString();
void G_ResetEntityParsePoint();

struct SpawnVar;
struct gentity_s;

SpawnVar* G_GetGentitySpawnVars(const gentity_s* gent);

