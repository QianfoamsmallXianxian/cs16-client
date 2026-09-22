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
//  EV_CreateExplo() was an empty stub (CS:CZDS leftovers).  It now spawns
//  sparks at the blast centre plus debris chunks and dust streaks kicked up
//  from the ground underneath the explosion.
//
//  Design notes:
//    * The engine FX API takes raw float pointers, and event_args_t::origin
//      is a raw float[3]; everything below therefore works on vec3_t arrays.
//    * Debris chunks use R_BreakModel, which needs a *model* index.  If the
//      game assets do not ship those gib models, EV_FindModelIndex returns 0
//      and we silently skip the chunks.
//    * To guarantee visible feedback even without a gib model, we ALWAYS add
//      engine point particles (R_ParticleExplosion) + dust streaks.  Those
//      are textureless point particles, so they cannot produce the flat black
//      sprite squares that Alpha-blended sprites used to cause.
// ---------------------------------------------------------------------------

// debris models, tried in order until one resolves on the client
static const char *g_DebrisModels[] =
{
	"models/concretegibs.mdl",
	"models/woodgibs.mdl",
	"models/metalgibs.mdl",
	"models/glassgibs.mdl",
	NULL
};

static int FindFirstDebrisModel( void )
{
	for( int i = 0; g_DebrisModels[i] != NULL; i++ )
	{
		int idx = gEngfuncs.pEventAPI->EV_FindModelIndex( g_DebrisModels[i] );

		if( idx > 0 )
			return idx;
	}

	return 0;
}

void EV_CreateExplo(event_args_s *args)
{
	if( !args )
		return;

	// blast centre copied out of the raw event origin
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
		// nothing solid below (in air) -> just use a spot under the blast
		groundPos[0] = org[0];
		groundPos[1] = org[1];
		groundPos[2] = org[2] - 24.0f;
	}

	// ---- sparks / flash at the blast centre -------------------------------
	gEngfuncs.pEfxAPI->R_SparkShower( org );
	gEngfuncs.pEfxAPI->R_SparkEffect( org, 12, -180, 180 );

	// ---- physical debris chunks kicked out of the ground ------------------
	// (only if the game actually ships a gib model)
	int debrisIdx = FindFirstDebrisModel();

	if( debrisIdx > 0 )
	{
		vec3_t boxSize;
		boxSize[0] = boxSize[1] = boxSize[2] = 8.0f;

		vec3_t dir;
		dir[0] = dir[1] = dir[2] = 0.0f;

		// several bursts -> a scattered pile of chunks, not one clump
		for( int i = 0; i < 3; i++ )
		{
			gEngfuncs.pEfxAPI->R_BreakModel(
				groundPos, boxSize, dir,
				60.0f,		// random velocity spread
				0.4f,		// life
				4,			// chunk count per burst
				debrisIdx,
				0 );			// flags
		}
	}

	// ---- guaranteed point-particle debris (no sprite, no black squares) ---
	// engine built-in particle explosion at the ground contact point
	gEngfuncs.pEfxAPI->R_ParticleExplosion( groundPos );

	// a second one slightly above, for volume
	vec3_t upPos;
	upPos[0] = groundPos[0];
	upPos[1] = groundPos[1];
	upPos[2] = groundPos[2] + 12.0f;

	gEngfuncs.pEfxAPI->R_ParticleExplosion( upPos );

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
