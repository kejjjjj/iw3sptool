#include "r_debug.hpp"

#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include <com/com_channel.hpp>

DebugGlobals* R_GetDebugGlobals()
{
	DebugGlobals* globptr = nullptr;
	__asm
	{
		mov eax, ds:1621DECh;
		add eax, 11E71Ch;
		mov globptr, eax;
	}

	return globptr;
}

void R_DebugAlloc(unsigned int _size, void** memPtr, const char* name)
{
	__asm
	{
		push name;
		push memPtr;
		mov edi, _size;
		mov esi, 0x5EE5E0;
		call esi;
		add esp, 8;
	}
}


void R_AddDebugLine(const float* start, const float* end, const float* color)
{
	DebugGlobals* debugGlobalsEntry = R_GetDebugGlobals();

	constexpr auto func = 0x5EE720;
	__asm
	{
		mov esi, debugGlobalsEntry;
		push color;
		push end;
		push start;
		call func;
		add esp, 12;
	}
}
void R_AddDebugBox(const float* mins, const float* maxs, const float* color)
{
	//DebugGlobals* globals = (DebugGlobals*)(0x1621DEC + 1173276);

	constexpr auto func = 0x5EE7D0;
	__asm
	{
		mov ecx, ds:1621DECh;
		add ecx, 11E71Ch;
		push color;
		push ecx;
		mov edx, maxs;
		mov eax, mins;
		call func;
		add esp, 8;
	}
}

void R_AddDebugPolygon(const std::vector<fvec3>& pts, const float* color)
{
	DebugGlobals* debugGlobalsEntry = R_GetDebugGlobals();

	int polyLimit{};
	const auto pointCount = (int)pts.size();

	EnterCriticalSection((LPCRITICAL_SECTION)0x13DDE88);
	debugGlobalsEntry->polyLimit = 512 << 7;
	debugGlobalsEntry->vertLimit = 4096 << 5;
	if (pointCount + debugGlobalsEntry->vertCount > debugGlobalsEntry->vertLimit || (polyLimit = debugGlobalsEntry->polyLimit, debugGlobalsEntry->polyCount + 1 > polyLimit)) {
		Com_Printf("Bye!\n");
		return LeaveCriticalSection((LPCRITICAL_SECTION)0x13DDE88);
	}
	
	auto p_polys = &debugGlobalsEntry->polys;
	if (!debugGlobalsEntry->polys) {
		R_DebugAlloc(24 * polyLimit, (void**)&debugGlobalsEntry->polys, "(debugGlobalsEntry->polys)");
		R_DebugAlloc(12 * debugGlobalsEntry->vertLimit, (void**)debugGlobalsEntry, "(debugGlobalsEntry->verts)");
	}
	if (*p_polys) {

		if (debugGlobalsEntry->verts)
		{
			(*p_polys)[debugGlobalsEntry->polyCount].firstVert = debugGlobalsEntry->vertCount;
			(*p_polys)[debugGlobalsEntry->polyCount].vertCount = pointCount;
			float* _color = (*p_polys)[debugGlobalsEntry->polyCount].color;
			_color[0] = color[0]; _color[1] = color[1];
			_color[2] = color[2]; _color[3] = color[3];
			auto verts = (float*)debugGlobalsEntry->verts;
			++debugGlobalsEntry->polyCount;
			memcpy(&verts[3 * debugGlobalsEntry->vertCount], pts.data(), 12 * pointCount);
			debugGlobalsEntry->vertCount += pointCount;
			return LeaveCriticalSection((LPCRITICAL_SECTION)0x13DDE88);
		}
	}
	
}
