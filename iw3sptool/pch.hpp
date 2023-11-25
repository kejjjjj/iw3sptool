// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define NOMINMAX
constexpr auto GAME_NAME = "Call of Duty 4";
constexpr auto MODULE_NAME = "iw3sp.exe";

#define PI 3.14159265f

#ifndef M_PI
#define M_PI        3.14159265358979323846f // matches value in gcc v2 math.h
#endif

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorAddAll(a,b,c)		((c)[0]=(a)[0]+(b),(c)[1]=(a)[1]+(b),(c)[2]=(a)[2]+(b))
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define VectorScaleAll(a,b,c)	((c)[0]=(a)[0]*(b),(c)[1]=(a)[1]*(b),(c)[2]=(a)[2]*(b))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
#define VectorClear( a )              ( ( a )[0] = ( a )[1] = ( a )[2] = 0 )

#define ANGLE2SHORT( x )  ( (int)( ( x ) * 65536 / 360 ) & 65535 )
#define SHORT2ANGLE( x )  ( ( x ) * ( 360.0f / 65536 ) )

#define DEG2RAD(a) (((a) * M_PI) / 180.0F)
#define RAD2DEG(a) (((a) * 180.0f) / M_PI)
#define RAD2SHORT(a) ((a) * (32768.f / (float)M_PI))
#define SHORT2RAD(a) ((a) * ((float)M_PI / 32768.f))
#define SHORT2DEG(a) (((a) / 32768.f) * 180.0f)

#define NOT_SERVER (*(int*)0x0797520 == 0)

#ifndef BUILD_DEFS_H
#define BUILD_DEFS_H


// Example of __DATE__ string: "Jul 27 2012"
//                              01234567890

#define BUILD_YEAR_CH0 (__DATE__[ 7])
#define BUILD_YEAR_CH1 (__DATE__[ 8])
#define BUILD_YEAR_CH2 (__DATE__[ 9])
#define BUILD_YEAR_CH3 (__DATE__[10])


#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')


#define BUILD_MONTH_CH0 \
    ((BUILD_MONTH_IS_OCT || BUILD_MONTH_IS_NOV || BUILD_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_MONTH_CH1 \
    ( \
        (BUILD_MONTH_IS_JAN) ? '1' : \
        (BUILD_MONTH_IS_FEB) ? '2' : \
        (BUILD_MONTH_IS_MAR) ? '3' : \
        (BUILD_MONTH_IS_APR) ? '4' : \
        (BUILD_MONTH_IS_MAY) ? '5' : \
        (BUILD_MONTH_IS_JUN) ? '6' : \
        (BUILD_MONTH_IS_JUL) ? '7' : \
        (BUILD_MONTH_IS_AUG) ? '8' : \
        (BUILD_MONTH_IS_SEP) ? '9' : \
        (BUILD_MONTH_IS_OCT) ? '0' : \
        (BUILD_MONTH_IS_NOV) ? '1' : \
        (BUILD_MONTH_IS_DEC) ? '2' : \
        /* error default */    '?' \
    )

#define BUILD_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DAY_CH1 (__DATE__[ 5])

#define BUILD_HOUR_CH0 (__TIME__[0])
#define BUILD_HOUR_CH1 (__TIME__[1])

#define BUILD_MIN_CH0 (__TIME__[3])
#define BUILD_MIN_CH1 (__TIME__[4])

#define BUILD_SEC_CH0 (__TIME__[6])
#define BUILD_SEC_CH1 (__TIME__[7])

#endif


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#pragma warning(disable : 26495)


#include <Windows.h>
#include <string>
#include <sstream>
#include <thread>
#include <optional>
#include <format>
#include <iostream>
#include <TlHelp32.h>
#include <Psapi.h>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <list>
#include <timeapi.h>
#include <filesystem>
#include <fstream>
#include <direct.h>
#include <type_traits>
#include <stdexcept>
#include <random> // for std::mt19937
#include <source_location>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#pragma warning (1007 : disable)

//DIRECTX
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9core.h>


#include "mh/MinHook.h"
#include "typedefs.hpp"

#include "errors.hpp"
#include "koukku.hpp"

#include "cg_local.hpp"
#include "cg_offsets.hpp"

#include "dvar.hpp"


#include "cg_init.hpp"
#include "cg_hooks.hpp"

#include "cl_main.hpp"


#include "r_active.hpp"
#include "r_drawtools.hpp"

#include "rb_endscene.hpp"

#include "cm_brush.hpp"
#include "cm_terrain.hpp"

#include "e_utilities.hpp"


#include "com_channel.hpp"
#include "com_vector.hpp"

#include "g_entity.hpp"


#include "sys_tools.hpp"



using namespace std::chrono_literals;


#endif