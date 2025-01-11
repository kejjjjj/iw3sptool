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

	virtual void RB_Render3D(const cm_renderinfo& info) const;
	virtual void CG_Render2D(float drawDist, entity_info_type entType) const;

protected:

	[[nodiscard]] bool IsBrushModel() const noexcept;

	fvec3& m_vecOrigin;
	fvec3& m_vecAngles;

	mutable fvec3 m_vecOldOrigin;
	mutable fvec3 m_vecOldAngles;

	std::unordered_map<std::string, std::string> m_oEntityFields;
	gentity_s* const m_pOwner{};

private:
	void ParseEntityFields();
};

class CBrushModel : public CGameEntity
{
	NONCOPYABLE(CBrushModel);
public:

	CBrushModel(gentity_s* const g);
	~CBrushModel();

	[[nodiscard]] constexpr EGentityType Type() const override { return EGentityType::gt_brushmodel; }

	void RB_Render3D(const cm_renderinfo& info) const override;

	struct CIndividualBrushModel
	{
		CIndividualBrushModel(gentity_s* const g);
		virtual ~CIndividualBrushModel();

		virtual void RB_Render3D(const cm_renderinfo& info) const;

		[[nodiscard]] virtual const cm_geometry& GetSource() const noexcept = 0;
		virtual void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) = 0;

	protected:
		[[nodiscard]] virtual fvec3 GetCenter() const noexcept;
		gentity_s* const m_pOwner{};
	};

	struct CBrush : public CIndividualBrushModel
	{
		CBrush(gentity_s* const g, const cbrush_t* const brush);
		~CBrush();

		void RB_Render3D(const cm_renderinfo& info) const override;


		void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) override;
		[[nodiscard]] const cm_geometry& GetSource() const noexcept override;

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

		void OnPositionChanged(const fvec3& newOrigin, const fvec3& newAngles) override;
		[[nodiscard]] const cm_geometry& GetSource() const noexcept override;

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

	void RB_Render3D(const cm_renderinfo& info) const override;

private:
	mutable std::vector<fvec3> geometry;
};

class CRadiusEntity : public CGameEntity
{
public:
	CRadiusEntity(gentity_s* gent) : CGameEntity(gent) {

		RefreshGeometry();

	}
	~CRadiusEntity() = default;

	[[nodiscard]] constexpr EGentityType Type() const override { return EGentityType::gt_radius; }

	void RefreshGeometry();
	void RB_Render3D(const cm_renderinfo& info) const override;

protected:

	std::vector<fvec3> xy_cylinder_top;
	std::vector<fvec3> xy_cylinder_bottom;
	std::vector<fvec3> xy_cylinder_side;
};

bool G_EntityIsSpawner(const std::string& classname);
