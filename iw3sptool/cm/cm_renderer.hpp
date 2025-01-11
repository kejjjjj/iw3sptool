#pragma once

#include <vector>

struct GfxViewParms;
struct cbrush_t;

template<typename T>
struct vec3;

using fvec3 = vec3<float>;
using ivec3 = vec3<int>;

void CM_DrawCollisionPoly(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest);
void CM_DrawCollisionEdges(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest);
void CM_ShowCollision(GfxViewParms* viewParms);

void CM_DrawBrushBounds(const cbrush_t* brush, bool depthTest);
