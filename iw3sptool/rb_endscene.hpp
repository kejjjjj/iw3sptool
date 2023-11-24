#pragma once

#include "pch.hpp"

void RB_FixEndscene();

char RB_DrawDebug(GfxViewParms* viewParms);
int RB_BeginSurface(MaterialTechniqueType tech, Material* material);
void RB_EndTessSurface();
void RB_SetPolyVertice(const vec3_t pos, const BYTE* col, const int vert, const int index, float* normal);
void RB_DrawPolyInteriors(int n_points, std::vector<fvec3>& points, const BYTE* color, bool two_sided = false, bool depthTest = false, bool show = false);
char RB_DrawLines3D(int count, int width, GfxPointVertex* verts, char depthTest);
void RB_DrawTriangleOutline(const fvec3& a, const fvec3& b, const fvec3& c, vec4_t color, int width = 3, bool depthTest = false);
int RB_AddDebugLine(GfxPointVertex* verts, char depthTest, const vec_t* start, vec_t* end, const BYTE* color, int vertCount);
void R_ConvertColorToBytes(const vec4_t in, uint8_t* out);
void R_AddDebugBox(const float* mins, const float* maxs, DebugGlobals* debugGlobaksEntry, float* color);
void RB_DrawCollisionPoly(int numPoints, float(*points)[3], const float* colorFloat, bool depthtest);
void RB_DrawCollisionEdges(int numPoints, float(*points)[3], const float* colorFloat, bool depthtest);

void CM_DrawPoly(Poly* poly, float* color);

HRESULT R_DrawXModelSkinnedCached(GfxCmdBufSourceState* src, GfxCmdBufState* state, GfxModelSkinnedSurface* modelSurf);

void CL_AddDebugString(int fromServer, float* xyz, float* color, float scale, char* text, int duration);