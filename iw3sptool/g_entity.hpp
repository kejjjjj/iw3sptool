#pragma once

#include "pch.hpp"

enum class gentity_type
{
	BRUSHMODEL,
	SPAWNER,
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
		//og_mins = linked_brush->mins;
		//og_maxs = linked_brush->maxs;

	}
	void render(cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) {

		if (brush_geometry.brush == nullptr)
			return;

		if (*origin != oldOrigin || *orientation != oldOrientation) {
			brush_geometry = original_geometry;

			//VectorCopy(og_mins, linked_brush->mins);
			//VectorCopy(og_maxs, linked_brush->maxs);

			brush_geometry.origin += *origin;
			//linked_brush->mins[0] += origin->x;
			//linked_brush->mins[1] += origin->y;
			//linked_brush->mins[2] += origin->z;

			//linked_brush->maxs[0] += origin->x;
			//linked_brush->maxs[1] += origin->y;
			//linked_brush->maxs[2] += origin->z;
			


			//if ((Dvar_FindMalleableVar("cm_experimental")->current.enabled && g->classname == SL_GetStringOfSize("script_brushmodel")) == false) {

				for (auto& winding : brush_geometry.windings) {
					for (auto& w : winding.points) {
						w.x += origin->x;
						w.y += origin->y;
						w.z += origin->z;
					}
				}
			//}
			

			oldOrigin = *origin;
			oldOrientation = *orientation;
		}

		fvec3 mins = *origin + fvec3(linked_brush->mins);
		fvec3 maxs = *origin + fvec3(linked_brush->maxs);
		
		fvec3 center = mins + (maxs - mins) / 2;

		static std::string str1;
		static std::string str2;
		static std::string str3;


		float Z_offset = 0.f;

		if (CM_BoundsInView(mins, maxs, frustum_planes, numPlanes)) {

			if (Dvar_FindMalleableVar("cm_entityInfo")->current.enabled) {
				if (brush_geometry.origin.dist(predictedPlayerState->origin) > drawdist)
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
			RB_RenderWinding(brush_geometry, poly_type, depth_test, drawdist, false, false);
		}


	}
	gentity_type get_type() const override
	{
		return gentity_type::BRUSHMODEL;
	}

	void restore_original_state() 
	{
		if (!linked_brush)
			return;

		//VectorCopy(og_mins, linked_brush->mins);
		//VectorCopy(og_maxs, linked_brush->maxs);
	}

private:
	cbrush_t* linked_brush = 0;
	showcol_brush brush_geometry;
	showcol_brush original_geometry;
	fvec3 og_mins, og_maxs;

};

class spawnerEntity : public gameEntity
{
public:
	spawnerEntity(gentity_s* gent) : gameEntity(gent) {


		geometry = CM_CreateCube(*origin, { 32, 32, 72 });

	}

	gentity_type get_type() const override
	{
		return gentity_type::SPAWNER;
	}

	void render(cplane_s* frustum_planes, int numPlanes, const polyType poly_type, bool depth_test, float drawdist) {

		if (*origin != oldOrigin || *orientation != oldOrientation) {
			

			fvec3 copy = *origin;

			copy.z += 36;

			geometry = CM_CreateSphere(copy, 1.f, 5, 5, { 16,16,36 });

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

	gentity_s* g;
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
	void clear() {
		brushModelEntity* bmodel = 0;

		for (auto& e : entities) {
			
			switch (e->get_type()) {
			case gentity_type::BRUSHMODEL:


				bmodel = dynamic_cast<brushModelEntity*>(e.get());

				if(bmodel)
					bmodel->restore_original_state();
				else {
					Com_Printf("^1what the actual fuck\n");
				}
				break;
			default:
				break;
			}

		}

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

private:


	gameEntities(const gameEntities&) = delete;
	gameEntities& operator=(const gameEntities&) = delete;

	std::vector<std::unique_ptr<gameEntity>> entities;
};

inline std::unique_ptr<gameEntity> gameEntity::createEntity(gentity_s* gent) {
	if (gent->r.bmodel) {
		return std::move(std::make_unique<brushModelEntity>(gent));
	}
	else if (std::string(Scr_GetString(gent->classname)).find("actor_") != std::string::npos) {
		return std::move(std::make_unique<spawnerEntity>(gent));
	}

	return nullptr;
}