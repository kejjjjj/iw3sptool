#pragma once

#include "pch.hpp"

void CG_AdjustFrom640(float& x, float& y, float& w, float& h);

Material* R_RegisterMaterial(const std::string& mtl);
Font_s* R_RegisterFont(const std::string& fontname);

char* __cdecl R_AddCmdDrawText(const char* text, int maxChars, Font_s* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style);
void R_DrawText(const std::string& text, const char* fontname, float x, float y, float xScale, float yScale, float rotation, vec4_t color, int style);

void R_AddCmdDrawTextWithEffects(char* text, Font_s* font, float x, float y, float xScale, float yScale, float rotation, float* color,
	int style, float* glowColor, Material* fxMaterial, Material* fxMaterialGlow, int fxBirthTime,
	int fxLetterTime, int fxDecayStartTime, int fxDecayDuration); //holy this takes a lot of args

void R_DrawTextWithEffects(const std::string& text, const char* fontname, float x, float y, float xScale, float yScale, float rotation, float* color, int style, float* glowColor);

void R_AddCmdDrawStretchPic(Material* material, float x, float y, float w, float h, float s0, float t0, float s1, float t1, const float* color);
void R_DrawRect(const char* material, float x, float y, float w, float h, const float* color);

void CL_AddDebugString(float* xyz, float* color, float scale, char* text, int duration);