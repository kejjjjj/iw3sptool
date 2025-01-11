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
volatile bool this_entity_is_relevant = false;
volatile DWORD time_since_loadgame{};

bool G_RepopulateEntities()
{
	//let a few frames pass
	if (Sys_MilliSeconds() < time_since_loadgame + 50) {
		return false;
	}

	time_since_loadgame = 0;
	CGentities::CM_LoadAllEntitiesToClipMapWithFilters(ent_filter);

	return true;
}
void Cmd_ShowEntities_f()
{


	int num_args = cmd_args->argc[cmd_args->nesting];

	if (num_args == 1) {
		if (CGentities::Size() == 0) {
			return Com_Printf(CON_CHANNEL_CONSOLEONLY, "there are no entities to be cleared.. did you intend to use cm_showEntities <classname>?\n");
		}

		if (Dvar_FindMalleableVar("developer")->current.enabled)
			Com_Printf("clearing %i entities from the render queue\n", CGentities::Size());

		CGentities::ClearThreadSafe();
		return;
	}

	std::unordered_set<std::string> filters;
	for (int i = 1; i < num_args; i++) {
		filters.insert(*(cmd_args->argv[cmd_args->nesting] + i));
	}

	ent_filter = filters;
	CGentities::CM_LoadAllEntitiesToClipMapWithFilters(filters);

	Com_Printf(CON_CHANNEL_CONSOLEONLY, "adding %i entities to the render queue\n", CGentities::Size());

}


void G_DiscoverGentities(level_locals_t* l, const std::unordered_set<std::string>& filters)
{

	CGentities::ClearThreadSafe();

	for (auto i = 0; i < l->num_entities; i++) {

		if (CM_IsMatchingFilter(filters, Scr_GetString(l->gentities[i].classname)) == false)
			continue;

		CGentities::Insert(CGameEntity::CreateEntity(&l->gentities[i]));
	}

}
void G_FreeEntity(gentity_s* gent)
{

	if (CGentities::Size() && gent) {
		this_entity_is_relevant = CM_IsMatchingFilter(ent_filter, Scr_GetString(gent->classname));
	}
	return;
}
void G_FreeEntity2()
{
	if (this_entity_is_relevant) {
		if (Dvar_FindMalleableVar("developer")->current.enabled)
			Com_Printf("removing an entity from the render queue\n");

		CGentities::CM_LoadAllEntitiesToClipMapWithFilters(ent_filter);
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
	if (CGentities::Size() && gent) {
		if (CM_IsMatchingFilter(ent_filter, Scr_GetString(gent->classname))) {

			CGentities::CM_LoadAllEntitiesToClipMapWithFilters(ent_filter);

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

void G_LoadGame_f()
{
	hooktable::find<void>(HOOK_PREFIX(__func__))->call();

	time_since_loadgame = Sys_MilliSeconds();

	if(CGentities::Size())
		CGentities::Repopulate();

	CGentities::ClearThreadSafe();
}