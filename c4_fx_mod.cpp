#include "c4_fx_mod.h"
cvar_t *cl_c4_shake_scale = nullptr;
cvar_t *cl_c4_smoke_scale = nullptr;
cvar_t *cl_c4_fire_scale = nullptr;
void C4FX_Init() {
    if (!cl_c4_shake_scale) cl_c4_shake_scale = gEngfuncs.pfnRegisterVariable("cl_c4_shake_scale", "1.0", FCVAR_ARCHIVE);
    if (!cl_c4_smoke_scale) cl_c4_smoke_scale = gEngfuncs.pfnRegisterVariable("cl_c4_smoke_scale", "1.0", FCVAR_ARCHIVE);
    if (!cl_c4_fire_scale) cl_c4_fire_scale = gEngfuncs.pfnRegisterVariable("cl_c4_fire_scale", "1.0", FCVAR_ARCHIVE);
}
void C4FX_Reset() { gEngfuncs.Con_Printf("[C4FX] Loaded.\n"); }
float C4FX_GetShakeScale() { return cl_c4_shake_scale ? cl_c4_shake_scale->value : 1.0f; }
float C4FX_GetSmokeScale() { return cl_c4_smoke_scale ? cl_c4_smoke_scale->value : 1.0f; }
float C4FX_GetFireScale() { return cl_c4_fire_scale ? cl_c4_fire_scale->value : 1.0f; }
