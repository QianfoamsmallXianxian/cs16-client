#ifndef CS16_EV_MUZZLE_LIGHT_H
#define CS16_EV_MUZZLE_LIGHT_H

#include "cl_util.h"   // gEngfuncs
#include "const.h"     // byte
#include "r_efx.h"     // dlight_t, CL_AllocDlight

// ============================================================================
// Muzzle-flash dynamic light.
//
// Works for BOTH view modes:
//   1st person - the local player's own gun lights up the world around him
//   3rd person - other players' guns light up too, so when you spectate or
//                watch an enemy fire you see the flash
//
// Callers should invoke this once per volley from the shared bullet-firing
// routine, so every weapon that shoots gets a light from one place.
//
// Rate-limited: full-auto at 600 RPM emits ~10 events/sec, and a 40 ms gate
// keeps the engine dlight pool healthy without visible flicker. Remote
// shooters key their light on their own entity index, so several players
// firing at once each keep their own dlight slot.
//
// NOTE: GetClientTime() resets to 0 on map change, so the gate tolerates a
// backwards clock (now < last). A plain (now - last < 0.04f) test would latch
// off forever after the first level transition.
// ============================================================================
inline void EV_MuzzleLight( int idx, const float *muzzle, bool isLocal )
{
	static float s_lastLocal = -1.0f;

	float now = gEngfuncs.GetClientTime();

	if( isLocal )
	{
		if( now >= s_lastLocal && now - s_lastLocal < 0.04f )
			return;
		s_lastLocal = now;
	}

	dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( isLocal ? 0 : ( idx + 1 ) );
	if( !dl )
		return;

	dl->origin[0] = muzzle[0];
	dl->origin[1] = muzzle[1];
	dl->origin[2] = muzzle[2];
	dl->radius = isLocal ? 220.0f : 190.0f;
	dl->color.r = (byte)255;
	dl->color.g = (byte)215;
	dl->color.b = (byte)140;
	dl->decay = 500.0f;
	dl->minlight = 20.0f;
	dl->die = now + 0.06f;
}

#endif
