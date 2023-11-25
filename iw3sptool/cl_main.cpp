#include "pch.hpp"

void CL_Disconnect()
{

	if (clientUI->connectionState != CA_DISCONNECTED) { //gets called in the loading screen in 1.7
		std::cout << "disconnecting\n";

	}

	s_brushes.clear();
	cm_terrainpoints.clear();
	gameEntities::getInstance().clear();

	return ((void(*)())0x444F10)();

}


void __cdecl G_Trigger2(gentity_s* activator, gentity_s* trigger)
{
	//if (Dvar_FindMalleableVar("cm_disableTriggers")->current.enabled)
	//	return;



	//std::cout << "trigger: " << trigger << '\n';
}
__declspec(naked) void G_Trigger(gentity_s* activator, gentity_s* trigger)
{
	static DWORD j = 0x4EBDA1;
	static DWORD je_label = 0x04EBE21;

	static DWORD _esi, _edx;

	static const char* txt = "cm_disableTriggers";

	__asm
	{
		push ecx;

		push txt;
		call Dvar_FindMalleableVar;
		add esp, 0x4;


		mov _esi, esi;
		mov _edx, edx;

	
		push esi;
		push edx;
		call G_Trigger2;
		add esp, 8;

		pop ecx;
		retn;
	}
}