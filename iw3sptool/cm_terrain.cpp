#include "pch.hpp"

std::unordered_map<CollisionPartition*, CollisionPartition*> discovered_partitions;

void CM_DiscoverTerrain(const std::unordered_set<std::string>& filters)
{
	discovered_partitions.clear();
	for (uint32_t i = 0; i < cm->numLeafs; i++) {
		auto terrain = CM_GetTerrainTriangles(&cm->leafs[i], filters);

		if(terrain)
			CClipMap::InsertGeometry(terrain);

	}
}
bool CM_AabbTreeHasCollisions(const CollisionAabbTree* tree)
{
	dmaterial_t* materialInfo = &cm->materials[tree->materialIndex];
	return (materialInfo->contentFlags & MASK_PLAYERSOLID) != 0;
}
void CM_AdvanceAabbTree(CollisionAabbTree* aabbTree, cm_terrain* terrain, const std::unordered_set<std::string>& filters, const vec4_t color)
{
	if (aabbTree->childCount) {
		//std::cout << "numChildren: " << aabbTree->childCount << '\n';

		//terrain->children = std::vector<cm_terrain>(aabbTree->childCount);

		auto child = &cm->aabbTrees[aabbTree->u.firstChildIndex];
		for (int i = 0; i < aabbTree->childCount; i++) {
			CM_AdvanceAabbTree(child, terrain, filters, color);
			++child;
		}
		return;
	}
	//std::cout << "halfSize: " << fvec3(aabbTree->halfSize) << '\n';

	char* mat = cm->materials[aabbTree->materialIndex].material;

	if (CM_IsMatchingFilter(filters, mat) == false) {
		return;
	}

	terrain->material = mat;
	terrain->color[0] = color[0];
	terrain->color[1] = color[1];
	terrain->color[2] = color[2];
	terrain->color[3] = color[3];

	CollisionAabbTreeIndex fChild = aabbTree->u;
	CollisionPartition* partition = &cm->partitions[fChild.partitionIndex];

	if (discovered_partitions.find(partition) != discovered_partitions.end())
		return;

	discovered_partitions.insert({ partition, partition });

	int firstTri = partition->firstTri;
	if (firstTri < firstTri + partition->triCount)
	{

		int triIndice = 3 * firstTri;

		do {
			cm_triangle tri;
			//tri.edge_walkable = CM_IsEdgeWalkable(2, firstTri);
			tri.has_collision = CM_AabbTreeHasCollisions(aabbTree);
			tri.a = cm->verts[cm->triIndices[triIndice]];
			tri.b = cm->verts[cm->triIndices[triIndice + 1]];
			tri.c = cm->verts[cm->triIndices[triIndice + 2]];

			PlaneFromPointsASM(tri.plane, tri.a, tri.b, tri.c);


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
std::unique_ptr<cm_geometry> CM_GetTerrainTriangles(cLeaf_t* leaf, const std::unordered_set<std::string>& filters)
{
	if (!leaf)
		return 0;

	if (!leaf->collAabbCount)
		return 0;

	CClipMap::wip_geom = std::make_unique<cm_terrain>();

	int aabbIdx = 0;

	auto terrain = dynamic_cast<cm_terrain*>(CClipMap::wip_geom.get());

	terrain->leaf = leaf;


	do {
		CollisionAabbTree* aabb = &cm->aabbTrees[aabbIdx + leaf->firstCollAabbIndex];
		CM_AdvanceAabbTree(aabb, terrain, filters, vec4_t{ 0,0.1f,1.f, 0.8f });

		++aabbIdx;
	} while (aabbIdx < leaf->collAabbCount);

	//if (terrain->tris.empty() && terrain->children.empty()) {
	//	CClipMap::wip_geom.reset();
	//	CClipMap::wip_geom = 0;
	//	return;
	//}

	return std::move(CClipMap::wip_geom);

}
//std::optional<cm_terrain> CM_GetTerrainTriangles(cLeaf_t* leaf, const vec4_t color)
//{
//	if (!leaf)
//		return std::nullopt;
//
//	if (!leaf->collAabbCount)
//		return std::nullopt;
//
//	int aabbIdx = 0;
//
//	cm_terrain terrain{};
//	terrain.leaf = leaf;
//	do {
//		CollisionAabbTree* aabb = &cm->aabbTrees[aabbIdx + leaf->firstCollAabbIndex];
//		CM_AdvanceAabbTree(aabb, &terrain, { "all" }, color);
//		++aabbIdx;
//	} while (aabbIdx < leaf->collAabbCount);
//
//	return terrain.tris.empty() ? std::nullopt : std::make_optional(terrain);
//
//}
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