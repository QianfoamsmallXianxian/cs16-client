#include "c4_fx_mod.h"
void __MsgFunc_Shake(const char *pszName, int iSize, void *pbuf) {
    BEGIN_READ(pbuf, iSize);
    float amplitude = READ_SHORT(); float duration = READ_SHORT(); float frequency = READ_SHORT();
    float scale = C4FX_GetShakeScale();
    if (scale < 0.0f) scale = 0.0f; if (scale > 5.0f) scale = 5.0f;
    amplitude *= scale;
    gEngfuncs.pfnV_CalcShake(); gEngfuncs.pfnV_ApplyShake(amplitude, duration, frequency);
}
