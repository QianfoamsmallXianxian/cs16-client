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
#define CS16_IMPACT_MAX_PARTICLES 40
#endif

namespace CS16Fx
{

struct ImpactFxParams
{
	int   sparkCount;
	float sparkSpeed;
	int   sparkR, sparkG, sparkB;
	int   debrisCount;
	int   debrisR, debrisG, debrisB;
	float debrisScale;
};

inline int ImpactFindSprite( const char *a, const char *b, const char *c, const char *d )
{
	int idx = 0;

	if( a ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( a ); if( idx > 0 ) return idx; }
	if( b ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( b ); if( idx > 0 ) return idx; }
	if( c ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( c ); if( idx > 0 ) return idx; }
	if( d ) { idx = gEngfuncs.pEventAPI->EV_FindModelIndex( d ); if( idx > 0 ) return idx; }

	return 0;
}

inline int ImpactDebrisSprite( char tex )
{
	switch( tex )
	{
	case CHAR_TEX_METAL:
		return ImpactFindSprite( "sprites/debris1.spr", "sprites/metal1.spr", "sprites/black_smoke1.spr", 0 );
	case CHAR_TEX_WOOD:
		return ImpactFindSprite( "sprites/wood1.spr", "sprites/debris1.spr", "sprites/black_smoke1.spr", 0 );
	case CHAR_TEX_GLASS:
		return ImpactFindSprite( "sprites/glass1.spr", "sprites/debris1.spr", "sprites/black_smoke1.spr", 0 );
	case CHAR_TEX_COMPUTER:
		return ImpactFindSprite( "sprites/debris1.spr", "sprites/metal1.spr", "sprites/black_smoke1.spr", 0 );
	case CHAR_TEX_GRASS:
		return ImpactFindSprite( "sprites/grass1.spr", "sprites/debris1.spr", "sprites/black_smoke1.spr", 0 );
	case CHAR_TEX_SNOW:
		return ImpactFindSprite( "sprites/snow1.spr", "sprites/debris1.spr", "sprites/black_smoke1.spr", 0 );
	case CHAR_TEX_SLOSH:
		return ImpactFindSprite( "sprites/slosh1.spr", "sprites/debris1.spr", "sprites/black_smoke1.spr", 0 );
	default:
		return ImpactFindSprite( "sprites/debris1.spr", "sprites/black_smoke1.spr", 0, 0 );
	}
}

inline int ImpactBloodSprite()
{
	return ImpactFindSprite( "sprites/bloodspray.spr", "sprites/blood.spr", "sprites/black_smoke1.spr", 0 );
}

inline int ImpactSparkSprite()
{
	return ImpactFindSprite( "sprites/spark1.spr", "sprites/muzzleflash.spr", "sprites/black_smoke1.spr", 0 );
}

inline const ImpactFxParams *ImpactMaterial( char tex )
{
	static const ImpactFxParams concrete = { 14, 1.20f, 255, 245, 210, 12, 150, 148, 140, 0.55f };
	static const ImpactFxParams metal    = { 22, 1.60f, 255, 235, 150, 10, 120, 120, 135, 0.55f };
	static const ImpactFxParams dirt     = {  0, 1.00f,   0,   0,   0, 14, 140, 110,  70, 0.65f };
	static const ImpactFxParams vent     = {  8, 1.10f, 240, 230, 200, 10, 140, 140, 140, 0.55f };
	static const ImpactFxParams grate    = { 12, 1.30f, 250, 240, 200, 10, 130, 130, 130, 0.55f };
	static const ImpactFxParams tile     = { 10, 1.15f, 250, 245, 230, 12, 210, 210, 200, 0.55f };
	static const ImpactFxParams slosh    = {  0, 1.00f,   0,   0,   0, 12, 100, 150, 210, 0.60f };
	static const ImpactFxParams wood     = {  0, 1.00f,   0,   0,   0, 14, 160, 105,  50, 0.65f };
	static const ImpactFxParams computer = { 20, 1.45f, 255, 240, 170, 12,  90,  90,  95, 0.55f };
	static const ImpactFxParams grass    = {  0, 1.00f,   0,   0,   0, 12, 105, 160,  70, 0.60f };
	static const ImpactFxParams glass    = { 14, 1.35f, 255, 255, 255, 16, 215, 235, 245, 0.45f };
	static const ImpactFxParams snow     = {  0, 1.00f,   0,   0,   0, 12, 240, 240, 248, 0.55f };
	static const ImpactFxParams flesh    = {  0, 1.00f,   0,   0,   0, 14, 180,  20,  20, 0.55f };

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

inline void ImpactEmitSparks( const Vector &pos, const Vector &normal, const ImpactFxParams *fx )
{
	if( !fx || fx->sparkCount <= 0 )
		return;

	int count = fx->sparkCount;
	if( count > CS16_IMPACT_MAX_PARTICLES )
		count = CS16_IMPACT_MAX_PARTICLES;

	Vector dir;
	dir.x = normal.x * normal.x * gEngfuncs.pfnRandomFloat( 4.0f, 12.0f );
	dir.y = normal.y * normal.y * gEngfuncs.pfnRandomFloat( 4.0f, 12.0f );
	dir.z = normal.z * normal.z * gEngfuncs.pfnRandomFloat( 4.0f, 12.0f );

	gEngfuncs.pEfxAPI->R_StreakSplash( (float *)&pos, (float *)&dir, 4, count, dir.z * fx->sparkSpeed, -95.0f, 95.0f );

	int sparkIdx = ImpactSparkSprite();
	if( sparkIdx <= 0 )
		return;

	int bursts = count / 4;
	if( bursts < 2 ) bursts = 2;

	for( int i = 0; i < bursts; i++ )
	{
		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&pos, sparkIdx, 30.0f + i * 4.0f );
		if( !te )
			continue;

		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.rendercolor.r = (unsigned char)fx->sparkR;
		te->entity.curstate.rendercolor.g = (unsigned char)fx->sparkG;
		te->entity.curstate.rendercolor.b = (unsigned char)fx->sparkB;
		te->entity.curstate.renderamt = 230;
		te->entity.curstate.scale = 0.35f * gEngfuncs.pfnRandomFloat( 0.7f, 1.6f );

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 40.0f, 110.0f ) + gEngfuncs.pfnRandomFloat( -120.0f, 120.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 40.0f, 110.0f ) + gEngfuncs.pfnRandomFloat( -120.0f, 120.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 40.0f, 110.0f ) + gEngfuncs.pfnRandomFloat( -30.0f, 130.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.25f, 0.55f );
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
		te->entity.curstate.renderamt = 210;
		te->entity.curstate.scale = fx->debrisScale * gEngfuncs.pfnRandomFloat( 0.55f, 1.45f );

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 25.0f, 80.0f ) + gEngfuncs.pfnRandomFloat( -140.0f, 140.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 25.0f, 80.0f ) + gEngfuncs.pfnRandomFloat( -140.0f, 140.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 25.0f, 80.0f ) + gEngfuncs.pfnRandomFloat( 10.0f, 160.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.7f, 1.6f );
	}
}

inline void ImpactEmitBlood( const Vector &pos, const Vector &normal )
{
	int spriteIdx = ImpactBloodSprite();
	if( spriteIdx <= 0 )
		return;

	const ImpactFxParams *fx = ImpactMaterial( CHAR_TEX_FLESH );

	for( int i = 0; i < 22; i++ )
	{
		TEMPENTITY *te = gEngfuncs.pEfxAPI->R_DefaultSprite( (float *)&pos, spriteIdx, 22.0f + i * 2.0f );
		if( !te )
			continue;

		te->entity.curstate.rendermode = kRenderTransAlpha;
		te->entity.curstate.rendercolor.r = (unsigned char)fx->debrisR;
		te->entity.curstate.rendercolor.g = (unsigned char)fx->debrisG;
		te->entity.curstate.rendercolor.b = (unsigned char)fx->debrisB;
		te->entity.curstate.renderamt = 235;
		te->entity.curstate.scale = fx->debrisScale * gEngfuncs.pfnRandomFloat( 0.5f, 1.5f );

		Vector vel;
		vel.x = normal.x * gEngfuncs.pfnRandomFloat( 20.0f, 70.0f ) + gEngfuncs.pfnRandomFloat( -170.0f, 170.0f );
		vel.y = normal.y * gEngfuncs.pfnRandomFloat( 20.0f, 70.0f ) + gEngfuncs.pfnRandomFloat( -170.0f, 170.0f );
		vel.z = normal.z * gEngfuncs.pfnRandomFloat( 20.0f, 70.0f ) + gEngfuncs.pfnRandomFloat( -10.0f, 150.0f );

		te->entity.baseline.origin = vel;
		te->flags |= FTENT_COLLIDEWORLD | FTENT_GRAVITY | FTENT_PERSIST;
		te->die = gEngfuncs.GetClientTime() + gEngfuncs.pfnRandomFloat( 0.5f, 1.1f );
	}

	gEngfuncs.pEfxAPI->R_StreakSplash( (float *)&pos, (float *)&normal, 4, 8, 60.0f, -60.0f, 60.0f );
}

inline bool ImpactIsEnemy( int hitEntity )
{
	if( hitEntity <= 0 )
		return true;

	cl_entity_t *local = gEngfuncs.GetLocalPlayer();
	if( !local )
		return true;

	int localIdx = local->index;

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

inline void ImpactFx( pmtrace_t *tr, int iBulletType, char cTextureType, bool isSky )
{
#if CS16_IMPACT_FX_ON
	if( !tr || isSky )
		return;

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
#endif
}

}

#endif
