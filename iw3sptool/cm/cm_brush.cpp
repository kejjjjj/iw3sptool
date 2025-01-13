#include "cg/cg_offsets.hpp"
#include "cm_brush.hpp"
#include "cm_entity.hpp"
#include "cm_terrain.hpp"
#include "com/com_channel.hpp"
#include "com/com_vector.hpp"
#include "dvar/dvar.hpp"
#include "global_macros.hpp"
#include "r/rb_endscene.hpp"

#include <ranges>
#include <string>

#define NUM_SIDES 128

SimplePlaneIntersection pts[1024];
SimplePlaneIntersection* pts_results[1024];

void CM_ShowCollisionFilter()
{
	const auto num_args = cmd_args->argc[cmd_args->nesting];

	if (num_args == 1) {
		if (CClipMap::Size() == 0)
			Com_Printf(CON_CHANNEL_CONSOLEONLY, "there is no geometry to be cleared.. did you intend to use cm_showCollisionFilter <material>?\n");
		CClipMap::ClearThreadSafe();
		return;
	}

	CClipMap::ClearThreadSafe();


	std::unordered_set<std::string> filters;
	for (int i = 1; i < num_args; i++) {
		filters.insert(*(cmd_args->argv[cmd_args->nesting] + i));
	}

	CM_LoadAllBrushWindingsToClipMapWithFilter(filters);
	CM_LoadAllTerrainToClipMapWithFilters(filters);
}

void CM_LoadAllBrushWindingsToClipMapWithFilter(const std::unordered_set<std::string>& filters)
{
	std::unique_lock<std::mutex> lock(CClipMap::GetLock());

	CClipMap::ClearAllOfType(cm_geomtype::brush);

	if (filters.empty())
		return;

	for (const auto i : std::views::iota(0u, cm->numBrushes)) {

		const auto materials = CM_GetBrushMaterials(&cm->brushes[i]);

		bool yes = {};

		for (const auto& material : materials) {
			if (CM_IsMatchingFilter(filters, material.c_str())) {
				yes = true;
				break;
			}
		}

		if (!yes)
			continue;

		CM_LoadBrushWindingsToClipMap(&cm->brushes[i]);
	}

}

void CM_LoadBrushWindingsToClipMap(const cbrush_t* brush)
{
	if (!brush)
		return;

	CClipMap::m_pWipGeometry = CM_GetBrushPoints(brush, { 0.f, 1.f, 0.f });
	CClipMap::Insert(CClipMap::m_pWipGeometry);

}

std::unique_ptr<cm_geometry> CM_GetBrushPoints(const cbrush_t* brush, const fvec3& poly_col)
{
	if (!brush)
		return nullptr;

	float outPlanes[NUM_SIDES][4]{};
	const auto planeCount = BrushToPlanes(brush, outPlanes);
	const auto intersections = GetPlaneIntersections((const float**)outPlanes, planeCount, pts);
	adjacencyWinding_t windings[NUM_SIDES]{};

	auto intersection = 0;
	auto num_verts = 0;

	CClipMap::m_pWipGeometry = std::make_unique<cm_brush>();
	CClipMap::m_vecWipGeometryColor = poly_col;

	auto c_brush = dynamic_cast<cm_brush*>(CClipMap::m_pWipGeometry.get());

	c_brush->brush = const_cast<cbrush_t*>(brush);
	c_brush->origin = fvec3(brush->mins) + ((fvec3(brush->maxs) - fvec3(brush->mins)) / 2);
	c_brush->originalContents = brush->contents;
	c_brush->mins = brush->mins;
	c_brush->maxs = brush->maxs;

	do {
		auto w = BuildBrushdAdjacencyWindingForSide(intersections, pts, outPlanes[intersection], intersection, &windings[intersection]);
		if (w) {
			num_verts += w->numsides;
		}
		++intersection;
	} while (intersection < planeCount);

	c_brush->num_verts = num_verts;
	c_brush->create_corners();

	return std::move(CClipMap::m_pWipGeometry);
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
void __cdecl adjacency_winding(adjacencyWinding_t* w, float* points, vec3_t normal, unsigned int i0, unsigned int i1, unsigned int i2)
{
	auto brush = dynamic_cast<cm_brush*>(CClipMap::m_pWipGeometry.get());
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

	brush->windings.push_back(cm_winding{ winding_points, normal, CClipMap::m_vecWipGeometryColor });
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
constexpr std::array<std::int32_t, 24> iEdgePairs =
{
  0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3,
  2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7
};
constexpr auto iNextEdgePair = &iEdgePairs[1];

std::vector<fvec3> CM_CreateHitbox(const fvec3& mins, const fvec3& maxs)
{
	constexpr auto iota = std::views::iota;

	float v[8][3]{};
	for (const auto i : iota(0u, 8u)) {
		for (const auto j : iota(0u, 3u)) {

			if ((i & (1 << j)) != 0)
				v[i][j] = maxs[j];
			else
				v[i][j] = mins[j];
		}
	}


	std::vector<fvec3> points;
	for (const auto i : iota(0u, 12u)) {
		points.emplace_back(v[iEdgePairs[i * 2]]);
		points.emplace_back(v[iNextEdgePair[i * 2]]);
	}

	return points;
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

	float outPlanes[NUM_SIDES][4]{};
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
float RadiusFromBounds(const float* mins, const float* maxs)
{

	float v2 = fabsf(*mins);
	float v3 = fabsf(*maxs);
	if (v2 <= v3)
		v2 = v3;

	float v4 = v2;
	float v5 = fabsf(mins[1]);
	float v6 = fabsf(maxs[1]);
	if (v5 <= v6)
		v5 = v6;

	float v7 = v5;
	float v8 = fabsf(mins[2]);
	float v9 = fabsf(maxs[2]);
	if (v8 <= v9)
		v8 = v9;

	float v10 = sqrtf(((v8 * v8) + ((v4 * v4) + (v7 * v7))));
	return v10;
}