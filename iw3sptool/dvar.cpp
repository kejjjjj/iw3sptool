#include "pch.hpp"

dvar_s* Dvar_FindMalleableVar(const char* name)
{
    return ((dvar_s* (__cdecl*)(const char* name))0x0587B00)(name);

}
dvar_s* Dvar_RegisterNew(const char* name, dvar_type type, int flags, const char* description, dvar_value defaultValue, dvar_limits domain)
{
    dvar_s* dvar = Dvar_FindMalleableVar(name);

    if (dvar) {
        Com_Printf(CON_CHANNEL_CONSOLEONLY, "dvar (%s) already exists\n", name);
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

	if (cm_disableTriggers->modified) {

		if (cm_disableTriggers->current.enabled) {
			hook::write_addr(0x004EBDA0, "\xC3", 1);
		}
		else {
			hook::write_addr(0x004EBDA0, "\x51", 1);

		}

		cm_disableTriggers->modified = false;

		Com_Printf("triggers have been %s\n", cm_disableTriggers->current.enabled ? "disabled" : "enabled");

	}

	if (cm_onlyElevators->current.enabled && cm_onlyElevators->modified) {
		cm_onlyBounces->current.enabled = false;
		cm_onlyBounces->latched.enabled = false;
		cm_onlyBounces->modified = false;

	}

	if (cm_onlyBounces->current.enabled && cm_onlyBounces->modified) {
		cm_onlyElevators->current.enabled = false;
		cm_onlyElevators->latched.enabled = false;
		cm_onlyElevators->modified = false;
	}

	static std::string old_mapname = Dvar_FindMalleableVar("mapname")->current.string;

	if (old_mapname != Dvar_FindMalleableVar("mapname")->current.string) {

		s_brushes.clear();
		cm_terrainpoints.clear();

		if(gameEntities::getInstance().empty() == false)
			gameEntities::getInstance().clear(true);

		old_mapname = Dvar_FindMalleableVar("mapname")->current.string;
	}

}