#pragma once

#include "cg/cg_local.hpp"
#include <vector>

void RB_TessOverflow(bool two_sided, bool depthTest);
bool RB_CheckTessOverflow(int vertexCount, int indexCount);
int RB_BeginSurface(MaterialTechniqueType tech, Material* material);
void RB_BeginSurfaceInternal(bool two_sided, bool depthTest);
void RB_EndTessSurface();


char RB_DrawDebug(GfxViewParms* viewParms);
void RB_SetPolyVertice(float* xyz, const GfxColor color, const int vert, const int index, float* normal);
void RB_DrawPolyInteriors(const std::vector<fvec3>& points, const float* color, bool two_sided = false, bool depthTest = false);
char RB_DrawLines3D(int count, int width, GfxPointVertex* verts, char depthTest);
int RB_AddDebugLine(GfxPointVertex* verts, char depthTest, const vec_t* start, const vec_t* end, const float* color, int vertCount);
void R_ConvertColorToBytes(const vec4_t in, GfxColor* out);
void CL_AddDebugString(int fromServer, float* xyz, float* color, float scale, char* text, int duration);