#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "cm_brush.hpp"
#include "cm_entity.hpp"
#include "cm_terrain.hpp"
#include "cm_renderer.hpp"
#include "cm_typedefs.hpp"
#include "com/com_vector.hpp"
#include "dvar/dvar.hpp"
#include "r/r_drawtools.hpp"
#include "r/rb_endscene.hpp"
#include "scr/scr.hpp"
#include "utils/functions.hpp"
#include "g/g_spawn.hpp"

#include <array>
#include <cassert>
#include <ranges>

using namespace std::string_literals;

static std::array<std::string, 6> nonVerboseInfoStrings = {
	"classname", "targetname", "spawnflags", 
	"target", "script_noteworthy", "script_flag"
};

void CGentities::CM_LoadAllEntitiesToClipMapWithFilters(const std::unordered_set<std::string>& filters)
{

	ClearThreadSafe();
	std::unique_lock<std::mutex> lock(GetLock());

	//reset spawnvars
	G_ResetEntityParsePoint();

	for (const auto i : std::views::iota(0, level->num_entities)) {

		auto gentity = &level->gentities[i];

		if (!CM_IsMatchingFilter(filters, Scr_GetString(gentity->classname)))
			continue;

		Insert(CGameEntity::CreateEntity(gentity));
	}

	//ForEach([](GentityPtr_t& p) {
	//	p->GenerateConnections(m_pLevelGentities);
	//});
}

CGameEntity::CGameEntity(gentity_s* const g) :
	m_pOwner(g),
	m_vecOrigin((fvec3&)g->r.currentOrigin),
	m_vecAngles((fvec3&)g->r.currentAngles)
{
	assert(m_pOwner != nullptr);

	ParseEntityFields();

}
CGameEntity::~CGameEntity() = default;

void CGameEntity::ParseEntityFields()
{
	const auto spawnVar = G_GetGentitySpawnVars(m_pOwner);

	if (!spawnVar)
		return;

	for (const auto index : std::views::iota(0, spawnVar->numSpawnVars)) {
		const auto [key, value] = std::tie(spawnVar->spawnVars[index][0], spawnVar->spawnVars[index][1]);

		for (auto f = ent_fields; f->name; ++f) {
			if (!strcmp(f->name, key)) {
				m_oEntityFields[key] = value;
			}
		}
	}

}
std::unique_ptr<CGameEntity> CGameEntity::CreateEntity(gentity_s* const g)
{
	if (g->r.bmodel)
		return std::make_unique<CBrushModel>(g);

	else if (G_EntityIsSpawner(Scr_GetString(g->classname)))
		return std::make_unique<CSpawnerEntity>(g);

	else if (Scr_GetString(g->classname) == "trigger_radius"s) {
		return std::move(std::make_unique<CRadiusEntity>(g));
	}

	return std::make_unique<CGameEntity>(g);
}

bool CGameEntity::IsBrushModel() const noexcept
{
	assert(m_pOwner != nullptr);
	return m_pOwner->r.bmodel;
}
void CGameEntity::CG_Render2D(float draw_dist, entity_info_type entType) const
{
	if (entType == entity_info_type::eit_disabled)
		return;

	const auto dist = m_vecOrigin.dist(predictedPlayerState->origin);

	if (dist > draw_dist)
		return;

	const fvec3 center = {
		m_pOwner->r.currentOrigin[0],
		m_pOwner->r.currentOrigin[1],
		m_pOwner->r.currentOrigin[2] + (m_pOwner->r.maxs[2] - m_pOwner->r.mins[2]) / 2
	};


	std::string buff;
	for (const auto& [key, value] : m_oEntityFields) {
		if (entType == entity_info_type::eit_enabled) {
			if (std::ranges::find(nonVerboseInfoStrings, key) == nonVerboseInfoStrings.end())
				continue;
		}

		buff += std::string(key) + " - " + value + "\n";
	}

	if (auto op = WorldToScreen(center)) {
		const float scale = ScaleByDistance(dist) * 0.15f;
		R_DrawTextWithEffects(buff, "fonts/bigdevFont", op->x, op->y, scale, scale, 0, vec4_t{ 1,1,1,1 }, 3, vec4_t{ 1,0,0,0 });
	}
}

/***********************************************************************
 > BRUSHMODELS
***********************************************************************/

CBrushModel::CBrushModel(gentity_s* const g) : CGameEntity(g) 
{
	assert(IsBrushModel());

	&cm->cmodels[1].leaf;
	const auto leaf = &cm->cmodels[g->s.index.brushmodel].leaf;
	const auto& leafBrushNode = cm->leafbrushNodes[leaf->leafBrushNode];
	const auto numBrushes = leafBrushNode.leafBrushCount;


	//brush
	if (numBrushes > 0) {
		for (const auto brushIndex : std::views::iota(0, numBrushes)) {
			const auto brushWorldIndex = leafBrushNode.data.leaf.brushes[brushIndex];
			if (brushWorldIndex > cm->numBrushes)
				break;

			m_oBrushModels.emplace_back(std::make_unique<CBrush>(g, &cm->brushes[brushWorldIndex]));
		}
		return;
	}

	//terrain
	cm_terrain terrain(leaf, { "all " });

	if (terrain.IsValid()) {
		m_oBrushModels.emplace_back(std::make_unique<CTerrain>(g, leaf, terrain));
	}

}
CBrushModel::~CBrushModel() = default;

void CBrushModel::RB_MakeInteriorsRenderable([[maybe_unused]] const cm_renderinfo& info) const 
{
	for (auto& bmodel : m_oBrushModels) {

		if (m_vecOrigin != m_vecOldOrigin || m_vecAngles != m_vecOldAngles)
			bmodel->OnPositionChanged(m_vecOrigin, m_vecAngles);

		bmodel->RB_MakeInteriorsRenderable(info);
	}

	m_vecOldOrigin = m_vecOrigin;
	m_vecOldAngles = m_vecAngles;
}
int CBrushModel::RB_MakeOutlinesRenderable([[maybe_unused]] const cm_renderinfo& info, int nverts) const 
{
	for (auto& bmodel : m_oBrushModels) {

		if (m_vecOrigin != m_vecOldOrigin || m_vecAngles != m_vecOldAngles)
			bmodel->OnPositionChanged(m_vecOrigin, m_vecAngles);

		nverts = bmodel->RB_MakeOutlinesRenderable(info, nverts);
	}

	m_vecOldOrigin = m_vecOrigin;
	m_vecOldAngles = m_vecAngles;

	return nverts;
}

CBrushModel::CIndividualBrushModel::CIndividualBrushModel(gentity_s* const g) : m_pOwner(g) { assert(g != nullptr); }
CBrushModel::CIndividualBrushModel::~CIndividualBrushModel() = default;

void CBrushModel::CIndividualBrushModel::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const {
	GetSource().RB_MakeInteriorsRenderable(info);
}
int CBrushModel::CIndividualBrushModel::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int nverts) const {
	return GetSource().RB_MakeOutlinesRenderable(info, nverts);
}

fvec3 CBrushModel::CIndividualBrushModel::GetCenter() const noexcept
{
	assert(m_pOwner != nullptr);
	return m_pOwner->r.currentOrigin;
}

CBrushModel::CBrush::CBrush(gentity_s* const g, const cbrush_t* const brush) : CIndividualBrushModel(g), m_pSourceBrush(brush)
{
	assert(m_pSourceBrush != nullptr);

	const vec3_t CYAN = { 0.f, 1.f, 1.f };

	//questionable for sure!
	m_oOriginalGeometry = *dynamic_cast<cm_brush*>(&*CM_GetBrushPoints(m_pSourceBrush, CYAN));
	m_oCurrentGeometry = m_oOriginalGeometry;

	OnPositionChanged(g->r.currentOrigin, g->r.currentAngles);


}
CBrushModel::CBrush::~CBrush() = default;

void CBrushModel::CBrush::OnPositionChanged(const fvec3& newOrigin, const fvec3 & newAngles)
{
	m_oCurrentGeometry = m_oOriginalGeometry;

	for (auto& winding : m_oCurrentGeometry.windings) {
		for (auto& point : winding.points) {
			point = VectorRotate(point + newOrigin, newAngles, m_oCurrentGeometry.origin);
		}
	}

	m_oCurrentGeometry.mins = m_pOwner->r.absmin;
	m_oCurrentGeometry.maxs = m_pOwner->r.absmax;
	m_oCurrentGeometry.origin = m_pOwner->r.currentOrigin;

}

const cm_geometry& CBrushModel::CBrush::GetSource() const noexcept
{
	return m_oCurrentGeometry;
}
void CBrushModel::CBrush::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const
{
	if (((fvec3&)m_pOwner->r.currentOrigin).dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return;

	const auto center = GetCenter();

	if (BoundsInView(center + m_pSourceBrush->mins, center + m_pSourceBrush->maxs, info.frustum_planes, info.num_planes) == false)
		return;

	m_oCurrentGeometry.RB_MakeInteriorsRenderable(info);
}
int CBrushModel::CBrush::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int nverts) const
{
	if (((fvec3&)m_pOwner->r.currentOrigin).dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return nverts;

	const auto center = GetCenter();

	if (BoundsInView(center + m_pSourceBrush->mins, center + m_pSourceBrush->maxs, info.frustum_planes, info.num_planes) == false)
		return nverts;

	return m_oCurrentGeometry.RB_MakeOutlinesRenderable(info, nverts);
}

CBrushModel::CTerrain::CTerrain(gentity_s* const g, const cLeaf_t* const leaf) : CIndividualBrushModel(g), m_pSourceLeaf(leaf) {}
CBrushModel::CTerrain::CTerrain(gentity_s* const g, const cLeaf_t* const leaf, const cm_terrain & terrain)
	: CIndividualBrushModel(g), m_pSourceLeaf(leaf), m_oOriginalGeometry(terrain), m_oCurrentGeometry(terrain)
{
	OnPositionChanged(g->r.currentOrigin, g->r.currentAngles);
}

CBrushModel::CTerrain::~CTerrain() = default;

void CBrushModel::CTerrain::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const
{
	if (((fvec3&)m_pOwner->r.currentOrigin).dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return;

	const auto center = GetCenter();

	m_oCurrentGeometry.RB_MakeInteriorsRenderable(info);
}
int CBrushModel::CTerrain::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int nverts) const
{
	if (((fvec3&)m_pOwner->r.currentOrigin).dist(cgs->predictedPlayerState.origin) > info.draw_dist)
		return nverts;

	const auto center = GetCenter();

	return m_oCurrentGeometry.RB_MakeOutlinesRenderable(info, nverts);
}

void CBrushModel::CTerrain::OnPositionChanged(const fvec3 & newOrigin, const fvec3& newAngles)
{
	m_oCurrentGeometry = m_oOriginalGeometry;

	const auto center = GetCenter();

	for (auto& tri : m_oCurrentGeometry.tris) {
		tri.a = VectorRotate(tri.a + newOrigin, newAngles, center);
		tri.b = VectorRotate(tri.b + newOrigin, newAngles, center);
		tri.c = VectorRotate(tri.c + newOrigin, newAngles, center);
	}

	m_oCurrentGeometry.mins = m_pOwner->r.absmin;
	m_oCurrentGeometry.maxs = m_pOwner->r.absmax;
	m_oCurrentGeometry.origin = m_pOwner->r.currentOrigin;

}
const cm_geometry& CBrushModel::CTerrain::GetSource() const noexcept
{
	return m_oCurrentGeometry;
}

CSpawnerEntity::CSpawnerEntity(gentity_s* gent) : CGameEntity(gent),
	geometry(CM_CreateSphere({ m_vecOrigin.x, m_vecOrigin.y, m_vecOrigin.z + 36 }, 1.f, 5, 5, { 16,16,36 })){}

void CSpawnerEntity::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const
{
	if (m_vecOrigin != m_vecOldOrigin || m_vecAngles != m_vecOldAngles) {

		fvec3 org = m_pOwner->r.currentOrigin;
		fvec3 maxs = m_pOwner->r.maxs;

		org.z += (maxs.z - m_pOwner->r.mins[2]) / 2;
		maxs.z /= 2;

		geometry = CM_CreateSphere(org, 1.f, 5, 5, maxs);

		m_vecOldOrigin = m_vecOrigin;
		m_vecOldAngles = m_vecAngles;
	}

	if (m_vecOrigin.dist(predictedPlayerState->origin) > info.draw_dist)
		return;

	CM_MakeInteriorRenderable(geometry, vec4_t{ 1,0,0,1 });

}
int CSpawnerEntity::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int nverts) const
{
	if (m_vecOrigin != m_vecOldOrigin || m_vecAngles != m_vecOldAngles) {

		fvec3 org = m_pOwner->r.currentOrigin;
		fvec3 maxs = m_pOwner->r.maxs;

		org.z += (maxs.z - m_pOwner->r.mins[2]) / 2;
		maxs.z /= 2;

		geometry = CM_CreateSphere(org, 1.f, 5, 5, maxs);

		m_vecOldOrigin = m_vecOrigin;
		m_vecOldAngles = m_vecAngles;
	}

	if (m_vecOrigin.dist(predictedPlayerState->origin) > info.draw_dist)
		return nverts;

	return CM_MakeOutlinesRenderable(geometry, vec4_t{ 1,0,0,1 }, info.depth_test, nverts);
}

void CRadiusEntity::RefreshGeometry() {

	if (m_oEntityFields.empty())
		return;

	auto radius = RadiusFromBounds(m_pOwner->r.mins, m_pOwner->r.maxs);

	for (const auto& [key, value] : m_oEntityFields) {
		if (key == "targetname" && value.contains("radiation"))
			radius *= Dvar_FindMalleableVar("cm_radiation_radius_scale")->current.value;

	}

	auto& g = m_pOwner;

	xy_cylinder_bottom.clear();
	xy_cylinder_side.clear();
	xy_cylinder_top.clear();

	constexpr auto NUM_VERTS = 24;
	constexpr auto angleIncrement = 2.f * M_PI / NUM_VERTS;

	for (int i = 0; i < NUM_VERTS; ++i) {
		auto angle = i * angleIncrement;
		auto x = m_vecOrigin.x + radius * cosf(angle);
		auto y = m_vecOrigin.y + radius * sinf(angle);

		xy_cylinder_top.push_back({ x, y, g->r.currentOrigin[2] + g->r.maxs[2] });
		xy_cylinder_bottom.push_back({ x, y, g->r.currentOrigin[2] - g->r.mins[2] });
	}

	for (int i = 0; i < NUM_VERTS; ++i) {
		auto angle1 = i * angleIncrement;
		auto angle2 = (i + 1) * angleIncrement;
		auto x1 = m_vecOrigin.x + radius * cosf(angle1);
		auto y1 = m_vecOrigin.y + radius * sinf(angle1);
		auto x2 = m_vecOrigin.x + radius * cosf(angle2);
		auto y2 = m_vecOrigin.y + radius * sinf(angle2);

		// Quad vertices
		xy_cylinder_side.push_back({ x1, y1, g->r.currentOrigin[2] + g->r.maxs[2] }); // Vertex on top circle
		xy_cylinder_side.push_back({ x1, y1, g->r.currentOrigin[2] - g->r.mins[2] }); // Vertex on bottom circle
		xy_cylinder_side.push_back({ x2, y2, g->r.currentOrigin[2] - g->r.mins[2] }); // Next vertex on bottom circle
		xy_cylinder_side.push_back({ x2, y2, g->r.currentOrigin[2] + g->r.maxs[2] }); // Next vertex on top circle
	}
}

void CRadiusEntity::RB_MakeInteriorsRenderable(const cm_renderinfo& info) const
{
	if (m_vecOrigin.dist(predictedPlayerState->origin) > info.draw_dist)
		return;

	const auto c = vec4_t{ 0.f, 1.f, 1.f, info.alpha };

	CM_MakeInteriorRenderable(xy_cylinder_top, c);
	CM_MakeInteriorRenderable(xy_cylinder_bottom, c);
	CM_MakeInteriorRenderable(xy_cylinder_side, c);
}
int CRadiusEntity::RB_MakeOutlinesRenderable(const cm_renderinfo& info, int nverts) const
{
	if (m_vecOrigin.dist(predictedPlayerState->origin) > info.draw_dist)
		return nverts;

	const auto c = vec4_t{ 0.f, 1.f, 1.f, info.alpha };

	nverts = CM_MakeOutlinesRenderable(xy_cylinder_top, c, info.depth_test, nverts);
	nverts = CM_MakeOutlinesRenderable(xy_cylinder_bottom, c, info.depth_test, nverts);
	return CM_MakeOutlinesRenderable(xy_cylinder_side, c, info.depth_test, nverts);
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