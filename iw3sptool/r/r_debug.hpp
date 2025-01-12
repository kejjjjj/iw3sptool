#pragma once

#include "typedefs.hpp"

struct DebugGlobals;

DebugGlobals* R_GetDebugGlobals();

void R_AddDebugLine(const float* start, const float* end, const float* color);
void R_AddDebugBox(const float* mins, const float* maxs, const float* color);
void R_AddDebugPolygon(const std::vector<fvec3>& pts, const float* color);
