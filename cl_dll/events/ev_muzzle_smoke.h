#ifndef CS16_EV_MUZZLE_SMOKE_H
#define CS16_EV_MUZZLE_SMOKE_H

#include "hud.h"
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

#ifndef CS16_MUZZLE_SMOKE_PUFFS_DUAL
#define CS16_MUZZLE_SMOKE_PUFFS_DUAL 6
#endif

#ifndef CS16_MUZZLE_SMOKE_GRAY_MIN
#define CS16_MUZZLE_SMOKE_GRAY_MIN 40
#endif

#ifndef CS16_MUZZLE_SMOKE_GRAY_MAX
#define CS16_MUZZLE_SMOKE_GRAY_MAX 90
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

#ifndef CS16_MUZZLE_SMOKE_DEDUP
#define CS16_MUZZLE_SMOKE_DEDUP 0.001f
#endif

#ifndef CS16_MUZZLE_SMOKE_MAX_PUFFS
#define CS16_MUZZLE_SMOKE_MAX_PUFFS 16
#endif

struct tempent_s;
void EV_WallPuff_Wind( struct tempent_s *te, float frametime, float currenttime );
void EV_SmokeRise( struct tempent_s *te, float frametime, float currenttime );

extern Vector g_vPlayerVelocity;
extern vec3_t v_angles;

namespace CS16Fx
{

inline unsigned int MuzzleSmokeHash( const char *s )
{
	unsigned int h = 2166136261u;

	if( !s )
		return 0;

	while( *s )
	{
		h ^= (unsigned char)( *s++ );
		h *= 16777619u;
	}

	return h;
}

inline bool MuzzleSmokeContains( const char *s, const char *sub )
{
	if( !s || !sub )
		return false;

	size_t n = strlen( sub );

	if( !n )
		return false;

	for( const char *p = s; *p; p++ )
	{
		if( *p == *sub && !strncmp( p, sub, n ) )
			return true;
	}

	return false;
}

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
		if( MuzzleSmokeContains( name, excluded[i] ) )
			return true;
	}

	return false;
}

inline bool MuzzleSmokeIsDualWeapon( const char *name )
{
	if( !name || !name[0] )
		return false;

	static const char *dual[] =
	{
		"elite", "dual", "beretta", "96g", "dualberetta", "dualpistol",
		0
	};

	for( int i = 0; dual[i]; i++ )
	{
		if( MuzzleSmokeContains( name, dual[i] ) )
			return true;
	}

	return false;
}

inline unsigned int MuzzleSmokeWeaponClass( const char *name )
{
	static unsigned int s_lastHash = 0xFFFFFFFFu;
	static int          s_lastClass = 0;

	if( !name || !name[0] )
		return 0;

	unsigned int h = MuzzleSmokeHash( name );

	if( h == s_lastHash )
		return (unsigned int)s_lastClass;

	int cls = 0;

	if( !MuzzleSmokeIsExcluded( name ) )
	{
		cls = MuzzleSmokeIsDualWeapon( name ) ? 2 : 1;
	}

	s_lastHash = h;
	s_lastClass = cls;

	return (unsigned int)cls;
}

inline bool MuzzleSmokeWeaponAllowed( const char *modelName )
{
	return MuzzleSmokeWeaponClass( modelName ) != 0;
}

inline bool MuzzleSmokeLeftHanded()
{
	if( gHUD.cl_righthand )
		return ( gHUD.cl_righthand->value == 0.0f );

	return false;
}

inline int MuzzleSmokeGray( float t, float seed )
{
	int span = CS16_MUZZLE_SMOKE_GRAY_MAX - CS16_MUZZLE_SMOKE_GRAY_MIN + 1;
	int h = (int)( t * 613.0f ) ^ (int)( seed * 157.0f );

	h = h & 0x7FFFFFFF;

	return CS16_MUZZLE_SMOKE_GRAY_MIN + ( h % span );
}

inline int MuzzleSmokeSpriteIndex()
{
	static int   s_modelIndex = -1;
	static float s_lastTime   = -1.0f;
	static char  s_lastMap[128] = { 0 };

	const char *map = 0;

	if( gEngfuncs.pfnGetLevelName )
		map = gEngfuncs.pfnGetLevelName();

	if( map && map[0] )
	{
		if( strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
		{
			strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
			s_lastMap[sizeof( s_lastMap ) - 1] = 0;
			s_modelIndex = -1;
		}
	}

	float now = gEngfuncs.GetClientTime();

	if( now < s_lastTime )
		s_modelIndex = -1;

	s_lastTime = now;

	if( s_modelIndex <= 0 )
		s_modelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/black_smoke1.spr" );

	return s_modelIndex;
}

inline bool MuzzleSmokeDedup( int entindex, int attach, float t )
{
	static int   s_lastEnt[4]  = { -1, -1, -1, -1 };
	static float s_lastTime[4] = { -100.0f, -100.0f, -100.0f, -100.0f };

	if( attach < 0 || attach > 3 )
		return false;

	if( s_lastEnt[attach] == entindex && ( t - s_lastTime[attach] ) < CS16_MUZZLE_SMOKE_DEDUP )
		return true;

	s_lastEnt[attach] = entindex;
	s_lastTime[attach] = t;

	return false;
}

inline void MuzzleSmokePuff( const Vector &origin, const Vector &forward,
	const Vector &velocity, float scale, int gray, float life,
	float framerate, bool wind )
{
	int modelIndex = MuzzleSmokeSpriteIndex();

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
	te->entity.curstate.renderamt = 55 + ( gray & 0x3F );
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

inline void MuzzleSmokeEmit( const Vector &muzzlePos, const Vector &forward,
	const Vector &velocity, float scale, int puffs, bool mirror )
{
#if CS16_MUZZLE_SMOKE_ON
	if( puffs > CS16_MUZZLE_SMOKE_MAX_PUFFS )
		puffs = CS16_MUZZLE_SMOKE_MAX_PUFFS;

	if( puffs < 1 )
		return;

	float baseScale = ( scale > 0.0f ) ? scale : 0.5f;

	float t = gEngfuncs.GetClientTime();

	float seed = muzzlePos.x * 0.13f + muzzlePos.y * 0.07f + velocity.x * 0.05f + velocity.y * 0.03f;

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

		if( mirror )
			sx = -sx;

		Vector dir;
		dir.x = forward.x + sx;
		dir.y = forward.y + sy;
		dir.z = forward.z + sz;

		float dist = 2.0f + step * CS16_MUZZLE_SMOKE_FWD;

		Vector spawn = muzzlePos;
		spawn.x += dir.x * dist;
		spawn.y += dir.y * dist;
		spawn.z += dir.z * dist + step * 1.5f;

		MuzzleSmokePuff( spawn, dir, velocity, sc, gray, life, framerate, wind );
	}
#endif
}

inline void MuzzleSmokeFromAngles( const Vector &muzzlePos, const Vector &angles,
	const Vector &velocity, float scale, int puffs, bool mirror )
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

	MuzzleSmokeEmit( muzzlePos, forward, velocity, scale, puffs, mirror );
#endif
}

inline void MuzzleSmokeEvent( struct cl_entity_s *entity, const float *attachment,
	bool firstPerson, float scale, int attachIndex )
{
#if CS16_MUZZLE_SMOKE_ON
	if( !entity || !attachment )
		return;

	if( attachIndex < 0 || attachIndex > 3 )
		return;

	const char *modelName = ( entity->model ) ? entity->model->name : 0;

	unsigned int cls = MuzzleSmokeWeaponClass( modelName );

	if( cls == 0 )
		return;

	float t = gEngfuncs.GetClientTime();

	if( MuzzleSmokeDedup( entity->index, attachIndex, t ) )
		return;

	Vector ang;

	if( firstPerson )
		ang = v_angles;
	else
		ang = entity->angles;

	bool leftHanded = MuzzleSmokeLeftHanded();
	bool dual = ( cls == 2 );

	bool mirror = leftHanded;

	if( dual && attachIndex == 1 )
		mirror = !mirror;

	Vector muzzlePos;
	muzzlePos.x = attachment[0];
	muzzlePos.y = attachment[1];
	muzzlePos.z = attachment[2];

	int puffs;

	if( dual )
		puffs = CS16_MUZZLE_SMOKE_PUFFS_DUAL;
	else
		puffs = firstPerson ? CS16_MUZZLE_SMOKE_PUFFS_FP : CS16_MUZZLE_SMOKE_PUFFS_TP;

	MuzzleSmokeFromAngles( muzzlePos, ang, g_vPlayerVelocity, scale, puffs, mirror );
#endif
}

}

#endif
