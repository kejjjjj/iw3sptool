#include "pch.hpp"


SimplePlaneIntersection pts[1024];
SimplePlaneIntersection* pts_results[1024];

void Cmd_CollisionFilter_f()
{
	int num_args = cmd_args->argc[cmd_args->nesting];

	__brush::rb_requesting_to_stop_rendering = true;


	if (num_args == 1) {
		auto& g = CClipMap::get();


		if (g.empty())
			Com_Printf("there is no geometry to be cleared.. did you intend to use cm_showCollisionFilter <material>?\n");
		CClipMap::clear();

		__brush::rb_requesting_to_stop_rendering = false;

		return;
	}
	CClipMap::clear();


	std::unordered_set<std::string> filters;
	for (int i = 1; i < num_args; i++) {
		filters.insert(*(cmd_args->argv[cmd_args->nesting] + i));
	}

	for (unsigned short i = 0; i < cm->numBrushes; i++) {

		auto materials = CM_GetBrushMaterials(&cm->brushes[i]);

		bool yes = {};

		for (auto& material : materials) {
			if (CM_IsMatchingFilter(filters, material.c_str())) {
				yes = true;
				break;
			}
		}

		if (!yes)
			continue;

		CM_GetBrushWindings(&cm->brushes[i]);

	}

	CM_DiscoverTerrain(filters);

	__brush::rb_requesting_to_stop_rendering = false;

}

std::unique_ptr<cm_geometry> CClipMap::wip_geom;
std::vector<std::unique_ptr<cm_geometry>> CClipMap::geometry;
fvec3 CClipMap::wip_color = {};

std::unique_ptr<cm_geometry> CM_GetBrushPoints(cbrush_t* brush, const fvec3& poly_col)
{
	if (!brush)
		return nullptr;

	float outPlanes[40][4]{};
	int planeCount = BrushToPlanes(brush, outPlanes);
	int intersections = GetPlaneIntersections((const float**)outPlanes, planeCount, pts);
	adjacencyWinding_t windings[40]{};

	int intersection = 0;
	int num_verts = 0;

	CClipMap::wip_geom = std::make_unique<cm_brush>();
	CClipMap::wip_color = poly_col;

	auto c_brush = dynamic_cast<cm_brush*>(CClipMap::wip_geom.get());

	c_brush->brush = brush;
	c_brush->origin = fvec3(brush->mins) + ((fvec3(brush->maxs) - fvec3(brush->mins)) / 2);

	do {
		auto w = BuildBrushdAdjacencyWindingForSide(intersections, pts, outPlanes[intersection], intersection, &windings[intersection]);
		if (w) {
			num_verts += w->numsides;
		}
		++intersection;
	} while (intersection < planeCount);

	c_brush->num_verts = num_verts;
	c_brush->create_corners();

	return std::move(CClipMap::wip_geom);
}
void CM_GetBrushWindings(cbrush_t* brush)
{
	if (!brush)
		return;

	CClipMap::wip_geom = CM_GetBrushPoints(brush, { 0.f, 1.f, 0.f });
	CClipMap::InsertGeometry(CClipMap::wip_geom);

}
void CM_BuildAxialPlanes(float(*planes)[6][4], const cbrush_t* brush)
{

	(*planes)[0][0] = -1.0;
	(*planes)[0][1] = 0.0;
	(*planes)[0][2] = 0.0;
	(*planes)[0][3] = -brush->mins[0];
	(*planes)[1][0] = 1.0;
	(*planes)[1][1] = 0.0;
	(*planes)[1][2] = 0.0;
	(*planes)[1][3] = brush->maxs[0];
	(*planes)[2][0] = 0.0;
	(*planes)[2][2] = 0.0;
	(*planes)[2][1] = -1.0;
	(*planes)[2][3] = -brush->mins[1];
	(*planes)[3][0] = 0.0;
	(*planes)[3][2] = 0.0;
	(*planes)[3][1] = 1.0;
	(*planes)[3][3] = brush->maxs[1];
	(*planes)[4][0] = 0.0;
	(*planes)[4][1] = 0.0;
	(*planes)[4][2] = -1.0;
	(*planes)[4][3] = -brush->mins[2];
	(*planes)[5][0] = 0.0;
	(*planes)[5][1] = 0.0;
	(*planes)[5][2] = 1.0;
	(*planes)[5][3] = brush->maxs[2];
}
int GetPlaneIntersections(const float** planes, int planeCount, SimplePlaneIntersection* OutPts)
{
	int r = 0;
	__asm
	{
		push OutPts;
		push planeCount;
		push planes;
		mov esi, 0x5AB150;
		call esi;
		add esp, 12;
		mov r, eax;
	}

	return r;
}
char* CM_MaterialForNormal(const cbrush_t* target, const fvec3& normals)
{
	//non-axial!
	for (unsigned int i = 0; i < target->numsides; i++) {

		cbrushside_t* side = &target->sides[i];

		if (normals == side->plane->normal)
			return cm->materials[side->materialNum].material;
	}


	short mtl = -1;

	if (normals.z == 1.f)
		mtl = target->axialMaterialNum[1][2];
	else if (normals.z == -1.f)
		mtl = target->axialMaterialNum[0][2];

	if (normals.x == 1)
		mtl = target->axialMaterialNum[1][0];
	else if (normals.x == -1)
		mtl = target->axialMaterialNum[0][0];

	if (normals.y == 1.f)
		mtl = target->axialMaterialNum[1][1];
	else if (normals.y == -1.f)
		mtl = target->axialMaterialNum[0][1];


	if (mtl >= 0)
		return cm->materials[mtl].material;

	return nullptr;

}
int BrushToPlanes(const cbrush_t* brush, float(*outPlanes)[4])
{
	float planes[6][4]{};
	CM_BuildAxialPlanes((float(*)[6][4])planes, brush);
	uint32_t i = 0;
	do {
		CM_GetPlaneVec4Form(brush->sides, planes, i, outPlanes[i]);

	} while (++i < brush->numsides + 6);

	return i;
}
bool CM_BrushHasCollisions(const cbrush_t* brush)
{
	return (brush->contents & MASK_PLAYERSOLID) != 0;
}
void CM_GetPlaneVec4Form(const cbrushside_t* sides, const float(*axialPlanes)[4], int index, float* expandedPlane)
{
	if (index >= 6) {
		cplane_s* plane = sides[index - 6].plane;

		expandedPlane[0] = plane->normal[0];
		expandedPlane[1] = plane->normal[1];
		expandedPlane[2] = plane->normal[2];
		expandedPlane[3] = plane->dist;
		return;
	}

	const float* plane = axialPlanes[index];

	*expandedPlane = plane[0];
	expandedPlane[1] = plane[1];
	expandedPlane[2] = plane[2];
	expandedPlane[3] = plane[3];

}
adjacencyWinding_t* BuildBrushdAdjacencyWindingForSide(int ptCount, SimplePlaneIntersection* _pts, float* sideNormal, int planeIndex, adjacencyWinding_t* optionalOutWinding)
{
	adjacencyWinding_t* r = 0;

	__asm
	{
		mov edx, ptCount;
		mov ecx, _pts;
		push optionalOutWinding;
		push planeIndex;
		push sideNormal;
		mov esi, 0x598260;
		call esi;
		add esp, 12;
		mov r, eax;
	}

	return optionalOutWinding;
}
static void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2)
{
	auto brush = dynamic_cast<cm_brush*>(CClipMap::wip_geom.get());
	cm_triangle tri;
	std::vector<fvec3> winding_points;

	tri.a = &points[i2];
	tri.b = &points[i1];
	tri.c = &points[i0];

	PlaneFromPointsASM(tri.plane, tri.a, tri.b, tri.c);

	if (DotProduct(tri.plane, normal) < 0.f) {
		std::swap(tri.a, tri.c);

	}
	tri.material = CM_MaterialForNormal(brush->brush, normal);
	brush->triangles.push_back(tri);

	for (int winding = 0; winding < w->numsides; winding++) {
		winding_points.push_back({ &points[winding * 3] });
	}

	brush->windings.push_back(cm_winding{ winding_points, normal, CClipMap::wip_color });
}
__declspec(naked) void __brush::__asm_adjacency_winding()
{
	static constexpr DWORD dst = 0x5985AC;

	__asm
	{
		mov eax, [esp + 6030h + -6020h]; //i2
		lea edx, [eax + eax * 2];
		mov eax, [esp + 6030h + -6014h]; //i1
		lea ecx, [eax + eax * 2];
		mov eax, [esp + 6030h + -601Ch]; //i0
		lea esi, [eax + eax * 2];

		push edx; //i2
		push ecx; //i1
		push esi; //i0
		push ebp; //normal
		lea eax, [esp + 3044h-4];
		push eax; //points
		push ebx; //winding

		call adjacency_winding;
		add esp, 24;

		fld dword ptr[ebp + 04h];
		fmul dword ptr[esp + 6030h + -600Ch];
		fld dword ptr[ebp + 00h];
		fmul dword ptr[esp + 6030h + -6010h];
		faddp st(1), st;
		jmp dst;
	}
}


void RB_ShowCollision(GfxViewParms* viewParms)
{
	decltype(auto) ents = gameEntities::getInstance();

	if (__brush::rb_requesting_to_stop_rendering) {
		return;
	}

	cplane_s frustum_planes[6];
	CreateFrustumPlanes(viewParms, frustum_planes);

	auto& geo = CClipMap::get();
	if (geo.empty() && ents.empty())
		return;

	showCollisionType collisionType = static_cast<showCollisionType>(Dvar_FindMalleableVar("cm_showCollision")->current.integer);

	cm_renderinfo render_info =
	{
		.frustum_planes = frustum_planes,
		.num_planes = 5,
		.draw_dist = Dvar_FindMalleableVar("cm_showCollisionDist")->current.value,
		.depth_test = Dvar_FindMalleableVar("cm_showCollisionDepthTest")->current.enabled,
		.as_polygons = static_cast<polyType>(Dvar_FindMalleableVar("cm_showCollisionPolyType")->current.integer) == polyType::POLYS,
		.only_colliding = Dvar_FindMalleableVar("cm_ignoreNonColliding")->current.enabled,
		.only_bounces = Dvar_FindMalleableVar("cm_onlyBounces")->current.enabled,
		.only_elevators = Dvar_FindMalleableVar("cm_onlyElevators")->current.integer,
		.alpha = Dvar_FindMalleableVar("cm_showCollisionPolyAlpha")->current.value
	};

	const bool brush_allowed = collisionType == showCollisionType::BRUSHES || collisionType == showCollisionType::BOTH;
	const bool terrain_allowed = collisionType == showCollisionType::TERRAIN || collisionType == showCollisionType::BOTH;


	for (auto& geom : geo) {

		if (geom->type() == cm_geomtype::brush && brush_allowed || geom->type() == cm_geomtype::terrain && terrain_allowed)
			geom->render(render_info);
	}

	for (auto& ent : ents)
		ent->render(render_info);


}
bool CM_BrushInView(const cbrush_t* brush, struct cplane_s* frustumPlanes, int numPlanes)
{
	if (numPlanes <= 0)
		return 1;

	cplane_s* plane = frustumPlanes;
	int idx = 0;
	while ((BoxOnPlaneSide(brush->mins, brush->maxs, plane) & 1) != 0) {
		++plane;
		++idx;

		if (idx >= numPlanes)
			return 1;
	}

	return 0;
}
bool CM_BoundsInView(const fvec3& mins, const fvec3& maxs, struct cplane_s* frustumPlanes, int numPlanes)
{
	if (numPlanes <= 0)
		return 1;

	cplane_s* plane = frustumPlanes;
	int idx = 0;
	while ((BoxOnPlaneSide(mins, maxs, plane) & 1) != 0) {
		++plane;
		++idx;

		if (idx >= numPlanes)
			return 1;
	}

	return 0;
}
std::vector<fvec3> CM_CreateCube(const fvec3& origin, const fvec3& size)
{
	std::vector<fvec3> vertices;

	// Define the eight vertices of the cube
	fvec3 v0 = origin;
	fvec3 v1 = origin + fvec3(size.x, 0, 0);
	fvec3 v2 = origin + fvec3(size.x, size.y, 0);
	fvec3 v3 = origin + fvec3(0, size.y, 0);

	fvec3 v4 = origin + fvec3(0, 0, size.z);
	fvec3 v5 = origin + fvec3(size.x, 0, size.z);
	fvec3 v6 = origin + fvec3(size.x, size.y, size.z);
	fvec3 v7 = origin + fvec3(0, size.y, size.z);

	// Order the vertices for the lines
	vertices.push_back(v0);
	vertices.push_back(v1);

	vertices.push_back(v1);
	vertices.push_back(v2);

	vertices.push_back(v2);
	vertices.push_back(v3);

	vertices.push_back(v3);
	vertices.push_back(v0);

	vertices.push_back(v4);
	vertices.push_back(v5);

	vertices.push_back(v5);
	vertices.push_back(v6);

	vertices.push_back(v6);
	vertices.push_back(v7);

	vertices.push_back(v7);
	vertices.push_back(v4);

	vertices.push_back(v0);
	vertices.push_back(v4);

	vertices.push_back(v1);
	vertices.push_back(v5);

	vertices.push_back(v2);
	vertices.push_back(v6);

	vertices.push_back(v3);
	vertices.push_back(v7);

	return vertices;
}
std::vector<fvec3> CM_CreateSphere(const fvec3& ref_org, const float radius, const int32_t latitudeSegments, const int32_t longitudeSegments, const fvec3& scale)
{
	std::vector<fvec3> points;
	points.clear();
	std::vector<fvec3> verts;

	float phiStep = M_PI / latitudeSegments;
	float thetaStep = 2.0f * M_PI / longitudeSegments;

	for (int lat = 0; lat <= latitudeSegments; ++lat) {
		float phi = lat * phiStep;
		for (int lon = 0; lon <= longitudeSegments; ++lon) {
			float theta = lon * thetaStep;

			float x = ref_org.x + (radius * scale.x * std::sin(phi) * std::cos(theta));
			float y = ref_org.y + (radius * scale.y * std::cos(phi));
			float z = ref_org.z + (radius * scale.z * std::sin(phi) * std::sin(theta));

			verts.push_back({ x, y, z });
		}
	}

	for (int lat = 0; lat < latitudeSegments; ++lat) {
		for (int lon = 0; lon < longitudeSegments; ++lon) {
			int v0 = lat * (longitudeSegments + 1) + lon;
			int v1 = v0 + 1;
			int v2 = (lat + 1) * (longitudeSegments + 1) + lon;
			int v3 = v2 + 1;

			points.push_back(verts[v0]);
			points.push_back(verts[v2]);
			points.push_back(verts[v1]);

			points.push_back(verts[v1]);
			points.push_back(verts[v2]);
			points.push_back(verts[v3]);
		}
	}

	return points;
}
void RB_DrawCollisionPoly(int numPoints, float(*points)[3], const float* colorFloat, bool depthtest)
{
	std::vector<fvec3> _pts;
	for (int i = 0; i < numPoints; i++)
		_pts.push_back(points[i]);

	RB_DrawPolyInteriors(numPoints, _pts, colorFloat, true, depthtest);

}

GfxPointVertex verts[2075];

void RB_DrawCollisionEdges(int numPoints, float(*points)[3], const float* colorFloat, bool depthtest)
{
	uint8_t c[4];

	R_ConvertColorToBytes(colorFloat, c);

	int vert_count = 0;
	int vert_index_prev = numPoints - 1;


	for (int i = 0; i < numPoints; i++) {
		vert_count = RB_AddDebugLine(verts, points[i], (float*)points[vert_index_prev], c, vert_count);

		vert_index_prev = i;
	}

	RB_DrawLines3D(vert_count/2, 1, verts, depthtest);

	//delete[] verts;

}

bool CM_IsMatchingFilter(const std::unordered_set<std::string>& filters, char* material)
{

	for (const auto& filter : filters) {

		if (filter == "all" || std::string(material).contains(filter))
			return true;
	}

	return false;
}
std::vector<std::string> CM_GetBrushMaterials(const cbrush_t* brush)
{
	std::vector<std::string> result;

	float outPlanes[40][4]{};
	int planeCount = BrushToPlanes(brush, outPlanes);
	[[maybe_unused]] int intersections = GetPlaneIntersections((const float**)outPlanes, planeCount, pts);

	for (int i = 0; i < planeCount; i++)
	{
		fvec3 plane = outPlanes[i];

		if (auto mtl = CM_MaterialForNormal(brush, plane))
		{
			result.push_back(mtl);
		}

	}

	return result;
}