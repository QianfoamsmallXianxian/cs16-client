#ifndef CS16_EV_FX_PARTICLE_H
#define CS16_EV_FX_PARTICLE_H

#include "hud.h"
#include "const.h"

#ifndef CS16_FX_PARTICLE_NO_BLACK_SQUARE
#define CS16_FX_PARTICLE_NO_BLACK_SQUARE 1
#endif

namespace CS16Fx
{

inline int SafeParticleMode()
{
#if CS16_FX_PARTICLE_NO_BLACK_SQUARE
	return kRenderTransAdd;
#else
	return kRenderTransAlpha;
#endif
}

inline int SafeParticleModeOr( int requested )
{
#if CS16_FX_PARTICLE_NO_BLACK_SQUARE
	if( requested == kRenderTransAlpha )
		return kRenderTransAdd;
#endif
	return requested;
}

}

#endif
