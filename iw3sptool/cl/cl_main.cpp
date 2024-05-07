#include "pch.hpp"
#include <cg/cg_offsets.hpp>
#include <cg/cg_local.hpp>
#include "cm/cm_brush.hpp"
#include "cm/cm_entity.hpp"
#include <utils/functions.hpp>

void CL_Disconnect()
{
	__brush::rb_requesting_to_stop_rendering = true;

	if (clientUI->connectionState != CA_DISCONNECTED) {
	}

	CClipMap::clear();
	gameEntities::getInstance().clear(true);
	entity_globals::ent_fields.clear();

	__brush::rb_requesting_to_stop_rendering = false;


	return ((void(*)())0x444F10)();

}
void Map_Restart_f()
{

	CClipMap::clear();
	gameEntities::getInstance().clear(true);
	entity_globals::ent_fields.clear();

	return engine_call<void>(0x005C4E30, 1);

}
void Fast_Restart_f()
{

	CClipMap::clear();
	gameEntities::getInstance().clear(true);
	entity_globals::ent_fields.clear();

	return engine_call<void>(0x005C4E30, 0);

}