#ifndef CS16_EV_FX_HELPER_H
#define CS16_EV_FX_HELPER_H

#include "hud.h"
#include "const.h"
#include "r_efx.h"
#include "event_api.h"

#ifndef CS16_FX_NO_BLACK_SQUARE
#define CS16_FX_NO_BLACK_SQUARE 1
#endif

struct tempent_s;
void EV_WallPuff_Wind( struct tempent_s *te, float frametime, float currenttime );
void EV_SmokeRise( struct tempent_s *te, float frametime, float currenttime );
void EV_CS16Client_KillEveryRound( struct tempent_s *te, float frametime, float currenttime );

namespace CS16Fx
{

inline void FxDefaultCallback( struct tempent_s *te, float frametime, float currenttime )
{
	EV_CS16Client_KillEveryRound( te, frametime, currenttime );
}

struct SpriteFxParams
{
	const char *sprite;
	Vector      origin;
	Vector      velocity;
	float       scale;
	int         r, g, b;
	int         alpha;
	float       framerate;
	float       life;
	int         flags;
	bool        wind;
	bool        rise;

	SpriteFxParams()
		: sprite( 0 ),
		  scale( 1.0f ),
		  r( 255 ), g( 255 ), b( 255 ), alpha( 200 ),
		  framerate( 30.0f ), life( 0.5f ),
		  flags( 0 ),
		  wind( false ), rise( false )
	{
		origin.x = origin.y = origin.z = 0.0f;
		velocity.x = velocity.y = velocity.z = 0.0f;
	}
};

inline TEMPENTITY *CreateSpriteFx( const SpriteFxParams &p )
{
	if( !p.sprite || !p.sprite[0] )
		return 0;

	int modelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( p.sprite );
	if( modelIndex <= 0 )
		return 0;

	Vector sprOrigin = p.origin;

	TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite(
		sprOrigin, modelIndex, p.framerate );

	if( !te )
		return 0;

#if CS16_FX_NO_BLACK_SQUARE
	te->entity.curstate.rendermode = kRenderTransAdd;
#else
	te->entity.curstate.rendermode = kRenderTransAlpha;
#endif

	te->entity.curstate.rendercolor.r = (unsigned char)p.r;
	te->entity.curstate.rendercolor.g = (unsigned char)p.g;
	te->entity.curstate.rendercolor.b = (unsigned char)p.b;
	te->entity.curstate.renderamt   = p.alpha;
	te->entity.curstate.scale       = p.scale;
	te->entity.baseline.origin      = p.velocity;

	int flags = p.flags;

	if( p.wind )
	{
		te->callback = EV_WallPuff_Wind;
		flags |= FTENT_CLIENTCUSTOM;
	}
	else if( p.rise )
	{
		te->callback = EV_SmokeRise;
		flags |= FTENT_CLIENTCUSTOM;
	}
	else if( flags & FTENT_CLIENTCUSTOM )
	{
		te->callback = FxDefaultCallback;
	}

	te->flags |= flags | FTENT_PERSIST;

	float life = ( p.life > 0.0f ) ? p.life : 0.5f;
	te->die = gEngfuncs.GetClientTime() + life;

	return te;
}

inline TEMPENTITY *MuzzleFlash( const Vector &origin, const Vector &dir,
	float scale = 0.5f )
{
	SpriteFxParams p;
	p.sprite    = "sprites/muzzleflash.spr";
	p.origin    = origin;
	p.velocity  = dir;
	p.scale     = scale;
	p.r = 255; p.g = 220; p.b = 120;
	p.alpha     = 240;
	p.framerate = 20.0f;
	p.life      = 0.08f;
	p.flags     = FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
	return CreateSpriteFx( p );
}

inline TEMPENTITY *HitSpark( const Vector &origin, const Vector &dir,
	float scale = 0.3f )
{
	SpriteFxParams p;
	p.sprite    = "sprites/spark1.spr";
	p.origin    = origin;
	p.velocity  = dir * 120.0f;
	p.scale     = scale;
	p.r = 255; p.g = 200; p.b = 90;
	p.alpha     = 220;
	p.framerate = 35.0f;
	p.life      = 0.25f;
	p.flags     = FTENT_COLLIDEWORLD | FTENT_GRAVITY;
	return CreateSpriteFx( p );
}

inline TEMPENTITY *HitDebris( const Vector &origin, const Vector &dir,
	float scale = 0.4f )
{
	SpriteFxParams p;
	p.sprite    = "sprites/debris1.spr";
	p.origin    = origin;
	p.velocity  = dir * 80.0f;
	p.scale     = scale;
	p.r = 90; p.g = 90; p.b = 90;
	p.alpha     = 170;
	p.framerate = 25.0f;
	p.life      = 0.6f;
	p.flags     = FTENT_COLLIDEWORLD | FTENT_GRAVITY;
	return CreateSpriteFx( p );
}

inline TEMPENTITY *SmokePuff( const char *spr, const Vector &origin,
	const Vector &dir, float scale, int gray, bool wind,
	float framerate = 25.0f, float life = 1.0f )
{
	SpriteFxParams p;
	p.sprite    = spr;
	p.origin    = origin;
	p.velocity  = dir;
	p.scale     = scale;
	p.r = gray; p.g = gray; p.b = gray;
	p.alpha     = 180;
	p.framerate = framerate;
	p.life      = life;
	p.wind      = wind;
	p.rise      = !wind;
	return CreateSpriteFx( p );
}

}

#endif
