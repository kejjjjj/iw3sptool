#include "pch.hpp"

void CG_Init()
{
    MH_STATUS state = MH_STATUS::MH_OK;

    if (state = MH_Initialize(), state != MH_STATUS::MH_OK) {
        return FatalError(MH_StatusToString(state));
    }

    hook::nop(0x592B9C);

    hook::write_addr(0x628948, "\xEB\x00\xBA\x20\x38", 5); //jnz -> jmp

    Cmd_AddCommand("cm_materialFilter", Cmd_MaterialFilter_f);

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
    Dvar_RegisterNew("cm_showCollisionDepthTest", dvar_type::boolean, dvar_flags::saved, "Select whether to use depth test in collision surfaces display", v, l);

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