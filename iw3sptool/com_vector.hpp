#pragma once

#include "pch.hpp"

void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
vec_t VectorNormalize(vec3_t v);       // returns vector length
vec_t VectorNormalize2(const vec3_t v, vec3_t out);
void VectorInverse(vec3_t v);

int BoxOnPlaneSide(const fvec3& emins, const fvec3& emaxs, struct cplane_s* p);

void PlaneFromPointsASM(float* plane, float* v0, float* v1, float* v2);


void BuildFrustumPlanes(const struct GfxViewParms* viewParms, cplane_s* frustumPlanes);
void SetPlaneSignbits(cplane_s* out);
void CreateFrustumPlanes(const struct GfxViewParms* viewParms, cplane_s* frustumPlanes);
//bool PointInFront(const fvec3& origin, const fvec3& target, const cardinalDirection_e d);
fvec3 SetSurfaceBrightness(const fvec3& color, const fvec3& normal, const fvec3& lightDirection);

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);

void AnglesToAxis(const vec3_t angles, vec3_t axis[3]);

fvec3 VectorRotate(const fvec3& va, const fvec3& vb);
fvec3 VectorRotate(const fvec3& vIn, const fvec3& vRotation, const fvec3& vOrigin);

void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void PerpendicularVector(vec3_t dst, const vec3_t src);
void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
void vectoangles(const vec3_t value1, vec3_t angles);
fvec3 AxisToAngles(vec3_t axis[3]);

float AngleNormalizePI(float angle);
float AngleNormalize360(float angle);
float AngleNormalize180(float angle);
float AngleNormalize90(float angle);

float AngleDelta(float angle1, float angle2);