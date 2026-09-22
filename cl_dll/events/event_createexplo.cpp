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
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/
#include "events.h"

#include "com_model.h"
#include "pm_defs.h"
#include "pmtrace.h"

// ---------------------------------------------------------------------------
//  Grenade explosion ground debris
//
//  NOTE: R_BreakModel was dropped - it needs a gib *model* (concretegibs.mdl
//  etc) which cstrike does NOT ship, so EV_FindModelIndex returns 0 and the
//  chunks never appeared.  We now use model-free point particles, which are
//  guaranteed to render: a burst of small specks flying up and outward plus
//  dust streaks.  These are engine point particles, never sprites, so they
//  cannot produce flat black squares.
// ---------------------------------------------------------------------------

void EV_CreateExplo(event_args_s *args)
{
	if( !args )
		return;

	// blast centre
	vec3_t org;
	org[0] = args->origin[0];
	org[1] = args->origin[1];
	org[2] = args->origin[2];

	// ---- find the ground directly below the blast -------------------------
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

	// ---- sparks / flash at the blast centre -------------------------------
	gEngfuncs.pEfxAPI->R_SparkShower( org );
	gEngfuncs.pEfxAPI->R_SparkEffect( org, 12, -180, 180 );

	// ---- classic little debris specks from the ground ---------------------
	// dir = up, so R_RunParticleEffect spreads the specks upward/outward
	vec3_t up;
	up[0] = 0.0f;
	up[1] = 0.0f;
	up[2] = 1.0f;

	// several small bursts -> scattered specks instead of one clump
	for( int i = 0; i < 4; i++ )
	{
		gEngfuncs.pEfxAPI->R_RunParticleEffect(
			groundPos, up, 0, Com_RandomLong( 8, 14 ) );
	}

	// point-particle explosion at the impact point (extra specks)
	gEngfuncs.pEfxAPI->R_ParticleExplosion( groundPos );

	// ---- dust / dirt streaks rising from the impact point -----------------
	vec3_t dustDir;
	dustDir[0] = dustDir[1] = 0.0f;
	dustDir[2] = 1.0f;

	for( int i = 0; i < 2; i++ )
	{
		gEngfuncs.pEfxAPI->R_StreakSplash(
			groundPos,
			dustDir,
			4,							// color index (same as gunshot sparks)
			Com_RandomLong( 10, 16 ),	// streak count
			120.0f,					// speed
			-40,					// velocity min
			90 );					// velocity max
	}
}
