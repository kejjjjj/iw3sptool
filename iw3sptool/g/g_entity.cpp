#include "g_entity.hpp"
#include <unordered_map>
#include <cm/cm_entity.hpp>
#include <utils/functions.hpp>
#include <thread>
#include <cg/cg_offsets.hpp>
#include <com/com_channel.hpp>
#include <cm/cm_brush.hpp>
#include <scr/scr.hpp>
#include <utils/hook.hpp>
#include <dvar/dvar.hpp>

using namespace std::chrono_literals;

std::unordered_set<std::string> ent_filter;
bool monitoring_entities = false;
bool this_entity_is_relevant = false;
bool thread_exists = false;

std::unordered_map<int, entity_fields> entity_globals::ent_fields = {};


void update_entities_thread()
{
	decltype(auto) ents = gameEntities::getInstance();

	if (monitoring_entities == false) {
		ents.it_is_ok_to_load_entities = true;
		ents.time_since_loadgame = 0;
		ents.clear();

		return;
	}
	ents.clear();

	while (Sys_MilliSeconds() < ents.time_since_loadgame + 150) {
		std::this_thread::sleep_for(10ms);
	}
	ents.it_is_ok_to_load_entities = true;
	ents.time_since_loadgame = 0;


	G_DiscoverGentities(level, ent_filter);

}

void Cmd_ShowEntities_f()
{


	decltype(auto) ents = gameEntities::getInstance();

	int num_args = cmd_args->argc[cmd_args->nesting];

	if (num_args == 1) {
		monitoring_entities = false;
		if (ents.empty()) {
			return Com_Printf(CON_CHANNEL_CONSOLEONLY, "there are no entities to be cleared.. did you intend to use cm_showEntities <classname>?\n");
		}

		if (Dvar_FindMalleableVar("developer")->current.enabled)
			Com_Printf("clearing %i entities from the render queue\n", ents.size());

		ents.clear();
		return;
	}
	std::unordered_set<std::string> filters;
	for (int i = 1; i < num_args; i++) {
		filters.insert(*(cmd_args->argv[cmd_args->nesting] + i));
	}

	monitoring_entities = true;
	ent_filter = filters;

	G_DiscoverGentities(level, filters);

	Com_Printf(CON_CHANNEL_CONSOLEONLY, "adding %i entities to the render queue\n", ents.size());

}


void G_DiscoverGentities(level_locals_t* l, const std::unordered_set<std::string>& filters)
{
	decltype(auto) ents = gameEntities::getInstance();

	if (ents.it_is_ok_to_load_entities == false) {

		return;
	}

	thread_exists = false;

	__brush::rb_requesting_to_stop_rendering = true;


	ents.clear();
	for (int i = 0; i < l->num_entities; i++) {

		if (CM_IsMatchingFilter(filters, Scr_GetString(l->gentities[i].classname)) == false)
			continue;

		ents.push_back(&l->gentities[i]);
		++ents.spawned_entities;
	}


	//std::cout << "a total of " << ents.size() << " entities\n";

	__brush::rb_requesting_to_stop_rendering = false;

}
__declspec(naked) void G_ParseEntityFieldsASM()
{
	static constexpr DWORD r = 0x4E9403;
	__asm
	{
		push ecx;
		//push ebx;

		//mov ebx, [esp + 0x8 + 4];

		push 0;
		push esi;
		call G_ParseEntityFields;
		add esp, 8;

		pop ecx;
		retn;
	}
}
void G_ParseEntityFields(gentity_s* gent, int a2)
{
	entity_fields f;

	constexpr DWORD fnc = 0x4E9180;
	for (int i = 0; i < level->spawnVar.numSpawnVars; i++)
	{

		auto key = level->spawnVar.spawnVars[i][0];
		auto value = level->spawnVar.spawnVars[i][1];

		__asm
		{
			push a2;
			push gent;
			mov ecx, key;
			mov eax, value;
			call fnc;
			add esp, 8;

		}

		if (key && value)
			f.key_value.push_back({ key, value });

	}

	if (f.key_value.size())
		entity_globals::ent_fields.insert({ gent->s.number, std::move(f) });

	gent->s.lerp.pos.trBase[0] = gent->r.currentOrigin[0];
	gent->s.lerp.pos.trBase[1] = gent->r.currentOrigin[1];
	gent->s.lerp.pos.trBase[2] = gent->r.currentOrigin[2];
	gent->s.lerp.pos.trType = TR_STATIONARY;
	gent->s.lerp.pos.trTime = 0;
	gent->s.lerp.pos.trDuration = 0;
	gent->s.lerp.pos.trDelta[0] = 0.0;
	gent->s.lerp.pos.trDelta[1] = 0.0;
	gent->s.lerp.pos.trDelta[2] = 0.0;
	gent->r.currentOrigin[0] = gent->r.currentOrigin[0];
	gent->r.currentOrigin[1] = gent->r.currentOrigin[1];
	gent->r.currentOrigin[2] = gent->r.currentOrigin[2];
	gent->s.lerp.apos.trBase[0] = gent->r.currentAngles[0];
	gent->s.lerp.apos.trBase[1] = gent->r.currentAngles[1];
	gent->s.lerp.apos.trBase[2] = gent->r.currentAngles[2];
	gent->s.lerp.apos.trType = TR_STATIONARY;
	gent->s.lerp.apos.trTime = 0;
	gent->s.lerp.apos.trDuration = 0;
	gent->s.lerp.apos.trDelta[0] = 0.0;
	gent->s.lerp.apos.trDelta[1] = 0.0;
	gent->s.lerp.apos.trDelta[2] = 0.0;
	gent->r.currentAngles[0] = gent->r.currentAngles[0];
	gent->r.currentAngles[1] = gent->r.currentAngles[1];
	gent->r.currentAngles[2] = gent->r.currentAngles[2];

}
void G_FreeEntity(gentity_s* gent)
{
	//std::cout << "freeing entity\n";
	//find_hook(hookEnums_e::HOOK_G_FREE_ENTITY).cast_call<void(*)(gentity_s*)>(ent);

	if (monitoring_entities && gent) {
		this_entity_is_relevant = gameEntity::is_supported_entity(gent) && CM_IsMatchingFilter(ent_filter, Scr_GetString(gent->classname));
	}
	return;
}
void G_FreeEntity2()
{
	if (this_entity_is_relevant) {
		if (Dvar_FindMalleableVar("developer")->current.enabled)
			Com_Printf("removing an entity from the render queue\n");

		G_DiscoverGentities(level, ent_filter);
		this_entity_is_relevant = false;
	}
	return;
}
__declspec(naked) void G_FreeEntityASM()
{
	static DWORD _jmp = 0x4EFF98;
	__asm
	{
		push ebp;
		mov ebp, esp;
		and esp, 0FFFFFFF8h;
		push ecx;
		push ebx;
		mov ebx, [ebp + 08h];
		push ebx;
		call G_FreeEntity;
		add esp, 0x4;
		jmp _jmp;
	}
}
__declspec(naked) void G_FreeEntityASM2()
{
	static DWORD _jmp = 0x4EFF98;
	__asm
	{
		pop edi;
		pop esi;
		mov byte ptr[ecx + 00DCEE28h], 00;
		pop ebx;
		mov esp, ebp;
		pop ebp;
		call G_FreeEntity2;
		retn;
	}
}
void G_Spawn(gentity_s* gent)
{
	//std::cout << "total spawned entities: " << ++spawned_entities << '\n';
	if (monitoring_entities && gent) {
		if (CM_IsMatchingFilter(ent_filter, Scr_GetString(gent->classname)) && gameEntity::is_supported_entity(gent)) {
			G_DiscoverGentities(level, ent_filter);
			if(Dvar_FindMalleableVar("developer")->current.enabled)
				Com_Printf("adding a new entity to the render queue\n");
		}

	}



	return;

}
__declspec(naked) void G_SpawnASM()
{
	__asm
	{
		push esi;
		call G_Spawn;
		add esp, 0x4;
		mov eax, esi;
		pop esi;

		retn;
	}
}
__declspec(naked) void G_SpawnASM2()
{
	static const DWORD fnc = 0x4EFAF0;
	__asm
	{
		call fnc;
		push esi;
		call G_Spawn;
		add esp, 0x4;
		mov eax, esi;
		pop esi;
		retn;
	}
}
void G_LoadGameAAA()
{
	gameEntities::getInstance().clear();
	gameEntities::getInstance().time_since_loadgame = Sys_MilliSeconds();



}
void G_LoadGame_f()
{
	hooktable::find<void>(HOOK_PREFIX(__func__))->call();
	gameEntities::getInstance().clear();
	gameEntities::getInstance().time_since_loadgame = Sys_MilliSeconds();
	gameEntities::getInstance().it_is_ok_to_load_entities = false;
	std::thread(update_entities_thread).detach();

	return;

}