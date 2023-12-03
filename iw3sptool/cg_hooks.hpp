#pragma once

#include "pch.hpp"

enum class hookEnums_e : short
{
	//PERMA HOOKS
	HOOK_CL_DISCONNECT,


	//TEMP HOOKS
	HOOK_DRAWACTIVE,
	HOOK_RB_ENDSCENE,


	HOOK_BRUSH_ADJACENCY,
	HOOK_G_TRIGGER,
	HOOK_G_FREE_ENTITY,
	HOOK_G_FREE_ENTITY2,

	HOOK_SV_LINKENTITY1,
	HOOK_SV_LINKENTITY2,
	HOOK_FIRSTPERSON,
	HOOK_THIRDPERSON,

	HOOK_LOADGAME,
	HOOK_MAP_RESTART,
	HOOK_FAST_RESTART

};

class HookTable
{
public:
	static HookTable& getInstance() {
		static HookTable _table;
		return _table;
	}
	decltype(auto) find(const hookEnums_e e) const {
		auto found = table.find(e);
		return found != table.end() ? std::make_optional(found) : std::nullopt;
	}
	decltype(auto) [[maybe_unused]] insert(const hookEnums_e e, const hook::hookobj<void*>& h) {

		return &table.insert({ e, h }).first->second;
	}
	void enable(const hookEnums_e h) {
		auto found = table.find(h);

		if (found == table.end())
			return;

		found->second.enable();
	}
	void disable(const hookEnums_e h) {
		auto found = table.find(h);

		if (found == table.end())
			return;

		found->second.disable();
	}
	decltype(auto) getter() const noexcept { return std::ref(table); };
private:
	std::unordered_map<hookEnums_e, hook::hookobj<void*>> table;
};


hook::hookobj<void*>& find_hook(const hookEnums_e hook);


void CG_CreatePermaHooks();
void CG_CreateHooks();
void CG_ReleaseHooks();
