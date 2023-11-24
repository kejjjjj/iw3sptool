#include "pch.hpp"

void RB_DrawPolyInteriors(int n_points, std::vector<fvec3>& points, const BYTE* color, bool two_sided, bool depthTest, bool show)
{
	if (n_points < 3)
		return;

	Material material = *rgp->whiteMaterial;

	static unsigned int loadBits[2] = { material.stateBitsTable->loadBits[0], material.stateBitsTable->loadBits[1] };
	static GfxStateBits bits = { .loadBits = { material.stateBitsTable->loadBits[0], material.stateBitsTable->loadBits[1] } };

	memcpy(material.stateBitsTable, rgp->whiteMaterial->stateBitsTable, sizeof(GfxStateBits));
	material.stateBitsTable = &bits;
	constexpr MaterialTechniqueType tech = MaterialTechniqueType::TECHNIQUE_UNLIT;
	static uint32_t ogBits = material.stateBitsTable->loadBits[1];

	if (gfxCmdBufState->origMaterial != &material || gfxCmdBufState->origTechType != tech) {
		if (tess->indexCount)
			RB_EndTessSurface();

		if (depthTest)
			material.stateBitsTable->loadBits[1] = 44;
		else
			material.stateBitsTable->loadBits[1] = ogBits;

		if (two_sided)
			material.stateBitsTable->loadBits[0] = 422072677;
		else
			material.stateBitsTable->loadBits[0] = 422089061;

		//material.stateBitsTable->loadBits[1] = 44;

		RB_BeginSurface(tech, &material);

	}
	if (n_points + tess->vertexCount > 5450 || tess->indexCount + 2 * (n_points - 2) > 1048576)// RB_CheckTessOverflow
	{
		RB_EndTessSurface();
		RB_BeginSurface(gfxCmdBufState->origTechType, gfxCmdBufState->origMaterial);
	}
	int idx = 0;

	for (; idx < n_points; ++idx) {
		vec3_t p = { points[idx].x, points[idx].y, points[idx].z };
		RB_SetPolyVertice(p, color, tess->vertexCount + idx, idx, 0);
	}

	for (idx = 2; idx < n_points; ++idx)
	{
		tess->indices[tess->indexCount + 0] = tess->vertexCount;
		tess->indices[tess->indexCount + 1] = (idx + tess->vertexCount);
		tess->indices[tess->indexCount + 2] = (idx + tess->vertexCount - 1);
		tess->indexCount += 3;
	}

	tess->vertexCount += n_points;

	RB_EndTessSurface();


}
int RB_AddDebugLine(GfxPointVertex* verts, char depthTest, const vec_t* start, vec_t* end, const BYTE* color, int vertCount)
{
	int _vc = vertCount;
	uint8_t _color[4]{ 0,0,0,0 };
	if (vertCount + 2 > 2725)
	{
		//RB_DrawLines3D(vertCount / 2, 1, verts, depthTest);
		_vc = 0;
	}

	//if (color) {
	//	R_ConvertColorToBytes(color, vert->color);
	//}

	VectorCopy(color, verts[_vc].color);
	verts[_vc].color[3] = color[3];
	verts[_vc + 1].color[0] = verts[_vc].color[0];
	verts[_vc + 1].color[1] = verts[_vc].color[1];
	verts[_vc + 1].color[2] = verts[_vc].color[2];
	verts[_vc + 1].color[3] = verts[_vc].color[3];

	VectorCopy(start, verts[_vc].xyz);
	VectorCopy(end, verts[_vc + 1].xyz);


	return _vc + 2;

	//return ((int(*)(GfxPointVertex *, char, const vec_t *, vec_t *, const vec_t *, int))0x658210)(verts, depthTest, start, end, color, vertCount);
}
void R_ConvertColorToBytes(const vec4_t in, uint8_t* out)
{
	//__asm
	//{
	//	lea edx, out;
	//	mov ecx, in;
	//	mov esi, 0x493530;
	//	call esi;
	//}
	((char(__fastcall*)(const float* in, uint8_t * out))0x464860)(in, out);

	return;
}
char RB_DrawLines3D(int count, int width, GfxPointVertex* verts, char depthTest)
{
	if(count >= 2)
		((char(__cdecl*)(int, int, GfxPointVertex*, char))0x5FD130)(count, width, verts, depthTest);
	return 1;

}
int RB_BeginSurface(MaterialTechniqueType tech, Material* material)
{
	int rval = 0;
	const static DWORD fnc = 0x6026B0;
	__asm
	{
		mov edi, tech;
		mov esi, material;
		call fnc;
		mov rval, eax;
	}
	return rval;
}
void RB_EndTessSurface()
{
	((void(*)())0x602780)();

}
int Vec3PackUnitVec(float* v) {
	int r = 0;
	__asm
	{
		mov eax, v;
		mov esi, 0x580C40;
		call esi;
		mov r, eax;
	}
	return r;
}
void RB_SetPolyVertice(const vec3_t pos, const BYTE* col, const int vert, const int index, float* normal)
{
	VectorCopy(pos, tess->verts[vert].xyzw);
	//tess->verts[vert].color.packed = 0xFF00FFAA;
	tess->verts[vert].color.array[0] = col[0];
	tess->verts[vert].color.array[1] = col[1];
	tess->verts[vert].color.array[2] = col[2];
	tess->verts[vert].color.array[3] = col[3];

	//std::cout << "color: " << std::hex << tess->verts[vert].color.packed << '\n';

	switch (index)
	{
	case 0:
		tess->verts[vert].texCoord[0] = 0.0f;
		tess->verts[vert].texCoord[1] = 0.0f;
		break;

	case 1:
		tess->verts[vert].texCoord[0] = 0.0f;
		tess->verts[vert].texCoord[1] = 1.0f;
		break;
	case 2:
		tess->verts[vert].texCoord[0] = 1.0f;
		tess->verts[vert].texCoord[1] = 1.0f;
		break;
	case 3:
		tess->verts[vert].texCoord[0] = 1.0f;
		tess->verts[vert].texCoord[1] = 0.0f;
		break;

	default:
		tess->verts[vert].texCoord[0] = 0.0f;
		tess->verts[vert].texCoord[1] = 0.0f;
		break;
	}

	tess->verts[vert].normal.packed = normal ? Vec3PackUnitVec(normal) : 1073643391;
}
char RB_DrawDebug(GfxViewParms* viewParms)
{
	decltype(auto) detour_func = find_hook(hookEnums_e::HOOK_RB_ENDSCENE);

	RB_ShowCollision(viewParms);


	return detour_func.cast_call<char(*)(GfxViewParms*)>(viewParms);
}

void RB_DrawTriangleOutline(const fvec3& a1, const fvec3& b1, const fvec3& c1, vec4_t color, int width, bool depthTest)
{
	GfxPointVertex verts[6]{};

	BYTE c[4];

	R_ConvertColorToBytes(color, c);

	RB_AddDebugLine(verts, depthTest, a1, b1, c, 0);
	RB_AddDebugLine(verts, depthTest, a1, c1, c, 2);
	RB_AddDebugLine(verts, depthTest, b1, c1, c, 4);

	RB_DrawLines3D(3, width, verts, depthTest);

}
void R_AddDebugBox(const float* mins, const float* maxs, DebugGlobals* debugGlobalsEntry, float* color)
{
	__asm
	{
		push color;
		push debugGlobalsEntry;
		mov edx, maxs;
		mov eax, mins;
		mov esi, 0x60DC60;
		call esi;
		add esp, 0x8;
	}
}
void CM_DrawPoly(Poly* poly, float* color)
{
	__asm
	{
		push color;
		push poly;
		mov esi, 0x597200;
		call esi;
		add esp, 0x8;
	}
}
void CL_AddDebugString(int fromServer, float* xyz, float* color, float scale, char* text, int duration)
{
	static DWORD const addr = 0x43D170;
	__asm
	{
		mov esi, fromServer;
		push duration;
		push text;
		push scale;
		push color;
		push xyz;
		call addr;
		add esp, 20;

	}
}