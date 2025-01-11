#include "pch.hpp"
#include <cg/cg_offsets.hpp>
#include <cg/cg_local.hpp>
#include "cm/cm_brush.hpp"
#include "cm/cm_entity.hpp"
#include <utils/functions.hpp>

void CL_Disconnect()
{

	CClipMap::ClearThreadSafe();
	CGentities::ClearThreadSafe();

	return ((void(*)())0x444F10)();

}
void Map_Restart_f()
{

	CClipMap::ClearThreadSafe();
	CGentities::ClearThreadSafe();

	return engine_call<void>(0x005C4E30, 1);

}
void Fast_Restart_f()
{

	CClipMap::ClearThreadSafe();
	CGentities::ClearThreadSafe();

	return engine_call<void>(0x005C4E30, 0);

}