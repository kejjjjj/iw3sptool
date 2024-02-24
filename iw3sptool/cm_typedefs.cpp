#include "pch.hpp"

void cm_brush::render(const cm_renderinfo& info)
{
	if (info.only_colliding && brush->has_collision() == false)
		return;

	if (origin.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return;

	if (!CM_BrushInView(brush, info.frustum_planes, info.num_planes))
		return;

	for (auto& w : windings)
	{


		if (info.only_bounces && w.is_bounce == false)
			continue;

		if (info.only_elevators && w.is_elevator == false)
			continue;

		vec4_t c = { 0,1,1,0.3f };

		c[0] = w.color[0];
		c[1] = w.color[1];
		c[2] = w.color[2];
		c[3] = info.alpha;

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

		if (__brush::rb_requesting_to_stop_rendering)
			return;

		if (info.as_polygons)
			RB_DrawCollisionPoly(w.points.size(), (float(*)[3])w.points.data(), c, info.depth_test);
		else
			RB_DrawCollisionEdges(w.points.size(), (float(*)[3])w.points.data(), c, info.depth_test);
	}

	if (info.only_elevators == 2 && brush->has_collision()) {

		std::vector<fvec3> pts(2);

		for (auto& w : corners) {

			if (info.as_polygons)
				RB_DrawCollisionPoly(w->points.size(), (float(*)[3])w->points.data(), vec4_t{1,0,0,info.alpha}, info.depth_test);
			else
				RB_DrawCollisionEdges(w->points.size(), (float(*)[3])w->points.data(), vec4_t{ 1,0,0,info.alpha }, info.depth_test);

		}

	}

};
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

	//if (ele_windings.size() < 2)
	//	return;

	corners = ele_windings;

	//const auto get_corners = [](const cm_winding* target)
	//	{
	//		for (auto& p1 : target->points) {
	//			
	//		}
	//	};

	//const auto find_common_edge = [&ele_windings](const cm_winding* target) -> std::optional<cm_brush::ele_corner>
	//	{
	//		std::vector<fvec3> common_points;
	//		for (auto& w : ele_windings)
	//		{
	//			if ((DWORD)target == (DWORD)&w)
	//				continue;

	//			for (auto& p1 : target->points) {
	//				for (auto& p2 : w->points) {
	//					if (p1 == p2 && std::find(common_points.begin(), common_points.end(), p1) == common_points.end()) {
	//						common_points.push_back(p1);

	//					}
	//				}
	//			}
	//		}

	//		if (common_points.size() >= 2) {


	//			//if (fabs((common_points[0] - common_points[1]).toangles().x) == 90.f)
	//			return cm_brush::ele_corner{ common_points[0], common_points[1] };
	//		}

	//		return std::nullopt;

	//	};

	//for (auto& w : ele_windings)
	//{
	//	auto result = find_common_edge(w);

	//	if (result) {
	//		corners.push_back(result.value());
	//	}
	//}

}
int cm_brush::map_export(std::ofstream& o, int index)
{
	o << "// brush " << index << '\n';
	o << "{\n";

	for (auto& tri : triangles)
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

void cm_terrain::render(const cm_renderinfo& info)
{
	if (info.only_elevators)
		return;
	//if (children.size()) {

	//	for (auto& child : children)
	//		child.render(frustum);

	//	return;
	//}

	std::vector<fvec3> points(3);
	fvec3 center;

	for (auto& tri : tris)
	{
		if (tri.has_collision == false && info.only_colliding)
			continue;



		if ((tri.plane[2] < 0.3f || tri.plane[2] > 0.7f) && info.only_bounces)
			continue;

		if (tri.a.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
			continue;

		if (!CM_TriangleInView(&tri, info.frustum_planes, info.num_planes))
			continue;

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

		center.x = { (points[0].x + points[1].x + points[2].x) / 3 };
		center.y = { (points[0].y + points[1].y + points[2].y) / 3 };
		center.z = { (points[0].z + points[1].z + points[2].z) / 3 };

		if (center.dist(cgs->predictedPlayerState.origin) > info.draw_dist)
			continue;

		if (info.as_polygons)
			RB_DrawPolyInteriors(3, points, c, true, info.depth_test);
		else
			RB_DrawCollisionEdges(3, (float(*)[3])points.data(), c, info.depth_test);
	}
}
int cm_terrain::map_export(std::ofstream& o, int index)
{

	//if (children.size()) {

	//	for (auto& child : children)
	//		index = child.map_export(o, index);

	//	return index;
	//}
	//for (auto& quad : quads)
	//	index = map_export_quad(o, quad, index);
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
//int cm_terrain::map_export_quad(std::ofstream& o, const cm_quad& quad, int index) const
//{
//	auto& a = quad.verts[0];
//	auto& b = quad.verts[1];
//	auto& c = quad.verts[2];
//	auto& d = quad.verts[3];
//
//	o << "// brush " << index << '\n';
//	o << " {\n";
//	o << "  mesh\n";
//	o << "  {\n";
//	o << "   " << material << '\n';
//	o << "   lightmap_gray\n";
//	o << "   smoothing smoothing_hard\n";
//	o << "   2 2 16 8\n";
//	o << "   (\n";
//	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", a.x, a.y, a.z);
//	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", b.x, b.y, b.z);
//	o << "   )\n";
//	o << "   (\n";
//	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", c.x, c.y, c.z);
//	o << "    v " << std::format("{} {} {} t -5760 5824 -46 54\n", d.x, d.y, d.z);
//	o << "   )\n";
//	o << "  }\n";
//	o << " }\n";
//
//	return ++index;
//}
void cm_terrain::render2d()
{
	//if (children.size()) {

	//	for (auto& child : children)
	//		child.render2d();

	//	return;
	//}


	size_t index = 0;
	for (auto& tri : tris)
	{
		auto center = (tri.a + tri.b + tri.c) / 3;

		if (center.dist(cgs->predictedPlayerState.origin) > 1000) {
			index++;
			continue;
		}

		if (const auto pos_opt = WorldToScreen(center)) {
			auto& v = pos_opt.value();
			std::string str = std::to_string(index);
			R_DrawTextWithEffects(str, "fonts/bigDevFont", float(v.x), float(v.y), 1.f, 1.f, 0.f, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,1 });

		}

		index++;

	}
}
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

	for (unsigned short i = 0; i < cm->numBrushes; i++)
		CM_GetBrushWindings(&cm->brushes[i]);

	CM_DiscoverTerrain({ "all" });

	for (unsigned int i = 0; i < gfxWorld->dpvs.smodelCount; i++)
		CM_AddModel(&gfxWorld->dpvs.smodelDrawInsts[i]);


}