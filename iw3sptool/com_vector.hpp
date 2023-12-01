#pragma once

#include "pch.hpp"

int VectorCompare(const vec3_t v1, const vec3_t v2);
vec_t VectorLength(const vec3_t v);
vec_t VectorLengthSquared(const vec3_t v);
vec_t Distance(const vec3_t p1, const vec3_t p2);
vec_t DistanceSquared(const vec3_t p1, const vec3_t p2);
void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
vec_t VectorNormalize(vec3_t v);       // returns vector length
void VectorNormalizeFast(vec3_t v);     // does NOT return vector length, uses rsqrt approximation
vec_t VectorNormalize2(const vec3_t v, vec3_t out);
void VectorInverse(vec3_t v);

void MatrixTransformVector43(const float* in1, const float(*in2)[3], float* out);
int BoxOnPlaneSide(const vec3_t emins, const vec3_t emaxs, struct cplane_s* p);

void BuildFrustumPlanes(const struct GfxViewParms* viewParms, cplane_s* frustumPlanes);
void SetPlaneSignbits(cplane_s* out);
void CreateFrustumPlanes(const struct GfxViewParms* viewParms, cplane_s* frustumPlanes);
//bool PointInFront(const fvec3& origin, const fvec3& target, const cardinalDirection_e d);
fvec3 SetSurfaceBrightness(const fvec3& color, const fvec3& normal, const fvec3& lightDirection);

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

void AnglesToAxis(const vec3_t angles, vec3_t axis[3]);

fvec3 VectorRotate(const fvec3& va, const fvec3& vb);
fvec3 VectorRotate(const fvec3& vIn, const fvec3& vRotation, const fvec3& vOrigin);