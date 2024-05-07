#include "cm_entity.hpp"
#include <cg/cg_offsets.hpp>
#include "cm_brush.hpp"
#include "cm_terrain.hpp"
#include <dvar/dvar.hpp>
#include <utils/functions.hpp>
#include <com/com_vector.hpp>
#include <r/r_drawtools.hpp>
#include "r/rb_endscene.hpp"
#include <scr/scr.hpp>

void gameEntity::render2d(float draw_dist)
{
	fvec3 org = g->r.currentOrigin;
	fvec3 maxs = g->r.maxs;

	org.z += (g->r.maxs[2] - g->r.mins[2]) / 2;

	auto center = org;
	float dist = center.dist(predictedPlayerState->origin);

	if (dist > draw_dist)
		return;

	if (fields) {
		std::string buff;
		for (auto& field : fields->key_value)
		{
			buff += std::string(field.first) + " - " + field.second + "\n";
		}

		if (auto op = WorldToScreen(center)) {
			const auto p = (fvec2)op.value();
			const float scale = ScaleByDistance(dist) * 0.2f;
			R_DrawTextWithEffects(buff, "fonts/bigdevFont", p.x, p.y, scale, scale, 0, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,0 });
		}

	}
}

brushModelEntity::brushModelEntity(gentity_s* gent) : gameEntity(gent) {

	auto leaf = &cm->cmodels[g->s.index.brushmodel].leaf;

	auto& leafBrushNode = cm->leafbrushNodes[leaf->leafBrushNode];
	short numBrushes = leafBrushNode.leafBrushCount;

	//a brush
	if (numBrushes >= 1) {
		for (short i = 0; i < numBrushes; i++) {
			uint16_t brushIdx = leafBrushNode.data.leaf.brushes[i];

			if (brushIdx >= cm->numBrushes)
				return;

			brushmodel bmodel(g);

			bmodel.linked_brush = &cm->brushes[brushIdx];
			auto result = CM_GetBrushPoints(bmodel.linked_brush, vec4_t{ 0.984f, 0.494f, 0.004f });
			bmodel.brush_geometry = *dynamic_cast<cm_brush*>(result.get());
			bmodel.original_geometry = bmodel.brush_geometry;

			brushmodels.push_back(std::move(std::make_unique<brushmodel>(bmodel)));
		}
	}
	if (const auto t = CM_GetTerrainTriangles(leaf, { "all" })) {
		terrainmodel tm(g);

		tm.leaf = leaf;
		tm.terrain = *dynamic_cast<cm_terrain*>(t.get());
		tm.original_terrain = tm.terrain;

		brushmodels.push_back(std::move(std::make_unique<terrainmodel>(tm)));
	}

}
void brushModelEntity::render(const cm_renderinfo& info) {

	const showCollisionType cm_type = static_cast<showCollisionType>(Dvar_FindMalleableVar("cm_showCollision")->current.integer);
	if (*origin != oldOrigin || *orientation != oldOrientation) {

		for (auto& b : brushmodels) {
			b->on_position_changed(*origin, *orientation);
		}

		oldOrigin = *origin;
		oldOrientation = *orientation;
	}

	for (auto& b : brushmodels) {

		if (b->get_type() == brushmodel_type::BRUSH && cm_type == showCollisionType::TERRAIN ||
			b->get_type() == brushmodel_type::TERRAIN && cm_type == showCollisionType::BRUSHES)
			continue;

		b->render(*origin, info);
	}

}
void brushModelEntity::render2d(float draw_dist)
{

	for (auto& b : brushmodels)
	{
		auto center = b->get_center();
		float dist = center.dist(predictedPlayerState->origin);

		if (dist > draw_dist)
			continue;

		if (fields) {
			std::string buff;
			for (auto& field : fields->key_value)
			{
				buff += std::string(field.first) + " - " + field.second + "\n";
			}

			if (auto op = WorldToScreen(center)) {
				auto p = (fvec2)op.value();
				float scale = ScaleByDistance(dist) * 0.2f;
				R_DrawTextWithEffects(buff, "fonts/bigdevFont", p.x, p.y, scale, scale, 0, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,0 });

			}

		}
	}
}

void brushModelEntity::brushmodel::render(const fvec3& _origin, const cm_renderinfo& info) {

	if (brush_geometry.brush == nullptr)
		return;

	fvec3 mins = _origin + fvec3(linked_brush->mins);
	fvec3 maxs = _origin + fvec3(linked_brush->maxs);

	if (brush_geometry.origin.dist(predictedPlayerState->origin) > info.draw_dist)
		return;

	if (CM_BoundsInView(mins, maxs, info.frustum_planes, info.num_planes) == false)
		return;

	for (auto& w : brush_geometry.windings)
	{
		vec4_t c = { 0,1,1,0.3f };

		c[0] = w.color[0];
		c[1] = w.color[1];
		c[2] = w.color[2];
		c[3] = info.alpha;


		auto func = info.as_polygons ? RB_DrawCollisionPoly : RB_DrawCollisionEdges;
		func(w.points.size(), (float(*)[3])w.points.data(), vec4_t{ 1,0,0,0.3f }, info.depth_test);
	}

}

void brushModelEntity::brushmodel::on_position_changed(const fvec3& _origin, const fvec3& delta_angles) noexcept(true) {
	brush_geometry = original_geometry;
	brush_geometry.origin = _origin;

	for (auto& winding : brush_geometry.windings) {
		for (auto& w : winding.points) {
			w += _origin;

			w = VectorRotate(w, delta_angles, brush_geometry.origin);
		}
	}
}
fvec3 brushModelEntity::brushmodel::get_center() const noexcept(true) {
	return g->r.currentOrigin;
}


void brushModelEntity::terrainmodel::render([[maybe_unused]] const fvec3& _origin, const cm_renderinfo& info) {
	terrain.render(info);
}
void brushModelEntity::terrainmodel::on_position_changed(const fvec3& _origin, const fvec3& delta_angles) noexcept(true) {
	terrain = original_terrain;

	for (auto& t : terrain.tris) {
		t.a += _origin;
		t.b += _origin;
		t.c += _origin;

		fvec3 c = g->r.currentOrigin;

		t.a = VectorRotate(t.a, delta_angles, c);
		t.b = VectorRotate(t.b, delta_angles, c);
		t.c = VectorRotate(t.c, delta_angles, c);

	}
}
fvec3 brushModelEntity::terrainmodel::get_center() const noexcept(true) {
	return g->r.currentOrigin;

}


spawnerEntity::spawnerEntity(gentity_s* gent) : gameEntity(gent),
	geometry(CM_CreateSphere({ origin->x, origin->y, origin->z + 36 }, 1.f, 5, 5, { 16,16,36 })) 
{
}

void spawnerEntity::render(const cm_renderinfo& info) {

	if (*origin != oldOrigin || *orientation != oldOrientation) {

		fvec3 org = g->r.currentOrigin;
		fvec3 maxs = g->r.maxs;

		org.z += (maxs.z - g->r.mins[2]) / 2;
		maxs.z /= 2;

		geometry = CM_CreateSphere(org, 1.f, 5, 5, maxs);

		oldOrigin = *origin;
		oldOrientation = *orientation;
	}

	if (origin->dist(predictedPlayerState->origin) > info.draw_dist)
		return;

	auto func = info.as_polygons ? RB_DrawCollisionPoly : RB_DrawCollisionEdges;

	func(geometry.size(), (float(*)[3])geometry.data(), vec4_t{ 1,0,0,0.3f }, info.depth_test);

}


void radiusEntity::refresh_geometry() {

	float radius = RadiusFromBounds(g->r.mins, g->r.maxs);
	if (fields) {
		for (auto& field : fields->key_value) {
			if (field.first == "targetname" && field.second.contains("radiation"))
				radius *= Dvar_FindMalleableVar("cm_radiation_radius_scale")->current.value;

		}

	}

	xy_cylinder_bottom.clear();
	xy_cylinder_side.clear();
	xy_cylinder_top.clear();

	constexpr int NUM_VERTS = 24;
	float angleIncrement = 2.f * M_PI / NUM_VERTS;
	for (int i = 0; i < NUM_VERTS; ++i) {
		auto angle = i * angleIncrement;
		auto x = origin->x + radius * cosf(angle);
		auto y = origin->y + radius * sinf(angle);

		xy_cylinder_top.push_back({ x, y, g->r.currentOrigin[2] + g->r.maxs[2] });
		xy_cylinder_bottom.push_back({ x, y, g->r.currentOrigin[2] - g->r.mins[2] });
	}

	for (int i = 0; i < NUM_VERTS; ++i) {
		auto angle1 = i * angleIncrement;
		auto angle2 = (i + 1) * angleIncrement;
		auto x1 = origin->x + radius * cosf(angle1);
		auto y1 = origin->y + radius * sinf(angle1);
		auto x2 = origin->x + radius * cosf(angle2);
		auto y2 = origin->y + radius * sinf(angle2);

		// Quad vertices
		xy_cylinder_side.push_back({ x1, y1, g->r.currentOrigin[2] + g->r.maxs[2] }); // Vertex on top circle
		xy_cylinder_side.push_back({ x1, y1, g->r.currentOrigin[2] - g->r.mins[2] }); // Vertex on bottom circle
		xy_cylinder_side.push_back({ x2, y2, g->r.currentOrigin[2] - g->r.mins[2] }); // Next vertex on bottom circle
		xy_cylinder_side.push_back({ x2, y2, g->r.currentOrigin[2] + g->r.maxs[2] }); // Next vertex on top circle
	}
}

void radiusEntity::render(const cm_renderinfo& info) {

	if (origin->dist(predictedPlayerState->origin) > info.draw_dist)
		return;

	auto func = info.as_polygons ? RB_DrawCollisionPoly : RB_DrawCollisionEdges;

	func(xy_cylinder_top.size(), (float(*)[3])xy_cylinder_top.data(), vec4_t{ 0.f, 1.f, 1.f, info.alpha }, info.depth_test);
	func(xy_cylinder_bottom.size(), (float(*)[3])xy_cylinder_bottom.data(), vec4_t{ 0.f, 1.f, 1.f, info.alpha }, info.depth_test);
	func(xy_cylinder_side.size(), (float(*)[3])xy_cylinder_side.data(), vec4_t{ 0.f, 1.f, 1.f, info.alpha }, info.depth_test);

}

bool G_EntityIsSpawner(const std::string& classname)
{
	const static std::vector<const char*> classnames = { "actor_", "script_origin", "script_struct", "strict_vehicle", "struct_model" };

	for (auto& c : classnames) {
		if (classname.contains(c))
			return true;
	}
	return false;
}
std::unique_ptr<gameEntity> gameEntity::createEntity(gentity_s* gent) {

	if (gameEntities::getInstance().it_is_ok_to_load_entities == false)
		return nullptr;

	if (gent->r.bmodel) {
		std::unique_ptr<brushModelEntity>&& bmodel = std::move(std::make_unique<brushModelEntity>(gent));
		return bmodel->valid_entity() ? std::move(bmodel) : nullptr;
	}
	else if (G_EntityIsSpawner(Scr_GetString(gent->classname))) {
		return std::move(std::make_unique<spawnerEntity>(gent));
	}
	else if (Scr_GetString(gent->classname) == std::string("trigger_radius")) {
		return std::move(std::make_unique<radiusEntity>(gent));
	}

	return nullptr;
}
