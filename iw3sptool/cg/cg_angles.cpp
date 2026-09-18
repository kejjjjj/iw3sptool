#include "cg_angles.hpp"
#include "cg_local.hpp"
#include "cg_offsets.hpp"
#include "com/com_vector.hpp"

float CG_SmoothAngle(const float src, const float dst, float smoothingFactor) noexcept {

	smoothingFactor = 1.f - std::max(0.f, std::min(1.f, smoothingFactor));
	return src + smoothingFactor * (dst - src);
}
void CL_SetPlayerAngles(usercmd_s* cmd, float* delta_angles, const fvec3& target)
{
	for (int i = 0; i < 3; i++) {

		float cmdAngle = SHORT2ANGLE(cmd->angles[i]);

		float delta = AngleDelta(delta_angles[i], cmdAngle);
		float real_delta = AngleDelta(delta, delta_angles[i]);
		float final = AngleDelta(delta_angles[i], target[i]);
		cmd->angles[i] += ANGLE2SHORT(real_delta - final);
		(*(fvec3*)0x868728)[i] += real_delta - final;
	}
}
void CL_SetPlayerAngle(usercmd_s* cmd, float* delta_angles, const float target, int AXIS)
{
	float cmdAngle = SHORT2ANGLE(cmd->angles[AXIS]);

	float delta = AngleDelta(delta_angles[AXIS], cmdAngle);
	float real_delta = AngleDelta(delta, delta_angles[AXIS]);
	float final = AngleDelta(delta_angles[AXIS], target);
	cmd->angles[AXIS] += ANGLE2SHORT(real_delta - final);
}
void CL_SetPlayerYaw(usercmd_s* cmd, float* delta_angles, const float target)
{
	return CL_SetPlayerAngle(cmd, delta_angles, target, YAW);
}
void CL_SetPlayerPitch(usercmd_s* cmd, float* delta_angles, const float target)
{
	return CL_SetPlayerAngle(cmd, delta_angles, target, PITCH);
}
fvec3 CG_AnglesFromCmd(usercmd_s* cmd)
{
	return (ivec3(cmd->angles).from_short() + cgs->predictedPlayerState.delta_angles).normalize180();
}
fvec3 CG_AnglesFromCmd(usercmd_s* cmd, playerState_s* ps)
{
	return (ivec3(cmd->angles).from_short() + ps->delta_angles).normalize180();

}
axis_t CG_GetAxisBasedOnYaw(float yaw)
{
	float nearest = CG_GetNearestCardinalAngle(yaw);
	return (nearest == 0 || nearest == 180 ? axis_t::X : axis_t::Y);
}
cardinal_dir CG_RoundAngleToCardinalDirection(const float a)
{
	const float rounded = CG_GetNearestCardinalAngle(a);

	if (rounded == 0)
		return cardinal_dir::N;
	if (rounded == 90)
		return cardinal_dir::W;
	if (rounded == 270)
		return cardinal_dir::E;
	return cardinal_dir::S;
}


float CG_GetNearestCardinalAngle(float yawangle)
{
	if (yawangle > 180 || yawangle < -180)
		yawangle = AngleNormalize180(yawangle);

	if (yawangle >= 135 || yawangle <= -135)
		return 180;  //X backward
	else if (yawangle <= -45 && yawangle >= -135)
		return 270; //Y backward
	else if (yawangle >= 45 && yawangle <= 135)
		return 90;  //Y forward
	return 0; // x forward


}

float AngleWrap90(float angle)
{
	angle = AngleNormalize180(angle);

	if (angle > -180 && angle <= -90)
		return angle + 180;
	else if (angle > -90 && angle < 0)
		return angle + 90;
	else if (angle >= 0 && angle <= 90)
		return angle;
	else if (angle > 90 && angle < 180)
		return angle - 90;
	
	return 0;
}
int AngleWrap90(int angle)
{
	angle = static_cast<int>(AngleNormalize180(static_cast<float>(angle)));

	if (angle > -180 && angle <= -90)
		return angle + 180;
	else if (angle > -90 && angle < 0)
		return angle + 90;
	else if (angle >= 0 && angle <= 90)
		return angle;
	else if (angle > 90 && angle < 180)
		return angle - 90;

	return 0;
}

float AngularDistance(float value1, float value2) {
	float diff = fmodf(value2 - value1 + 180, 360) - 180;
	if (diff < -180) {
		diff += 360;
	}
	return std::abs(diff);
}