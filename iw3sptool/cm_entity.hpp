#pragma once

#include "pch.hpp"

enum class gentity_type
{
	BRUSHMODEL,
	SPAWNER,
	OTHER
};

struct entity_fields
{
	std::vector<std::pair<std::string, std::string>> key_value;
};


struct entity_globals
{
	static std::unordered_map<int, entity_fields> ent_fields;
};

void G_DiscoverGentities(level_locals_t* l, const std::unordered_set<std::string>& filters);
void Cmd_ShowEntities_f();

void G_ParseEntityFieldsASM();
void G_ParseEntityFields(gentity_s* gent, int);

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
	virtual ~gameEntity() = default;
	virtual bool valid_entity() const noexcept { return g != nullptr; }
	bool is_brush_model() const noexcept  { return valid_entity() && g->r.bmodel; }

	fvec3 get_origin() const noexcept { return *origin; }
	fvec3 get_angles() const noexcept { return *orientation; }

	static std::unique_ptr<gameEntity> createEntity(gentity_s* gent);
	static bool is_supported_entity(gentity_s* g) { return createEntity(g).get(); }
	virtual gentity_type get_type() const = 0;
	virtual void render(const cm_renderinfo& info) = 0;
	virtual void render2d(float draw_dist) = 0;

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
	~brushModelEntity() = default;
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
				auto result = CM_GetBrushPoints(bmodel.linked_brush, vec4_t{ 0.984f, 0.494f, 0.004f });
				bmodel.brush_geometry = *dynamic_cast<cm_brush*>(result.get());
				bmodel.original_geometry = bmodel.brush_geometry;

				brushmodels.push_back(std::move(std::make_unique<brushmodel>(bmodel)));
			}
		}
		if (const auto t = CM_GetTerrainTriangles(leaf, {"all"} )) {
			terrainmodel tm(g);

			tm.leaf = leaf;
			tm.terrain = *dynamic_cast<cm_terrain*>(t.get());
			tm.original_terrain = tm.terrain;
			
			brushmodels.push_back(std::move(std::make_unique<terrainmodel>(tm)));
		}

	}
	void render(const cm_renderinfo& info) override {

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
	void render2d(float draw_dist) override
	{

		for (auto& b : brushmodels)
		{
			auto center = b->get_center();
			float dist = center.dist(predictedPlayerState->origin);

			if (dist > draw_dist)
				continue;

			auto fields = entity_globals::ent_fields.find(g->s.number);

			if (fields != entity_globals::ent_fields.end()) {
				std::string buff;
				for (auto& field : fields->second.key_value)
				{
					buff += std::string(field.first) + " - " + field.second + "\n";
				}

				if (auto op = WorldToScreen(center)) {
					auto p = (fvec2)op.value();
					float scale = R_ScaleByDistance(dist) * 0.2f;
					R_DrawTextWithEffects(buff, "fonts/bigdevFont", p.x, p.y, scale, scale, 0, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,0 });

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
		virtual ~brushmodelbase() = default;
		virtual brushmodel_type get_type() const noexcept = 0;
		virtual void render(const fvec3& origin, const cm_renderinfo& info) = 0;
		virtual void on_position_changed(const fvec3& origin, const fvec3& orientation) noexcept(true) = 0;
		virtual fvec3 get_center() const noexcept(true) = 0;

	protected:
		gentity_s* g;
	};

	struct brushmodel : public brushmodelbase {
		brushmodel(gentity_s* gent) : brushmodelbase(gent) {};
		~brushmodel() = default;
		cbrush_t* linked_brush = 0;
		cm_brush brush_geometry;
		cm_brush original_geometry;
		brushmodel_type get_type() const noexcept override { return brushmodel_type::BRUSH; }
		void render(const fvec3& _origin, const cm_renderinfo& info) override {

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


				if (info.as_polygons)
					RB_DrawCollisionPoly(w.points.size(), (float(*)[3])w.points.data(), c, info.depth_test);
				else
					RB_DrawCollisionEdges(w.points.size(), (float(*)[3])w.points.data(), c, info.depth_test);
			}

		}

		void on_position_changed(const fvec3& _origin, const fvec3& delta_angles) noexcept(true) override {
			brush_geometry = original_geometry;
			brush_geometry.origin = _origin;

			for (auto& winding : brush_geometry.windings) {
				for (auto& w : winding.points) {
					w += _origin;
					
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
		~terrainmodel() = default;
		brushmodel_type get_type() const noexcept override { return brushmodel_type::TERRAIN; }

		void render([[maybe_unused]]const fvec3& _origin, const cm_renderinfo& info) override {
			terrain.render(info);
		}
		void on_position_changed(const fvec3& _origin, const fvec3& delta_angles) noexcept(true) override {
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
		fvec3 get_center() const noexcept(true) override {
			return g->r.currentOrigin;

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
	~spawnerEntity() = default;

	gentity_type get_type() const override
	{
		return gentity_type::SPAWNER;
	}

	void render(const cm_renderinfo& info) override {

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

		if (info.as_polygons)
			RB_DrawCollisionPoly(geometry.size(), (float(*)[3])geometry.data(), vec4_t{1,0,0,0.3f}, info.depth_test);
		else
			RB_DrawCollisionEdges(geometry.size(), (float(*)[3])geometry.data(), vec4_t{ 1,0,0,0.3f }, info.depth_test);

	}
	void render2d(float draw_dist) override 
	{
		fvec3 org = g->r.currentOrigin;
		fvec3 maxs = g->r.maxs;

		org.z += (g->r.maxs[2] - g->r.mins[2]) / 2;

		auto center = org;
		float dist = center.dist(predictedPlayerState->origin);

		if (dist > draw_dist)
			return;

		auto fields = entity_globals::ent_fields.find(g->s.number);

		if (fields != entity_globals::ent_fields.end()) {
			std::string buff;
			for (auto& field : fields->second.key_value)
			{
				buff += std::string(field.first) + " - " + field.second + "\n";
			}

			if (auto op = WorldToScreen(center)) {
				auto p = (fvec2)op.value();
				float scale = R_ScaleByDistance(dist) * 0.2f;
				R_DrawTextWithEffects(buff, "fonts/bigdevFont", p.x, p.y, scale, scale, 0, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,0 });

			}

		}
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
