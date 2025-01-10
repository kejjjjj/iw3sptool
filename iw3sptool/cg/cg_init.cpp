#include <chrono>
#include <thread>

#include "pch.hpp"
#include "cg_offsets.hpp"
#include "utils/engine.hpp"
#include <cmd.hpp>
#include "cg_hooks.hpp"
#include <dvar/dvar.hpp>
#include "cm/cm_export.hpp"
#include "cm/cm_entity.hpp"
#include "cm/cm_brush.hpp"

const unsigned char completeVersion[] =
{
    BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
    '-',
    BUILD_MONTH_CH0, BUILD_MONTH_CH1,
    '-',
    BUILD_DAY_CH0, BUILD_DAY_CH1,
    'T',
    BUILD_HOUR_CH0, BUILD_HOUR_CH1,
    ':',
    BUILD_MIN_CH0, BUILD_MIN_CH1,
    ':',
    BUILD_SEC_CH0, BUILD_SEC_CH1,
    '\0'
};

void CG_Init()
{
    using namespace std::chrono_literals;

    while (!dx->device) {
        std::this_thread::sleep_for(300ms);
    }

 
    Engine::Tools::nop(0x592B9C);

    Engine::Tools::write_bytes(0x628948, "\x90\x90"); //jnz -> nop

    Engine::Tools::nop(0x43DCA9);
    Engine::Tools::nop(0x43DCCB);
    Engine::Tools::nop(0x43DCD9);
    
    //hook::write_addr(0x433BB0, "\xC3", 1);

    Cmd_AddCommand("cm_showCollisionFilter", Cmd_CollisionFilter_f);
    Cmd_AddCommand("cm_showEntities", Cmd_ShowEntities_f);
    Cmd_AddCommand("cm_mapexport", CM_MapExport);

    CG_CreatePermaHooks();

    dvar_limits l{};
    dvar_value v{};

    v.integer = 0;

    static const char* poly_types[] = {"Edges", "Polygons"};
    static const char* showCollisionNames[] = { "Disabled", "Brushes", "Terrain", "Both"};
    static const char* elevator_types[] = { "Disabled", "Enabled", "Include Corners" };
    static const char* entity_info[] = { "Disabled", "Enabled", "Verbose" };


    l.enumeration.stringCount = 4;
    l.enumeration.strings = showCollisionNames;



    Dvar_RegisterNew("cm_showCollision", dvar_type::enumeration, dvar_flags::saved, 
        "Select whether to show the terrain, brush or all collision surface groups", v, l);
    
    l.enumeration.stringCount = 2;
    l.enumeration.strings = poly_types;

    Dvar_RegisterNew("cm_showCollisionPolyType", dvar_type::enumeration, dvar_flags::saved, 
        "Select whether to display the collision surfaces as wireframe or poly interiors", v, l);

    v.value = 2000;
    l.value.max = FLT_MAX;
    l.value.min = 0.f;

    Dvar_RegisterNew("cm_showCollisionDist", dvar_type::value, dvar_flags::saved, "Maximum distance to show collision surfaces", v, l);

    v.value = 0.5f;
    l.value.max = 1.f;
    l.value.min = 0.f;
    Dvar_RegisterNew("cm_showCollisionPolyAlpha", dvar_type::value, dvar_flags::saved, "Collision transparency", v, l);

    v.value = 0.5f;
    l.value.max = 1.f;
    l.value.min = 0.f;
    Dvar_RegisterNew("cm_radiation_radius_scale", dvar_type::value, dvar_flags::saved, 
        "the scale of the actual radiation zone within the radius of the trigger (restart the level if this doesn't do anything)\n"
        "see maps\\_radiation::updateRadiationDosage()" , v, l);

    v.enabled = true;

    static std::string date = std::string(__DATE__) + " - " + std::string(__TIME__);

    Dvar_RegisterNew("cm_showCollisionDepthTest", dvar_type::boolean, dvar_flags::saved, "Select whether to use depth test in collision surfaces display", v, l);

    v.enabled = false;

    Dvar_RegisterNew("cm_onlyBounces", dvar_type::boolean, dvar_flags::none, "Only display surfaces which can be bounced", v, l);

    l.enumeration.stringCount = 3;
    l.enumeration.strings = elevator_types;
    v.integer = 0;

    Dvar_RegisterNew("cm_onlyElevators", dvar_type::enumeration, dvar_flags::none, "Only display surfaces which can be elevated", v, l);

    v.enabled = false;
    Dvar_RegisterNew("cm_disableTriggers", dvar_type::boolean, dvar_flags::none, "Triggers will not have any effect", v, l);
    
    l.enumeration.stringCount = 3;
    l.enumeration.strings = entity_info;

    Dvar_RegisterNew("cm_entityInfo", dvar_type::enumeration, dvar_flags::saved, "Display entity information", v, l);
    v.enabled = true;

    Dvar_RegisterNew("cm_ignoreNonColliding", dvar_type::boolean, dvar_flags::saved, "Don't display surfaces which don't have collisions", v, l);

    v.string = date.c_str();
    Dvar_RegisterNew("tool_version", dvar_type::string, dvar_flags::write_protected, date.c_str(), v, l);

    v.enabled = false;

    Dvar_RegisterNew("cg_thirdPerson", dvar_type::boolean, dvar_flags::none, "Use third person view", v, l);

    l.value.min = -180;
    l.value.max = 360;
    v.value = 0;

    Dvar_RegisterNew("cg_thirdPersonAngle", dvar_type::value, dvar_flags::none, "The angle of the camera from the player in third person view", v, l);

    l.value.min = 0;
    l.value.max = 1024;
    v.value = 120;

    Dvar_RegisterNew("cg_thirdPersonRange", dvar_type::value, dvar_flags::none, "The range of the camera from the player in third person view", v, l);

    v.enabled = false;
    Dvar_RegisterNew("pm_bounceFix", dvar_type::boolean, dvar_flags::none, "bounce more than once", v, l);
    Dvar_RegisterNew("pm_fixed", dvar_type::boolean, dvar_flags::none, "forces fps to com_maxfps value even if the computer can't reach the fps", v, l);
    Dvar_RegisterNew("pm_multiplayer", dvar_type::boolean, dvar_flags::none, "fps behaves the same way as it does in multiplayer", v, l);

    Dvar_RegisterNew("pm_coordinates", dvar_type::boolean, dvar_flags::saved, "show player coordinates", v, l);
    Dvar_RegisterNew("pm_velocity", dvar_type::boolean, dvar_flags::saved, "show player velocity", v, l);

    //Dvar_RegisterNew("cm_experimental", dvar_type::boolean, dvar_flags::none, "Use experimental features", v, l);

    return;
}
void CG_Cleanup()
{
    CG_ReleaseHooks();
}