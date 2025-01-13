#include "cg/cg_local.hpp"
#include "cg/cg_offsets.hpp"
#include "cg/cg_view.hpp"
#include "cm_brush.hpp"
#include "cm_entity.hpp"
#include "cm_renderer.hpp"
#include "cm_typedefs.hpp"
#include "com/com_vector.hpp"
#include "dvar/dvar.hpp"
#include "g/g_entity.hpp"
#include "r/r_debug.hpp"
#include "r/rb_endscene.hpp"
#include "r/r_active.hpp"
#include "typedefs.hpp"
#include "utils/hook.hpp"

using namespace std::string_literals;

#include <ranges>

char RB_DrawDebug(GfxViewParms* viewParms)
{
	CM_ShowCollision(viewParms);
	RB_RenderPlayerHitboxes();

	return hooktable::find<char, GfxViewParms*>(HOOK_PREFIX(__func__))->call(viewParms);
}

void CM_MakeInteriorRenderable(const std::vector<fvec3>& points, const float* _color)
{
	GfxColor color;
	std::size_t idx{};

	auto n_points = points.size();

	R_ConvertColorToBytes(_color, &color);

	for (idx = 0; idx < n_points; ++idx) {
		RB_SetPolyVertice(points[idx].As<float*>(), color, tess->vertexCount + idx, idx, 0);
	}

	for (idx = 2; idx < n_points; ++idx) {
		tess->indices[tess->indexCount + 0] = static_cast<short>(tess->vertexCount);
		tess->indices[tess->indexCount + 1] = static_cast<short>(idx + tess->vertexCount);
		tess->indices[tess->indexCount + 2] = static_cast<short>(idx + tess->vertexCount - 1);
		tess->indexCount += 3;
	}

	tess->vertexCount += n_points;
}
int CM_MakeOutlinesRenderable(const std::vector<fvec3>& points, const float* color, bool depthTest, int nverts)
{
	auto n_points = points.size();
	auto vert_index_prev = n_points - 1u;

	for (auto i : std::views::iota(0u, n_points)) {

		nverts = RB_AddDebugLine(g_debugPolyVerts, depthTest,
			points[vert_index_prev].As<float*>(), points[i].As<float*>(), color, nverts);
		vert_index_prev = i;
	}

	return nverts;
}
void CM_DrawCollisionPoly(const std::vector<fvec3>& points, const float* colorFloat, [[maybe_unused]]bool depthtest)
{
	RB_DrawPolyInteriors(points, colorFloat, true, depthtest);

	//R_AddDebugPolygon(points, colorFloat);
}

GfxPointVertex verts[2075];
void CM_DrawCollisionEdges(const std::vector<fvec3>& points, const float* colorFloat, bool depthtest)
{
	const auto numPoints = points.size();
	auto vert_count = 0;
	auto vert_index_prev = numPoints - 1u;


	for (auto i : std::views::iota(0u, numPoints)) {
		vert_count = RB_AddDebugLine(verts, depthtest, points[i].As<vec_t*>(), points[vert_index_prev].As<vec_t*>(), colorFloat, vert_count);
		vert_index_prev = i;
	}

	RB_DrawLines3D(vert_count / 2, 1, verts, depthtest);
}

void CM_ShowCollision([[maybe_unused]]GfxViewParms* GfxViewParms)
{
	if (CGentities::TimeToRepopulate()) {
		if (G_RepopulateEntities())
			CGentities::Depopulate();
	}

	if (CClipMap::Size() == 0 && CGentities::Size() == 0)
		return;

	cplane_s frustum_planes[6];
	CreateFrustumPlanes(frustum_planes);


	cm_renderinfo render_info =
	{
		.frustum_planes = frustum_planes,
		.num_planes = 5,
		.draw_dist = Dvar_FindMalleableVar("cm_showCollisionDist")->current.value,
		.depth_test = Dvar_FindMalleableVar("cm_showCollisionDepthTest")->current.enabled,
		.poly_render_type = Dvar_FindMalleableVar("cm_showCollisionPolyType")->current.integer,
		.only_colliding = Dvar_FindMalleableVar("cm_ignoreNonColliding")->current.enabled,
		.only_bounces = Dvar_FindMalleableVar("cm_onlyBounces")->current.enabled,
		.only_elevators = Dvar_FindMalleableVar("cm_onlyElevators")->current.enabled,
		.alpha = Dvar_FindMalleableVar("cm_showCollisionPolyAlpha")->current.value
	};

	showCollisionType collisionType = static_cast<showCollisionType>(Dvar_FindMalleableVar("cm_showCollision")->current.integer);
	const bool brush_allowed = collisionType == showCollisionType::BRUSHES || collisionType == showCollisionType::BOTH;
	const bool terrain_allowed = collisionType == showCollisionType::TERRAIN || collisionType == showCollisionType::BOTH;

	CGDebugData::tessVerts = 0;
	CGDebugData::tessIndices = 0;

	if (render_info.poly_render_type != pt_edges)  {

		std::unique_lock<std::mutex> lock(CClipMap::GetLock());
		RB_BeginSurfaceInternal(true, render_info.depth_test);

		CClipMap::ForEach([&](const GeometryPtr_t& poly) {

			if (RB_CheckTessOverflow(poly->num_verts, 3 * (poly->num_verts - 2))) 
				RB_TessOverflow(true, render_info.depth_test);
			
			if (poly->type() == cm_geomtype::brush && brush_allowed || poly->type() == cm_geomtype::terrain && terrain_allowed) {
				if (poly->RB_MakeInteriorsRenderable(render_info)) {
					CGDebugData::tessVerts += poly->num_verts;
					CGDebugData::tessIndices += 3 * (poly->num_verts - 2);
				}
			}
		});

		std::unique_lock<std::mutex> gentLock(CGentities::GetLock());

		CGentities::ForEach([&render_info](const GentityPtr_t& gent) {

			auto numVerts = gent->GetNumVerts();
			if (RB_CheckTessOverflow(numVerts, 3 * (numVerts - 2))) 
				RB_TessOverflow(true, render_info.depth_test);
			
			if(gent->RB_MakeInteriorsRenderable(render_info)) {
				CGDebugData::tessVerts += numVerts;
				CGDebugData::tessIndices += 3 * (numVerts - 2);
			}
		});

		RB_EndTessSurface();
	}
	if (render_info.poly_render_type != pt_polys){

		int vert_count = 0;
		std::unique_lock<std::mutex> lock(CClipMap::GetLock());

		CClipMap::ForEach([&](const GeometryPtr_t& poly) {

			if (poly->type() == cm_geomtype::brush && brush_allowed || poly->type() == cm_geomtype::terrain && terrain_allowed) {
				if (poly->RB_MakeOutlinesRenderable(render_info, vert_count)) {
					CGDebugData::tessVerts += poly->num_verts;
					CGDebugData::tessIndices += 3 * (poly->num_verts - 2);
				}
			}
		});

		std::unique_lock<std::mutex> gentLock(CGentities::GetLock());

		CGentities::ForEach([&](const GentityPtr_t& gent) {
			auto numVerts = gent->GetNumVerts();

			if (gent->RB_MakeOutlinesRenderable(render_info, vert_count)) {
				CGDebugData::tessVerts += numVerts;
				CGDebugData::tessIndices += 3 * (numVerts - 2);
			}
		});

		if (vert_count)
			RB_DrawLines3D(vert_count / 2, 1, g_debugPolyVerts, render_info.depth_test);


	}


}
