#include "pch.hpp"

fvec3 current_normals;
showcol_brush current_winding;
cbrush_t* current_brush = 0;
vec4_t poly_color;

SimplePlaneIntersection pts[1024];
SimplePlaneIntersection* pts_results[1024];

void Cmd_CollisionFilter_f()
{
	if (cmd_args->argc[cmd_args->nesting] > 2) {
		Com_Printf(CON_CHANNEL_CONSOLEONLY, "usage: cm_showCollisionFilter <material>\n");
		return;
	}

	rb_requesting_to_stop_rendering = true;

	if (cmd_args->argc[cmd_args->nesting] == 1) {

		if (s_brushes.empty()) {
			rb_requesting_to_stop_rendering = false;

			return Com_Printf("there are no brushes to be cleared.. did you intend to use cm_showCollisionFilter <material>?\n");
		}

		Com_Printf("clearing %i brushes from the render queue\n", s_brushes.size());
		s_brushes.clear();
		cm_terrainpoints.clear();

		rb_requesting_to_stop_rendering = false;

		return;
	}

	auto filter = *(cmd_args->argv[cmd_args->nesting] + 1);
	s_brushes.clear();
	cm_terrainpoints.clear();

	for (int i = 0; i < cm->numBrushes; i++) {

		char* mat = (cm->materials[cm->brushes[i].axialMaterialNum[0][0]].material);

		if (strstr(mat, filter) == 0 && strcmp(filter, "all"))
			continue;

		if (strstr(mat, "clip_foliage")) //ignore clip_foliage because it's so useless
			continue;

		s_brushes.push_back(CM_GetBrushWindings(&cm->brushes[i], vec4_t{0,1,0,0.3f}));

	}

	CM_DiscoverTerrain(filter);

	Com_Printf("adding %i brushes and %i terrain pieces to the render queue\n", s_brushes.size(), cm_terrainpoints.size());
	
	rb_requesting_to_stop_rendering = false;

}

showcol_brush CM_GetBrushWindings(cbrush_t* brush, vec4_t polycolor)
{
	poly_color[0] = polycolor[0];
	poly_color[1] = polycolor[1];
	poly_color[2] = polycolor[2];
	poly_color[3] = polycolor[3];

	float outPlanes[40][4]{};
	int planeCount = BrushToPlanes(brush, outPlanes);
	int intersections = GetPlaneIntersections((const float**)outPlanes, planeCount, pts);
	adjacencyWinding_t windings[40]{};

	int verts = 0;
	int intersection = 0;

	current_winding.windings.clear();
	current_winding.numVerts = 0;
	current_winding.brush = brush;
	current_winding.origin = brush->get_origin();

	current_brush = brush;

	do {
		current_normals = outPlanes[intersection];
		adjacencyWinding_t* w = 0;
		if (w = BuildBrushdAdjacencyWindingForSide(intersections, pts, outPlanes[intersection], intersection, &windings[intersection])) {
			current_winding.numVerts += w->numsides;
		}

		//current_winding.intersections.push_back(pts[intersection]);

		++intersection;

	} while (intersection < planeCount);

	return current_winding;

	//current_winding.intersections.clear();

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
int BrushToPlanes(const cbrush_t* brush, float(*outPlanes)[4])
{
	float planes[6][4];
	CM_BuildAxialPlanes((float(*)[6][4])planes, brush);
	int i = 0;
	do {
		CM_GetPlaneVec4Form(brush->sides, planes, i, outPlanes[i]);

	} while (++i < brush->numsides + 6);

	return i;
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
adjacencyWinding_t* BuildBrushdAdjacencyWindingForSide(int ptCount, SimplePlaneIntersection* pts, float* sideNormal, int planeIndex, adjacencyWinding_t* optionalOutWinding)
{
	adjacencyWinding_t* r = 0;

	__asm
	{
		mov edx, ptCount;
		mov ecx, pts;
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

void __cdecl winding_hook__(adjacencyWinding_t* w, float* a, float* b, float* c, float* points)
{
	//float plane[4];
	//PlaneFromPoints(plane, a, b, c);



	//if ((DotProduct(plane, current_normals) < 0.f)) {
	//	int* sides = w->sides;
	//	int* end = &w->sides[w->numsides];

	//	if (w->sides < end)
	//	{
	//		do
	//		{
	//			int temp = *sides;
	//			*sides = *end;
	//			*end-- = temp;
	//			++sides;
	//		} while (sides < end);
	//	}
	//}


	std::vector<fvec3> winding_points;

	for (int winding = 0; winding < w->numsides; winding++) {
		winding_points.push_back({ &points[winding * 3] });
	}

	current_winding.windings.push_back({ sc_winding_t{ winding_points } });
	current_winding.brush = current_brush;

	auto& back = current_winding.windings.back();

	back.is_bounce = current_normals[2] >= 0.3f && current_normals[2] <= 0.7f;
	back.is_elevator = std::fabs(current_normals[0]) == 1.f || std::fabs(current_normals[1]) == 1.f;
	back.normals = current_normals;

	fvec3 new_color = SetSurfaceBrightness(poly_color, current_normals, Dvar_FindMalleableVar("r_lightTweakSunDirection")->current.vector);

	VectorCopy(new_color, back.color);
	back.color[3] = poly_color[3];

}

__declspec(naked) void __brush_hook::stealerino()
{
	__asm
	{
		mov eax, [esp + 10h];
		lea edx, [eax + eax * 2];
		mov eax, [esp + 1Ch];
		lea ecx, [eax + eax * 2];
		mov eax, [esp + 14h];
		lea esi, [eax + eax * 2];

		lea edx, [esp + edx * 4 + 00003030h]; //a
		lea esi, [esp + esi * 4 + 00003030h]; //b
		lea ecx, [esp + ecx * 4 + 00003030h]; //c

		lea eax, [esp + 00003040h - 16];

		push eax;
		push ecx;
		push esi;
		push edx;
		push ebx;

		call winding_hook__;
		add esp, 20;

		xor eax, eax;
		pop edi;
		pop esi;
		pop ebp;
		pop ebx;
		add esp, 6020h;
		retn;
	}
}
bool PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c)
{
	vec3_t d1, d2;

	VectorSubtract(b, a, d1);
	VectorSubtract(c, a, d2);


	CrossProduct(d2, d1, plane);

	float len = VectorLength(plane);

	if (!len)
		return false;

	if (len < 2.f) {
		if (VectorLength(d1) * VectorLength(d2) * 0.0000010000001f >= len) {
			VectorSubtract(c, b, d1);
			VectorSubtract(a, b, d2);

			CrossProduct(d2, d1, plane);

			if (VectorLength(d1) * VectorLength(d2) * 0.0000010000001f >= len) {
				return false;
			}

		}
	}

	if (VectorNormalize(plane) == 0) {
		return 0;
	}

	plane[3] = DotProduct(a, plane);
	return 1;
}


void RB_ShowCollision(GfxViewParms* viewParms)
{
	decltype(auto) ents = gameEntities::getInstance();

	if (rb_requesting_to_stop_rendering) {
		return;
	}

	if (s_brushes.empty() && ents.empty() && cm_terrainpoints.empty() || clientUI->connectionState == CA_DISCONNECTED)
		return;


	cplane_s frustum_planes[6];

	CreateFrustumPlanes(viewParms, frustum_planes);

	polyType poly_type = static_cast<polyType>(Dvar_FindMalleableVar("cm_showCollisionPolyType")->current.integer);
	showCollisionType collisionType = static_cast<showCollisionType>(Dvar_FindMalleableVar("cm_showCollision")->current.integer);
	float draw_dist = Dvar_FindMalleableVar("cm_showCollisionDist")->current.value;
	bool only_bounces = Dvar_FindMalleableVar("cm_onlyBounces")->current.enabled;
	bool only_elevators = Dvar_FindMalleableVar("cm_onlyElevators")->current.enabled;
	bool depth_test = Dvar_FindMalleableVar("cm_showCollisionDepthTest")->current.enabled;


	if (collisionType == showCollisionType::DISABLED)
		return;

	if (collisionType == showCollisionType::BRUSHES || collisionType == showCollisionType::BOTH) {
		for (auto& i : s_brushes)
			if (CM_BrushInView(i.brush, frustum_planes, 5)) {
				RB_RenderWinding(i, poly_type, depth_test, draw_dist, only_bounces, only_elevators);
			}

		if (only_bounces == false && only_elevators == false) {
			for (auto& i : ents) {

				switch (i->get_type()) {

				case gentity_type::BRUSHMODEL:
					dynamic_cast<brushModelEntity*>(i.get())->render(frustum_planes, 5, poly_type, depth_test, draw_dist);
					break;
				default:
					break;

				}

			}
		}
	}
	if (collisionType == showCollisionType::TERRAIN || collisionType == showCollisionType::BOTH) {

		if (only_elevators == false) {
			for (auto& i : cm_terrainpoints) {
				CM_ShowTerrain(&i, frustum_planes, poly_type, depth_test, draw_dist, only_bounces);
			}
		}

	}


}
void RB_RenderWinding(const showcol_brush& sb, polyType poly_type, bool depth_test, float drawdist, bool only_bounces, bool only_elevators)
{
	if (sb.origin.dist(predictedPlayerState->origin) > drawdist)
		return;

	for (auto& i : sb.windings) {

		if (only_bounces && i.is_bounce == false)
			continue;

		if (only_elevators && i.is_elevator == false)
			continue;

		vec4_t c = { 0,1,1,0.3f };

		c[0] = i.color[0];
		c[1] = i.color[1];
		c[2] = i.color[2];
		c[3] = i.color[3];


		if (only_bounces) {
			float n = i.normals[2];

			if (n > 0.7f || n < 0.3f)
				n = 0.f;
			else
				n = 1.f - (n - 0.3f) / (0.7f - 0.3f);

			c[0] = 1.f - n;
			c[1] = n;
			c[2] = 0.f;
		}


		if (rb_requesting_to_stop_rendering) {
			return;
		}

		if (poly_type == polyType::POLYS)
			RB_DrawCollisionPoly(i.points.size(), (float(*)[3])i.points.data(), c, depth_test);
		else if (poly_type == polyType::EDGES)
			RB_DrawCollisionEdges(i.points.size(), (float(*)[3])i.points.data(), c, depth_test);

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
void RB_DrawCollisionPoly(int numPoints, float(*points)[3], const float* colorFloat, bool depthtest)
{
	uint8_t c[4];
	std::vector<fvec3> pts;

	R_ConvertColorToBytes(colorFloat, c);

	for (int i = 0; i < numPoints; i++)
		pts.push_back(points[i]);


	GfxPointVertex verts[4]{};

	//for (int i = 0; i < numPoints -1; i++) {


	//	RB_AddDebugLine(verts, depthtest, points[i], (float*)points[i + 1], c, 0);
	//	RB_DrawLines3D(1, 3, verts, depthtest);

	//}

	RB_DrawPolyInteriors(numPoints, pts, c, true, depthtest);

}

GfxPointVertex verts[2075];

void RB_DrawCollisionEdges(int numPoints, float(*points)[3], const float* colorFloat, bool depthtest)
{
	uint8_t c[4];

	R_ConvertColorToBytes(colorFloat, c);

	int vert_count = 0;
	int vert_index_prev = numPoints - 1;


	for (int i = 0; i < numPoints; i++) {
		vert_count = RB_AddDebugLine(verts, depthtest, points[i], (float*)points[vert_index_prev], c, vert_count);

		vert_index_prev = i;
	}

	RB_DrawLines3D(vert_count/2, 1, verts, depthtest);

	//delete[] verts;

}