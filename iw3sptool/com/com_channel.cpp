
#include "com_channel.hpp"

void Com_Printf($B87C0110D100A68234FECCEB9075A41E channel, const char* msg, ...)
{
	char v2[4096];
	va_list va; 

	va_start(va, msg);
	_vsnprintf_s(v2, 0x1000u, msg, va);
	v2[4095] = 0;

	return ((void(*)($B87C0110D100A68234FECCEB9075A41E channel, char* Format, int buf))0x532DB0)(channel, v2, 0);

}
void Com_Printf(const char* msg, ...)
{
	char v2[4096];
	va_list va;

	va_start(va, msg);
	_vsnprintf_s(v2, 0x1000u, msg, va);
	v2[4095] = 0;

	return ((void(*)($B87C0110D100A68234FECCEB9075A41E channel, char* Format, int buf))0x532DB0)(CON_CHANNEL_OBITUARY, v2, 0);
}
#pragma warning (disable : 4996)
void Com_PrintWarning($B87C0110D100A68234FECCEB9075A41E channel, const char* msg, ...)
{
	unsigned int v2; // kr00_4
	char v3[4095]; // [esp+0h] [ebp-1000h] BYREF
	char v4; // [esp+FFFh] [ebp-1h]
	va_list va; // [esp+100Ch] [ebp+Ch] BYREF

	va_start(va, msg);
	strncpy_s(v3, "^3", 0xFFFu);
	v4 = 0;
	v2 = strlen(v3);
	_vsnprintf(&v3[v2], 4096 - v2, msg, va);
	v4 = 0;


	return ((void(*)($B87C0110D100A68234FECCEB9075A41E channel, char* Format, int buf))0x532F30)(channel, v3, 2);

}
void Com_PrintError($B87C0110D100A68234FECCEB9075A41E channel, const char* msg, ...)
{
	unsigned int v2; // kr00_4
	char _buffer[4095]; // [esp+8h] [ebp-1000h] BYREF
	char v4; // [esp+1007h] [ebp-1h]
	va_list va; // [esp+1018h] [ebp+10h] BYREF

	va_start(va, msg);
	if (strstr("error", msg))
	{
		strncpy_s(_buffer, "^1", 0xFFFu);
	}
	else
	{
		strncpy_s(_buffer, "^1Error: ", 0xFFFu);
	}
	v4 = 0;
	v2 = strlen(_buffer);
	_vsnprintf(&_buffer[v2], 4096 - v2, msg, va);
	++*(DWORD*)0xF8CE44;
	v4 = 0;

	return ((void(*)($B87C0110D100A68234FECCEB9075A41E channel, char* Format, int buf))0x532E70)(channel, _buffer, 2);

}

void Com_Error(errorParm_t channel, const char* msg, ...)
{
	char v2[4096];
	va_list va;

	va_start(va, msg);
	_vsnprintf_s(v2, 0x1000u, msg, va);
	v2[4095] = 0;

	return ((void(*)(errorParm_t channel, char* Format))0x533580)(channel, v2);
}