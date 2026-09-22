#ifndef CS16_EV_BLOOD_FX_H
#define CS16_EV_BLOOD_FX_H

// ============================================================================
// Independent heavy blood-impact module.
//
// Goals:
//   * much MORE blood than the default impact fx
//   * ignores armour completely -- this client module never reads any
//     kevlar / armour cvar, so a hit always throws the full blood spray.
//     (Actual damage numbers are still computed server-side; this only
//     controls the visual, which is what the client owns.)
//
// All effects are model-free engine calls (R_Blood / R_RunParticleEffect),
// so nothing here can fail on a missing .spr or draw black squares.
// Sprite layers are additive-only and silently skip if unavailable.
// ============================================================================

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "r_efx.h"
#include "event_api.h"
#include "com_model.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "pm_materials.h"
#include <string.h>

#ifndef CS16_BLOOD_FX_ON
#define CS16_BLOOD_FX_ON 1
#endif

// how many engine blood puffs per hit
#ifndef CS16_BLOOD_PUFFS
#define CS16_BLOOD_PUFFS 6
#endif

// palette colour index for red blood (classic Half-Life value)
#ifndef CS16_BLOOD_COLOR_RED
#define CS16_BLOOD_COLOR_RED 247
#endif

namespace CS16Fx
{

// Resolve a blood sprite, cached per level.
inline int BloodFindSprite( const char *a, const char *b, const char *c )
{
	int idx = 0;

	if( a ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( a ); if( idx > 0 ) return idx; }
	if( b ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( b ); if( idx > 0 ) return idx; }
	if( c ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( c ); if( idx > 0 ) return idx; }

	return 0;
}

inline int BloodSpraySprite()
{
	static int  s_cache = -1;
	static char s_lastMap[128] = { 0 };

	const char *map = ( gEngfuncs.pfnGetLevelName ) ? gEngfuncs.pfnGetLevelName() : 0;

	if( map && map[0] && strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
	{
		strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
		s_lastMap[sizeof( s_lastMap ) - 1] = 0;
		s_cache = -1;
	}

	if( s_cache <= 0 )
		s_cache = BloodFindSprite( "sprites/bloodspray.spr", "sprites/blood.spr", "sprites/blood_impact.spr" );

	return s_cache;
}

inline int BloodDropSprite()
{
	static int  s_cache = -1;
	static char s_lastMap[128] = { 0 };

	const char *map = ( gEngfuncs.pfnGetLevelName ) ? gEngfuncs.pfnGetLevelName() : 0;

	if( map && map[0] && strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
	{
		strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
		s_lastMap[sizeof( s_lastMap ) - 1] = 0;
		s_cache = -1;
	}

	if( s_cache <= 0 )
		s_cache = BloodFindSprite( "sprites/blood.spr", "sprites/bloodspray.spr", 0 );

	return s_cache;
}

// ============================================================================
// Engine blood burst -- no sprite needed, always works.
// ============================================================================
inline void BloodEmitEngine( const Vector &pos, const Vector &normal )
{
	Vector dir;
	dir.x = normal.x;
	dir.y = normal.y;
	dir.z = normal.z;

	// R_Blood(org, dir, pcolor, speed): classic HL blood spray
	for( int i = 0; i < CS16_BLOOD_PUFFS; i++ )
	{
		Vector d;
		d.x = dir.x + gEngfuncs.pfnRandomFloat( -0.55f, 0.55f );
		d.y = dir.y + gEngfuncs.pfnRandomFloat( -0.55f, 0.55f );
		d.z = dir.z + gEngfuncs.pfnRandomFloat( -0.25f, 0.55f );

		gEngfuncs.pEfxAPI->R_Blood( (float *)&pos, (float *)&d,
			CS16_BLOOD_COLOR_RED, gEngfuncs.pfnRandomLong( 120, 260 ) );
	}

	// extra red specks that arc away
	gEngfuncs.pEfxAPI->R_RunParticleEffect( (float *)&pos, (float *)&dir, CS16_BLOOD_COLOR_RED, 24 );

	Vector up;
	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;
	gEngfuncs.pEfxAPI->R_RunParticleEffect( (float *)&pos, (float *)&up, CS16_BLOOD_COLOR_RED, 12 );
}

// ============================================================================
// Sprite blood cloud -- additive, skipped if the sprite is missing.
// ============================================================================
inline void BloodEmitSprites( const Vector &pos )
{
	int sprayIdx = BloodSpraySprite();
	int dropIdx  = BloodDropSprite();

	if( sprayIdx <= 0 && dropIdx <= 0 )
		return;

	const int puffs = 48;

	for( int i = 0; i < puffs; i++ )
	{
		int idx = ( ( i & 1 ) && dropIdx > 0 ) ? dropIdx : sprayIdx;
		if( idx <= 0 )
			continue;

		Vector spawn = pos;
		spawn.x += gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );
		spawn.y += gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );
		spawn.z += gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );

		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&spawn, idx, 20.0f + i * 1.5f );
		if( !te )
			continue;

		bool bright = ( ( i % 3 ) == 0 );

		te->entity.curstate.rendermode = kRenderTransAdd;

		if( bright )
		{
			te->entity.curstate.rendercolor.r = 240;
			te->entity.curstate.rendercolor.g = 45;
			te->entity.curstate.rendercolor.b = 45;
			te->entity.curstate.renderamt = 210;
			te->entity.curstate.scale = gEngfuncs.pfnRandomFloat( 0.30f, 0.70f );
		}
		else
		{
			te->entity.curstate.rendercolor.r = 135;
			te->entity.curstate.rendercolor.g = 12;
			te->entity.curstate.rendercolor.b = 12;
			te->entity.curstate.renderamt = 255;
			te->entity.curstate.scale = gEngfuncs.pfnRandomFloat( 0.50f, 1.15f );
		}

		Vector vel;
		vel.x = gEngfuncs.pfnRandomFloat( -190.0f, 190.0f );
		vel.y = gEngfuncs.pfnRandomFloat( -190.0f, 190.0f );
		vel.z = gEngfuncs.pfnRandomFloat( -15.0f, 165.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.45f, 1.20f );
	}
}

// ============================================================================
// Public entry: full heavy blood burst.
// Deliberately contains NO armour / kevlar check of any kind.
// ============================================================================
inline void ImpactEmitBloodHeavy( const Vector &pos, const Vector &normal )
{
#if CS16_BLOOD_FX_ON
	BloodEmitEngine( pos, normal );
	BloodEmitSprites( pos );
#endif
}

}

#endif
