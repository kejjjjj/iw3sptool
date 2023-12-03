#include "pch.hpp"

void CM_DiscoverTerrain(const std::unordered_set<std::string>& filters)
{
	for (int i = 0; i < cm->numLeafs; i++) {
		CM_GetTerrainTriangles(&cm->leafs[i], filters);

	}
}

void CM_AdvanceAabbTree(CollisionAabbTree* aabbTree, cm_terrain* terrain, const std::unordered_set<std::string>& filters, const vec4_t color)
{

	if (aabbTree->childCount) {
		auto child = &cm->aabbTrees[aabbTree->u.firstChildIndex];
		for (int i = 0; i < aabbTree->childCount; i++) {
			CM_AdvanceAabbTree(child, terrain, filters, color);
			++child;
		}
		return;
	}
	char* mat = cm->materials[aabbTree->materialIndex].material;

	if (CM_IsMatchingFilter(filters, mat) == false) {
		return;
	}

	CollisionAabbTreeIndex fChild = aabbTree->u;
	CollisionPartition* partition = &cm->partitions[fChild.firstChildIndex];

	int firstTri = partition->firstTri;



	if (firstTri < firstTri + partition->triCount)
	{

		int triIndice = 3 * firstTri;

		terrain->aabb = aabbTree;


		do {
			cm_triangle tri;
			//tri.edge_walkable = CM_IsEdgeWalkable(2, firstTri);

			tri.a = cm->verts[cm->triIndices[triIndice]];
			tri.b = cm->verts[cm->triIndices[triIndice + 1]];
			tri.c = cm->verts[cm->triIndices[triIndice + 2]];

			PlaneFromPoints(tri.plane, tri.a, tri.b, tri.c);


			const fvec3 col_ = SetSurfaceBrightness(color, tri.plane, Dvar_FindMalleableVar("r_lightTweakSunDirection")->current.vector);

			tri.color[0] = col_[0];
			tri.color[1] = col_[1];
			tri.color[2] = col_[2];
			tri.color[3] = 0.3f;


			terrain->tris.push_back(std::move(tri));


			++firstTri;
			triIndice += 3;

		} while (firstTri < partition->firstTri + partition->triCount);

	}

}
void CM_GetTerrainTriangles(cLeaf_t* leaf, const std::unordered_set<std::string>& filters)
{
	if (!leaf)
		return;

	if (!leaf->collAabbCount)
		return;

	int aabbIdx = 0;

	cm_terrain terrain{};
	terrain.leaf = leaf;
	do {
		CollisionAabbTree* aabb = &cm->aabbTrees[aabbIdx + leaf->firstCollAabbIndex];
		CM_AdvanceAabbTree(aabb, &terrain, filters, vec4_t{0,0.1f,1.f, 0.8f});

		++aabbIdx;
	} while (aabbIdx < leaf->collAabbCount);

	if(terrain.tris.empty() == false)
		cm_terrainpoints.push_back(std::move(terrain));

}
std::optional<cm_terrain> CM_GetTerrainTriangles(cLeaf_t* leaf, const vec4_t color)
{
	if (!leaf)
		return std::nullopt;

	if (!leaf->collAabbCount)
		return std::nullopt;

	int aabbIdx = 0;

	cm_terrain terrain{};
	terrain.leaf = leaf;
	do {
		CollisionAabbTree* aabb = &cm->aabbTrees[aabbIdx + leaf->firstCollAabbIndex];
		CM_AdvanceAabbTree(aabb, &terrain, { "all" }, color);
		++aabbIdx;
	} while (aabbIdx < leaf->collAabbCount);

	return terrain.tris.empty() ? std::nullopt : std::make_optional(terrain);

}
void CM_ShowTerrain(cm_terrain* terrain, struct cplane_s* frustumPlanes, polyType poly_type, bool depth_test, float drawdist, bool only_bounces)
{
	uint8_t col[4];
	vec3_t tris[3];
	fvec3 center;
	std::vector<fvec3> points(3);
	int i = 2;

	bool unwalkable_edges = false;

	for (auto it = terrain->tris.begin(); it != terrain->tris.end(); ++it) {

		if ((it->plane[2] < 0.3f || it->plane[2] > 0.7f) && only_bounces) {
				continue;
		}

		//don't render if not visible
		if (!CM_TriangleInView(&*it, frustumPlanes, 5))
			continue;

		float* c = it->color;

		c[3] = Dvar_FindMalleableVar("cm_showCollisionPolyAlpha")->current.value;

		if (only_bounces) {
			float n = it->plane[2];

			if (n > 0.7f || n < 0.3f)
				n = 0.f;
			else
				n = 1.f - (n - 0.3f) / (0.7f - 0.3f);

			c[0] = 1.f - n;
			c[1] = n;
			c[2] = 0.f;
		}

		R_ConvertColorToBytes(c, col);


		points[0] = (it->a);
		points[1] = (it->b);
		points[2] = (it->c);

		center.x = { (points[0].x + points[1].x + points[2].x) / 3 };
		center.y = { (points[0].y + points[1].y + points[2].y) / 3 };
		center.z = { (points[0].z + points[1].z + points[2].z) / 3 };

		if (center.dist(predictedPlayerState->origin) > drawdist)
			continue;


		if (poly_type == polyType::POLYS)
			RB_DrawPolyInteriors(3, points, col, true, depth_test);

		else if (poly_type == polyType::EDGES)
			RB_DrawCollisionEdges(3, (float(*)[3])points.data(), c, depth_test);

	}

}
bool CM_TriangleInView(const cm_triangle* tris, struct cplane_s* frustumPlanes, int numPlanes)
{
	if (numPlanes <= 0)
		return 1;



	cplane_s* plane = frustumPlanes;
	int idx = 0;
	while ((BoxOnPlaneSide(tris->get_mins(), tris->get_maxs(), plane) & 1) != 0) {
		++plane;
		++idx;

		if (idx >= numPlanes)
			return 1;
	}

	return 0;
}
//char CM_IsEdgeWalkable(int edgeIndex, int triIndex)
//{
//	char r{};
//	__asm
//	{
//		mov ecx, triIndex;
//		mov eax, edgeIndex;
//		mov esi, 0x4EFCB0;
//		call esi;
//		mov r, al;
//	}
//
//	return r;
//}