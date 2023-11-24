#include "pch.hpp"

void CL_Disconnect()
{

	if (clientUI->connectionState != CA_DISCONNECTED) { //gets called in the loading screen in 1.7
		std::cout << "disconnecting\n";

	}

	s_brushes.clear();

	return ((void(*)())0x444F10)();

}


void __cdecl G_Trigger2(gentity_s* activator, gentity_s* trigger)
{
	if (GetAsyncKeyState(VK_NUMPAD0) < 0 )
		VectorCopy(trigger->s.lerp.pos.trBase, ps_loc->origin);

	if (trigger->r.bmodel) {
		static bool once = true;
		if (once) {

			auto leaf = &cm->cmodels[trigger->s.index.brushmodel].leaf;
			auto brush = &cm->brushes[cm->leafbrushNodes[leaf->leafBrushNode].data.leaf.brushes[0]];

			if (brush) {

				s_brushes.push_back(CM_GetBrushWindings(brush));

				brush->mins[0] += trigger->s.lerp.pos.trBase[0];
				brush->mins[1] += trigger->s.lerp.pos.trBase[1];
				brush->mins[2] += trigger->s.lerp.pos.trBase[2];

				brush->maxs[0] += trigger->s.lerp.pos.trBase[0];
				brush->maxs[1] += trigger->s.lerp.pos.trBase[1];
				brush->maxs[2] += trigger->s.lerp.pos.trBase[2];

				for (auto& brush : s_brushes) {
					for (auto& winding : brush.windings) {
						for (auto& w : winding.points) {

							w.x += trigger->s.lerp.pos.trBase[0];
							w.y += trigger->s.lerp.pos.trBase[1];
							w.z += trigger->s.lerp.pos.trBase[2];

							std::cout << w << '\n';
						}

					}

				}

			}
			once = false;

		}

	}

	//std::cout << "trigger: " << trigger << '\n';
}
void RevealTrigger(gentity_s* trigger)
{
	if (trigger->classname == SL_GetStringOfSize("trigger_multiple") == false)
		return;

	std::cout << "discovered a trigger\n";

	if (trigger->r.bmodel) {
		auto leaf = &cm->cmodels[trigger->s.index.brushmodel].leaf;
		auto brush = &cm->brushes[cm->leafbrushNodes[leaf->leafBrushNode].data.leaf.brushes[0]];

		if (brush) {

			s_brushes.push_back(CM_GetBrushWindings(brush));

			brush->mins[0] += trigger->s.lerp.pos.trBase[0];
			brush->mins[1] += trigger->s.lerp.pos.trBase[1];
			brush->mins[2] += trigger->s.lerp.pos.trBase[2];

			brush->maxs[0] += trigger->s.lerp.pos.trBase[0];
			brush->maxs[1] += trigger->s.lerp.pos.trBase[1];
			brush->maxs[2] += trigger->s.lerp.pos.trBase[2];

			auto& b = s_brushes.back();

			for (auto& winding : b.windings) {
				for (auto& w : winding.points) {

					w.x += trigger->s.lerp.pos.trBase[0];
					w.y += trigger->s.lerp.pos.trBase[1];
					w.z += trigger->s.lerp.pos.trBase[2];
				}
			}

		}

	}
}
void IWonderIfThisWorks()
{
	gentity_s* g = &gentities[0];

	for(int i = 0; i < 512; i++){

		RevealTrigger(g);

		

		++g;
	}
}
__declspec(naked) void G_Trigger(gentity_s* activator, gentity_s* trigger)
{
	static DWORD j = 0x4EBDA1;
	static DWORD je_label = 0x04EBE21;

	static DWORD _esi, _edx;

	__asm
	{
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