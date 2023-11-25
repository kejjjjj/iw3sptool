#include "pch.hpp"

void CG_CreatePermaHooks()
{
	decltype(auto) hooktable = HookTable::getInstance();

	CG_CreateHooks();

	hooktable.insert(hookEnums_e::HOOK_CL_DISCONNECT, hook::hookobj<void*>(0x444F80, CL_Disconnect, TRUE));

}
void CG_CreateHooks()
{

	decltype(auto) hooktable = HookTable::getInstance();

	hooktable.insert(hookEnums_e::HOOK_DRAWACTIVE,			hook::hookobj<void*>(0x4111D0, CG_DrawActive, TRUE));
	hooktable.insert(hookEnums_e::HOOK_RB_ENDSCENE,			hook::hookobj<void*>(0x636D30, RB_DrawDebug, TRUE));

	hooktable.insert(hookEnums_e::HOOK_BRUSH_ADJACENCY, hook::hookobj<void*>(0x59859C, __brush_hook::stealerino, TRUE));

	//hooktable.insert(hookEnums_e::G_TRIGGER, hook::hookobj<void*>(0x4EBDA0, G_Trigger, TRUE));


	std::this_thread::sleep_for(100ms);
}
void CG_ReleaseHooks()
{
	decltype(auto) hooktable = HookTable::getInstance();
	decltype(auto) table = const_cast<std::unordered_map<hookEnums_e, hook::hookobj<void*>>&>(hooktable.getter().get());

	auto it = table.begin();
	std::cout << "table size: " << table.size() << '\n';
	for (auto& i : table) {
		++it;
		if (i.first < hookEnums_e::HOOK_DRAWACTIVE)
			continue;

		if(const_cast<hook::hookobj<void*>&>(i.second).release() == false) {
			FatalError("failed to release hook!\n");
		}

		table.erase(i.first);

	}
	std::cout << "table size: " << table.size() << '\n';
	std::cout << "hooks released!\n";
}
inline namespace hook__ {
	static hook::hookobj<void*> empty_hook;
}
hook::hookobj<void*>& find_hook(const hookEnums_e hook)
{
	decltype(auto) _hook = HookTable::getInstance().find(hook);
	
	if (!_hook.has_value()) {
		FatalError(std::format("find_hook(): failed to find hook index {}\n", std::underlying_type_t<hookEnums_e>(hook)));
		return hook__::empty_hook;
	}

	return const_cast<hook::hookobj<void*>&>(_hook.value()->second);


}