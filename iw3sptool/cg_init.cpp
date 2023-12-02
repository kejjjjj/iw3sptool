#include "pch.hpp"

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
    MH_STATUS state = MH_STATUS::MH_OK;

    if (state = MH_Initialize(), state != MH_STATUS::MH_OK) {
        return FatalError(MH_StatusToString(state));
    }

    hook::nop(0x592B9C);

    hook::write_addr(0x628948, "\xEB\x00\xBA\x20\x38", 5); //jnz -> jmp

    Cmd_AddCommand("cm_showCollisionFilter", Cmd_CollisionFilter_f);
    Cmd_AddCommand("cm_showEntities", Cmd_ShowEntities_f);

    CG_CreatePermaHooks();

    dvar_limits l;
    dvar_value v;

    v.integer = 0;

    static const char* poly_types[] = {"Edges", "Polygons"};
    static const char* showCollisionNames[] = { "Disabled", "Brushes", "Terrain", "Both"};


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

    v.enabled = true;

    static std::string date = std::string(__DATE__) + " - " + std::string(__TIME__);

    Dvar_RegisterNew("cm_showCollisionDepthTest", dvar_type::boolean, dvar_flags::saved, "Select whether to use depth test in collision surfaces display", v, l);

    v.enabled = false;

    Dvar_RegisterNew("cm_onlyBounces", dvar_type::boolean, dvar_flags::none, "Only display surfaces that can be bounced", v, l);
    Dvar_RegisterNew("cm_onlyElevators", dvar_type::boolean, dvar_flags::none, "Only display surfaces that can be elevated", v, l);

    Dvar_RegisterNew("cm_disableTriggers", dvar_type::boolean, dvar_flags::none, "Triggers will not have any effect", v, l);
    Dvar_RegisterNew("cm_entityInfo", dvar_type::boolean, dvar_flags::saved, "Display brushmodel information", v, l);

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

    //Dvar_RegisterNew("cm_experimental", dvar_type::boolean, dvar_flags::none, "Use experimental features", v, l);

    return;
}
void CG_Cleanup()
{
    CG_ReleaseHooks();
    MH_STATUS state = MH_STATUS::MH_OK;

    if (state = MH_Uninitialize(), state != MH_STATUS::MH_OK) {
        return FatalError(MH_StatusToString(state));
    }
}