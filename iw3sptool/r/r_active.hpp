#pragma once

void CG_DrawActive();

struct CGDebugData
{
	static volatile int tessVerts;
	static volatile int tessIndices;
};
