/*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*/
#include "events.h"

#include "com_model.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include <string.h>

// Tunables: every hard-coded value is now a macro.

#ifndef CS16_EXPLO_FLASH_ON
#define CS16_EXPLO_FLASH_ON 1
#endif

#ifndef CS16_EXPLO_FIRE_ON
#define CS16_EXPLO_FIRE_ON 1
#endif

#ifndef CS16_EXPLO_SPARKS_ON
#define CS16_EXPLO_SPARKS_ON 1
#endif

#ifndef CS16_EXPLO_DEBRIS_ON
#define CS16_EXPLO_DEBRIS_ON 1
#endif

#ifndef CS16_EXPLO_SPRITE_DEBRIS_ON
#define CS16_EXPLO_SPRITE_DEBRIS_ON 1
#endif

#ifndef CS16_EXPLO_DUST_ON
#define CS16_EXPLO_DUST_ON 1
#endif

#ifndef CS16_EXPLO_DLIGHT_ON
#define CS16_EXPLO_DLIGHT_ON 1
#endif

#ifndef CS16_EXPLO_FLASH_SPARK_A
#define CS16_EXPLO_FLASH_SPARK_A 24
#endif

#ifndef CS16_EXPLO_FLASH_SPARK_B
#define CS16_EXPLO_FLASH_SPARK_B 16
#endif

#ifndef CS16_EXPLO_STREAK_A_COUNT
#define CS16_EXPLO_STREAK_A_COUNT 16
#endif

#ifndef CS16_EXPLO_STREAK_A_MIN
#define CS16_EXPLO_STREAK_A_MIN 60
#endif

#ifndef CS16_EXPLO_STREAK_A_MAX
#define CS16_EXPLO_STREAK_A_MAX 260
#endif

#ifndef CS16_EXPLO_STREAK_B_COUNT
#define CS16_EXPLO_STREAK_B_COUNT 12
#endif

#ifndef CS16_EXPLO_STREAK_B_MIN
#define CS16_EXPLO_STREAK_B_MIN 30
#endif

#ifndef CS16_EXPLO_STREAK_B_MAX
#define CS16_EXPLO_STREAK_B_MAX 180
#endif

#ifndef CS16_EXPLO_GROUND_SPARK_COUNT
#define CS16_EXPLO_GROUND_SPARK_COUNT 18
#endif

#ifndef CS16_EXPLO_GROUND_SPARK_VEL
#define CS16_EXPLO_GROUND_SPARK_VEL 160
#endif

#ifndef CS16_EXPLO_DOT_GROUND_LOOPS
#define CS16_EXPLO_DOT_GROUND_LOOPS 6
#endif

#ifndef CS16_EXPLO_DOT_GROUND_MIN
#define CS16_EXPLO_DOT_GROUND_MIN 10
#endif

#ifndef CS16_EXPLO_DOT_GROUND_MAX
#define CS16_EXPLO_DOT_GROUND_MAX 18
#endif

#ifndef CS16_EXPLO_DOT_AIR_LOOPS
#define CS16_EXPLO_DOT_AIR_LOOPS 2
#endif

#ifndef CS16_EXPLO_DOT_AIR_MIN
#define CS16_EXPLO_DOT_AIR_MIN 8
#endif

#ifndef CS16_EXPLO_DOT_AIR_MAX
#define CS16_EXPLO_DOT_AIR_MAX 14
#endif

#ifndef CS16_EXPLO_DOT_COLOR_R
#define CS16_EXPLO_DOT_COLOR_R 170
#endif

#ifndef CS16_EXPLO_DOT_COLOR_G
#define CS16_EXPLO_DOT_COLOR_G 150
#endif

#ifndef CS16_EXPLO_DOT_COLOR_B
#define CS16_EXPLO_DOT_COLOR_B 120
#endif

#ifndef CS16_EXPLO_SPRITE_COUNT
#define CS16_EXPLO_SPRITE_COUNT 10
#endif

#ifndef CS16_EXPLO_SPRITE_SCALE_MIN
#define CS16_EXPLO_SPRITE_SCALE_MIN 0.30f
#endif

#ifndef CS16_EXPLO_SPRITE_SCALE_MAX
#define CS16_EXPLO_SPRITE_SCALE_MAX 0.80f
#endif

#ifndef CS16_EXPLO_SPRITE_AMT
#define CS16_EXPLO_SPRITE_AMT 140
#endif

#ifndef CS16_EXPLO_SPRITE_LIFE_MIN
#define CS16_EXPLO_SPRITE_LIFE_MIN 0.60f
#endif

#ifndef CS16_EXPLO_SPRITE_LIFE_MAX
#define CS16_EXPLO_SPRITE_LIFE_MAX 1.40f
#endif

#ifndef CS16_EXPLO_DUST_GROUND_LOOPS
#define CS16_EXPLO_DUST_GROUND_LOOPS 4
#endif

#ifndef CS16_EXPLO_DUST_GROUND_MIN
#define CS16_EXPLO_DUST_GROUND_MIN 12
#endif

#ifndef CS16_EXPLO_DUST_GROUND_MAX
#define CS16_EXPLO_DUST_GROUND_MAX 20
#endif

#ifndef CS16_EXPLO_DUST_AIR_LOOPS
#define CS16_EXPLO_DUST_AIR_LOOPS 2
#endif

#ifndef CS16_EXPLO_DUST_AIR_MIN
#define CS16_EXPLO_DUST_AIR_MIN 10
#endif

#ifndef CS16_EXPLO_DUST_AIR_MAX
#define CS16_EXPLO_DUST_AIR_MAX 16
#endif

#ifndef CS16_EXPLO_DLIGHT_RADIUS_A
#define CS16_EXPLO_DLIGHT_RADIUS_A 260.0f
#endif

#ifndef CS16_EXPLO_DLIGHT_DECAY_A
#define CS16_EXPLO_DLIGHT_DECAY_A 700.0f
#endif

#ifndef CS16_EXPLO_DLIGHT_MINLIGHT_A
#define CS16_EXPLO_DLIGHT_MINLIGHT_A 40.0f
#endif

#ifndef CS16_EXPLO_DLIGHT_LIFE_A
#define CS16_EXPLO_DLIGHT_LIFE_A 0.35f
#endif

#ifndef CS16_EXPLO_DLIGHT_RADIUS_B
#define CS16_EXPLO_DLIGHT_RADIUS_B 380.0f
#endif

#ifndef CS16_EXPLO_DLIGHT_DECAY_B
#define CS16_EXPLO_DLIGHT_DECAY_B 900.0f
#endif

#ifndef CS16_EXPLO_DLIGHT_MINLIGHT_B
#define CS16_EXPLO_DLIGHT_MINLIGHT_B 30.0f
#endif

#ifndef CS16_EXPLO_DLIGHT_LIFE_B
#define CS16_EXPLO_DLIGHT_LIFE_B 0.20f
#endif

static int ExploDebrisSprite()
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
		s_cache = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/debris1.spr" );

	return s_cache;
}

static void ExploEmitSpriteDebris( const vec3_t org, const vec3_t up )
{
	( void )up;

	int spriteIdx = ExploDebrisSprite();
	if( spriteIdx <= 0 )
		return;

	for( int i = 0; i < CS16_EXPLO_SPRITE_COUNT; i++ )
	{
		vec3_t spriteOrg = org;
		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( spriteOrg, spriteIdx, 16.0f + i * 3.0f );
		if( !te )
			continue;

		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.rendercolor.r = (unsigned char)CS16_EXPLO_DOT_COLOR_R;
		te->entity.curstate.rendercolor.g = (unsigned char)CS16_EXPLO_DOT_COLOR_G;
		te->entity.curstate.rendercolor.b = (unsigned char)CS16_EXPLO_DOT_COLOR_B;
		te->entity.curstate.renderamt = CS16_EXPLO_SPRITE_AMT;
		te->entity.curstate.scale = gEngfuncs.pfnRandomFloat( CS16_EXPLO_SPRITE_SCALE_MIN, CS16_EXPLO_SPRITE_SCALE_MAX );

		te->entity.baseline.origin[0] = gEngfuncs.pfnRandomFloat( -180.0f, 180.0f );
		te->entity.baseline.origin[1] = gEngfuncs.pfnRandomFloat( -180.0f, 180.0f );
		te->entity.baseline.origin[2] = gEngfuncs.pfnRandomFloat( 40.0f, 220.0f );

		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST | FTENT_ROTATE;

		te->entity.angles[0] = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
		te->entity.angles[1] = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
		te->entity.angles[2] = gEngfuncs.pfnRandomFloat( 0.0f, 360.0f );
		te->entity.baseline.angles[0] = gEngfuncs.pfnRandomFloat( -260.0f, 260.0f );
		te->entity.baseline.angles[1] = gEngfuncs.pfnRandomFloat( -260.0f, 260.0f );
		te->entity.baseline.angles[2] = gEngfuncs.pfnRandomFloat( -260.0f, 260.0f );

		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( CS16_EXPLO_SPRITE_LIFE_MIN, CS16_EXPLO_SPRITE_LIFE_MAX );
	}
}

static int ExploDotColor()
{
	return gEngfuncs.pEfxAPI->R_LookupColor( (unsigned char)CS16_EXPLO_DOT_COLOR_R, (unsigned char)CS16_EXPLO_DOT_COLOR_G, (unsigned char)CS16_EXPLO_DOT_COLOR_B );
}

static void ExploScatterDir( const vec3_t up, float spread, vec3_t out )
{
	out[0] = up[0] + gEngfuncs.pfnRandomFloat( -spread, spread );
	out[1] = up[1] + gEngfuncs.pfnRandomFloat( -spread, spread );
	out[2] = up[2] + gEngfuncs.pfnRandomFloat( 0.0f, spread );
}

void EV_CreateExplo(event_args_s *args)
{
	if( !args )
		return;

	vec3_t org;
	org[0] = args->origin[0];
	org[1] = args->origin[1];
	org[2] = args->origin[2];

	vec3_t traceEnd;
	traceEnd[0] = org[0];
	traceEnd[1] = org[1];
	traceEnd[2] = org[2] - 64.0f;

	pmtrace_t tr;

	tr.fraction = 1.0f;
	tr.startsolid = 0;
	tr.allsolid = 0;
	tr.endpos[0] = traceEnd[0];
	tr.endpos[1] = traceEnd[1];
	tr.endpos[2] = traceEnd[2];

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( org, traceEnd, PM_WORLD_ONLY, -1, &tr );

	vec3_t groundPos;

	if( tr.fraction < 1.0f && !tr.startsolid && !tr.allsolid )
	{
		groundPos[0] = tr.endpos[0];
		groundPos[1] = tr.endpos[1];
		groundPos[2] = tr.endpos[2];
	}
	else
	{
		groundPos[0] = org[0];
		groundPos[1] = org[1];
		groundPos[2] = org[2] - 24.0f;
	}

	vec3_t up;
	up[0] = 0.0f;
	up[1] = 0.0f;
	up[2] = 1.0f;

	int dotColor = ExploDotColor();

#if CS16_EXPLO_FLASH_ON
	gEngfuncs.pEfxAPI->R_SparkShower( org );
	gEngfuncs.pEfxAPI->R_SparkEffect( org, CS16_EXPLO_FLASH_SPARK_A, -220, 220 );
	gEngfuncs.pEfxAPI->R_SparkEffect( org, CS16_EXPLO_FLASH_SPARK_B, -120, 120 );
	gEngfuncs.pEfxAPI->R_BlobExplosion( org );
	gEngfuncs.pEfxAPI->R_RocketFlare( org );
#endif

#if CS16_EXPLO_FIRE_ON
	gEngfuncs.pEfxAPI->R_ParticleExplosion( org );
	gEngfuncs.pEfxAPI->R_ParticleExplosion2( org, 0, 8 );

	vec3_t firePos;

	firePos[0] = org[0] + 20.0f;
	firePos[1] = org[1] - 16.0f;
	firePos[2] = org[2] + 10.0f;
	gEngfuncs.pEfxAPI->R_ParticleExplosion( firePos );

	firePos[0] = org[0] - 22.0f;
	firePos[1] = org[1] + 14.0f;
	firePos[2] = org[2] + 6.0f;
	gEngfuncs.pEfxAPI->R_ParticleExplosion( firePos );
#endif

#if CS16_EXPLO_SPARKS_ON
	gEngfuncs.pEfxAPI->R_SparkStreaks( org, CS16_EXPLO_STREAK_A_COUNT, CS16_EXPLO_STREAK_A_MIN, CS16_EXPLO_STREAK_A_MAX );
	gEngfuncs.pEfxAPI->R_SparkStreaks( org, CS16_EXPLO_STREAK_B_COUNT, CS16_EXPLO_STREAK_B_MIN, CS16_EXPLO_STREAK_B_MAX );
	gEngfuncs.pEfxAPI->R_SparkShower( groundPos );
	gEngfuncs.pEfxAPI->R_SparkEffect( groundPos, CS16_EXPLO_GROUND_SPARK_COUNT, -CS16_EXPLO_GROUND_SPARK_VEL, CS16_EXPLO_GROUND_SPARK_VEL );
#endif

#if CS16_EXPLO_DEBRIS_ON
	vec3_t scatter;

	for( int i = 0; i < CS16_EXPLO_DOT_GROUND_LOOPS; i++ )
	{
		ExploScatterDir( up, 0.35f, scatter );
		gEngfuncs.pEfxAPI->R_RunParticleEffect( groundPos, scatter, dotColor, Com_RandomLong( CS16_EXPLO_DOT_GROUND_MIN, CS16_EXPLO_DOT_GROUND_MAX ) );
	}

	for( int i = 0; i < CS16_EXPLO_DOT_AIR_LOOPS; i++ )
	{
		ExploScatterDir( up, 0.55f, scatter );
		gEngfuncs.pEfxAPI->R_RunParticleEffect( org, scatter, dotColor, Com_RandomLong( CS16_EXPLO_DOT_AIR_MIN, CS16_EXPLO_DOT_AIR_MAX ) );
	}
#endif

#if CS16_EXPLO_SPRITE_DEBRIS_ON
	ExploEmitSpriteDebris( org, up );
#endif

#if CS16_EXPLO_DUST_ON
	vec3_t dustDir;
	dustDir[0] = 0.0f;
	dustDir[1] = 0.0f;
	dustDir[2] = 1.0f;

	for( int i = 0; i < CS16_EXPLO_DUST_GROUND_LOOPS; i++ )
	{
		gEngfuncs.pEfxAPI->R_StreakSplash( groundPos, dustDir, 4, Com_RandomLong( CS16_EXPLO_DUST_GROUND_MIN, CS16_EXPLO_DUST_GROUND_MAX ), 140.0f, -60, 110 );
	}

	for( int i = 0; i < CS16_EXPLO_DUST_AIR_LOOPS; i++ )
	{
		gEngfuncs.pEfxAPI->R_StreakSplash( org, dustDir, 4, Com_RandomLong( CS16_EXPLO_DUST_AIR_MIN, CS16_EXPLO_DUST_AIR_MAX ), 120.0f, -40, 90 );
	}
#endif

#if CS16_EXPLO_DLIGHT_ON
	float now = gEngfuncs.GetClientTime();

	dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

	if( dl )
	{
		dl->origin[0] = org[0];
		dl->origin[1] = org[1];
		dl->origin[2] = org[2];
		dl->radius = CS16_EXPLO_DLIGHT_RADIUS_A;
		dl->color.r = (byte)255;
		dl->color.g = (byte)175;
		dl->color.b = (byte)70;
		dl->decay = CS16_EXPLO_DLIGHT_DECAY_A;
		dl->minlight = CS16_EXPLO_DLIGHT_MINLIGHT_A;
		dl->die = now + CS16_EXPLO_DLIGHT_LIFE_A;
	}

	dlight_t *dl2 = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );

	if( dl2 )
	{
		dl2->origin[0] = groundPos[0];
		dl2->origin[1] = groundPos[1];
		dl2->origin[2] = groundPos[2] + 8.0f;
		dl2->radius = CS16_EXPLO_DLIGHT_RADIUS_B;
		dl2->color.r = (byte)255;
		dl2->color.g = (byte)220;
		dl2->color.b = (byte)150;
		dl2->decay = CS16_EXPLO_DLIGHT_DECAY_B;
		dl2->minlight = CS16_EXPLO_DLIGHT_MINLIGHT_B;
		dl2->die = now + CS16_EXPLO_DLIGHT_LIFE_B;
	}
#endif
}
