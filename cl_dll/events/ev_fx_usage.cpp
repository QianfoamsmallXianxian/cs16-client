#include "ev_fx_no_black_square.h"

void CS16Fx_UsageMuzzle( const Vector &origin, const Vector &dir )
{
	CS16Fx::MuzzleFlash( origin, dir, 0.5f );
}

void CS16Fx_UsageHit( const Vector &endpos, const Vector &normal )
{
	CS16Fx::HitSpark( endpos, normal, 0.3f );
	CS16Fx::HitDebris( endpos, normal, 0.4f );
}

void CS16Fx_UsageSmoke( const Vector &pos, const Vector &dir )
{
	CS16Fx::SmokePuff( "sprites/black_smoke1.spr", pos, dir, 1.0f, 90, true );
}

TEMPENTITY *CS16Fx_UsageCustom( const Vector &origin )
{
	CS16Fx::SpriteFxParams p;
	p.sprite = "sprites/muzzleflash.spr";
	p.origin = origin;
	p.wind = true;
	p.rise = true;
	return CS16Fx::CreateSpriteFx( p );
}

int CS16Fx_UsageParticleMode()
{
	return CS16Fx::SafeParticleMode();
}

int CS16Fx_UsageParticleModeOr( int requested )
{
	return CS16Fx::SafeParticleModeOr( requested );
}
