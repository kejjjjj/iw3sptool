#pragma once

#include "pch.hpp"

enum class gentity_type
{
	BRUSHMODEL,
	OTHER
};

void G_DiscoverGentities(level_locals_t* l, const char* classname);
void Cmd_ShowEntities_f();

class gameEntity
{
public:

	gameEntity(gentity_s* gent) : g(gent) 
	{
		if (!valid_entity())
			return;

		origin = (fvec3*)g->r.currentOrigin;
		orientation = (fvec3*)g->r.currentAngles;

	};

	bool valid_entity() const noexcept { return g != nullptr; }
	bool is_brush_model() const noexcept  { return valid_entity() && g->r.bmodel; }

	fvec3 get_origin() const noexcept { return *origin; }
	fvec3 get_angles() const noexcept { return *orientation; }

	static std::unique_ptr<gameEntity> createEntity(gentity_s* gent);

	virtual gentity_type get_type() const = 0;

protected:
	gentity_s* g = 0;
	fvec3* origin = 0;
	fvec3* orientation = 0;
	fvec3 oldOrigin;
	fvec3 oldOrientation;

};
class brushModelEntity : public gameEntity
{
public:
	brushModelEntity(gentity_s* gent) : gameEntity(gent) {

		auto leaf = &cm->cmodels[g->s.index.brushmodel].leaf;

		auto& leafBrushNode = cm->leafbrushNodes[leaf->leafBrushNode];
		short numBrushes = leafBrushNode.leafBrushCount;
	
		if (numBrushes < 1)
			return;

		uint16_t brushIdx = leafBrushNode.data.leaf.brushes[0];

		if (brushIdx >= cm->numBrushes)
			return;

		linked_brush = &cm->brushes[brushIdx];
		brush_geometry = CM_GetBrushWindings(linked_brush, vec4_t{ 0.984f, 0.494f, 0.004f, 0.3f });
		original_geometry = brush_geometry;
		og_mins = linked_brush->mins;
		og_maxs = linked_brush->maxs;

	}
	void render(cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) {

		if (brush_geometry.brush == nullptr)
			return;

		if (*origin != oldOrigin) {
			brush_geometry = original_geometry;

			VectorCopy(og_mins, linked_brush->mins);
			VectorCopy(og_maxs, linked_brush->maxs);

			linked_brush->mins[0] += origin->x;
			linked_brush->mins[1] += origin->y;
			linked_brush->mins[2] += origin->z;

			linked_brush->maxs[0] += origin->x;
			linked_brush->maxs[1] += origin->y;
			linked_brush->maxs[2] += origin->z;

			for (auto& winding : brush_geometry.windings) {
				for (auto& w : winding.points) {
					
					//std::cout << w << " + " << origin << " = " << w + *origin << '\n';

					w.x += origin->x;
					w.y += origin->y;
					w.z += origin->z;

					
				}
			}

			oldOrigin = *origin;
		}

		fvec3 mins = *origin + fvec3(linked_brush->mins);
		fvec3 maxs = *origin + fvec3(linked_brush->maxs);

		if (CM_BrushInView(linked_brush, frustum_planes, numPlanes)) {
			RB_RenderWinding(brush_geometry, poly_type, depth_test, drawdist, false, false);
		}


	}
	gentity_type get_type() const override
	{
		return gentity_type::BRUSHMODEL;
	}
private:
	cbrush_t* linked_brush = 0;
	showcol_brush brush_geometry;
	showcol_brush original_geometry;
	fvec3 og_mins, og_maxs;

};
class gameEntities
{
public:
	gameEntities() = default;

	static gameEntities& getInstance() { static gameEntities g; return g; }

	void push_back(gentity_s* gent) {
		if (!gent)
			return;

		auto&& ent = std::move(gameEntity::createEntity(gent));

		if (!ent)
			return;

		entities.push_back(std::move(ent));
	}
	bool empty() const noexcept { return entities.empty(); }
	void clear() { entities.clear(); }
	size_t size() const noexcept { return entities.size(); }

	using iterator = std::vector<std::unique_ptr<gameEntity>>::iterator;
	using const_iterator = std::vector<std::unique_ptr<gameEntity >>::const_iterator;

	iterator begin() { return entities.begin(); }
	iterator end() { return entities.end(); }
	const_iterator begin() const { return entities.begin(); }
	const_iterator end() const { return entities.end(); }

	std::unique_ptr<gameEntity>& operator[](int idx) {
		return entities[idx];
	}

private:


	gameEntities(const gameEntities&) = delete;
	gameEntities& operator=(const gameEntities&) = delete;

	std::vector<std::unique_ptr<gameEntity>> entities;
};

inline std::unique_ptr<gameEntity> gameEntity::createEntity(gentity_s* gent) {
	if (gent->r.bmodel) {
		return std::move(std::make_unique<brushModelEntity>(gent));
	}
	
	return nullptr;
}