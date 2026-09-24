#pragma once
#include "hud.h"
struct efx_api_s { void (*R_Explosion)(float*, float, float, float, int); int (*Draw_DecalIndex)(int); int (*DecalIndex)(const char*); int (*R_LoadSprite)(const char*); void (*R_Smoke)(float*, int, float, float, int); };
struct enginefuncs_s { cvar_t* (*pfnRegisterVariable)(const char*, const char*, int); void (*Con_Printf)(const char*, ...); void (*pfnV_CalcShake)(); void (*pfnV_ApplyShake)(float, float, float); efx_api_s* pEfxAPI; };
extern enginefuncs_s gEngfuncs;
#define BEGIN_READ(a,b)
#define READ_SHORT() 0
