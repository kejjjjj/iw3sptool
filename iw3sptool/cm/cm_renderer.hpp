#pragma once

#include <vector>

struct GfxViewParms;
struct cbrush_t;
struct cm_renderinfo;
struct cm_geometry;

template<typename T>
struct vec3;

using fvec3 = vec3<float>;
using ivec3 = vec3<int>;

void CM_MakeInteriorRenderable(const std::vector<fvec3>& points, const float* color);
[[nodiscard]] int CM_MakeOutlinesRenderable(const std::vector<fvec3>& points, const float* color, bool depthTest, int nverts);

void CM_DrawCollisionPoly(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest);
void CM_DrawCollisionEdges(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest);
void CM_ShowCollision(GfxViewParms* viewParms);
