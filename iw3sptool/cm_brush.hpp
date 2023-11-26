#pragma once

#include "pch.hpp"

struct sc_winding_t
{
	std::vector<fvec3> points;
	bool is_bounce = false;
	bool is_elevator = false;
	fvec3 normals;
	vec4_t color;

};

struct showcol_brush
{
	std::vector<sc_winding_t> windings;
	cbrush_t* brush;
	int numVerts = 0;
	fvec3 origin;
	//std::vector<SimplePlaneIntersection> intersections;
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

inline std::vector<showcol_brush> s_brushes;

showcol_brush CM_GetBrushWindings(cbrush_t* brush, vec4_t polycolor);
void Cmd_CollisionFilter_f();

void CM_BuildAxialPlanes(float(*planes)[6][4], const cbrush_t* brush);
int GetPlaneIntersections(const float** planes, int planeCount, SimplePlaneIntersection* OutPts);
void CM_GetPlaneVec4Form(const cbrushside_t* sides, const float(*axialPlanes)[4], int index, float* expandedPlane);
int BrushToPlanes(const cbrush_t* brush, float(*outPlanes)[4]);
adjacencyWinding_t* BuildBrushdAdjacencyWindingForSide(int ptCount, SimplePlaneIntersection* pts, float* sideNormal, int planeIndex, adjacencyWinding_t* optionalOutWinding);
bool PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c);

void RB_ShowCollision(GfxViewParms* viewParms);
void RB_RenderWinding(const showcol_brush& sb, polyType poly_type, bool depth_test, float drawdist, bool only_bounces, bool only_elevators);
bool CM_BrushInView(const cbrush_t* brush, struct cplane_s* frustumPlanes, int numPlanes);
bool CM_BoundsInView(const fvec3& mins, const fvec3& maxs, struct cplane_s* frustumPlanes, int numPlanes);
std::vector<fvec3> CM_CreateCube(const fvec3& origin, const fvec3& size);
std::vector<fvec3> CM_CreateSphere(const fvec3& ref_org, const float radius, const int32_t latitudeSegments, const int32_t longitudeSegments, const fvec3& scale);

inline bool rb_requesting_to_stop_rendering = false;
namespace __brush_hook
{
	void stealerino();
}