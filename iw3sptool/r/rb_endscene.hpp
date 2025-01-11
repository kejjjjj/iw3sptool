#pragma once

#include "cg/cg_local.hpp"
#include <vector>


char RB_DrawDebug(GfxViewParms* viewParms);
int RB_BeginSurface(MaterialTechniqueType tech, Material* material);
void RB_EndTessSurface();
void RB_SetPolyVertice(const vec3_t pos, const BYTE* col, const int vert, const int index, float* normal);
void RB_DrawPolyInteriors(const std::vector<fvec3>& points, const float* color, bool two_sided = false, bool depthTest = false);
char RB_DrawLines3D(int count, int width, GfxPointVertex* verts, char depthTest);
int RB_AddDebugLine(GfxPointVertex* verts, char depthTest, const vec_t* start, const vec_t* end, const float* color, int vertCount);
void R_ConvertColorToBytes(const vec4_t in, uint8_t* out);
void R_AddDebugBox(const float* mins, const float* maxs, DebugGlobals* debugGlobaksEntry, float* color);
void CM_DrawPoly(Poly* poly, float* color);
void CL_AddDebugString(int fromServer, float* xyz, float* color, float scale, char* text, int duration);