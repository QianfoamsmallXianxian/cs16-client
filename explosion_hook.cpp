#include "c4_fx_mod.h"
void EV_Explosion(struct event_args_s *args) {
    vec3_t pos; VectorCopy(args->origin, pos);
    float fire_scale = C4FX_GetFireScale(); float final_fire_scale = 2.5f * fire_scale;
    gEngfuncs.pEfxAPI->R_Explosion(pos, final_fire_scale, 1.0f, 0.0f, gEngfuncs.pEfxAPI->Draw_DecalIndex(gEngfuncs.pEfxAPI->DecalIndex("{expl1")));
    float smoke_scale = C4FX_GetSmokeScale(); int smoke_puffs = (int)(10.0f * smoke_scale); float smoke_size = 20.0f * smoke_scale;
    for (int i = 0; i < smoke_puffs; i++) { gEngfuncs.pEfxAPI->R_Smoke(pos, gEngfuncs.pEfxAPI->R_LoadSprite("sprites/smoke.spr"), smoke_size, 10.0f, 1); }
}
