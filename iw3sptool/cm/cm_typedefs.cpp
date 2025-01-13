#include "cg/cg_offsets.hpp"
#include "cm/cm_entity.hpp"
#include "cm_brush.hpp"
#include "cm_model.hpp"
#include "cm_renderer.hpp"
#include "cm_terrain.hpp"
#include "cm_typedefs.hpp"
#include "com/com_vector.hpp"
#include "r/r_drawtools.hpp"
#include "r/rb_endscene.hpp"

#include <algorithm>
#include <iostream>
#include <ranges>

LevelGeometry_t CClipMap::m_pLevelGeometry;
std::unique_ptr<cm_geometry> CClipMap::m_pWipGeometry;
fvec3 CClipMap::m_vecWipGeometryColor;
std::mutex CClipMap::mtx;

cm_winding::cm_winding(const std::vector<fvec3>& p, const fvec3& normal, const fvec3& col) : points(p), normals(normal)
{
	is_bounce = normal[2] >= 0.3f && normal[2] <= 0.7f;
	is_elevator = std::fabs(normal[0]) == 1.f || std::fabs(normal[1]) == 1.f;
	normals = normal;
	VectorCopy(col, color);

	//if (rgp && rgp->world) {
	//	fvec3 new_color = SetSurfaceBrightness(col, normal, rgp->world->sunParse.angles);
	//	VectorCopy(new_color, color);
	//}
	color[3] = 0.7f;

	mins = get_mins();
	maxs = get_maxs();
}

void cm_brush::create_corners()
{
	//get all ele surfaces
	std::vector<const cm_winding*> ele_windings;

	std::for_each(windings.begin(), windings.end(), [&ele_windings](const cm_winding& w)
		{
			const bool is_elevator = std::fabs(w.normals[0]) == 1.f || std::fabs(w.normals[1]) == 1.f;
			if (w.normals[2] == 0.f && !is_elevator)
				ele_windings.push_back(&w);
		});

	corners = ele_windings;

}
int cm_brush::map_export(std::ofstream& o, int index) const
{
	std::vector<cm_triangle> new_points = triangles;

	o << "// brush " << index << '\n';
	o << "{\n";

	for (auto& tri : new_points)
	{

		o << std::format(" ( {} {} {} )", tri.a.x, tri.a.y, tri.a.z);
		o << std::format(" ( {} {} {} )", tri.b.x, tri.b.y, tri.b.z);
		o << std::format(" ( {} {} {} )", tri.c.x, tri.c.y, tri.c.z);

		std::string material = tri.material == nullptr ? "caulk" : tri.material;

		o << " " << material;
		o << " 128 128 0 0 0 0 lightmap_gray 16384 16384 0 0 0 0\n";

	}

	o << "}\n";

	return ++index;
}
bool cm_brush::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const
{

	if (info.only_colliding && brush->has_collision() == false)
		return false;

	if (origin.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return false;

	if (!CM_BoundsInView(mins, maxs, info.frustum_planes, info.num_planes))
		return false;

	bool state = false;
	for (const auto& w : windings) {

		if (RB_CheckTessOverflow(num_verts, 3 * (num_verts - 2)))
			RB_TessOverflow(true, info.depth_test);

		if (info.only_bounces && w.is_bounce == false)
			continue;

		if (info.only_elevators && w.is_elevator == false)
			continue;

		vec4_t c = { 0,1,0, info.alpha };

		//only use sunlight when there are no outlines in a polygon
		//if (info.poly_render_type == pt_polys) {
			c[0] = w.color[0];
			c[1] = w.color[1];
			c[2] = w.color[2];
		//}

		if (info.only_bounces) {
			float n = w.normals[2];

			if (n > 0.7f || n < 0.3f)
				n = 0.f;
			else
				n = 1.f - (n - 0.3f) / (0.7f - 0.3f);

			c[0] = 1.f - n;
			c[1] = n;
			c[2] = 0.f;
		}

		CM_MakeInteriorRenderable(w.points, c);
		state = true;
	}

	return state;
}

bool cm_brush::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const
{

	if (info.only_colliding && brush->has_collision() == false)
		return false;

	if (origin.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return false;

	if (!CM_BoundsInView(mins, maxs, info.frustum_planes, info.num_planes))
		return false;



	for (const auto& w : windings) {

		if (info.only_bounces && w.is_bounce == false)
			continue;

		if (info.only_elevators && w.is_elevator == false)
			continue;

		vec4_t c = { w.color[0],w.color[1],w.color[2],info.alpha};

		if (info.only_bounces) {
			float n = w.normals[2];

			if (n > 0.7f || n < 0.3f)
				n = 0.f;
			else
				n = 1.f - (n - 0.3f) / (0.7f - 0.3f);

			c[0] = 1.f - n;
			c[1] = n;
			c[2] = 0.f;
		}

		nverts = CM_MakeOutlinesRenderable(w.points, c, info.depth_test, nverts);
	}

	return true;
}
int cm_terrain::map_export(std::ofstream& o, int index) const
{
	for (auto& tri : tris)
		index = map_export_triangle(o, tri, index);

	return index;

}
int cm_terrain::map_export_triangle(std::ofstream& o, const cm_triangle& tri, int index) const
{
	o << "// brush " << index << '\n';
	o << " {\n";
	o << "  mesh\n";
	o << "  {\n";
	o << "   " << material << '\n';
	o << "   lightmap_gray\n";
	o << "   smoothing smoothing_hard\n";
	o << "   2 2 16 8\n";
	o << "   (\n";
	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", tri.a.x, tri.a.y, tri.a.z);
	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", tri.b.x, tri.b.y, tri.b.z);
	o << "   )\n";
	o << "   (\n";
	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", tri.c.x, tri.c.y, tri.c.z);
	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", tri.c.x, tri.c.y, tri.c.z);
	o << "   )\n";
	o << "  }\n";
	o << " }\n";

	return ++index;
}
void cm_terrain::render2d()
{
	size_t index = 0;
	for (auto& tri : tris)
	{
		auto center = (tri.a + tri.b + tri.c) / 3;

		if (center.dist(cgs->predictedPlayerState.origin) > 1000) {
			index++;
			continue;
		}

		if (const auto op = WorldToScreen(center)) {
			R_DrawTextWithEffects(std::to_string(index), "fonts/bigDevFont", op->x, op->y, 1.f, 1.f, 0.f, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,1 });
		}

		index++;

	}
}

bool cm_terrain::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const
{
	if (info.only_elevators)
		return false;

	std::vector<fvec3> points(3);

	bool state = false;

	for (const auto& tri : tris) {

		if (tri.has_collision == false && info.only_colliding)
			continue;

		if ((tri.plane[2] < 0.3f || tri.plane[2] > 0.7f) && info.only_bounces)
			continue;

		if (tri.a.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
			continue;

		if (!CM_TriangleInView(&tri, info.frustum_planes, info.num_planes))
			continue;

		if (RB_CheckTessOverflow(num_verts, 3 * (num_verts - 2)))
			RB_TessOverflow(true, info.depth_test);

		vec4_t c =
		{
			tri.color[0],
			tri.color[1],
			tri.color[2],
			info.alpha
		};

		if (info.only_bounces) {
			float n = tri.plane[2];

			if (n > 0.7f || n < 0.3f)
				n = 0.f;
			else
				n = 1.f - (n - 0.3f) / (0.7f - 0.3f);

			c[0] = 1.f - n;
			c[1] = n;
			c[2] = 0.f;
		}


		points[0] = (tri.a);
		points[1] = (tri.b);
		points[2] = (tri.c);

		CM_MakeInteriorRenderable(points, c);

		state = true;
	}

	return state;
}
bool cm_terrain::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const
{

	if (info.only_elevators)
		return false;

	std::vector<fvec3> points(3);

	bool state = false;


	for (const auto& tri : tris) {

		if (tri.has_collision == false && info.only_colliding)
			continue;

		if ((tri.plane[2] < 0.3f || tri.plane[2] > 0.7f) && info.only_bounces)
			continue;

		if (tri.a.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
			continue;

		if (!CM_TriangleInView(&tri, info.frustum_planes, info.num_planes))
			continue;

		if (RB_CheckTessOverflow(num_verts, 3 * (num_verts - 2)))
			RB_TessOverflow(true, info.depth_test);

		vec4_t c =
		{
			tri.color[0],
			tri.color[1],
			tri.color[2],
			info.alpha
		};

		if (info.only_bounces) {
			float n = tri.plane[2];

			if (n > 0.7f || n < 0.3f)
				n = 0.f;
			else
				n = 1.f - (n - 0.3f) / (0.7f - 0.3f);

			c[0] = 1.f - n;
			c[1] = n;
			c[2] = 0.f;
		}

		points[0] = (tri.a);
		points[1] = (tri.b);
		points[2] = (tri.c);

		nverts = CM_MakeOutlinesRenderable(points, c, info.depth_test, nverts);
		state = true;
	}

	return state;
}
void CClipMap::Insert(std::unique_ptr<cm_geometry>& geom) {

	if (geom)
		m_pLevelGeometry.emplace_back(std::move(geom));

	m_pWipGeometry = nullptr;
}
void CClipMap::Insert(std::unique_ptr<cm_geometry>&& geom) {

	if (geom)
		m_pLevelGeometry.emplace_back(std::move(geom));

	m_pWipGeometry = nullptr;
}
void CClipMap::ClearAllOfType(const cm_geomtype t)
{
	if (t == cm_geomtype::brush)
		CClipMap::RestoreBrushCollisions();

	auto itr = std::remove_if(m_pLevelGeometry.begin(), m_pLevelGeometry.end(), [&t](std::unique_ptr<cm_geometry>& g)
		{
			return g->type() == t;
		});

	m_pLevelGeometry.erase(itr, m_pLevelGeometry.end());

}
auto CClipMap::GetAllOfType(const cm_geomtype t)
{
	std::vector<LevelGeometry_t::iterator> r;

	for (auto b = m_pLevelGeometry.begin(); b != m_pLevelGeometry.end(); ++b)
	{
		if (b->get()->type() == t)
			r.push_back(b);
	}

	return r;
}

void CClipMap::RemoveBrushCollisionsBasedOnVolume(const float volume)
{
	for (const auto& geom : m_pLevelGeometry) {

		if (geom->type() != cm_geomtype::brush)
			continue;

		const auto pbrush = dynamic_cast<cm_brush*>(&*geom);
		const float geomVolume = ((fvec3&)pbrush->brush->maxs - (fvec3&)pbrush->brush->mins).mag();

		if (geomVolume >= volume)
			pbrush->brush->contents &= ~0x10000;
	}
}

void CClipMap::RestoreBrushCollisions()
{
	for (const auto& geom : m_pLevelGeometry) {

		if (geom->type() != cm_geomtype::brush)
			continue;

		const auto pbrush = dynamic_cast<cm_brush*>(&*geom);
		pbrush->brush->contents = pbrush->originalContents;
	}
}

/***********************************************************************Z
 >
***********************************************************************/

LevelGentities_t CGentities::m_pLevelGentities;
std::mutex CGentities::mtx;
bool CGentities::m_bRePopulate{ false };

void CGentities::Insert(GentityPtr_t& geom)
{
	m_pLevelGentities.emplace_back(std::move(geom));
}
void CGentities::Insert(GentityPtr_t&& geom)
{
	m_pLevelGentities.emplace_back(std::forward<GentityPtr_t&&>(geom));
}

auto CGentities::begin() { return m_pLevelGentities.begin(); }
auto CGentities::end() { return m_pLevelGentities.end(); }
size_t CGentities::Size() { return m_pLevelGentities.size(); }
void CGentities::Clear() { m_pLevelGentities.clear(); }
void CGentities::ClearThreadSafe() { std::unique_lock<std::mutex> lock(mtx); Clear(); }

bool CM_IsMatchingFilter(const std::unordered_set<std::string>& filters, const char* material)
{

	for (const auto& filter : filters) {

		if (filter == "all" || std::string(material).contains(filter))
			return true;
	}

	return false;
}

void CM_LoadMap()
{
	CClipMap::ClearThreadSafe();

	std::unique_lock<std::mutex> lock(CClipMap::GetLock());

	for (unsigned short i = 0; i < cm->numBrushes; i++)
		CM_LoadBrushWindingsToClipMap(&cm->brushes[i]);

	CM_DiscoverTerrain({ "all" });

	for (unsigned int i = 0; i < gfxWorld->dpvs.smodelCount; i++)
		CM_AddModel(&gfxWorld->dpvs.smodelDrawInsts[i]);
}