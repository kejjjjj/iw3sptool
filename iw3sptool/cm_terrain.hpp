#pragma once

#include "pch.hpp"

bool CM_AabbTreeHasCollisions(const CollisionAabbTree* tree);

void CM_DiscoverTerrain(const std::unordered_set<std::string>& filters);

std::unique_ptr<cm_geometry> CM_GetTerrainTriangles(cLeaf_t* leaf, const std::unordered_set<std::string>& filters);
void CM_AdvanceAabbTree(CollisionAabbTree* aabbTree, cm_terrain* terrain, const std::unordered_set<std::string>& filters, const vec4_t color);
bool CM_TriangleInView(const cm_triangle* leaf, struct cplane_s* frustumPlanes, int numPlanes);
