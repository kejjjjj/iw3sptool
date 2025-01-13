#pragma once

#include "cm_typedefs.hpp"

#include <unordered_map>
#include <string>
#include <vector>

enum class EGentityType
{
	gt_brushmodel,
	gt_spawner,
	gt_radius,
	gt_other,
};

enum class entity_info_type
{
	eit_disabled,
	eit_enabled,
	eit_verbose
};

void Cmd_ShowEntities_f();

struct gentity_s;

class CGameEntity
{
public:

	CGameEntity(gentity_s* const g);
	virtual ~CGameEntity();

	[[nodiscard]] virtual constexpr EGentityType Type() const { return EGentityType::gt_other; }

	[[nodiscard]] static std::unique_ptr<CGameEntity> CreateEntity(gentity_s* const g);

	[[nodiscard]] virtual bool RB_MakeInteriorsRenderable([[maybe_unused]] const cm_renderinfo& info) const { return false; }
	[[nodiscard]] virtual bool RB_MakeOutlinesRenderable([[maybe_unused]] const cm_renderinfo& info, [[maybe_unused]] int& nverts) const { return false; }

	//virtual void RB_Render3D(const cm_renderinfo& info) const;
	virtual void CG_Render2D(float drawDist, entity_info_type entType) const;

	[[nodiscard]] virtual int GetNumVerts() const noexcept {return 0; }

protected:

	[[nodiscard]] bool IsBrushModel() const noexcept;

	fvec3& m_vecOrigin;
	fvec3& m_vecAngles;

	mutable fvec3 m_vecOldOrigin;
	mutable fvec3 m_vecOldAngles;

	std::unordered_map<std::string, std::string> m_oEntityFields;
	gentity_s* const m_pOwner{};

	void ParseEntityFields();

private:
};

class CBrushModel : public CGameEntity
{
	NONCOPYABLE(CBrushModel);
public:

	CBrushModel(gentity_s* const g);
	~CBrushModel();

	[[nodiscard]] constexpr EGentityType Type() const override { return EGentityType::gt_brushmodel; }

	[[nodiscard]] bool RB_MakeInteriorsRenderable([[maybe_unused]] const cm_renderinfo& info) const override;
	[[nodiscard]] bool RB_MakeOutlinesRenderable([[maybe_unused]] const cm_renderinfo& info, int& nverts) const override;

	[[nodiscard]] int GetNumVerts() const noexcept override;


	struct CIndividualBrushModel
	{
		CIndividualBrushModel(gentity_s* const g);
		virtual ~CIndividualBrushModel();

		[[nodiscard]] virtual bool RB_MakeInteriorsRenderable(const cm_renderinfo& info) const;
		[[nodiscard]] virtual bool RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const;

		[[nodiscard]] virtual const cm_geometry& GetSource() const noexcept = 0;
		virtual void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) = 0;

		[[nodiscard]] virtual int GetNumVerts() const noexcept = 0;


	protected:
		[[nodiscard]] virtual fvec3 GetCenter() const noexcept;
		gentity_s* const m_pOwner{};
	};

	struct CBrush : public CIndividualBrushModel
	{
		CBrush(gentity_s* const g, const cbrush_t* const brush);
		~CBrush();

		[[nodiscard]] bool RB_MakeInteriorsRenderable(const cm_renderinfo& info) const override;
		[[nodiscard]] bool RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const override;

		void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) override;
		[[nodiscard]] const cm_geometry& GetSource() const noexcept override;

		[[nodiscard]] int GetNumVerts() const noexcept override { return m_oCurrentGeometry.num_verts; }

	private:
		cm_brush m_oOriginalGeometry;
		cm_brush m_oCurrentGeometry;
		const cbrush_t* const m_pSourceBrush = {};
	};

	struct CTerrain : public CIndividualBrushModel
	{
		CTerrain(gentity_s* const g, const cLeaf_t* const leaf, const cm_terrain& terrain);
		CTerrain(gentity_s* const g, const cLeaf_t* const leaf);
		~CTerrain();

		[[nodiscard]] bool RB_MakeInteriorsRenderable(const cm_renderinfo& info) const override;
		[[nodiscard]] bool RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const override;

		void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) override;
		[[nodiscard]] const cm_geometry& GetSource() const noexcept override;

		[[nodiscard]] int GetNumVerts() const noexcept override { return m_oCurrentGeometry.num_verts; }

	private:
		cm_terrain m_oOriginalGeometry;
		cm_terrain m_oCurrentGeometry;
		const cLeaf_t* const m_pSourceLeaf = {};
	};

private:
	std::vector<std::unique_ptr<CIndividualBrushModel>> m_oBrushModels;
};

class CSpawnerEntity : public CGameEntity
{
public:
	CSpawnerEntity(gentity_s* gent);
	~CSpawnerEntity() = default;

	[[nodiscard]] constexpr EGentityType Type() const override { return EGentityType::gt_spawner; }

	[[nodiscard]] bool RB_MakeInteriorsRenderable(const cm_renderinfo& info) const override;
	[[nodiscard]] bool RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const override;

	[[nodiscard]] int GetNumVerts() const noexcept override { return int(geometry.size()); }

private:
	mutable std::vector<fvec3> geometry;
};

class CRadiusEntity : public CGameEntity
{
public:
	CRadiusEntity(gentity_s* gent) : CGameEntity(gent) {
		ParseEntityFields();
		RefreshGeometry();

	}
	~CRadiusEntity() = default;

	[[nodiscard]] constexpr EGentityType Type() const override { return EGentityType::gt_radius; }

	void RefreshGeometry();
	[[nodiscard]] bool RB_MakeInteriorsRenderable(const cm_renderinfo& info) const override;
	[[nodiscard]] bool RB_MakeOutlinesRenderable(const cm_renderinfo& info, int& nverts) const override;

	[[nodiscard]] int GetNumVerts() const noexcept override {
		return int(xy_cylinder_top.size() + xy_cylinder_bottom.size() + xy_cylinder_side.size());
	}

protected:

	std::vector<fvec3> xy_cylinder_top;
	std::vector<fvec3> xy_cylinder_bottom;
	std::vector<fvec3> xy_cylinder_side;
};

bool G_EntityIsSpawner(const std::string& classname);
