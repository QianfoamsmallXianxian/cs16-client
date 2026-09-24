#pragma once
#include "mock_sdk/hud.h"
#include "mock_sdk/cl_dll.h"
extern cvar_t *cl_c4_shake_scale;
extern cvar_t *cl_c4_smoke_scale;
extern cvar_t *cl_c4_fire_scale;
void C4FX_Init();
void C4FX_Reset();
float C4FX_GetShakeScale();
float C4FX_GetSmokeScale();
float C4FX_GetFireScale();
