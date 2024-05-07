#include "dvar.hpp"
#include <com/com_channel.hpp>
#include <cm/cm_brush.hpp>
#include <cm/cm_entity.hpp>
#include <utils/engine.hpp>

dvar_s* Dvar_FindMalleableVar(const char* name)
{
    return ((dvar_s* (__cdecl*)(const char* name))0x0587B00)(name);

}
dvar_s* Dvar_RegisterNew(const char* name, dvar_type type, int flags, const char* description, dvar_value defaultValue, dvar_limits domain)
{
    dvar_s* dvar = Dvar_FindMalleableVar(name);

    if (dvar) {
        return Dvar_Reregister(dvar, name, type, flags, description, defaultValue, domain);
    }

    return ((dvar_s * (*)(const char*, dvar_type, int, const char*, dvar_value, dvar_limits))0x588750)(name, type, flags, description, defaultValue, domain);
}
dvar_s* Dvar_Reregister(dvar_s* dvar, const char* name, dvar_type type, int flags, const char* description, dvar_value defaultValue, dvar_limits domain)
{
    return ((dvar_s * (*)(dvar_s*, const char*, dvar_type, int, const char*, dvar_value, dvar_limits))0x588610)(dvar, name, type, flags, description, defaultValue, domain);
}

void OnDvarsModified()
{
	static dvar_s* cm_disableTriggers = Dvar_FindMalleableVar("cm_disableTriggers");
	static dvar_s* cm_onlyBounces = Dvar_FindMalleableVar("cm_onlyBounces");
	static dvar_s* cm_onlyElevators = Dvar_FindMalleableVar("cm_onlyElevators");
	static dvar_s* cg_drawGun = Dvar_FindMalleableVar("cg_drawGun");

	if (cm_disableTriggers->modified) {

		if (cm_disableTriggers->current.enabled) {
			Engine::Tools::write_bytes(0x004EBDA0, "\xC3");
		}
		else {
			Engine::Tools::write_bytes(0x004EBDA0, "\x51");

		}

		cm_disableTriggers->modified = false;

		Com_Printf("triggers have been %s\n", cm_disableTriggers->current.enabled ? "disabled" : "enabled");

	}

	if (cm_onlyElevators->current.integer && cm_onlyElevators->modified) {
		cm_onlyBounces->current.enabled = false;
		cm_onlyBounces->latched.enabled = false;
		cm_onlyBounces->modified = false;

	}

	if (cm_onlyBounces->current.enabled && cm_onlyBounces->modified) {
		cm_onlyElevators->current.integer = false;
		cm_onlyElevators->latched.integer = false;
		cm_onlyElevators->modified = false;
	}

	static std::string old_mapname = Dvar_FindMalleableVar("mapname")->current.string;

	if (old_mapname != Dvar_FindMalleableVar("mapname")->current.string) {

		__brush::rb_requesting_to_stop_rendering = true;

		CClipMap::clear();
		gameEntities::getInstance().clear(true);
		entity_globals::ent_fields.clear();

		if(gameEntities::getInstance().empty() == false)
			gameEntities::getInstance().clear(true);

		old_mapname = Dvar_FindMalleableVar("mapname")->current.string;

		__brush::rb_requesting_to_stop_rendering = false;

	}
	
	static bool drawgun_was_enabled_before_demo_playback = false;

	if (*(bool*)0x85BC80) {

		if (cg_drawGun->current.enabled)
			drawgun_was_enabled_before_demo_playback = true;

		cg_drawGun->current.enabled = false;
		cg_drawGun->latched.enabled = false;
		cg_drawGun->modified = false;
	}
	else if (drawgun_was_enabled_before_demo_playback) {
		cg_drawGun->current.enabled = true;
		cg_drawGun->modified = true;
		drawgun_was_enabled_before_demo_playback = false;
	}


}