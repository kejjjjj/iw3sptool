#include "rb_endscene.hpp"
#include <cg/cg_offsets.hpp>
#include <cg/cg_view.hpp>
#include <cm/cm_brush.hpp>
#include "utils/hook.hpp"
#include <iostream>

void RB_TessOverflow(bool two_sided, bool depthTest)
{
	RB_EndTessSurface();
	RB_BeginSurfaceInternal(two_sided, depthTest);
}

bool RB_CheckTessOverflow(int vertexCount, int indexCount)
{
	if (vertexCount + tess->vertexCount > 5450 || indexCount + tess->indexCount > 0x100000)
		return true;

	return false;
}

void RB_BeginSurfaceInternal(bool two_sided, bool depthTest)
{
	static Material material;

	material = *rgp->whiteMaterial;

	static unsigned int loadBits[2] = { material.stateBitsTable->loadBits[0], material.stateBitsTable->loadBits[1] };
	static GfxStateBits bits = { .loadBits = { material.stateBitsTable->loadBits[0], material.stateBitsTable->loadBits[1] } };

	memcpy(material.stateBitsTable, rgp->whiteMaterial->stateBitsTable, sizeof(GfxStateBits));
	material.stateBitsTable = &bits;

	constexpr MaterialTechniqueType tech = MaterialTechniqueType::TECHNIQUE_UNLIT;
	static uint32_t ogBits = material.stateBitsTable->loadBits[1];

	if (tess->indexCount)
		RB_EndTessSurface();

	material.stateBitsTable->loadBits[1] = depthTest ? 44 : ogBits;
	material.stateBitsTable->loadBits[0] = two_sided ? 422072677 : 422089061;

	RB_BeginSurface(tech, &material);
}

void RB_DrawPolyInteriors(const std::vector<fvec3>& points, const float* c, bool two_sided, bool depthTest)
{
	GfxColor color;
	size_t idx{};

	auto n_points = points.size();

	//partly copied from iw3xo :)
	if (n_points < 3u)
		return;

	R_ConvertColorToBytes(c, &color);

	RB_BeginSurfaceInternal(two_sided, depthTest);
	

	if (RB_CheckTessOverflow(n_points, 3 * (n_points - 2)))
		RB_TessOverflow(two_sided, depthTest);

	for (; idx < n_points; ++idx) {
		RB_SetPolyVertice(points[idx].As<float*>(), color, tess->vertexCount + idx, idx, 0);
	}

	for (idx = 2; idx < n_points; ++idx) {
		tess->indices[tess->indexCount + 0] = static_cast<short>(tess->vertexCount);
		tess->indices[tess->indexCount + 1] = static_cast<short>(idx + tess->vertexCount);
		tess->indices[tess->indexCount + 2] = static_cast<short>(idx + tess->vertexCount - 1);
		tess->indexCount += 3;
	}

	tess->vertexCount += n_points;

	RB_EndTessSurface();


}
int RB_AddDebugLine(GfxPointVertex* verts, char depthTest, const vec_t* start, const vec_t* end, const float* color, int vertCount)
{

	int _vc = vertCount;
	if (vertCount + 2 > 2725)
	{
		RB_DrawLines3D(vertCount / 2, 1, verts, depthTest);
		_vc = 0;
	}

	GfxPointVertex* vert = &verts[_vc];
	if (color) {
		R_ConvertColorToBytes(color, (GfxColor*)&vert->color);
	}

	verts[_vc + 1].color[0] = vert->color[0];
	verts[_vc + 1].color[1] = vert->color[1];
	verts[_vc + 1].color[2] = vert->color[2];
	verts[_vc + 1].color[3] = vert->color[3];

	VectorCopy(start, vert->xyz);

	vert = &verts[_vc + 1];
	VectorCopy(end, vert->xyz);

	return _vc + 2;
}
void R_ConvertColorToBytes(const vec4_t in, GfxColor* out)
{
	((char(__fastcall*)(const float* in, GfxColor* out))0x464860)(in, out);

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
void RB_SetPolyVertice(float* xyz, const GfxColor color, const int vert, const int index, float* normal)
{
	VectorCopy(xyz, tess->verts[vert].xyzw);
	tess->verts[vert].color.packed = color.packed;

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