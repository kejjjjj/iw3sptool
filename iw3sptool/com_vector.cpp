#include "pch.hpp"


void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross) {
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

vec_t VectorLength(const vec3_t v) {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

vec_t VectorLengthSquared(const vec3_t v) {
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

vec_t Distance(const vec3_t p1, const vec3_t p2) {
	vec3_t v;

	VectorSubtract(p2, p1, v);
	return VectorLength(v);
}

vec_t DistanceSquared(const vec3_t p1, const vec3_t p2) {
	vec3_t v;

	VectorSubtract(p2, p1, v);
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}


void VectorInverse(vec3_t v) {
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

int VectorCompare(const vec3_t v1, const vec3_t v2) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}

	return 1;
}


vec_t VectorNormalize(vec3_t v) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);

	if (length) {
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}
vec_t VectorNormalize2(const vec3_t v, vec3_t out) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);

	if (length) {
		ilength = 1 / length;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
	}
	else {
		VectorClear(out);
	}

	return length;

}
void MatrixTransformVector43(const float* in1, const float(*in2)[3], float* out)
{
	*out = ((*in1 * (*in2)[0]) + (((*in2)[6] * in1[2]) + ((*in2)[3] * in1[1]))) + (*in2)[9];
	out[1] = (((*in2)[4] * in1[1]) + (((*in2)[1] * *in1) + ((*in2)[7] * in1[2]))) + (*in2)[10];
	out[2] = (((*in2)[5] * in1[1]) + (((*in2)[2] * *in1) + ((*in2)[8] * in1[2]))) + (*in2)[11];
}

int BoxOnPlaneSide(const vec3_t emins, const vec3_t emaxs, cplane_s* p) {
	float dist1, dist2;

	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 1:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 2:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 3:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 4:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;      // shut up compiler
		break;
	}

	return (2 * (dist2 < p->dist)) | (dist1 > p->dist);
}

void BuildFrustumPlanes([[maybe_unused]]const GfxViewParms* viewParms, cplane_s* frustumPlanes)
{

	for (int i = 0; i < 5; i++) {

		cplane_s* plane = &frustumPlanes[i];
		auto dpvs = &dpvsGlob->views[0].frustumPlanes[i];

		VectorCopy(dpvs->coeffs, plane->normal);
		plane->dist = dpvs->coeffs[3] * -1;

		char signbit = 0;

		if (plane->normal[0] != 1.f) {
			if (plane->normal[1] == 1.f)
				signbit = 1;
			else {
				signbit = plane->normal[2] == 1.f ? 2 : 3;
			}
		}
		
		plane->type = signbit;

		SetPlaneSignbits(plane);

	}
}
void CreateFrustumPlanes(const struct GfxViewParms* viewParms, cplane_s* frustum_planes)
{
	BuildFrustumPlanes(viewParms, frustum_planes);

	frustum_planes[5].normal[0] = -frustum_planes[4].normal[0];
	frustum_planes[5].normal[1] = -frustum_planes[4].normal[1];
	frustum_planes[5].normal[2] = -frustum_planes[4].normal[2];

	frustum_planes[5].dist = -frustum_planes[4].dist - 2000;
	auto plane = &frustum_planes[5];

	char signbit = 0;

	if (plane->normal[0] != 1.f) {
		if (plane->normal[1] == 1.f)
			signbit = 1;
		else {
			signbit = plane->normal[2] == 1.f ? 2 : 3;
		}
	}

	plane->type = signbit;

	SetPlaneSignbits(plane);
}
void SetPlaneSignbits(cplane_s* out)
{
	int bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j = 0; j < 3; j++) {
		if (out->normal[j] < 0) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}
//bool PointInFront(const fvec3& origin, const fvec3& target, const cardinalDirection_e d)
//{
//	
//	switch (d) {
//	case cardinalDirection_e::N:
//		return target.x > origin.x;
//	case cardinalDirection_e::E:
//		return target.y < origin.y;
//	case cardinalDirection_e::S:
//		return target.x < origin.x;
//	case cardinalDirection_e::W:
//		return target.y > origin.y;
//	}
//
//}


fvec3 SetSurfaceBrightness(const fvec3& color, const fvec3& normal, const fvec3& lightDirection) {
	
	fvec3 normalizedLight = (lightDirection).normalize();

	float dotProduct = normalizedLight.dot(normal);

	float brightnessAdjustment = 1.f - std::max(0.0f, dotProduct);
	float maxComponent = std::max({ color.x, color.y, color.z });
	brightnessAdjustment *= maxComponent; 

	return color * brightnessAdjustment;

}

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up) {
	float angle;
	static float sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * (M_PI * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);

	angle = angles[PITCH] * (M_PI * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles[ROLL] * (M_PI * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	if (forward) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if (right) {
		right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
		right[2] = -1 * sr * cp;
	}
	if (up) {
		up[0] = (cr * sp * cy + -sr * -sy);
		up[1] = (cr * sp * sy + -sr * cy);
		up[2] = cr * cp;
	}
}

void AnglesToAxis(const vec3_t angles, vec3_t axis[3]) {
	vec3_t right;

	// angle vectors returns "right" instead of "y axis"
	AngleVectors(angles, axis[0], right, axis[2]);
	vec3_t org = { 0,0,0 };
	VectorSubtract(org, right, axis[1]);
}
fvec3 VectorRotate(const fvec3& vIn, const fvec3& vRotation)
{
	fvec3 vWork, va;
	VectorCopy(vIn, va);
	VectorCopy(va, vWork);
	int nIndex[3][2];
	nIndex[0][0] = 1; nIndex[0][1] = 2;
	nIndex[1][0] = 2; nIndex[1][1] = 0;
	nIndex[2][0] = 0; nIndex[2][1] = 1;

	for (int i = 0; i < 3; i++)
	{
		if (vRotation[i] != 0.000000f)
		{
			float dAngle = vRotation[i] * PI / 180.0;
			float c = cos(dAngle);
			float s = sin(dAngle);
			vWork[nIndex[i][0]] = va[nIndex[i][0]] * c - va[nIndex[i][1]] * s;
			vWork[nIndex[i][1]] = va[nIndex[i][0]] * s + va[nIndex[i][1]] * c;
		}
		VectorCopy(vWork, va);
	}

	return vWork;
}
fvec3 VectorRotate(const fvec3& vIn, const fvec3& vRotation, const fvec3& vOrigin)
{
	fvec3 vRotation2 = vRotation;
	std::swap(vRotation2.y, vRotation2.z);
	std::swap(vRotation2.x, vRotation2.y);

	fvec3 vTemp;
	VectorSubtract(vIn, vOrigin, vTemp);
	fvec3 vTemp2 = VectorRotate(vTemp, vRotation2);

	return vTemp2 + vOrigin;
}