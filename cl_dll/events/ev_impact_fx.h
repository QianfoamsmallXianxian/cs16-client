#ifndef CS16_EV_IMPACT_FX_H
#define CS16_EV_IMPACT_FX_H

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "r_efx.h"
#include "event_api.h"
#include "com_model.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "pm_materials.h"
#include <math.h>
#include <string.h>

#ifndef CS16_IMPACT_FX_ON
#define CS16_IMPACT_FX_ON 1
#endif

#ifndef CS16_IMPACT_MAX_PARTICLES
#define CS16_IMPACT_MAX_PARTICLES 48
#endif

#ifndef CS16_IMPACT_MAX_BLOOD
#define CS16_IMPACT_MAX_BLOOD 32
#endif

namespace CS16Fx
{

struct ImpactFxParams
{
	int   sparkCount;
	float sparkSpeed;
	int   sparkR, sparkG, sparkB;
	float sparkScale;
	int   debrisCount;
	int   debrisR, debrisG, debrisB;
	float debrisScale;
	bool  debrisRotate;
};

inline int ImpactFindSprite( const char *a, const char *b, const char *c )
{
	int idx = 0;

	if( a ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( a ); if( idx > 0 ) return idx; }
	if( b ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( b ); if( idx > 0 ) return idx; }
	if( c ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( c ); if( idx > 0 ) return idx; }

	return 0;
}

inline int ImpactTexSlot( char tex )
{
	switch( tex )
	{
	case CHAR_TEX_METAL:    return 0;
	case CHAR_TEX_CONCRETE: return 1;
	case CHAR_TEX_DIRT:     return 2;
	case CHAR_TEX_VENT:     return 3;
	case CHAR_TEX_GRATE:    return 4;
	case CHAR_TEX_TILE:     return 5;
	case CHAR_TEX_SLOSH:    return 6;
	case CHAR_TEX_WOOD:     return 7;
	case CHAR_TEX_COMPUTER: return 8;
	case CHAR_TEX_GRASS:    return 9;
	case CHAR_TEX_GLASS:    return 10;
	case CHAR_TEX_SNOW:     return 11;
	case CHAR_TEX_FLESH:    return 12;
	default:                return 1;
	}
}

inline const char *ImpactDebrisNameA( char tex )
{
	switch( tex )
	{
	case CHAR_TEX_METAL:    return "sprites/metal1.spr";
	case CHAR_TEX_WOOD:     return "sprites/wood1.spr";
	case CHAR_TEX_GLASS:    return "sprites/glass1.spr";
	case CHAR_TEX_GRASS:    return "sprites/grass1.spr";
	case CHAR_TEX_SNOW:     return "sprites/snow1.spr";
	case CHAR_TEX_SLOSH:    return "sprites/slosh1.spr";
	default:                return "sprites/debris1.spr";
	}
}

inline int ImpactDebrisSprite( char tex )
{
	static int  s_cache[16];
	static bool s_ready = false;
	static char s_lastMap[128] = { 0 };

	const char *map = ( gEngfuncs.pfnGetLevelName ) ? gEngfuncs.pfnGetLevelName() : 0;

	if( map && map[0] && strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
	{
		strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
		s_lastMap[sizeof( s_lastMap ) - 1] = 0;
		s_ready = false;
	}

	if( !s_ready )
	{
		for( int i = 0; i < 16; i++ )
			s_cache[i] = 0;
		s_ready = true;
	}

	int slot = ImpactTexSlot( tex );

	if( slot < 0 || slot > 15 )
		slot = 1;

	if( s_cache[slot] > 0 )
		return s_cache[slot];

	int idx = ImpactFindSprite( ImpactDebrisNameA( tex ), "sprites/debris1.spr", "sprites/black_smoke1.spr" );

	s_cache[slot] = idx;

	return idx;
}

inline int ImpactSparkSprite()
{
	static int s_cache = -1;
	static char s_lastMap[128] = { 0 };

	const char *map = ( gEngfuncs.pfnGetLevelName ) ? gEngfuncs.pfnGetLevelName() : 0;

	if( map && map[0] && strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
	{
		strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
		s_lastMap[sizeof( s_lastMap ) - 1] = 0;
		s_cache = -1;
	}

	if( s_cache <= 0 )
		s_cache = ImpactFindSprite( "sprites/spark1.spr", "sprites/muzzleflash.spr", "sprites/black_smoke1.spr" );

	return s_cache;
}

inline int ImpactBloodSprite()
{
	static int s_cache = -1;
	static char s_lastMap[128] = { 0 };

	const char *map = ( gEngfuncs.pfnGetLevelName ) ? gEngfuncs.pfnGetLevelName() : 0;

	if( map && map[0] && strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
	{
		strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
		s_lastMap[sizeof( s_lastMap ) - 1] = 0;
		s_cache = -1;
	}

	if( s_cache <= 0 )
		s_cache = ImpactFindSprite( "sprites/bloodspray.spr", "sprites/blood.spr", "sprites/black_smoke1.spr" );

	return s_cache;
}

inline int ImpactSmokeSprite()
{
	static int s_cache = -1;
	static char s_lastMap[128] = { 0 };

	const char *map = ( gEngfuncs.pfnGetLevelName ) ? gEngfuncs.pfnGetLevelName() : 0;

	if( map && map[0] && strncmp( s_lastMap, map, sizeof( s_lastMap ) - 1 ) )
	{
		strncpy( s_lastMap, map, sizeof( s_lastMap ) - 1 );
		s_lastMap[sizeof( s_lastMap ) - 1] = 0;
		s_cache = -1;
	}

	if( s_cache <= 0 )
		s_cache = ImpactFindSprite( "sprites/gas_puff_01.spr", "sprites/black_smoke1.spr", 0 );

	return s_cache;
}

inline const ImpactFxParams *ImpactMaterial( char tex )
{
	static const ImpactFxParams concrete = { 18, 1.30f, 255, 245, 210, 0.34f, 16, 155, 152, 144, 0.60f, true };
	static const ImpactFxParams metal    = { 26, 1.75f, 255, 235, 150, 0.32f, 14, 125, 125, 140, 0.58f, true };
	static const ImpactFxParams dirt     = {  0, 1.00f,   0,   0,   0, 0.00f, 18, 145, 115,  72, 0.68f, false };
	static const ImpactFxParams vent     = { 12, 1.20f, 240, 230, 200, 0.30f, 14, 145, 145, 145, 0.58f, true };
	static const ImpactFxParams grate    = { 16, 1.40f, 250, 240, 200, 0.32f, 14, 135, 135, 135, 0.58f, true };
	static const ImpactFxParams tile     = { 14, 1.25f, 250, 245, 230, 0.32f, 16, 215, 215, 205, 0.58f, true };
	static const ImpactFxParams slosh    = {  0, 1.00f,   0,   0,   0, 0.00f, 16, 105, 155, 215, 0.62f, false };
	static const ImpactFxParams wood     = {  0, 1.00f,   0,   0,   0, 0.00f, 18, 165, 108,  52, 0.68f, true };
	static const ImpactFxParams computer = { 24, 1.55f, 255, 240, 170, 0.32f, 16,  95,  95, 100, 0.58f, true };
	static const ImpactFxParams grass    = {  0, 1.00f,   0,   0,   0, 0.00f, 16, 108, 165,  72, 0.62f, false };
	static const ImpactFxParams glass    = { 18, 1.45f, 255, 255, 255, 0.30f, 20, 220, 238, 248, 0.50f, true };
	static const ImpactFxParams snow     = {  0, 1.00f,   0,   0,   0, 0.00f, 16, 242, 242, 250, 0.58f, false };
	static const ImpactFxParams flesh    = {  0, 1.00f,   0,   0,   0, 0.00f, 18, 175,  22,  22, 0.58f, false };

	switch( tex )
	{
	case CHAR_TEX_METAL:    return &metal;
	case CHAR_TEX_CONCRETE: return &concrete;
	case CHAR_TEX_DIRT:     return &dirt;
	case CHAR_TEX_VENT:     return &vent;
	case CHAR_TEX_GRATE:    return &grate;
	case CHAR_TEX_TILE:     return &tile;
	case CHAR_TEX_SLOSH:    return &slosh;
	case CHAR_TEX_WOOD:     return &wood;
	case CHAR_TEX_COMPUTER: return &computer;
	case CHAR_TEX_GRASS:    return &grass;
	case CHAR_TEX_GLASS:    return &glass;
	case CHAR_TEX_SNOW:     return &snow;
	case CHAR_TEX_FLESH:    return &flesh;
	default:                return &concrete;
	}
}

inline void ImpactEmitSmoke( const Vector &pos, const Vector &normal, const ImpactFxParams *fx, char tex )
{
	if( !fx )
		return;

	if( tex == CHAR_TEX_GLASS || tex == CHAR_TEX_GRASS || tex == CHAR_TEX_SNOW )
		return;

	int spriteIdx = ImpactSmokeSprite();
	if( spriteIdx <= 0 )
		return;

	int gray = 90 + ( ImpactTexSlot( tex ) * 11 ) % 60;

	int puffs = ( tex == CHAR_TEX_METAL || tex == CHAR_TEX_CONCRETE ) ? 4 : 3;

	for( int i = 0; i < puffs; i++ )
	{
		Vector spawn = pos;
		spawn.x += normal.x * 2.0f + gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );
		spawn.y += normal.y * 2.0f + gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );
		spawn.z += normal.z * 2.0f + gEngfuncs.pfnRandomFloat( -6.0f, 6.0f );

		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&spawn, spriteIdx, 18.0f + i * 6.0f );
		if( !te )
			continue;

		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.rendercolor.r = (unsigned char)gray;
		te->entity.curstate.rendercolor.g = (unsigned char)gray;
		te->entity.curstate.rendercolor.b = (unsigned char)gray;
		te->entity.curstate.renderamt = 90 + i * 15;
		te->entity.curstate.scale = fx->debrisScale * gEngfuncs.pfnRandomFloat( 0.9f, 1.9f );

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 8.0f, 26.0f ) + gEngfuncs.pfnRandomFloat( -18.0f, 18.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 8.0f, 26.0f ) + gEngfuncs.pfnRandomFloat( -18.0f, 18.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 8.0f, 26.0f ) + gEngfuncs.pfnRandomFloat( 4.0f, 26.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.35f, 0.85f );
	}
}

inline void ImpactEmitSparks( const Vector &pos, const Vector &normal, const ImpactFxParams *fx )
{
	if( !fx || fx->sparkCount <= 0 )
		return;

	int count = fx->sparkCount;
	if( count > CS16_IMPACT_MAX_PARTICLES )
		count = CS16_IMPACT_MAX_PARTICLES;

	Vector streakPos = pos;

	Vector dir;
	dir.x = normal.x * normal.x * gEngfuncs.pfnRandomFloat( 4.0f, 12.0f );
	dir.y = normal.y * normal.y * gEngfuncs.pfnRandomFloat( 4.0f, 12.0f );
	dir.z = normal.z * normal.z * gEngfuncs.pfnRandomFloat( 4.0f, 12.0f );

	gEngfuncs.pEfxAPI->R_StreakSplash( (float *)&streakPos, (float *)&dir, 4, count, dir.z * fx->sparkSpeed, -110.0f, 110.0f );

	int sparkIdx = ImpactSparkSprite();
	if( sparkIdx <= 0 )
		return;

	int bursts = count / 3;
	if( bursts < 3 ) bursts = 3;
	if( bursts > 12 ) bursts = 12;

	for( int i = 0; i < bursts; i++ )
	{
		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&streakPos, sparkIdx, 30.0f + i * 4.0f );
		if( !te )
			continue;

		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.rendercolor.r = (unsigned char)fx->sparkR;
		te->entity.curstate.rendercolor.g = (unsigned char)fx->sparkG;
		te->entity.curstate.rendercolor.b = (unsigned char)fx->sparkB;
		te->entity.curstate.renderamt = 235;
		te->entity.curstate.scale = fx->sparkScale * gEngfuncs.pfnRandomFloat( 0.6f, 1.7f );

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 50.0f, 130.0f ) + gEngfuncs.pfnRandomFloat( -140.0f, 140.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 50.0f, 130.0f ) + gEngfuncs.pfnRandomFloat( -140.0f, 140.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 50.0f, 130.0f ) + gEngfuncs.pfnRandomFloat( -35.0f, 155.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.22f, 0.60f );
	}
}

inline void ImpactEmitDebris( const Vector &pos, const Vector &normal, const ImpactFxParams *fx, char tex )
{
	if( !fx || fx->debrisCount <= 0 )
		return;

	int spriteIdx = ImpactDebrisSprite( tex );
	if( spriteIdx <= 0 )
		return;

	int count = fx->debrisCount;
	if( count > CS16_IMPACT_MAX_PARTICLES )
		count = CS16_IMPACT_MAX_PARTICLES;

	for( int i = 0; i < count; i++ )
	{
		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&pos, spriteIdx, 18.0f + i * 2.0f );
		if( !te )
			continue;

		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.rendercolor.r = (unsigned char)fx->debrisR;
		te->entity.curstate.rendercolor.g = (unsigned char)fx->debrisG;
		te->entity.curstate.rendercolor.b = (unsigned char)fx->debrisB;
		te->entity.curstate.renderamt = 255;
		te->entity.curstate.scale = fx->debrisScale * gEngfuncs.pfnRandomFloat( 0.55f, 1.5f );

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 30.0f, 95.0f ) + gEngfuncs.pfnRandomFloat( -160.0f, 160.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 30.0f, 95.0f ) + gEngfuncs.pfnRandomFloat( -160.0f, 160.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 30.0f, 95.0f ) + gEngfuncs.pfnRandomFloat( 15.0f, 180.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;

		if( fx->debrisRotate )
		{
			te->entity.angles[0] = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
			te->entity.angles[1] = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
			te->entity.angles[2] = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
			te->entity.baseline.angles[0] = gEngfuncs.pfnRandomFloat( -220.0f, 220.0f );
			te->entity.baseline.angles[1] = gEngfuncs.pfnRandomFloat( -220.0f, 220.0f );
			te->entity.baseline.angles[2] = gEngfuncs.pfnRandomFloat( -220.0f, 220.0f );
			te->flags |= FTENT_ROTATE;
		}

		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.8f, 1.9f );
	}
}

inline void ImpactEmitBlood( const Vector &pos, const Vector &normal )
{
	int spriteIdx = ImpactBloodSprite();
	if( spriteIdx <= 0 )
		return;

	int puffs = 30;
	if( puffs > CS16_IMPACT_MAX_BLOOD )
		puffs = CS16_IMPACT_MAX_BLOOD;

	for( int i = 0; i < puffs; i++ )
	{
		Vector spawn = pos;
		spawn.x += gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
		spawn.y += gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );
		spawn.z += gEngfuncs.pfnRandomFloat( -5.0f, 5.0f );

		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&spawn, spriteIdx, 20.0f + i * 2.0f );
		if( !te )
			continue;

		bool bright = ( ( i & 1 ) != 0 );

		if( bright )
		{
			te->entity.curstate.rendermode = kRenderTransAdd;
			te->entity.curstate.rendercolor.r = 240;
			te->entity.curstate.rendercolor.g = 45;
			te->entity.curstate.rendercolor.b = 45;
			te->entity.curstate.renderamt = 210;
			te->entity.curstate.scale = gEngfuncs.pfnRandomFloat( 0.30f, 0.72f );
		}
		else
		{
			te->entity.curstate.rendermode = kRenderTransAdd;
			te->entity.curstate.rendercolor.r = 135;
			te->entity.curstate.rendercolor.g = 12;
			te->entity.curstate.rendercolor.b = 12;
			te->entity.curstate.renderamt = 255;
			te->entity.curstate.scale = gEngfuncs.pfnRandomFloat( 0.50f, 1.15f );
		}

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 25.0f, 85.0f ) + gEngfuncs.pfnRandomFloat( -190.0f, 190.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 25.0f, 85.0f ) + gEngfuncs.pfnRandomFloat( -190.0f, 190.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 25.0f, 85.0f ) + gEngfuncs.pfnRandomFloat( -15.0f, 165.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.45f, 1.20f );
	}

	Vector streakPos = pos;
	Vector streakDir;
	streakDir.x = normal.x;
	streakDir.y = normal.y;
	streakDir.z = normal.z + 0.25f;

	gEngfuncs.pEfxAPI->R_StreakSplash( (float *)&streakPos, (float *)&streakDir, 4, 12, 80.0f, -85.0f, 85.0f );
}

inline bool ImpactIsEnemy( int hitEntity )
{
	if( hitEntity <= 0 || hitEntity > MAX_PLAYERS )
		return true;

	cl_entity_t *local = gEngfuncs.GetLocalPlayer();
	if( !local )
		return true;

	int localIdx = local->index;

	if( localIdx < 0 || localIdx > MAX_PLAYERS )
		return true;

	int localTeam = g_PlayerExtraInfo[localIdx].teamnumber;
	int hitTeam   = g_PlayerExtraInfo[hitEntity].teamnumber;

	if( localTeam == 0 || hitTeam == 0 )
		return true;

	if( localTeam != hitTeam )
		return true;

	if( CVAR_GET_FLOAT( "mp_friendlyfire" ) != 0.0f )
		return true;

	return false;
}



// ============================================================================
// Engine-builtin impact FX: sparks + classic dot debris + fire dlight.
// Model-free only: never fails on missing .spr, never makes black squares.
//
// Debris uses R_RunParticleEffect (classic single-PIXEL particles, the tiny
// "dots" CS players expect), NOT R_StreakSplash (which draws long streaks).
// ============================================================================
inline void ImpactEmitEngineFx( const Vector &pos, const Vector &normal, char tex )
{
	Vector dir;
	dir.x = normal.x;
	dir.y = normal.y;
	dir.z = normal.z;

	Vector up;
	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;

	// ---- sparks: shower + dense spray + SHORT streaks ----
	gEngfuncs.pEfxAPI->R_SparkShower( (float *)&pos );
	gEngfuncs.pEfxAPI->R_SparkEffect( (float *)&pos, 40, 80, 260 );
	gEngfuncs.pEfxAPI->R_SparkEffect( (float *)&pos, 22, 50, 160 );
	// low velocity => short streaks (was 280, which drew long lines)
	gEngfuncs.pEfxAPI->R_SparkStreaks( (float *)&pos, 24, 60, 140 );
	gEngfuncs.pEfxAPI->R_SparkStreaks( (float *)&pos, 16, 40, 95 );

	// ---- bullet impact particle burst ----
	gEngfuncs.pEfxAPI->R_BulletImpactParticles( (float *)&pos );

	// ---- CLASSIC DOT DEBRIS (tiny pixels, not streaks) ----
	// count scales with material: harder surfaces throw more specks
	int dots = 14;

	if( tex == CHAR_TEX_METAL || tex == CHAR_TEX_CONCRETE || tex == CHAR_TEX_GRATE )
		dots = 26;
	else if( tex == CHAR_TEX_GLASS || tex == CHAR_TEX_COMPUTER )
		dots = 30;
	else if( tex == CHAR_TEX_WOOD || tex == CHAR_TEX_DIRT )
		dots = 20;
	else if( tex == CHAR_TEX_TILE || tex == CHAR_TEX_VENT )
		dots = 24;

	// main burst straight off the surface
	gEngfuncs.pEfxAPI->R_RunParticleEffect( (float *)&pos, (float *)&dir, 0, dots );

	// second burst drifting upward (gives the debris some life)
	gEngfuncs.pEfxAPI->R_RunParticleEffect( (float *)&pos, (float *)&up, 0, dots / 2 );

	// a few extra slow specks that hang in the air
	gEngfuncs.pEfxAPI->R_RunParticleEffect( (float *)&pos, (float *)&dir, 0, 6 );

	// ---- single fire dlight per hit: keeps dlight pool healthy ----
	float now = gEngfuncs.GetClientTime();

	dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

	if( dl )
	{
		dl->origin[0] = pos.x;
		dl->origin[1] = pos.y;
		dl->origin[2] = pos.z + 6.0f;
		dl->radius = 170.0f;
		dl->color.r = (byte)255;
		dl->color.g = (byte)215;
		dl->color.b = (byte)150;
		dl->decay = 520.0f;
		dl->minlight = 18.0f;
		dl->die = now + 0.11f;
	}
}
inline void ImpactFx( pmtrace_t *tr, int iBulletType, char cTextureType, bool isSky )
{
#if CS16_IMPACT_FX_ON
	if( !tr ) return;
	if( isSky ) return;
	if( tr->fraction >= 1.0f ) return;
	if( tr->allsolid ) return;

	Vector pos = tr->endpos;
	Vector normal = tr->plane.normal;

	int entity = gEngfuncs.pEventAPI->EV_IndexFromTrace( tr );
	int maxClients = gEngfuncs.GetMaxClients();

	if( entity >= 1 && entity <= maxClients )
	{
		if( !ImpactIsEnemy( entity ) )
			return;

		ImpactEmitBlood( pos, normal );
		return;
	}

	const ImpactFxParams *fx = ImpactMaterial( cTextureType );

	ImpactEmitSparks( pos, normal, fx );
	ImpactEmitDebris( pos, normal, fx, cTextureType );
	ImpactEmitSmoke( pos, normal, fx, cTextureType );
	ImpactEmitEngineFx( pos, normal, cTextureType );
#endif
}

}

#endif
