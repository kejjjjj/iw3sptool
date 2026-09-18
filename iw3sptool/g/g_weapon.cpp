#include "g_weapon.hpp"
#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "utils/functions.hpp"

int G_GetWeaponIndexForName(const char* name) {
	
	if (level->initializing) {
		void(*func)() = reinterpret_cast<void(*)()>(0x4B6140);
		return engine_call<int>(0x5BECE0, name, func);
	}

	return engine_call<int>(0x5BEC90, name);
}