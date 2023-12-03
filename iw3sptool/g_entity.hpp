#pragma once

#include "pch.hpp"

enum class gentity_type
{
	BRUSHMODEL,
	SPAWNER,
	OTHER
};

void G_DiscoverGentities(level_locals_t* l, const std::unordered_set<std::string>& filters);
void Cmd_ShowEntities_f();
void G_FreeEntity(gentity_s* gent);
void G_FreeEntityASM();
void G_FreeEntityASM2();

void G_Spawn(gentity_s* gent);
void G_SpawnASM();
void G_SpawnASM2();

void G_LoadGame_f();


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

	virtual bool valid_entity() const noexcept { return g != nullptr; }
	bool is_brush_model() const noexcept  { return valid_entity() && g->r.bmodel; }

	fvec3 get_origin() const noexcept { return *origin; }
	fvec3 get_angles() const noexcept { return *orientation; }

	static std::unique_ptr<gameEntity> createEntity(gentity_s* gent);
	static bool is_supported_entity(gentity_s* g) { return createEntity(g).get(); }
	virtual gentity_type get_type() const = 0;
	virtual void render(cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) = 0;

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
	
		//a brush
		if (numBrushes >= 1) {
			for (short i = 0; i < numBrushes; i++) {
				uint16_t brushIdx = leafBrushNode.data.leaf.brushes[i];

				if (brushIdx >= cm->numBrushes)
					return;

				brushmodel bmodel(g);

				bmodel.linked_brush = &cm->brushes[brushIdx];
				bmodel.brush_geometry = CM_GetBrushWindings(bmodel.linked_brush, vec4_t{ 0.984f, 0.494f, 0.004f, 0.3f });
				bmodel.original_geometry = bmodel.brush_geometry;

				brushmodels.push_back(std::move(std::make_unique<brushmodel>(bmodel)));
			}
		}
		if (const auto t = CM_GetTerrainTriangles(leaf, vec4_t{1,0,1,1})) {
			terrainmodel tm(g);

			tm.leaf = leaf;
			tm.terrain = t.value();
			tm.original_terrain = tm.terrain;
			
			brushmodels.push_back(std::move(std::make_unique<terrainmodel>(tm)));
		}

	}
	void render(cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) override {

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

			b->render(*origin, frustum_planes, numPlanes, poly_type, depth_test, drawdist);
		}
		for (auto& b : brushmodels) {

			const auto base = b.get();

			if (cm_type != showCollisionType::BOTH) {
				if (base->get_type() == brushmodel_type::BRUSH && cm_type != showCollisionType::BRUSHES)
					return;
				else if (base->get_type() == brushmodel_type::TERRAIN && cm_type != showCollisionType::TERRAIN) {
					return;
				}
			}


			if (Dvar_FindMalleableVar("cm_entityInfo")->current.enabled) {
				static std::string str1;
				static std::string str2;
				static std::string str3;
				float Z_offset = 0.f;
				const fvec3 center = b.get()->get_center();

				if (center.dist(predictedPlayerState->origin) > drawdist)
					return;

				if (g->classname) {
					str1 = "classname: ";
					str1 += Scr_GetString(g->classname);
					fvec3 n = center;
					n.z -= Z_offset;

					CL_AddDebugString(n, vec4_t{ 1,1,1,1 }, .5f, (char*)str1.c_str(), 2);
					Z_offset += 5.f;
				}
				if (g->targetname) {
					str2 = "targetname: ";
					str2 += Scr_GetString(g->targetname);
					fvec3 n = center;
					n.z -= Z_offset;
					CL_AddDebugString(n, vec4_t{ 1,1,1,1 }, .5f, (char*)str2.c_str(), 2);
					Z_offset += 5.f;

				}
				if (g->target) {
					str3 = "target: ";
					str3 += Scr_GetString(g->target);
					fvec3 n = center;
					n.z -= Z_offset;
					CL_AddDebugString(n, vec4_t{ 1,1,1,1 }, .5f, (char*)str3.c_str(), 2);
				}
			}

		}

	}
	gentity_type get_type() const override
	{
		return gentity_type::BRUSHMODEL;
	}
	bool valid_entity() const noexcept override { return g != nullptr && brushmodels.empty() == false; }

private:

	enum class brushmodel_type
	{
		BRUSH,
		TERRAIN
	};

	struct brushmodelbase
	{
		brushmodelbase(gentity_s* gent) : g(gent){}
		~brushmodelbase() = default;
		virtual brushmodel_type get_type() const noexcept = 0;
		virtual void render(const fvec3& origin, cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) = 0;
		virtual void on_position_changed(const fvec3& origin, const fvec3& orientation) noexcept(true) = 0;
		virtual fvec3 get_center() const noexcept(true) = 0;

	protected:
		gentity_s* g;
	};

	struct brushmodel : public brushmodelbase {
		brushmodel(gentity_s* gent) : brushmodelbase(gent) {};
		cbrush_t* linked_brush = 0;
		showcol_brush brush_geometry;
		showcol_brush original_geometry;
		brushmodel_type get_type() const noexcept override { return brushmodel_type::BRUSH; }
		void render(const fvec3& origin, cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) override {

			if (brush_geometry.brush == nullptr)
				return;

			fvec3 mins = origin + fvec3(linked_brush->mins);
			fvec3 maxs = origin + fvec3(linked_brush->maxs);

			if (brush_geometry.origin.dist(predictedPlayerState->origin) > drawdist)
				return;

			if (CM_BoundsInView(mins, maxs, frustum_planes, numPlanes)) {
				RB_RenderWinding(brush_geometry, poly_type, depth_test, drawdist, false, false);

			}
		}

		void on_position_changed(const fvec3& origin, const fvec3& delta_angles) noexcept(true) override {
			brush_geometry = original_geometry;
			brush_geometry.origin = origin;

			for (auto& winding : brush_geometry.windings) {
				for (auto& w : winding.points) {
					w += origin;
					
					w = VectorRotate(w, delta_angles, brush_geometry.origin);
				}
			}
		}
		fvec3 get_center() const noexcept(true) override {
			return g->r.currentOrigin;
			//const fvec3 mins = brush_geometry.origin + fvec3(linked_brush->mins);
			//const fvec3 maxs = brush_geometry.origin + fvec3(linked_brush->maxs);
			//return  mins + (maxs - mins) / 2;

		}
	};
	struct terrainmodel : public brushmodelbase {
		cLeaf_t* leaf;
		cm_terrain terrain;
		cm_terrain original_terrain;
		terrainmodel(gentity_s* gent) : brushmodelbase(gent) {};

		brushmodel_type get_type() const noexcept override { return brushmodel_type::TERRAIN; }

		void render(const fvec3& origin, cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) override {
			CM_ShowTerrain(&terrain, frustum_planes, poly_type, depth_test, drawdist, false);
		}
		void on_position_changed(const fvec3& origin, const fvec3& delta_angles) noexcept(true) override {
			terrain = original_terrain;

			for (auto& t : terrain.tris) {
				t.a += origin;
				t.b += origin;
				t.c += origin;

				fvec3 c = g->r.currentOrigin;

				t.a = VectorRotate(t.a, delta_angles, c);
				t.b = VectorRotate(t.b, delta_angles, c);
				t.c = VectorRotate(t.c, delta_angles, c);

			}
		}
		fvec3 get_center() const noexcept(true) override {
			return g->r.currentOrigin;
			//auto& front = terrain.tris.front();

			//const fvec3 mins = front.get_mins();
			//const fvec3 maxs = front.get_maxs();

			//return  mins + (maxs - mins) / 2;

		}
	};
	std::vector<std::unique_ptr<brushmodelbase>> brushmodels;

};

class spawnerEntity : public gameEntity
{
public:
	spawnerEntity(gentity_s* gent) : gameEntity(gent) {

		fvec3 copy = *origin;
		copy.z += 36;
		geometry = CM_CreateSphere(copy, 1.f, 5, 5, { 16,16,36 });

	}

	gentity_type get_type() const override
	{
		return gentity_type::SPAWNER;
	}

	void render(cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) override {

		if (*origin != oldOrigin || *orientation != oldOrientation) {
			
			fvec3 org = g->r.currentOrigin;
			fvec3 maxs = g->r.maxs;

			org.z += (maxs.z - g->r.mins[2]) / 2;
			maxs.z /= 2;


			geometry = CM_CreateSphere(org, 1.f, 5, 5, maxs);

			oldOrigin = *origin;
			oldOrientation = *orientation;
		}



		if (origin->dist(predictedPlayerState->origin) > drawdist)
			return;

		if (poly_type == polyType::POLYS)
			RB_DrawCollisionPoly(geometry.size(), (float(*)[3])geometry.data(), vec4_t{1,0,0,0.3f}, depth_test);
		else if (poly_type == polyType::EDGES)
			RB_DrawCollisionEdges(geometry.size(), (float(*)[3])geometry.data(), vec4_t{ 1,0,0,0.3f }, depth_test);

	}

private:
	std::vector<fvec3> geometry;
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
	void clear(bool clear_filter = false) {
		brushModelEntity* bmodel = 0;

		if (clear_filter)
			Cbuf_AddText("cm_showEntities\n");

		freed_entities = 0;
		spawned_entities = 0;
		entities.clear();
		
	}
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

	int freed_entities = 0;
	int spawned_entities = 0;
	DWORD time_since_loadgame = 0;
	bool it_is_ok_to_load_entities = true;
private:


	gameEntities(const gameEntities&) = delete;
	gameEntities& operator=(const gameEntities&) = delete;

	std::vector<std::unique_ptr<gameEntity>> entities;
};
inline bool G_EntityIsSpawner(const std::string& classname)
{
	const static std::vector<const char*> classnames = { "actor_", "script_origin", "script_struct", "strict_vehicle", "struct_model"};

	for (auto& c : classnames) {
		if (classname.contains(c))
			return true;
	}
	return false;
}
inline std::unique_ptr<gameEntity> gameEntity::createEntity(gentity_s* gent) {

	if (gameEntities::getInstance().it_is_ok_to_load_entities == false)
		return nullptr;

	if (gent->r.bmodel) {
		std::unique_ptr<brushModelEntity>&& bmodel = std::move(std::make_unique<brushModelEntity>(gent));
		return bmodel->valid_entity() ? std::move(bmodel) : nullptr;
	}
	else if (G_EntityIsSpawner(Scr_GetString(gent->classname))) {
		return std::move(std::make_unique<spawnerEntity>(gent));
	}

	return nullptr;
}