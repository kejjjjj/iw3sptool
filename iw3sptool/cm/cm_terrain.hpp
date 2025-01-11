#pragma once

#include <unordered_set>
#include <string>
#include <memory>

struct CollisionAabbTree;
struct cLeaf_t;

struct cm_terrain;
struct cm_geometry;
struct cm_triangle;

class CClipMap;

void CM_LoadAllTerrainToClipMapWithFilters(const std::unordered_set<std::string>& filters);


bool CM_AabbTreeHasCollisions(const CollisionAabbTree* tree);
bool CM_DiscoverTerrain(const std::unordered_set<std::string>& filters);
bool CM_TriangleInView(const cm_triangle* leaf, struct cplane_s* frustumPlanes, int numPlanes);
void CM_AdvanceAabbTree(cm_terrain& terrain, const CollisionAabbTree* aabbTree, const std::unordered_set<std::string>& filters, const float* color);
