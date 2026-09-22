/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/
#include "events.h"

#include "com_model.h"
#include "pm_defs.h"
#include "pmtrace.h"

// Grenade explosion - MAXIMUM EFFECTS version.
// All effects are engine built-ins from r_efx.h, all model-free,
// so none can silently fail and none can produce black squares.

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

	// 1. FLASH
	gEngfuncs.pEfxAPI->R_SparkShower( org );
	gEngfuncs.pEfxAPI->R_SparkEffect( org, 24, -220, 220 );
	gEngfuncs.pEfxAPI->R_SparkEffect( org, 16, -120, 120 );
	gEngfuncs.pEfxAPI->R_BlobExplosion( org );
	gEngfuncs.pEfxAPI->R_RocketFlare( org );

	// 2. FIRE
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

	// 3. SPARK / STREAK
	gEngfuncs.pEfxAPI->R_SparkStreaks( org, 16, 60, 260 );
	gEngfuncs.pEfxAPI->R_SparkStreaks( org, 12, 30, 180 );
	gEngfuncs.pEfxAPI->R_SparkShower( groundPos );
	gEngfuncs.pEfxAPI->R_SparkEffect( groundPos, 18, -160, 160 );

	// 4. GROUND DEBRIS
	for( int i = 0; i < 6; i++ )
	{
		gEngfuncs.pEfxAPI->R_RunParticleEffect( groundPos, up, 0, Com_RandomLong( 10, 18 ) );
	}
	for( int i = 0; i < 2; i++ )
	{
		gEngfuncs.pEfxAPI->R_RunParticleEffect( org, up, 0, Com_RandomLong( 8, 14 ) );
	}

	// 5. DUST
	vec3_t dustDir;
	dustDir[0] = 0.0f;
	dustDir[1] = 0.0f;
	dustDir[2] = 1.0f;

	for( int i = 0; i < 4; i++ )
	{
		gEngfuncs.pEfxAPI->R_StreakSplash( groundPos, dustDir, 4, Com_RandomLong( 12, 20 ), 140.0f, -60, 110 );
	}
	for( int i = 0; i < 2; i++ )
	{
		gEngfuncs.pEfxAPI->R_StreakSplash( org, dustDir, 4, Com_RandomLong( 10, 16 ), 120.0f, -40, 90 );
	}

	// 6. DYNAMIC LIGHT
	dlight_t *dl = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );
	if( dl )
	{
		dl->origin[0] = org[0];
		dl->origin[1] = org[1];
		dl->origin[2] = org[2];
		dl->radius = 260.0f;
		dl->color.r = 255;
		dl->color.g = 175;
		dl->color.b = 70;
		dl->die = gEngfuncs.GetClientTime() + 0.35f;
	}

	dlight_t *dl2 = gEngfuncs.pEfxAPI->CL_AllocDlight( 0 );
	if( dl2 )
	{
		dl2->origin[0] = groundPos[0];
		dl2->origin[1] = groundPos[1];
		dl2->origin[2] = groundPos[2] + 8.0f;
		dl2->radius = 380.0f;
		dl2->color.r = 255;
		dl2->color.g = 220;
		dl2->color.b = 150;
		dl2->die = gEngfuncs.GetClientTime() + 0.20f;
	}
}
