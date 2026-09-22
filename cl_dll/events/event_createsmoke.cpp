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

// smoke grenade cloud count
#define SMOKE_CLOUDS 45

// spread radius (units) for the initial burst clouds
#define SMOKE_SPREAD 250.0f

// lifetime of a cloud, and when it starts to fade out
#define SMOKE_LIFETIME 42.0f
#define SMOKE_FADE_START 22.0f

void EV_CreateSmoke(event_args_s *args)
{
	TEMPENTITY *pTemp;

	if( !args->bparam2 )
	{
		const model_t *pGasModel = gEngfuncs.GetSpritePointer(gHUD.m_hGasPuff);

		if( !pGasModel )
			return;

		int frameMax = max( 0, pGasModel->numframes - 1 );
		float now = gEngfuncs.GetClientTime();

		for( int i = 0; i < SMOKE_CLOUDS; i++ )
		{
			Vector org = args->origin;
			if( i != 0 )
			{
				org.x += Com_RandomFloat(-SMOKE_SPREAD, SMOKE_SPREAD);
				org.y += Com_RandomFloat(-SMOKE_SPREAD, SMOKE_SPREAD);
			}
			org.z += 60;

			pTemp = gEngfuncs.pEfxAPI->CL_TempEntAllocNoModel( org );
			if( pTemp )
			{
				pTemp->flags |= (FTENT_SPRANIMATELOOP | FTENT_COLLIDEWORLD | FTENT_CLIENTCUSTOM | FTENT_PERSIST);
				pTemp->flags &= ~(FTENT_NOMODEL);
				pTemp->die = now + SMOKE_LIFETIME;
				pTemp->callback = [](struct tempent_s *te, float frametime, float currenttime) -> void
				{
					// fuser1 = initial alpha, fuser2 = fade rate (alpha/sec),
					// fuser3 = time when fading starts, fuser4 = creation time
					if( te->entity.curstate.renderamt > 0 && currenttime >= te->entity.curstate.fuser3 )
					{
						float amt = te->entity.curstate.fuser1
						          - ( currenttime - te->entity.curstate.fuser3 ) * te->entity.curstate.fuser2;

						if( amt < 0.0f )
							amt = 0.0f;

						te->entity.curstate.renderamt = (int)amt;
					}
					EV_CS16Client_KillEveryRound( te, frametime, currenttime );
				};

				pTemp->entity.model = (struct model_s*)pGasModel;
				pTemp->frameMax = frameMax;

				// Alpha blending (classic CS smoke): solid, opaque wall of smoke.
				// NOT additive -- additive can only brighten, which looks like
				// thin glowing white haze instead of real smoke.
				float alpha = 255.0f;
				float fadeWindow = SMOKE_LIFETIME - SMOKE_FADE_START;

				pTemp->entity.curstate.fuser1 = alpha;                       // initial alpha
				pTemp->entity.curstate.fuser2 = alpha / fadeWindow;           // fade rate per second
				pTemp->entity.curstate.fuser3 = now + SMOKE_FADE_START;       // fade start time
				pTemp->entity.curstate.fuser4 = now;                          // creation time

				pTemp->entity.curstate.rendermode = kRenderTransAlpha;
				pTemp->entity.curstate.renderamt = (int)alpha;
				// classic near-white smoke
				pTemp->entity.curstate.rendercolor.r = Com_RandomLong( 210, 230 );
				pTemp->entity.curstate.rendercolor.g = Com_RandomLong( 210, 230 );
				pTemp->entity.curstate.rendercolor.b = Com_RandomLong( 210, 230 );
				// classic puff size
				pTemp->entity.curstate.scale = Com_RandomFloat( 7.0f, 9.5f );

				pTemp->entity.baseline.origin.x = Com_RandomLong(-5, 5);
				pTemp->entity.baseline.origin.y = Com_RandomLong(-5, 5);

				if( i == 0 )
				{
					// killTime must be an absolute client time, not a duration
					gHUD.m_Spectator.AddOverviewEntityToList( gHUD.m_hGasPuff, &pTemp->entity, now + 14.0f );
				}
			}
		}
	}
	else
	{
		int g = gEngfuncs.pfnRandomLong( 190, 220 );

		Vector dir( args->fparam1, args->fparam2, 0.0f );
		Vector vel( 0.0f, 0.0f, 0.0f );

		EV_CS16Client_CreateSmoke( SMOKE_BLACK, args->origin, dir, args->iparam1 / 100, 1.1f, g, g, g, true, vel, 25 );
	}
}
