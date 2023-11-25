#pragma once
#include "pch.hpp"

union dvar_limits
{
	struct
	{
		int stringCount;
		const char** strings;
	} enumeration;

	struct
	{
		int min;
		int max;
	} integer;

	struct
	{
		float min;
		float max;
	} value;

	struct
	{
		float min;
		float max;
	} vector;
};

union dvar_value
{
	bool enabled;
	int integer;
	unsigned int unsignedInt;
	float value;
	float vector[4];
	const char* string;
	char color[4];
};

enum class dvar_type : std::int8_t
{
	boolean = 0,
	value = 1,
	vec2 = 2,
	vec3 = 3,
	vec4 = 4,
	integer = 5,
	enumeration = 6,
	string = 7,
	color = 8,
	rgb = 9 // Color without alpha
};

enum dvar_flags : std::uint16_t
{
	none = 0x0,
	saved = 0x1,
	user_info = 0x2, // sent to server on connect or change
	server_info = 0x4, // sent in response to front end requests
	replicated = 0x8,
	write_protected = 0x10,
	latched = 0x20,
	read_only = 0x40,
	cheat_protected = 0x80,
	temp = 0x100,
	no_restart = 0x400, // do not clear when a cvar_restart is issued
	user_created = 0x4000, // created by a set command
};

struct dvar_s
{
	const char* name;
	const char* description;
	dvar_flags flags;
	dvar_type type;
	bool modified;
	dvar_value current;
	dvar_value latched;
	dvar_value reset;
	dvar_limits domain;
	bool(__cdecl* domainFunc)(dvar_s*, dvar_value);
	dvar_s* hashNext;
};
dvar_s* Dvar_FindMalleableVar(const char* name);
dvar_s* Dvar_RegisterNew(const char* name, dvar_type type, int flags, const char* description, dvar_value defaultValue, dvar_limits domain);
dvar_s* Dvar_Reregister(dvar_s* dvar, const char* name, dvar_type type, int flags, const char* description, dvar_value defaultValue, dvar_limits domain);

void OnDvarsModified();
