#pragma once

#include "typedefs.hpp"
#include "global_macros.hpp"

#include <unordered_set>
#include <string>
#include <fstream>
#include <mutex>
enum class cm_geomtype
{
	brush,
	terrain,
	model
};

struct cplane_s;
struct cbrush_t;
struct cLeaf_t;
struct CollisionAabbTree;
struct adjacencyWinding_t;
class CGameEntity;

struct cm_triangle
{
	fvec3 a;
	fvec3 b;
	fvec3 c;



	vec4_t plane = {};
	vec4_t color = {};
	char* material = {};
	bool has_collision = false;
	//bool edge_walkable = true;

	fvec3 get_mins() const noexcept {
		fvec3 lowest = FLT_MAX;

		lowest.x = a.x;
		if (b.x < lowest.x) lowest.x = b.x;
		if (c.x < lowest.x) lowest.x = c.x;

		lowest.y = a.y;
		if (b.y < lowest.y) lowest.y = b.y;
		if (c.y < lowest.y) lowest.y = c.y;

		lowest.z = a.z;
		if (b.z < lowest.z) lowest.z = b.z;
		if (c.z < lowest.z) lowest.z = c.z;

		return lowest;

	}
	fvec3 get_maxs() const noexcept {
		fvec3 highest = -FLT_MAX;

		highest.x = a.x;
		if (b.x > highest.x) highest.x = b.x;
		if (c.x > highest.x) highest.x = c.x;

		highest.y = a.y;
		if (b.y > highest.y) highest.y = b.y;
		if (c.y > highest.y) highest.y = c.y;

		highest.z = a.z;
		if (b.z > highest.z) highest.z = b.z;
		if (c.z > highest.z) highest.z = c.z;

		return highest;

	}
};
struct cm_winding
{
	cm_winding() = default;
	cm_winding(const std::vector<fvec3>& p, const fvec3& normal, const fvec3& col);

	std::vector<fvec3> points;
	fvec3 mins;
	fvec3 maxs;
	fvec3 normals;
	vec4_t color;
	bool is_bounce = {};
	bool is_elevator = {};

private:

	inline fvec3 get_mins() const noexcept
	{
		std::vector<float> x, y, z;

		for (auto& p : points) {
			x.push_back(p.x);
			y.push_back(p.y);
			z.push_back(p.z);

		}

		const float _x = *std::min_element(x.begin(), x.end());
		const float _y = *std::min_element(y.begin(), y.end());
		const float _z = *std::min_element(z.begin(), z.end());

		return { _x, _y, _z };
	}
	inline fvec3 get_maxs() const noexcept
	{
		std::vector<float> x, y, z;

		for (auto& p : points) {
			x.push_back(p.x);
			y.push_back(p.y);
			z.push_back(p.z);

		}

		const float _x = *std::max_element(x.begin(), x.end());
		const float _y = *std::max_element(y.begin(), y.end());
		const float _z = *std::max_element(z.begin(), z.end());

		return { _x, _y, _z };
	}
};

struct cm_renderinfo
{
	cplane_s* frustum_planes = {};
	int num_planes = {};
	float draw_dist = {};
	bool depth_test = {};
	bool as_polygons = {};
	bool only_colliding = {};
	bool only_bounces = {};
	int only_elevators = {};
	float alpha = 0.7f;


};

class brushModelEntity;

struct cm_geometry
{
	virtual ~cm_geometry() = default;
	virtual void render(const cm_renderinfo& info) const = 0;
	virtual cm_geomtype type() const noexcept = 0;
	virtual int map_export(std::ofstream& o, int index) const = 0;
	virtual void render2d() = 0;
	fvec3 origin;
	bool has_collisions = {};
	int originalContents = {};
	int num_verts = {};
};


struct cm_brush : public cm_geometry
{
	friend class CBrushModel;

	~cm_brush() = default;

	cm_geomtype type() const noexcept override { return cm_geomtype::brush; }

	void create_corners();
	void render(const cm_renderinfo& info) const override;
	void render2d() override {}

	friend void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2);
	friend std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col);

	cbrush_t* brush = {};

protected:
	int map_export(std::ofstream& o, int index) const override;
private:

	std::vector<cm_winding> windings; //used for rendering
	std::vector<cm_triangle> triangles; //used for exporting

	struct ele_corner { fvec3 mins; fvec3 maxs; };

	std::vector<const cm_winding*> corners;

};

struct cm_terrain : public cm_geometry
{
	friend class CBrushModel;

	~cm_terrain() = default;
	cm_terrain() = default;
	cm_terrain(const cLeaf_t* leaf, const std::unordered_set<std::string>& filters);

	[[nodiscard]] constexpr cm_geomtype type() const noexcept override { return cm_geomtype::terrain; }

	void render(const cm_renderinfo& info) const override;
	void render2d() override;

	[[nodiscard]] constexpr bool IsValid() const noexcept { return !!leaf; }

	friend void CM_AdvanceAabbTree(cm_terrain& terrain, const CollisionAabbTree* aabbTree, const std::unordered_set<std::string>& filters, const float* color);

protected:
	[[nodiscard]] int map_export(std::ofstream& o, int index) const override;
	[[nodiscard]] int map_export_triangle(std::ofstream& o, const cm_triangle& tri, int index) const;

private:
	const cLeaf_t* leaf = 0;
	std::vector<cm_triangle> tris;
	char* material = {};

};


struct cm_model : public cm_geometry
{
	~cm_model() = default;

	void render([[maybe_unused]] const cm_renderinfo& info) const override {};
	void render2d() override {};

	int map_export(std::ofstream& o, int index) const override;


	const char* name = {};
	fvec3 origin;
	fvec3 angles;
	float modelscale = {};


	cm_geomtype type() const noexcept override { return cm_geomtype::model; }

};

using GeometryPtr_t = std::unique_ptr<cm_geometry>;
using LevelGeometry_t = std::vector<GeometryPtr_t>;

using GentityPtr_t = std::unique_ptr<CGameEntity>;
using LevelGentities_t = std::vector<std::unique_ptr<CGameEntity>>;

class CClipMap
{
	NONCOPYABLE(CClipMap);

public:

	friend void CM_LoadBrushWindingsToClipMap(const cbrush_t* brush);
	friend std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col);
	friend void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2);

	static void Insert(GeometryPtr_t& geom);
	static void Insert(GeometryPtr_t&& geom);
	static void ClearAllOfType(const cm_geomtype t);
	static auto GetAllOfType(const cm_geomtype t);

	static void ClearAllOfTypeThreadSafe(const cm_geomtype t) { std::unique_lock<std::mutex> lock(mtx); ClearAllOfType(t); }

	//NOT thread safe
	static void RemoveBrushCollisionsBasedOnVolume(const float volume);

	//NOT thread safe
	static void RestoreBrushCollisions();

	static auto begin() { return m_pLevelGeometry.begin(); }
	static auto end() { return m_pLevelGeometry.end(); }
	[[nodiscard]] static size_t Size() { return m_pLevelGeometry.size(); }
	static void Clear() { CClipMap::RestoreBrushCollisions(); m_pLevelGeometry.clear(); m_pWipGeometry.reset(); }
	static void ClearThreadSafe() { std::unique_lock<std::mutex> lock(mtx); Clear(); }

	[[nodiscard]] inline static auto& GetLock() { return mtx; }

	template<typename Func>
	static void ForEach(Func func) {

		for (auto& geo : m_pLevelGeometry)
			func(geo);

	}

private:
	static std::unique_ptr<cm_geometry> m_pWipGeometry;
	static fvec3 m_vecWipGeometryColor;
	static LevelGeometry_t m_pLevelGeometry;

	static std::mutex mtx;
};

class CGentities
{
public:

	static void CM_LoadAllEntitiesToClipMapWithFilters(const std::unordered_set<std::string>& filters);

	static void Insert(GentityPtr_t& geom);
	static void Insert(GentityPtr_t&& geom);

	static auto begin();
	static auto end();
	[[nodiscard]] static size_t Size();
	static void Clear();
	static void ClearThreadSafe();

	static void Repopulate() { m_bRePopulate = true; }
	[[nodiscard]] static auto TimeToRepopulate() { return m_bRePopulate; }
	static void Depopulate() { m_bRePopulate = false; }

	[[nodiscard]] inline static auto& GetLock() { return mtx; }

	template<typename Func>
	static void ForEach(Func func) {

		for (auto& geo : m_pLevelGentities)
			func(geo);

	}

private:
	static LevelGentities_t m_pLevelGentities;
	static std::mutex mtx;
	static bool m_bRePopulate;

};

void CM_LoadMap();
bool CM_IsMatchingFilter(const std::unordered_set<std::string>& filters, const char* material);
