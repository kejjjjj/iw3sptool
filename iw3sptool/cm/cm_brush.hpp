#pragma once

#include <vector>
#include <memory>
#include <unordered_set>

#include "cg/cg_local.hpp"
#include "typedefs.hpp"
#include "cm_typedefs.hpp"

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

using geom_ptr = std::vector<std::unique_ptr<cm_geometry>>;
class CClipMap
{
public:

	static void InsertGeometry(std::unique_ptr<cm_geometry>& geom) {

		if (geom)
			geometry.push_back(std::move(geom));

		wip_geom = nullptr;
	}
	static void clear_type(const cm_geomtype t)
	{
		auto itr = std::remove_if(geometry.begin(), geometry.end(), [&t](std::unique_ptr<cm_geometry>& g)
			{
				return g->type() == t;
			});

		geometry.erase(itr, geometry.end());

	}
	static std::vector<geom_ptr::iterator> get_all_of_type(const cm_geomtype t)
	{
		std::vector<geom_ptr::iterator> r;

		for (auto b = geometry.begin(); b != geometry.end(); ++b)
		{
			if (b->get()->type() == t)
				r.push_back(b);
		}

		return r;
	}
	static auto begin() { return geometry.begin(); }
	static auto end() { return geometry.end(); }
	static size_t size() { return geometry.size(); }
	static void clear() { geometry.clear(); wip_geom.reset(); }
	static std::unique_ptr<cm_geometry> wip_geom;
	static fvec3 wip_color;
	static auto& get() { return geometry; }

private:
	static geom_ptr geometry;
};

bool CM_IsMatchingFilter(const std::unordered_set<std::string>& filters, char* material);

std::unique_ptr<cm_geometry> CM_GetBrushPoints(cbrush_t* brush, const fvec3& poly_col);

void CM_GetBrushWindings(cbrush_t* brush);
void Cmd_CollisionFilter_f();

void CM_BuildAxialPlanes(float(*planes)[6][4], const cbrush_t* brush);
int GetPlaneIntersections(const float** planes, int planeCount, SimplePlaneIntersection* OutPts);
char* CM_MaterialForNormal(const cbrush_t* target, const fvec3& normals);
void CM_GetPlaneVec4Form(const cbrushside_t* sides, const float(*axialPlanes)[4], int index, float* expandedPlane);
int BrushToPlanes(const cbrush_t* brush, float(*outPlanes)[4]);
bool CM_BrushHasCollisions(const cbrush_t* brush);
adjacencyWinding_t* BuildBrushdAdjacencyWindingForSide(int ptCount, SimplePlaneIntersection* pts, float* sideNormal, int planeIndex, adjacencyWinding_t* optionalOutWinding);

void RB_ShowCollision(GfxViewParms* viewParms);
bool CM_BrushInView(const cbrush_t* brush, struct cplane_s* frustumPlanes, int numPlanes);
bool CM_BoundsInView(const fvec3& mins, const fvec3& maxs, struct cplane_s* frustumPlanes, int numPlanes);
std::vector<fvec3> CM_CreateCube(const fvec3& origin, const fvec3& size);
std::vector<fvec3> CM_CreateCube(const fvec3& origin, const fvec3& mins, const fvec3& maxs);

std::vector<fvec3> CM_CreateSphere(const fvec3& ref_org, const float radius, const int32_t latitudeSegments, const int32_t longitudeSegments, const fvec3& scale);
std::vector<std::string> CM_GetBrushMaterials(const cbrush_t* brush);
float RadiusFromBounds(const float* mins, const float* maxs);
namespace __brush
{
	inline bool rb_requesting_to_stop_rendering = false; //a silly way to handle multithreaded rendering contexts
	void __asm_adjacency_winding();
}