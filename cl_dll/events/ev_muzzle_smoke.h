#ifndef CS16_EV_MUZZLE_SMOKE_H
#define CS16_EV_MUZZLE_SMOKE_H

#include "const.h"
#include "r_efx.h"
#include "event_api.h"
#include "com_model.h"
#include <math.h>
#include <string.h>

#ifndef CS16_MUZZLE_SMOKE_ON
#define CS16_MUZZLE_SMOKE_ON 1
#endif

#ifndef CS16_MUZZLE_SMOKE_PUFFS_FP
#define CS16_MUZZLE_SMOKE_PUFFS_FP 9
#endif

#ifndef CS16_MUZZLE_SMOKE_PUFFS_TP
#define CS16_MUZZLE_SMOKE_PUFFS_TP 6
#endif

#ifndef CS16_MUZZLE_SMOKE_GRAY_MIN
#define CS16_MUZZLE_SMOKE_GRAY_MIN 60
#endif

#ifndef CS16_MUZZLE_SMOKE_GRAY_MAX
#define CS16_MUZZLE_SMOKE_GRAY_MAX 150
#endif

#ifndef CS16_MUZZLE_SMOKE_DRIFT
#define CS16_MUZZLE_SMOKE_DRIFT 0.95f
#endif

#ifndef CS16_MUZZLE_SMOKE_RISE
#define CS16_MUZZLE_SMOKE_RISE 0.7f
#endif

#ifndef CS16_MUZZLE_SMOKE_FWD
#define CS16_MUZZLE_SMOKE_FWD 9.0f
#endif

struct tempent_s;
void EV_WallPuff_Wind( struct tempent_s *te, float frametime, float currenttime );
void EV_SmokeRise( struct tempent_s *te, float frametime, float currenttime );

extern Vector g_vPlayerVelocity;
extern vec3_t v_angles;

namespace CS16Fx
{

inline bool MuzzleSmokeIsExcluded( const char *name )
{
	if( !name || !name[0] )
		return false;

	static const char *excluded[] =
	{
		"smokegrenade", "flashbang", "hegrenade",
		"knife", "grenade", "c4",
		0
	};

	for( int i = 0; excluded[i]; i++ )
	{
		if( strstr( name, excluded[i] ) )
			return true;
	}

	return false;
}

inline bool MuzzleSmokeWeaponAllowed( const char *modelName )
{
	return !MuzzleSmokeIsExcluded( modelName );
}

inline int MuzzleSmokeGray( float t, float seed )
{
	int span = CS16_MUZZLE_SMOKE_GRAY_MAX - CS16_MUZZLE_SMOKE_GRAY_MIN + 1;
	int h = (int)( t * 613.0f ) ^ (int)( seed * 157.0f );
	if( h < 0 ) h = -h;
	return CS16_MUZZLE_SMOKE_GRAY_MIN + ( h % span );
}

inline void MuzzleSmokePuff( const Vector &origin, const Vector &forward,
	const Vector &velocity, float scale, int gray, float life,
	float framerate, bool wind )
{
	int modelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/black_smoke1.spr" );
	if( modelIndex <= 0 )
		return;

	Vector spawn = origin;

	TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( spawn, modelIndex, framerate );
	if( !te )
		return;

	te->entity.curstate.rendermode = kRenderTransAdd;
	te->entity.curstate.rendercolor.r = (unsigned char)gray;
	te->entity.curstate.rendercolor.g = (unsigned char)gray;
	te->entity.curstate.rendercolor.b = (unsigned char)gray;
	te->entity.curstate.renderamt = 165 + ( gray & 0x5F );
	te->entity.curstate.scale = scale;

	Vector drift;
	drift.x = forward.x * 13.0f + velocity.x * CS16_MUZZLE_SMOKE_DRIFT;
	drift.y = forward.y * 13.0f + velocity.y * CS16_MUZZLE_SMOKE_DRIFT;
	drift.z = forward.z * 5.0f + velocity.z * CS16_MUZZLE_SMOKE_DRIFT * 0.5f + CS16_MUZZLE_SMOKE_RISE;

	te->entity.baseline.origin = drift;

	te->flags |= FTENT_CLIENTCUSTOM | FTENT_COLLIDEWORLD | FTENT_PERSIST;

	if( wind )
		te->callback = EV_WallPuff_Wind;
	else
		te->callback = EV_SmokeRise;

	te->die = gEngfuncs.GetClientTime() + life;
}

inline void MuzzleSmokeAt( const Vector &muzzlePos, const Vector &forward,
	bool firstPerson, float scale )
{
#if CS16_MUZZLE_SMOKE_ON
	Vector vel = g_vPlayerVelocity;

	int puffs = firstPerson ? CS16_MUZZLE_SMOKE_PUFFS_FP : CS16_MUZZLE_SMOKE_PUFFS_TP;

	float baseScale = ( scale > 0.0f ) ? scale : 0.5f;

	float t = gEngfuncs.GetClientTime();

	float seed = muzzlePos.x * 0.13f + muzzlePos.y * 0.07f + vel.x * 0.05f + vel.y * 0.03f;

	for( int i = 0; i < puffs; i++ )
	{
		float step = ( puffs > 1 ) ? ( (float)i / (float)( puffs - 1 ) ) : 0.0f;

		int gray = MuzzleSmokeGray( t + step * 0.41f, seed + (float)i * 1.7f );

		float sc = baseScale * ( 0.45f + step * 0.95f );

		float life = 0.40f + step * 0.85f;

		float framerate = 20.0f + step * 12.0f;

		bool wind = ( i & 1 ) ? true : false;

		float spread = 0.30f + step * 0.55f;

		float sx = (float)( ( ( i * 37 ) % 11 ) - 5 ) * spread * 0.03f;
		float sy = (float)( ( ( i * 53 ) % 13 ) - 6 ) * spread * 0.03f;
		float sz = (float)( ( ( i * 29 ) % 7 ) - 3 ) * spread * 0.02f;

		Vector dir;
		dir.x = forward.x + sx;
		dir.y = forward.y + sy;
		dir.z = forward.z + sz;

		float dist = 2.0f + step * CS16_MUZZLE_SMOKE_FWD;

		Vector spawn = muzzlePos;
		spawn.x += dir.x * dist;
		spawn.y += dir.y * dist;
		spawn.z += dir.z * dist + step * 1.5f;

		MuzzleSmokePuff( spawn, dir, vel, sc, gray, life, framerate, wind );
	}
#endif
}

inline void MuzzleSmokeFromAngles( const Vector &muzzlePos, const Vector &angles,
	bool firstPerson, float scale )
{
#if CS16_MUZZLE_SMOKE_ON
	const float DEG2RAD = 3.14159265358979f / 180.0f;

	float pitch = angles.x * DEG2RAD;
	float yaw = angles.y * DEG2RAD;

	float cp = (float)cos( pitch );
	float sp = (float)sin( pitch );
	float cy = (float)cos( yaw );
	float sy = (float)sin( yaw );

	Vector forward;
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;

	MuzzleSmokeAt( muzzlePos, forward, firstPerson, scale );
#endif
}

inline void MuzzleSmokeEvent( struct cl_entity_s *entity, const float *attachment,
	bool firstPerson, float scale )
{
#if CS16_MUZZLE_SMOKE_ON
	if( !entity || !attachment )
		return;

	const char *modelName = ( entity->model ) ? entity->model->name : 0;

	if( !MuzzleSmokeWeaponAllowed( modelName ) )
		return;

	Vector muzzlePos;
	muzzlePos.x = attachment[0];
	muzzlePos.y = attachment[1];
	muzzlePos.z = attachment[2];

	Vector ang;
	if( firstPerson )
		ang = v_angles;
	else
		ang = entity->angles;

	MuzzleSmokeFromAngles( muzzlePos, ang, firstPerson, scale );
#endif
}

}

#endif
