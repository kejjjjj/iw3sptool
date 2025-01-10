#pragma once

#include <unordered_map>
#include <string>
#include "cg/cg_local.hpp"
#include "cm_typedefs.hpp"
#include <cmd.hpp>

enum class gentity_type
{
	BRUSHMODEL,
	SPAWNER,
	RADIUS,
	OTHER
};

enum class entity_info_type
{
	eit_disabled,
	eit_enabled,
	eit_verbose
};

struct entity_fields
{
	std::vector<std::pair<std::string, std::string>> key_value;
};


struct entity_globals
{
	static std::unordered_map<int, entity_fields> ent_fields;
};

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

		auto _fields = entity_globals::ent_fields.find(g->s.number);


		if (_fields != entity_globals::ent_fields.end()) {
			fields = &_fields->second;
		}

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
	virtual void render2d(float draw_dist, entity_info_type entType);

protected:
	gentity_s* g = 0;
	fvec3* origin = 0;
	fvec3* orientation = 0;
	fvec3 oldOrigin;
	fvec3 oldOrientation;
	entity_fields* fields = 0;

};
class brushModelEntity : public gameEntity
{
public:
	~brushModelEntity() = default;
	brushModelEntity(gentity_s* gent);
	void render(const cm_renderinfo& info) override;
	void render2d(float draw_dist, entity_info_type entType) override;

	constexpr gentity_type get_type() const override
	{
		return gentity_type::BRUSHMODEL;
	}
	bool valid_entity() const noexcept override { return g != nullptr && brushmodels.empty() == false; }


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
		constexpr brushmodel_type get_type() const noexcept override { return brushmodel_type::BRUSH; }
		
		void render(const fvec3& _origin, const cm_renderinfo& info) override;
		void on_position_changed(const fvec3& _origin, const fvec3& delta_angles) noexcept(true) override;
		fvec3 get_center() const noexcept(true) override;
	};
	struct terrainmodel : public brushmodelbase {
		cLeaf_t* leaf = {};
		cm_terrain terrain;
		cm_terrain original_terrain;
		terrainmodel(gentity_s* gent) : brushmodelbase(gent) {};
		~terrainmodel() = default;
		constexpr brushmodel_type get_type() const noexcept override { return brushmodel_type::TERRAIN; }

		void render([[maybe_unused]] const fvec3& _origin, const cm_renderinfo& info) override;
		void on_position_changed(const fvec3& _origin, const fvec3& delta_angles) noexcept(true) override;
		fvec3 get_center() const noexcept(true) override;
	};

private:
	std::vector<std::unique_ptr<brushmodelbase>> brushmodels;

};

class spawnerEntity : public gameEntity
{
public:
	spawnerEntity(gentity_s* gent);
	~spawnerEntity() = default;

	constexpr gentity_type get_type() const override
	{
		return gentity_type::SPAWNER;
	}

	void render(const cm_renderinfo& info) override;

private:
	std::vector<fvec3> geometry;
};

class radiusEntity : public gameEntity
{
public:
	radiusEntity(gentity_s* gent) : gameEntity(gent) {

		refresh_geometry();

	}
	~radiusEntity() = default;

	constexpr gentity_type get_type() const override
	{
		return gentity_type::RADIUS;
	}

	void refresh_geometry();
	void render(const cm_renderinfo& info) override;

protected:

	std::vector<fvec3> xy_cylinder_top;
	std::vector<fvec3> xy_cylinder_bottom;
	std::vector<fvec3> xy_cylinder_side;
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

		if (!ent) {
			return;
		}
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
bool G_EntityIsSpawner(const std::string& classname);
