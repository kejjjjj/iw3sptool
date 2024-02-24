#include "pch.hpp"


void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross) {
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void VectorInverse(vec3_t v) {
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
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
int BoxOnPlaneSide(const fvec3& emins, const fvec3& emaxs, cplane_s* p) {
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
	out->signbits = static_cast<char>(bits);
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
	int nIndex[3][2]{};
	nIndex[0][0] = 1; nIndex[0][1] = 2;
	nIndex[1][0] = 2; nIndex[1][1] = 0;
	nIndex[2][0] = 0; nIndex[2][1] = 1;

	for (int i = 0; i < 3; i++)
	{
		if (vRotation[i] != 0.000000f)
		{
			float dAngle = vRotation[i] * PI / 180.0f;
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

void PlaneFromPointsASM(float* plane, float* v0, float* v1, float* v2)
{
	static constexpr DWORD fnc = 0x57E7D0;
	__asm
	{
		mov esi, v0;
		mov edi, plane;
		mov ecx, v1;
		mov edx, v2;
		call fnc;
	}
}
void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]) {
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}
void ProjectPointOnPlane(vec3_t dst, const vec3_t p, const vec3_t normal) {
	float d{};
	vec3_t n{};
	float inv_denom = {};

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}
void PerpendicularVector(vec3_t dst, const vec3_t src) {
	int pos = {};
	int i = {};
	float minelem = 1.0F;
	vec3_t tempvec = {};

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for (pos = 0, i = 0; i < 3; i++)
	{
		if (fabs(src[i]) < minelem) {
			pos = i;
			minelem = fabs(src[i]);
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane(dst, tempvec, src);

	/*
	** normalize the result
	*/
	VectorNormalize(dst);
}

void RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point,
	float degrees) {
	float m[3][3] = {};
	float im[3][3] = {};
	float zrot[3][3] = {};
	float tmpmat[3][3] = {};
	float rot[3][3] = {};
	int i = {};
	vec3_t vr = {}, vup = {}, vf = {};
	float rad = {};

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector(vr, dir);
	CrossProduct(vr, vf, vup);

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy(im, m, sizeof(im));

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset(zrot, 0, sizeof(zrot));
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	rad = DEG2RAD(degrees);
	zrot[0][0] = cos(rad);
	zrot[0][1] = sin(rad);
	zrot[1][0] = -sin(rad);
	zrot[1][1] = cos(rad);

	MatrixMultiply(m, zrot, tmpmat);
	MatrixMultiply(tmpmat, im, rot);

	for (i = 0; i < 3; i++) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

fvec3 AxisToAngles(vec3_t axis[3]) {
	vec3_t angles{};
	vec3_t right{}, roll_angles{}, tvec{};
	fvec3 axisDefault[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

	

	// first get the pitch and yaw from the forward vector
	vectoangles(axis[0], angles);

	// now get the roll from the right vector
	VectorCopy(axis[1], right);
	// get the angle difference between the tmpAxis[2] and axis[2] after they have been reverse-rotated
	RotatePointAroundVector(tvec, axisDefault[2], right, -angles[YAW]);
	RotatePointAroundVector(right, axisDefault[1], tvec, -angles[PITCH]);
	// now find the angles, the PITCH is effectively our ROLL
	vectoangles(right, roll_angles);
	roll_angles[PITCH] = AngleNormalize180(roll_angles[PITCH]);
	// if the yaw is more than 90 degrees difference, we should adjust the pitch
	if (DotProduct(right, axisDefault[1]) < 0) {
		if (roll_angles[PITCH] < 0) {
			roll_angles[PITCH] = -90 + (-90 - roll_angles[PITCH]);
		}
		else {
			roll_angles[PITCH] = 90 + (90 - roll_angles[PITCH]);
		}
	}

	angles[ROLL] = -roll_angles[PITCH];

	return angles;
}

void vectoangles(const vec3_t value1, vec3_t angles) {
	float forward;
	float yaw, pitch;

	if (value1[1] == 0 && value1[0] == 0) {
		yaw = 0;
		if (value1[2] > 0) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		if (value1[0]) {
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		}
		else if (value1[1] > 0) {
			yaw = 90;
		}
		else {
			yaw = 270;
		}
		if (yaw < 0) {
			yaw += 360;
		}
		forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0) {
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}
float AngleNormalizePI(float angle)
{
	angle = fmodf(angle + (float)M_PI, 2 * (float)M_PI);
	return angle < 0 ? angle + (float)M_PI : angle - (float)M_PI;
}
float AngleNormalize360(float angle) {
	return (360.0f / 65536) * ((int)(angle * (65536 / 360.0f)) & 65535);
}
float AngleNormalize180(float angle) {
	angle = AngleNormalize360(angle);
	if (angle > 180.0) {
		angle -= 360.0;
	}
	return angle;
}
float AngleNormalize90(float angle)
{
	return fmodf(angle + 180 + 90, 180) - 90;

}
float AngleDelta(float angle1, float angle2) {
	return AngleNormalize180(angle1 - angle2);
}