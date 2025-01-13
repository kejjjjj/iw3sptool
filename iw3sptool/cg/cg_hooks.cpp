#include "utils/hook.hpp"

#include "cl/cl_main.hpp"
#include "cl/cl_keymove.hpp"
#include "r/r_active.hpp"
#include "r/rb_endscene.hpp"
#include <cm/cm_brush.hpp>
#include "g/g_entity.hpp"
#include "cg_view.hpp"
#include "bg/bg_pmove.hpp"

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

static void CG_CreateHooks();

void CG_CreatePermaHooks()
{
	hooktable::initialize();

	CG_CreateHooks();

}
void CG_CreateHooks()
{

	hooktable::preserver<void>(HOOK_PREFIX("CG_DrawActive"), 0x4111D0, CG_DrawActive);
	hooktable::preserver<char, GfxViewParms*>(HOOK_PREFIX("RB_DrawDebug"), 0x636D30, RB_DrawDebug);
	hooktable::preserver<void>(HOOK_PREFIX("__asm_adjacency_winding"), 0x59859C, __brush::__asm_adjacency_winding);

	hooktable::preserver<void, cg_s*>(HOOK_PREFIX("CG_OffsetFirstPersonView"), 0x42DB30, CG_OffsetFirstPersonView);
	hooktable::preserver<void>(HOOK_PREFIX("CG_OffsetThirdPersonViewASM"), 0x42ED10, CG_OffsetThirdPersonViewASM);

	hooktable::preserver<void>(HOOK_PREFIX("G_FreeEntityASM"), 0x4EFF90, G_FreeEntityASM);
	hooktable::preserver<void>(HOOK_PREFIX("G_FreeEntityASM2"), 0x4F029B, G_FreeEntityASM2);

	hooktable::preserver<void>(HOOK_PREFIX("G_SpawnASM"), 0x4EFDCA, G_SpawnASM);
	hooktable::preserver<void>(HOOK_PREFIX("G_SpawnASM2"), 0x4EFD9A, G_SpawnASM2);

	hooktable::preserver<void>(HOOK_PREFIX("G_LoadGame_f"), 0x5C4EC0, G_LoadGame_f);
	hooktable::preserver<void>(HOOK_PREFIX("Map_Restart_f"), 0x5C4EA0, Map_Restart_f);
	hooktable::preserver<void>(HOOK_PREFIX("Fast_Restart_f"), 0x5C4E90, Fast_Restart_f);

	hooktable::preserver<void, pmove_t*>(HOOK_PREFIX("Pmove"), 0x5BD440, Pmove);
	hooktable::preserver<void>(HOOK_PREFIX("PmoveSingleASM"), 0x5BD2F9, PmoveSingleASM);

	hooktable::preserver<void>(HOOK_PREFIX("PM_SprintFixASM"), 0x5B7B1E, PM_SprintFixASM);

	hooktable::preserver<void>(HOOK_PREFIX("CG_UpdateViewWeaponAnim"), 0x433BB0, CG_UpdateViewWeaponAnim);
	hooktable::preserver<void>(HOOK_PREFIX("CL_Disconnect"), 0x444F80, CL_Disconnect);

	std::this_thread::sleep_for(100ms);
}
void CG_ReleaseHooks()
{
}