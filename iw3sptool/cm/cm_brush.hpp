#pragma once


#include "cg/cg_local.hpp"
#include "typedefs.hpp"
#include "cm_typedefs.hpp"

#include <vector>
#include <memory>
#include <unordered_set>

struct sc_winding_t
{
	std::vector<fvec3> points;
	bool is_bounce = false;
	bool is_elevator = false;
	fvec3 normals;
	vec4_t color;

};
enum class showCollisionType
{
	DISABLED,
	BRUSHES,
	TERRAIN,
	BOTH
};
enum class polyType
{
	EDGES,
	POLYS,
};

void CM_ShowCollisionFilter();
void CM_LoadAllBrushWindingsToClipMapWithFilter(const std::unordered_set<std::string>& filters);
void CM_LoadBrushWindingsToClipMap(const cbrush_t* brush);


std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col);


void CM_BuildAxialPlanes(float(*planes)[6][4], const cbrush_t* brush);
int GetPlaneIntersections(const float** planes, int planeCount, SimplePlaneIntersection* OutPts);
void CM_GetPlaneVec4Form(const cbrushside_t* sides, const float(*axialPlanes)[4], int index, float* expandedPlane);
int BrushToPlanes(const cbrush_t* brush, float(*outPlanes)[4]);
adjacencyWinding_t* BuildBrushdAdjacencyWindingForSide(int ptCount, SimplePlaneIntersection* pts, float* sideNormal, int planeIndex, adjacencyWinding_t* optionalOutWinding);

bool CM_BrushInView(const cbrush_t* brush, struct cplane_s* frustumPlanes, int numPlanes);
bool CM_BoundsInView(const fvec3& mins, const fvec3& maxs, struct cplane_s* frustumPlanes, int numPlanes);

char* CM_MaterialForNormal(const cbrush_t* target, const fvec3& normals);
std::vector<std::string> CM_GetBrushMaterials(const cbrush_t* brush);

namespace __brush
{
	void __asm_adjacency_winding();
}
bool CM_IsMatchingFilter(const std::unordered_set<std::string>& filters, char* material);
bool CM_BrushHasCollisions(const cbrush_t* brush);


std::vector<fvec3> CM_CreateCube(const fvec3& origin, const fvec3& size);
std::vector<fvec3> CM_CreateCube(const fvec3& origin, const fvec3& mins, const fvec3& maxs);
std::vector<fvec3> CM_CreateSphere(const fvec3& ref_org, const float radius, const int32_t latitudeSegments, const int32_t longitudeSegments, const fvec3& scale);
float RadiusFromBounds(const float* mins, const float* maxs);
