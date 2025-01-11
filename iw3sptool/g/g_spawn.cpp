#include "g_spawn.hpp"

#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"

#include "dvar/dvar.hpp"

#include "utils/engine.hpp"

#include "com/com_channel.hpp"
#include "scr/scr.hpp"

#include <string.h>
#include <ranges>



void G_SpawnEntitiesFromString()
{
	G_ResetEntityParsePoint();

	__asm
	{
		mov esi, 0x4EA840;
		call esi;
	}

}
void G_ResetEntityParsePoint()
{
	*g_entityBeginParsePoint = cm->mapEnts->entityString;
	*g_entityEndParsePoint = *g_entityBeginParsePoint;
}

int G_ParseSpawnVars(SpawnVar* var)
{
	constexpr static auto vG_ParseSpawnVars = 0x4B6A60;
	__asm { 
		mov eax, var; 
		call vG_ParseSpawnVars; 
	}
}

int G_GetItemForClassname(const char* classname)
{
	constexpr static auto f = 0x4DFE30;

	__asm
	{
		mov esi, classname;
		call f;
	}

}

bool G_GetSpawnItemIndex(const gentity_s* gent)
{

	const char* classname = 0;
	const auto G_SpawnString = [&](const SpawnVar* spawnVar, const char* key, const char* defaultString, const char** out) {
		return Engine::call<int>(0x4B6BE0, spawnVar, key, defaultString, out); };
	
	G_SpawnString(&level->spawnVar, "classname", "", &classname);

	return classname && !strcmp(classname, Scr_GetString(gent->classname));
}

SpawnVar* G_GetGentitySpawnVars(const gentity_s* gent)
{
	SpawnVar* var = &level->spawnVar;
	auto parsed = false;

	while (parsed = G_ParseSpawnVars(var), parsed){
		if (G_GetSpawnItemIndex(gent))
			break;
	};

	return !parsed ? nullptr : var;
}
