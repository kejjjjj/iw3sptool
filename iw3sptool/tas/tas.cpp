#include "tas.hpp"

#include "cg/cg_offsets.hpp"
#include "g/g_weapon.hpp"
#include "com/com_channel.hpp"
#include "cg/cg_angles.hpp"
#include "r/r_drawtools.hpp"

#include <print>

std::int32_t& cmdNumber = *reinterpret_cast<std::int32_t*>(0x869534);
usercmd_s* cmds = reinterpret_cast<usercmd_s*>(8816436);
std::int32_t* serverTime = reinterpret_cast<std::int32_t*>(0x867090);
std::int32_t* realTime = reinterpret_cast<std::int32_t*>(0x85BC18);

[[nodiscard]] int AngleToGame(int angle) { 
    return static_cast<int>(std::lround(static_cast<double>(angle) * TAS::AngleToShort)); 
} 
[[nodiscard]] int AngleToGame(double angle) { 
    return static_cast<int>(std::lround(angle * TAS::AngleToShort));
}

std::vector<TAS::CompiledCommand> TAS::Compile(const Script& script) {
    std::vector<CompiledCommand> commands;

    int32_t timeline = 0;
    std::array<float, 3> currentAngles{};

    if (script.header.startViewangles)
        currentAngles = *script.header.startViewangles;

    CTasSegment fixSegment;
    fixSegment.durationMs = 700;
    fixSegment.commandRate = 250;
	fixSegment.angle.type = AngleType::Absolute;
	fixSegment.angle.angles = currentAngles;

    const_cast<Script&>(script).segments.insert(script.segments.begin(), fixSegment);

    for (const auto& segment : script.segments) {

        const int32_t cmdDelta = 1000 / std::max(1, segment.commandRate);
        const int32_t commandCount = segment.durationMs / cmdDelta;

        const auto startAngles = currentAngles;
        std::array<float, 3> targetAngles = currentAngles;

        switch (segment.angle.type)
        {
        case AngleType::None:
            break;

        case AngleType::Absolute:
            targetAngles = segment.angle.angles;
            break;

        case AngleType::Delta:
            for (int axis = 0; axis < 3; ++axis) {
                targetAngles[axis] = currentAngles[axis] + segment.angle.angles[axis];
            }
            break;
        }

        uint8_t weapon{}, offhand{};

        if (segment.weapon != "none") {
            weapon = static_cast<uint8_t>(G_GetWeaponIndexForName(segment.weapon.c_str()));

            if (!weapon)
                Com_Printf("^3Warning: ^7Invalid weapon name \"%s\" in TAS segment\n", segment.weapon.c_str());
        } if (segment.offHand != "none") {
            offhand = static_cast<uint8_t>(G_GetWeaponIndexForName(segment.offHand.c_str()));

            if (!offhand)
                Com_Printf("^3Warning: ^7Invalid off-hand weapon name \"%s\" in TAS segment\n", segment.offHand.c_str());
        }



        for (int32_t i = 0; i < commandCount; ++i) {
            usercmd_s cmd{};

            const int32_t commandTime = i * cmdDelta;

            cmd.serverTime = timeline + commandTime;
            cmd.buttons = segment.buttons;
            cmd.forwardmove = segment.forwardmove;

            char prevRightmove = commands.size() > 0 ? commands.back().cmd.rightmove : 0;

			if (segment.angle.type == AngleType::Strafebot && segment.rightmove == 0) {
				cmd.rightmove = prevRightmove > 0 ? -127 : 127;
			} else {
				cmd.rightmove = segment.rightmove;
			}

            cmd.weapon = weapon;
            cmd.offHandIndex = offhand;
            cmd.gunXOfs = 0.f;
            cmd.gunYOfs = 0.f;
            cmd.gunZOfs = 0.f;


            switch(segment.angle.type) {
            case AngleType::Delta: {

                const float fraction = commandCount > 1 ? static_cast<float>(i) / static_cast<float>(commandCount - 1) : 1.0f;

                for (int axis = 0; axis < 3; ++axis) {
                    const float angle = startAngles[axis] + (targetAngles[axis] - startAngles[axis]) * fraction;
                    cmd.angles[axis] = ANGLE2SHORT(angle);
                }
                break;
			}
            default:
                for (int axis = 0; axis < 3; ++axis)
                    cmd.angles[axis] = ANGLE2SHORT(targetAngles[axis]);
                break;
            }

            commands.push_back({ cmd, cmd.serverTime, segment.angle.type == AngleType::Strafebot, segment.commandRate });
        }

        timeline += commandCount * cmdDelta;

        if (segment.angle.type != AngleType::None)
            currentAngles = targetAngles;
    }

    CTasSegment lastfixSegment;
    lastfixSegment.durationMs = 1000;
    lastfixSegment.commandRate = 250;
    lastfixSegment.angle.type = AngleType::None;
    const_cast<Script&>(script).segments.insert(script.segments.begin(), lastfixSegment);
    return commands;
}

void TAS::Player::Start()
{
    Stop();

    m_active = true;
    m_started = false;

    m_commandIndex = 0;
    m_baseServerTime = *serverTime;
    m_startRealtime = 0;

    m_commandMeta.clear();
    m_commandMeta.reserve(m_script.m_commands.size());

    (fvec3&)ps_loc->delta_angles = fvec3(0.f);

    if (m_script.header.startOrigin) {
        VectorCopy(m_script.header.startOrigin->data(), ps_loc->origin);
    }

}
#include <print>
void TAS::Player::Predict() {
    m_active = true;

    m_commandIndex = 0;
    m_baseServerTime = *serverTime;
    m_startRealtime = 0;

    m_commandMeta.clear();
    m_commandMeta.reserve(m_script.m_commands.size());

    playerState_s tempPs = *predictedPlayerState;
    (fvec3&)tempPs.delta_angles = fvec3(0.f);

    if (m_script.header.startOrigin) {
        VectorCopy(m_script.header.startOrigin->data(), tempPs.origin);
    }

    if (m_script.header.startViewangles) {
        VectorCopy(m_script.header.startViewangles->data(), tempPs.viewangles);
    }

    tempPs.commandTime = m_baseServerTime;
    usercmd_s oldcmd = cmds[(cmdNumber - 1) & CMD_MASK];

    m_predictedPositions.clear();
    m_predictedPositions.reserve(m_script.m_commands.size());

    for (auto& compiled : m_script.m_commands) {
        usercmd_s cmd = compiled.cmd;
        cmd.serverTime += m_baseServerTime;

        m_commandMeta[cmd.serverTime] = {
            compiled.strafebot,
            compiled.targetFps,
        };

        compiled.cmd.gunPitch = tempPs.viewangles[PITCH];
        compiled.cmd.gunYaw = tempPs.viewangles[YAW];

        PredictPlayerState(&tempPs, &cmd, &oldcmd);
        oldcmd = cmd;

        m_predictedPositions.push_back(tempPs.origin);
    }

    m_active = false;
}
void TAS::Player::Stop()
{
    m_active = false;
    m_started = false;

    m_commandIndex = 0;
    m_startRealtime = 0;
    m_baseServerTime = 0;

}

void TAS::Player::Update()
{
    if (!m_active)
        return;

    if (!m_started)
    {
        if (((fvec3&)predictedPlayerState->velocity).mag_sq() != 0.f)
            return;

        m_baseServerTime = *serverTime;
        m_startRealtime = GetRealtime();
        m_started = true;
    }

    if (playersKb[KB_MOVELEFT].active || playersKb[KB_MOVERIGHT].active || playersKb[KB_FORWARD].active || playersKb[KB_BACKWARD].active)
        return Stop();

    const int32_t elapsed = GetRealtime() - m_startRealtime;

    while (m_commandIndex < m_script.m_commands.size())
    {
        const auto& command = m_script.m_commands[m_commandIndex];

        if (command.time > elapsed)
            break;

        InjectNext();
    }

    if (m_commandIndex >= m_script.m_commands.size())
        Stop();
}

void TAS::Player::InjectNext()
{
    if (m_commandIndex >= m_script.m_commands.size())
        return;

    const auto& compiled = m_script.m_commands[m_commandIndex];

    playerState_s* state = predictedPlayerState;
    usercmd_s cmd = BuildNextCommand(compiled, state);

    const int cmdNum = (++cmdNumber) & CMD_MASK;
    cmds[cmdNum] = cmd;

    m_commandMeta[cmd.serverTime] = {
        .strafebot = compiled.strafebot,
        .targetFps = compiled.targetFps
    };

    ++m_commandIndex;
}

int32_t TAS::Player::GetRealtime() {
    return static_cast<int32_t>(*realTime);
}
usercmd_s TAS::Player::BuildNextCommand(const CompiledCommand& compiled, const playerState_s* state) {
    usercmd_s cmd = compiled.cmd;
    cmd.serverTime += m_baseServerTime;

    if (!cmd.weapon)
        cmd.weapon = static_cast<uint8_t>(state->weapon);

    if (!cmd.offHandIndex)
        cmd.offHandIndex = static_cast<uint8_t>(state->offHandIndex);

    return cmd;
}
#include <print>
std::optional<float> TAS::Player::GetOptYaw(pmove_t* _pm, const usercmd_s* cmd, const playerState_s* ps, int32_t fps) {

    if (!cmd || !ps)
        return std::nullopt;

    char forwardmove = cmd->forwardmove;
    char rightmove = cmd->rightmove;

    if ((!forwardmove && !rightmove) || ps->pm_type != PM_NORMAL)
        return std::nullopt;

    float g_speed = static_cast<float>(ps->speed);
    const float accel = g_speed / fps;

    WeaponDef* weapon = BG_WeaponNames[ps->weapon];

    pmove_t pm = *_pm;
	pm.cmd = *cmd;
    pm.ps = const_cast<playerState_s*>(ps);

    fvec3 start = ps->origin;
    fvec3 end = ps->origin;
    start.z += 0.25f;
    end.z -= 0.25f;

    trace_t trace;
    //CG_TraceCapsule(&trace, start.As<float*>(), end.As<float*>(), pm.mins, pm.maxs, ps->clientNum, pm.tracemask);

    pmoveHandler_t* handler = reinterpret_cast<pmoveHandler_t*>(0x06DA590);
    handler->trace(&trace, start, pm.mins, pm.maxs, end, ps->clientNum, pm.tracemask);

    // grounded
    if (ps->groundEntityNum == 2174) {

		float scale = trace.fraction > 0.f && trace.fraction < 1.f ? trace.normal[Z] : 1.f;

   //     if(trace.fraction > 0.f && trace.fraction < 1.f)
			//std::println("grounded trace fraction: {}, normal: {}, scale: {}", trace.fraction, trace.normal[Z], scale);

        if ((cmd->buttons & cmdEnums::sprint) == 0)
            g_speed = (186.f * weapon->moveSpeedScale) * scale;
        else {
            g_speed = (226 * weapon->moveSpeedScale) * scale; //hardcoded to g_speed 190 :new_moon_with_face:
        }   
    }

    const auto velocitydirection = atan2f(ps->velocity[1], ps->velocity[0]) * 180.f / PI;
    const auto accelerationAng = atan2f(static_cast<float>(-rightmove), static_cast<float>(forwardmove)) * 180.f / PI;
    float speed = fvec2(ps->velocity).mag();
    const auto diff = acosf((g_speed - accel) / speed) * 180.f / PI;

    if (std::isnan(diff))
        return std::nullopt;

    const float yaw = ps->viewangles[YAW];

    if (rightmove > 0) {
        return std::optional<float>(-AngleDelta(yaw + accelerationAng, (velocitydirection - diff)));
    }
    else if (rightmove < 0) {
        return std::optional<float>(-AngleDelta(yaw + accelerationAng, (velocitydirection + diff)));
    }

    return std::nullopt;
}

#include "bg/bg_pmove.hpp"
void TAS::Player::PredictPlayerState(playerState_s* ps, usercmd_s* cmd, const usercmd_s* oldCmd) {

    pmove_t pm = *(pmove_t*)0x7FDE88;
    pm.ps = ps;
	pm.cmd = *cmd;
	pm.oldcmd = *oldCmd;
    //Pmove(&pm);
	
    SimulatePmove(&pm);
    *cmd = pm.cmd;

}