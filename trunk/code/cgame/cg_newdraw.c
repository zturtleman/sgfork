/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2003 Iron Claw Interactive
Copyright (C) 2005-2009 Smokin' Guns

This file is part of Smokin' Guns.

Smokin' Guns is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Smokin' Guns is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Smokin' Guns; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "cg_local.h"
#include "../ui/ui_shared.h"

extern displayContextDef_t cgDC;

void CG_DrawField (int x, int y, int width, int value);
void CG_DrawMoneyField (int x, int y, int width, int value);

// set in CG_ParseTeamInfo

//static int sortedTeamPlayers[TEAM_MAXOVERLAY];
//static int numSortedTeamPlayers;
//int drawTeamOverlayModificationCount = -1;

//static char systemChat[256];
//static char teamChat1[256];
//static char teamChat2[256];

void CG_InitTeamChat(void) {
  memset(teamChat1, 0, sizeof(teamChat1));
  memset(teamChat2, 0, sizeof(teamChat2));
  memset(systemChat, 0, sizeof(systemChat));
}

void CG_SetPrintString(int type, const char *p) {
  if (type == SYSTEM_PRINT) {
    strcpy(systemChat, p);
  } else {
    strcpy(teamChat2, teamChat1);
    strcpy(teamChat1, p);
  }
}

void CG_CheckOrderPending(void) {
	if (cgs.gametype < GT_RTP) {
		return;
	}
	if (cgs.orderPending) {
		//clientInfo_t *ci = cgs.clientinfo + sortedTeamPlayers[cg_currentSelectedPlayer.integer];
		const char *p1, *p2, *b;
		p1 = p2 = b = NULL;
		switch (cgs.currentOrder) {
			case TEAMTASK_OFFENSE:
				p1 = VOICECHAT_ONOFFENSE;
				p2 = VOICECHAT_OFFENSE;
				b = "+button7; wait; -button7";
			break;
			case TEAMTASK_DEFENSE:
				p1 = VOICECHAT_ONDEFENSE;
				p2 = VOICECHAT_DEFEND;
				b = "+button8; wait; -button8";
			break;
			case TEAMTASK_PATROL:
				p1 = VOICECHAT_ONPATROL;
				p2 = VOICECHAT_PATROL;
				b = "+button9; wait; -button9";
			break;
			case TEAMTASK_FOLLOW:
				p1 = VOICECHAT_ONFOLLOW;
				p2 = VOICECHAT_FOLLOWME;
				b = "+button10; wait; -button10";
			break;
			case TEAMTASK_CAMP:
				p1 = VOICECHAT_ONCAMPING;
				p2 = VOICECHAT_CAMP;
			break;
			case TEAMTASK_RETRIEVE:
				p1 = VOICECHAT_ONGETFLAG;
				p2 = VOICECHAT_RETURNFLAG;
			break;
			case TEAMTASK_ESCORT:
				p1 = VOICECHAT_ONFOLLOWCARRIER;
				p2 = VOICECHAT_FOLLOWFLAGCARRIER;
			break;
		}

		if (cg_currentSelectedPlayer.integer == numSortedTeamPlayers) {
			// to everyone
			trap_SendConsoleCommand(va("cmd vsay_team %s\n", p2));
		} else {
			// for the player self
			if (sortedTeamPlayers[cg_currentSelectedPlayer.integer] == cg.snap->ps.clientNum && p1) {
				trap_SendConsoleCommand(va("teamtask %i\n", cgs.currentOrder));
				//trap_SendConsoleCommand(va("cmd say_team %s\n", p2));
				trap_SendConsoleCommand(va("cmd vsay_team %s\n", p1));
			} else if (p2) {
				//trap_SendConsoleCommand(va("cmd say_team %s, %s\n", ci->name,p));
				trap_SendConsoleCommand(va("cmd vtell %d %s\n", sortedTeamPlayers[cg_currentSelectedPlayer.integer], p2));
			}
		}
		if (b) {
			trap_SendConsoleCommand(b);
		}
		cgs.orderPending = qfalse;
	}
}

static void CG_SetSelectedPlayerName( void ) {
  if (cg_currentSelectedPlayer.integer >= 0 && cg_currentSelectedPlayer.integer < numSortedTeamPlayers) {
		clientInfo_t *ci = cgs.clientinfo + sortedTeamPlayers[cg_currentSelectedPlayer.integer];
	  if (ci) {
			trap_Cvar_Set("cg_selectedPlayerName", ci->name);
			trap_Cvar_Set("cg_selectedPlayer", va("%d", sortedTeamPlayers[cg_currentSelectedPlayer.integer]));
			cgs.currentOrder = ci->teamTask;
	  }
	} else {
		trap_Cvar_Set("cg_selectedPlayerName", "Everyone");
	}
}
int CG_GetSelectedPlayer( void ) {
	if (cg_currentSelectedPlayer.integer < 0 || cg_currentSelectedPlayer.integer >= numSortedTeamPlayers) {
		cg_currentSelectedPlayer.integer = 0;
	}
	return cg_currentSelectedPlayer.integer;
}

void CG_SelectNextPlayer( void ) {
	CG_CheckOrderPending();
	if (cg_currentSelectedPlayer.integer >= 0 && cg_currentSelectedPlayer.integer < numSortedTeamPlayers) {
		cg_currentSelectedPlayer.integer++;
	} else {
		cg_currentSelectedPlayer.integer = 0;
	}
	CG_SetSelectedPlayerName();
}

void CG_SelectPrevPlayer( void ) {
	CG_CheckOrderPending();
	if (cg_currentSelectedPlayer.integer > 0 && cg_currentSelectedPlayer.integer < numSortedTeamPlayers) {
		cg_currentSelectedPlayer.integer--;
	} else {
		cg_currentSelectedPlayer.integer = numSortedTeamPlayers;
	}
	CG_SetSelectedPlayerName();
}


static void CG_DrawPlayerArmorIcon( rectDef_t *rect, qboolean draw2D ) {
	centity_t	*cent;
	playerState_t	*ps;
	vec3_t		angles;
	vec3_t		origin;

  if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	if ( draw2D || ( !cg_draw3dIcons.integer && cg_drawIcons.integer) ) {
		CG_DrawPic( rect->x, rect->y + rect->h/2 + 1, rect->w, rect->h, cgs.media.armorIcon );
  } else if (cg_draw3dIcons.integer) {
	  VectorClear( angles );
    origin[0] = 90;
  	origin[1] = 0;
  	origin[2] = -10;
  	angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;

    CG_Draw3DModel( rect->x, rect->y, rect->w, rect->h, cgs.media.armorModel, 0, origin, angles );
  }

}

static void CG_DrawPlayerArmorValue(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
	char	num[16];
  int value;
	centity_t	*cent;
	playerState_t	*ps;

  cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	value = ps->stats[STAT_ARMOR];


	if (shader) {
    trap_R_SetColor( color );
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
	  trap_R_SetColor( NULL );
	} else {
		Com_sprintf (num, sizeof(num), "%i", value);
		value = CG_Text_Width(num, scale, 0);
	  CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
	}
}

static void CG_DrawPlayerAmmoIcon( rectDef_t *rect, qboolean draw2D ) {
	centity_t	*cent;
	playerState_t	*ps;
	vec3_t		angles;
	vec3_t		origin;

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	if ( draw2D || (!cg_draw3dIcons.integer && cg_drawIcons.integer) ) {
	  qhandle_t	icon;
    icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
		if ( icon ) {
		  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, icon );
		}
  } else if (cg_draw3dIcons.integer) {
  	if ( cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel ) {
	    VectorClear( angles );
	  	origin[0] = 70;
  		origin[1] = 0;
  		origin[2] = 0;
  		angles[YAW] = 90 + 20 * sin( cg.time / 1000.0 );
  		CG_Draw3DModel( rect->x, rect->y, rect->w, rect->h, cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles );
  	}
  }
}

static void CG_DrawPlayerAmmoValue(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
	char	num[16];
	int value;
	centity_t	*cent;
	playerState_t	*ps;

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	if ( cent->currentState.weapon ) {
		value = ps->ammo[cent->currentState.weapon];
		if ( value > -1 ) {
			if (shader) {
		    trap_R_SetColor( color );
				CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
			  trap_R_SetColor( NULL );
			} else {
				Com_sprintf (num, sizeof(num), "%i", value);
				value = CG_Text_Width(num, scale, 0);
				CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
			}
		}
	}

}



static void CG_DrawPlayerHead(rectDef_t *rect, qboolean draw2D) {
	vec3_t		angles;
	float		size, stretch;
	float		frac;
	float		x = rect->x;

	VectorClear( angles );

	if ( cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME ) {
		frac = (float)(cg.time - cg.damageTime ) / DAMAGE_TIME;
		size = rect->w * 1.25 * ( 1.5 - frac * 0.5 );

		stretch = size - rect->w * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
		cg.headEndPitch = 5 * cos( crandom()*M_PI );

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	} else {
		if ( cg.time >= cg.headEndTime ) {
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;

			cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
			cg.headEndPitch = 5 * cos( crandom()*M_PI );
		}

		size = rect->w * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if ( cg.headStartTime > cg.time ) {
		cg.headStartTime = cg.time;
	}

	frac = ( cg.time - cg.headStartTime ) / (float)( cg.headEndTime - cg.headStartTime );
	frac = frac * frac * ( 3 - 2 * frac );
	angles[YAW] = cg.headStartYaw + ( cg.headEndYaw - cg.headStartYaw ) * frac;
	angles[PITCH] = cg.headStartPitch + ( cg.headEndPitch - cg.headStartPitch ) * frac;

	CG_DrawHead( x, rect->y, rect->w, rect->h, cg.snap->ps.clientNum, angles );
}

static void CG_DrawSelectedPlayerHealth( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	clientInfo_t *ci;
	int value;
	char num[16];

  ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
  if (ci) {
		if (shader) {
			trap_R_SetColor( color );
			CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
			trap_R_SetColor( NULL );
		} else {
			Com_sprintf (num, sizeof(num), "%i", ci->health);
		  value = CG_Text_Width(num, scale, 0);
		  CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
		}
	}
}

static void CG_DrawSelectedPlayerArmor( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	clientInfo_t *ci;
	int value;
	char num[16];
  ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
  if (ci) {
    if (ci->armor > 0) {
			if (shader) {
				trap_R_SetColor( color );
				CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
				trap_R_SetColor( NULL );
			} else {
				Com_sprintf (num, sizeof(num), "%i", ci->armor);
				value = CG_Text_Width(num, scale, 0);
				CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
			}
		}
 	}
}

qhandle_t CG_StatusHandle(int task) {
	qhandle_t h = -1;
	return h;
}

static void CG_DrawSelectedPlayerStatus( rectDef_t *rect ) {
	clientInfo_t *ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
	if (ci) {
		qhandle_t h;
		if (cgs.orderPending) {
			// blink the icon
			if ( cg.time > cgs.orderTime - 2500 && (cg.time >> 9 ) & 1 ) {
				return;
			}
			h = CG_StatusHandle(cgs.currentOrder);
		}	else {
			h = CG_StatusHandle(ci->teamTask);
		}
		CG_DrawPic( rect->x, rect->y, rect->w, rect->h, h );
	}
}


static void CG_DrawPlayerStatus( rectDef_t *rect ) {
	clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
	if (ci) {
		qhandle_t h = CG_StatusHandle(ci->teamTask);
		CG_DrawPic( rect->x, rect->y, rect->w, rect->h, h);
	}
}


static void CG_DrawSelectedPlayerName( rectDef_t *rect, float scale, vec4_t color, qboolean voice, int textStyle) {
	clientInfo_t *ci;
  ci = cgs.clientinfo + ((voice) ? cgs.currentVoiceClient : sortedTeamPlayers[CG_GetSelectedPlayer()]);
  if (ci) {
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, ci->name, 0, 0, textStyle);
  }
}

static void CG_DrawSelectedPlayerLocation( rectDef_t *rect, float scale, vec4_t color, int textStyle ) {
	clientInfo_t *ci;
  ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
  if (ci) {
		const char *p = CG_ConfigString(CS_LOCATIONS + ci->location);
		if (!p || !*p) {
			p = "unknown";
		}
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, p, 0, 0, textStyle);
  }
}

static void CG_DrawPlayerLocation( rectDef_t *rect, float scale, vec4_t color, int textStyle  ) {
	clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
  if (ci) {
		const char *p = CG_ConfigString(CS_LOCATIONS + ci->location);
		if (!p || !*p) {
			p = "unknown";
		}
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, p, 0, 0, textStyle);
  }
}



static void CG_DrawSelectedPlayerWeapon( rectDef_t *rect ) {
	clientInfo_t *ci;

  ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
  if (ci) {
	  if ( cg_weapons[ci->curWeapon].weaponIcon ) {
	    CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cg_weapons[ci->curWeapon].weaponIcon );
		} else {
  	  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.deferShader);
    }
  }
}

static void CG_DrawPlayerScore( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	char num[16];
	int value = cg.snap->ps.persistant[PERS_SCORE];

	if(cgs.gametype == GT_DUEL){
		value = cgs.clientinfo[cg.snap->ps.clientNum].losses;
	}

	if (shader) {
		trap_R_SetColor( color );
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
		trap_R_SetColor( NULL );
	} else {
		Com_sprintf (num, sizeof(num), "%i", value);
		value = CG_Text_Width(num, scale, 0);
	  CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
	}
}

static void CG_DrawRoundtime( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	int value;

	char		*s;
	int			mins, seconds, tens;
	int			msec;

	if(!cg.roundendtime)
		return;

	msec = cg.roundendtime - cg.time;

	if(msec < 0)
		msec = 0;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );

	value = CG_Text_Width(s, scale, 0);
	CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, s, 0, 0, textStyle);
}

static void CG_DrawPlayerItem( rectDef_t *rect, float scale, qboolean draw2D) {
	int		value;
  vec3_t origin, angles;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );

		if (qtrue) {
		  CG_RegisterItemVisuals( value );
		  CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cg_items[ value ].icon );
		} else {
 			VectorClear( angles );
			origin[0] = 90;
  		origin[1] = 0;
   		origin[2] = -10;
  		angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
			CG_Draw3DModel(rect->x, rect->y, rect->w, rect->h, cg_items[ value ].models[0], 0, origin, angles );
		}
	}

}

static void CG_DrawSelectedPlayerHead( rectDef_t *rect, qboolean draw2D, qboolean voice ) {
	clipHandle_t	cm;
	clientInfo_t	*ci;
	float			len;
	vec3_t			origin;
	vec3_t			mins, maxs, angles;


  ci = cgs.clientinfo + ((voice) ? cgs.currentVoiceClient : sortedTeamPlayers[CG_GetSelectedPlayer()]);

  if (ci) {
  	if ( cg_draw3dIcons.integer ) {
	  	cm = ci->headModel;
  		if ( !cm ) {
  			return;
	  	}

  		// offset the origin y and z to center the head
  		trap_R_ModelBounds( cm, mins, maxs );

	  	origin[2] = -0.5 * ( mins[2] + maxs[2] );
  		origin[1] = 0.5 * ( mins[1] + maxs[1] );

	  	// calculate distance so the head nearly fills the box
  		// assume heads are taller than wide
  		len = 0.7 * ( maxs[2] - mins[2] );
  		origin[0] = len / 0.268;	// len / tan( fov/2 )

  		// allow per-model tweaking
  		VectorAdd( origin, ci->headOffset, origin );

    	angles[PITCH] = 0;
    	angles[YAW] = 180;
    	angles[ROLL] = 0;

      CG_Draw3DModel( rect->x, rect->y, rect->w, rect->h, ci->headModel, ci->headSkin, origin, angles );
  	} else if ( cg_drawIcons.integer ) {
	  	CG_DrawPic( rect->x, rect->y, rect->w, rect->h, ci->modelIcon );
  	}

  	// if they are deferred, draw a cross out
  	if ( ci->deferred ) {
  		CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.deferShader );
  	}
  }

}


static void CG_DrawPlayerHealth(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	playerState_t	*ps;
  int value;
	char	num[16];

	ps = &cg.snap->ps;

	value = ps->stats[STAT_HEALTH];
	if(value < 0)
		value = 0;

	if (shader) {
		trap_R_SetColor( color );
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
		trap_R_SetColor( NULL );
	} else {
		Com_sprintf (num, sizeof(num), "%i", value);
	  value = CG_Text_Width(num, scale, 0);
	  CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
	}
}


static void CG_DrawRedScore(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	int value;
	char num[16];
	if ( cgs.scores1 == SCORE_NOT_PRESENT ) {
		Com_sprintf (num, sizeof(num), "-");
	}
	else {
		Com_sprintf (num, sizeof(num), "%i", cgs.scores1);
	}
	value = CG_Text_Width(num, scale, 0);
	CG_Text_Paint(rect->x + rect->w - value, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
}

static void CG_DrawBlueScore(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	int value;
	char num[16];

	if ( cgs.scores2 == SCORE_NOT_PRESENT ) {
		Com_sprintf (num, sizeof(num), "-");
	}
	else {
		Com_sprintf (num, sizeof(num), "%i", cgs.scores2);
	}
	value = CG_Text_Width(num, scale, 0);
	CG_Text_Paint(rect->x + rect->w - value, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
}

// FIXME: team name support
static void CG_DrawRedName(rectDef_t *rect, float scale, vec4_t color, int textStyle ) {
	// draw moneybag to indicate that this is the robber team
	if(cgs.gametype == GT_BR){
		gitem_t *item = BG_FindItem("Moneybag");
		int x = 14;
		int	y = 50;
		vec4_t colorw = { 1.0f, 1.0f, 1.0f, 1.0f};

		CG_DrawPic(x, y-32+5, 32, 32, cg_items[ITEM_INDEX(item)].icon);
		x += 32 + 5;

		if(cg.snap->ps.persistant[PERS_ROBBER])
			CG_Text_Paint(x, y, scale, colorw, "Get the Moneybag!", 0, -1, 3);
		else
			CG_Text_Paint(x, y, scale, colorw, "Defend the Moneybag!", 0, -1, 3);
	}

	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, cg_redTeamName.string , 0, 0, textStyle);
}

static void CG_DrawBlueName(rectDef_t *rect, float scale, vec4_t color, int textStyle ) {
  CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, cg_blueTeamName.string, 0, 0, textStyle);
}

static void CG_DrawTeamColor(rectDef_t *rect, vec4_t color) {
	CG_DrawTeamBackground(rect->x, rect->y, rect->w, rect->h, color[3], cg.snap->ps.persistant[PERS_TEAM]);
}

float CG_GetValue(int ownerDraw) {
	centity_t	*cent;
 	clientInfo_t *ci;
	playerState_t	*ps;

  cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

  switch (ownerDraw) {
  case CG_SELECTEDPLAYER_ARMOR:
    ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
    return ci->armor;
    break;
  case CG_SELECTEDPLAYER_HEALTH:
    ci = cgs.clientinfo + sortedTeamPlayers[CG_GetSelectedPlayer()];
    return ci->health;
    break;
  case CG_PLAYER_ARMOR_VALUE:
		return ps->stats[STAT_ARMOR];
    break;
  case CG_PLAYER_AMMO_VALUE:
		if ( cent->currentState.weapon ) {
		  return ps->ammo[cent->currentState.weapon];
		}
    break;
  case CG_PLAYER_SCORE:
	  return cg.snap->ps.persistant[PERS_SCORE];
    break;
  case CG_PLAYER_HEALTH:
		return ps->stats[STAT_HEALTH];
    break;
  case CG_RED_SCORE:
		return cgs.scores1;
    break;
  case CG_BLUE_SCORE:
		return cgs.scores2;
    break;
  default:
    break;
  }
	return -1;
}

// THINKABOUTME: should these be exclusive or inclusive..
//
qboolean CG_OwnerDrawVisible(int flags) {

	if (flags & CG_SHOW_TEAMINFO) {
		return (cg_currentSelectedPlayer.integer == numSortedTeamPlayers);
	}

	if (flags & CG_SHOW_NOTEAMINFO) {
		return !(cg_currentSelectedPlayer.integer == numSortedTeamPlayers);
	}

	if (flags & CG_SHOW_ANYTEAMGAME) {
		if( cgs.gametype >= GT_TEAM) {
			return qtrue;
		}
	}

	if (flags & CG_SHOW_ANYNONTEAMGAME) {
		if( cgs.gametype < GT_TEAM) {
			return qtrue;
		}
	}

	if (flags & CG_SHOW_HEALTHCRITICAL) {
		if (cg.snap->ps.stats[STAT_HEALTH] < 25) {
			return qtrue;
		}
	}

	if (flags & CG_SHOW_HEALTHOK) {
		if (cg.snap->ps.stats[STAT_HEALTH] >= 25) {
			return qtrue;
		}
	}

	if (flags & CG_SHOW_SINGLEPLAYER) {
		if( cgs.gametype == GT_SINGLE_PLAYER ) {
			return qtrue;
		}
	}

	if (flags & CG_SHOW_TOURNAMENT) {
		if( cgs.gametype == GT_DUEL ) {
			return qtrue;
		}
	}

	if (flags & CG_SHOW_DURINGINCOMINGVOICE) {
	}

	if (flags & CG_SHOW_WEAPON2){
		if( cg.snap->ps.weapon2)
			return qtrue;
	}
	return qfalse;
}



static void CG_DrawPlayerHasFlag(rectDef_t *rect, qboolean force2D) {
}

static void CG_DrawAreaSystemChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
  CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, systemChat, 0, 0, 0);
}

static void CG_DrawAreaTeamChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
  CG_Text_Paint(rect->x, rect->y + rect->h, scale, color,teamChat1, 0, 0, 0);
}

static void CG_DrawAreaChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
  CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, teamChat2, 0, 0, 0);
}

const char *CG_GetKillerText(void) {
	const char *s = "";
	if ( cg.killerName[0] ) {
		s = va("Killed by %s", cg.killerName );
	}
	return s;
}


static void CG_DrawKiller(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	// fragged by ... line
	if ( cg.killerName[0] ) {
		int x = rect->x + rect->w / 2;
	  CG_Text_Paint(x - CG_Text_Width(CG_GetKillerText(), scale, 0) / 2, rect->y + rect->h, scale, color, CG_GetKillerText(), 0, 0, textStyle);
	}

}


static void CG_DrawCapFragLimit(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
	int limit = (cgs.gametype >= GT_RTP) ? cgs.scorelimit : cgs.fraglimit;
	if(cgs.gametype == GT_DUEL)
		limit = cgs.duellimit;

	CG_Text_Paint(rect->x, rect->y, scale, color, va("%2i", limit),0, 0, textStyle);
}

static void CG_Draw1stPlace(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
	if (cgs.scores1 != SCORE_NOT_PRESENT) {
		CG_Text_Paint(rect->x, rect->y, scale, color, va("%2i", cgs.scores1),0, 0, textStyle);
	}
}

static void CG_Draw2ndPlace(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
	if (cgs.scores2 != SCORE_NOT_PRESENT) {
		CG_Text_Paint(rect->x, rect->y, scale, color, va("%2i", cgs.scores2),0, 0, textStyle);
	}
}

const char *CG_GetGameStatusText(void) {
	const char *s = "";
	if ( cgs.gametype < GT_TEAM) {
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),cg.snap->ps.persistant[PERS_SCORE] );
		}
	} else {
		s = va("%i : %i", cg.teamScores[0], cg.teamScores[1] );
	}
	return s;
}

static void CG_DrawGameStatus(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GetGameStatusText(), 0, 0, textStyle);
}

const char *CG_GameTypeString(void) {
	if ( cgs.gametype == GT_FFA ) {
		return "Deathmatch";
	} else if ( cgs.gametype == GT_TEAM ) {
		return "Team Deathmatch";
	} else if ( cgs.gametype == GT_DUEL) {
		return "Duel";
	} else if( cgs.gametype == GT_RTP) {
		return "Round Teamplay";
	} else if( cgs.gametype == GT_BR ){
		return "Bank Robbery";
	}
	return "";
}
static void CG_DrawGameType(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GameTypeString(), 0, 0, textStyle);
}

static void CG_Text_Paint_Limit(float *maxX, float x, float y, float scale, vec4_t color, const char* text, float adjust, int limit) {
  int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
  if (text) {
		const char *s = text;
		float max = *maxX;
		float useScale;
		fontInfo_t *font = &cgDC.Assets.textFont;
		if (scale <= cg_smallFont.value) {
			font = &cgDC.Assets.smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &cgDC.Assets.bigFont;
		}
		useScale = scale * font->glyphScale;
		trap_R_SetColor( color );
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
			if ( Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			} else {
	      float yadj = useScale * glyph->top;
				if (CG_Text_Width(s, useScale, 1) + x > max) {
					*maxX = 0;
					break;
				}
		    CG_Text_PaintChar(x, y - yadj,
			                    glyph->imageWidth,
				                  glyph->imageHeight,
					                useScale,
						              glyph->s,
							            glyph->t,
								          glyph->s2,
									        glyph->t2,
										      glyph->glyph);
	      x += (glyph->xSkip * useScale) + adjust;
				*maxX = x;
				count++;
				s++;
	    }
		}
	  trap_R_SetColor( NULL );
  }

}



#define PIC_WIDTH 12

void CG_DrawTeamSpectators(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
	if (cg.spectatorLen) {
		float maxX;
		vec4_t newColor ;

		if (cg.spectatorWidth == -1) {
			cg.spectatorWidth = 0;
			cg.spectatorPaintX = rect->x + 1;
			cg.spectatorPaintX2 = -1;
		}

		if (cg.spectatorOffset > cg.spectatorLen) {
			cg.spectatorOffset = 0;
			cg.spectatorPaintX = rect->x + 1;
			cg.spectatorPaintX2 = -1;
			Vector4Copy( color, cg.spectatorCurrentColor );
		}

		if (cg.time > cg.spectatorTime) {
			cg.spectatorTime = cg.time + 10;
			if (cg.spectatorPaintX <= rect->x + 2) { // Left border is reached
				if (cg.spectatorOffset < cg.spectatorLen) {
					// A spectator name is about to be shrinked
					while ( Q_IsColorString( &cg.spectatorList[cg.spectatorOffset])
						&& cg.spectatorOffset+2 < cg.spectatorLen ) {
						// Handle found colors
						memcpy( newColor, g_color_table[ColorIndex(cg.spectatorList[cg.spectatorOffset+1])], sizeof( newColor ) );
						newColor[3] = color[3];
						Vector4Copy( newColor, cg.spectatorCurrentColor );
						cg.spectatorOffset+=2;
					}
					if (cg.spectatorOffset < cg.spectatorLen) {
						cg.spectatorPaintX += CG_Text_Width(&cg.spectatorList[cg.spectatorOffset], scale, 1) - 1;
						cg.spectatorOffset++;
					}
				} else {
					cg.spectatorOffset = 0;
					if (cg.spectatorPaintX2 >= 0) {
						cg.spectatorPaintX = cg.spectatorPaintX2;
					} else {
						cg.spectatorPaintX = rect->x + rect->w - 2;
					}
					cg.spectatorPaintX2 = -1;
					Vector4Copy( color, cg.spectatorCurrentColor );
				}
			} else {
				// Move list to the left
				cg.spectatorPaintX--;
				if (cg.spectatorPaintX2 >= 0) {
					cg.spectatorPaintX2--;
				}
			}
		}

		maxX = rect->x + rect->w - 2;
		CG_Text_Paint_Limit(&maxX, cg.spectatorPaintX, rect->y + rect->h - 3, scale, cg.spectatorCurrentColor, &cg.spectatorList[cg.spectatorOffset], 0, 0);
		if (cg.spectatorPaintX2 >= 0) {
			float maxX2 = rect->x + rect->w - 2;
			CG_Text_Paint_Limit(&maxX2, cg.spectatorPaintX2, rect->y + rect->h - 3, scale, color, cg.spectatorList, 0, cg.spectatorOffset);
		}
		if (cg.spectatorOffset && maxX > 0) {
			// if we have an offset ( we are skipping the first part of the string ) and we fit the string
			if (cg.spectatorPaintX2 == -1) {
						cg.spectatorPaintX2 = rect->x + rect->w - 2;
			}
		} else {
			cg.spectatorPaintX2 = -1;
		}

	}
}

static float colors[5][4] = {
	{ 1.0f, 1.0f, 1.0f, 1.0f } ,		// normal
	{ 1.0f, 1.0f, 1.0f, 0.75f },
	{ 1.0f, 1.0f, 1.0f, 0.5f},
	{ 1.0f, 1.0f, 1.0f, 0.1f},
	{ 0.2f, 1.0f, 0.05f, 0.5f},
};

static void CG_DrawHoldables(int rectx, int recty){
	int x;

	x = rectx;

	// if in duel mode draw stars instead of items
	if(cgs.gametype != GT_DUEL){

		if(cg.snap->ps.stats[STAT_ARMOR]){
			gitem_t	*item = BG_FindItem("Boiler Plate");

			CG_FillRect(x, recty, 32, 32, colors[3]);
			CG_DrawPic(x, recty, 32, 32, cg_items[ITEM_INDEX(item)].icon);
			x += 32;
		}

		if(cg.snap->ps.powerups[PW_BELT]){
			gitem_t *item = BG_FindItem("Ammunition Belt");

			CG_FillRect(x, recty, 32, 32, colors[3]);
			CG_DrawPic(x, recty, 32, 32, cg_items[ITEM_INDEX(item)].icon);
			x += 32;
		}

		if(cg.snap->ps.powerups[PW_GOLD] && cgs.gametype == GT_BR){
			gitem_t *item = BG_FindItem("Moneybag");

			CG_FillRect(x, recty, 32, 32, colors[3]);
			CG_DrawPic(x, recty, 32, 32, cg_items[ITEM_INDEX(item)].icon);
			x += 32;
		}
	} else {

#define	MEDAL_START_TIME 1250
#define MEDAL_SIZE 19
#define MEDAL_MAX_SIZE 150

		int	i;
		float y = 480-34, newx = 320 + 14;
		qhandle_t cm = cgs.media.du_medal;
		vec3_t origin, angles, mins, maxs;
		float len;

		// offset the origin y and z to center the star
		trap_R_ModelBounds( cm, mins, maxs );

		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * ( maxs[2] - mins[2] );
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		VectorClear(angles);
		angles[1] = (cg.time/4)%360;

		for( i=0; i < cg.snap->ps.stats[STAT_WINS]; i++){
			float size = MEDAL_SIZE;

			if(i == cg.snap->ps.stats[STAT_WINS]-1){

				// a new star has to be added,
				// only show this sequence if it's a star of this round
				if(cg.lastmedals+1 == cg.snap->ps.stats[STAT_WINS] &&
					cg.medaltime < cg.time && cg.round == cg.oldround){
					cg.medaltime = cg.time + MEDAL_TIME;

					if(cg.snap->ps.stats[STAT_WINS] == 5){
						trap_S_StartSound(NULL, cg.snap->ps.clientNum,
							CHAN_ANNOUNCER,
							cgs.media.du_won_snd);
					} else {
						trap_S_StartSound(NULL,
							cg.snap->ps.clientNum,
							CHAN_ANNOUNCER, cgs.media.du_medal_snd);
					}
				}

				// calculate new position
				if(cg.medaltime >= cg.time){
					int delta = cg.medaltime - cg.time;
					delta = MEDAL_TIME - delta;

					angles[1] = (cg.time/2)%360;

					// the first 1/4 of MEDAL_TIME : get bigger
					if(delta <= MEDAL_START_TIME){
						float factor = MEDAL_START_TIME;

						factor = (float)(delta/factor);
						size = factor*MEDAL_MAX_SIZE;

						newx = 320-size/2;
						y = 240-size/2;

					} else {
						float factor = (MEDAL_TIME-MEDAL_START_TIME);
						float xdist = 320 - MEDAL_MAX_SIZE/2;
						float ydist = 240 - MEDAL_MAX_SIZE/2;

						delta -= MEDAL_START_TIME;

						factor = (float)(delta/factor);
						factor = factor*factor;
						// star is shrinking
						size = MEDAL_MAX_SIZE-factor*(MEDAL_MAX_SIZE-MEDAL_SIZE);

						newx = xdist - factor*(xdist-newx);
						y = ydist - factor*(ydist-y);
					}
				}
			}

			CG_Draw3DModel( newx, y, size, size, cm, 0, origin, angles );
			newx += MEDAL_SIZE;
		}

		cg.lastmedals = cg.snap->ps.stats[STAT_WINS];
		cg.oldround = cg.round;
	}
}

static void CG_DrawAmmoWeapon1(int rectx, int recty){
	int clip;
	int ammo;
	int weapon = cg.snap->ps.weapon;
	playerState_t *ps = &cg.snap->ps;
	int y = recty;
	int value;

	if(cg.snap->ps.weapon2)
		weapon = cg.snap->ps.weapon2;

	clip = bg_weaponlist[weapon].clip;
	ammo = weapon;

	if(clip) value = ps->ammo[clip];
	else value = ps->ammo[ammo];

	if ( value != -1 ) {

		if(clip){
			qhandle_t bullet;
			int	i, distance;
			int height, width;

			switch(weapon){
			case WP_GATLING:
			case WP_SHARPS:
				bullet = cgs.media.hud_ammo_sharps;
				width = 69;
				break;
			case WP_WINCHESTER66:
			case WP_LIGHTNING:
				bullet = cgs.media.hud_ammo_cart;
				width = 54;
				break;
			case WP_REMINGTON_GAUGE:
			case WP_SAWEDOFF:
			case WP_WINCH97:
				bullet = cgs.media.hud_ammo_shell;
				width = 42;
				break;
			default:
				bullet = cgs.media.hud_ammo_bullet;
				width = 43;
				break;
			}

			height = 13;
			value = ps->ammo[weapon];
			y -= height;

			y -= height/5;

			//calculate the distance
			distance = height/5;

			trap_R_SetColor( colors[1] );

			//real ammo, shown by bullets
			for(i = 0; i < value; i++){

				if(y < 0)
					break;

				CG_DrawPic(rectx-width-5, y, width, height, bullet);

				y -= height;

				y -= distance;
			}
			trap_R_SetColor(NULL );
		}
	}
}

static void CG_DrawAmmoWeapon2(int rectx, int recty){
	int clip;
	int ammo;
	int weapon = cg.snap->ps.weapon;
	playerState_t *ps = &cg.snap->ps;
	int y = recty;
	int value;

	// 2nd weapon
	if(cg.snap->ps.weapon2){

		weapon = cg.snap->ps.weapon;
		ammo = (cg.snap->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL) ? WP_AKIMBO : weapon;
		clip = bg_weaponlist[weapon].clip;

		if(clip) value = ps->ammo[clip];
		else value = ps->ammo[ammo];

		if ( value != -1 ) {

			if(clip){
				qhandle_t	bullet;
				int	i, distance;
				int height, width;

				switch(weapon){
				case WP_GATLING:
				case WP_SHARPS:
					bullet = cgs.media.hud_ammo_sharps;
					width = 69;
					break;
				case WP_WINCHESTER66:
				case WP_LIGHTNING:
				case WP_WINCH97:
					bullet = cgs.media.hud_ammo_cart;
					width = 54;
					break;
				case WP_REMINGTON_GAUGE:
				case WP_SAWEDOFF:
					bullet = cgs.media.hud_ammo_shell;
					width = 42;
					break;
				default:
					bullet = cgs.media.hud_ammo_bullet;
					width = 43;
					break;
				}

				height = 13;
				value = ps->ammo[ammo];
				y -= height;

				y -= height/5;

				//calculate the distance
				distance = height/5;

				trap_R_SetColor( colors[1] );

				//real ammo, shown by bullets
				for(i = 0; i < value; i++){

					if(y < 0)
						break;

					CG_DrawPic(rectx+5, y, width, height, bullet);

					y -= height;

					y -= distance;
				}
				trap_R_SetColor(NULL );
			}
		}
	}
}

//
void Item_Text_Paint(itemDef_t *item);
extern menu_items_t menu_items[MAX_MENU_ITEMS];
extern menuDef_t *menuItem;
extern int menuItemCount;

void CG_OwnerDraw(itemDef_t *item, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle) {
	rectDef_t rect;
	int dyn_y;
	gitem_t *buyitem;
	int	j, score;
	int value;
	static float color2[4] = {1.0f, 1.0f, 1.0f, 0.6f};
	static float colorgold[4] = {1.0f, 0.75f, 0.0f, 0.8f};
	static float colors[5][4] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 1.0f },
		{ 1.0f, 0.8f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 0.6f },
		{ 0.5f, 0.5f, 0.5f, 1.0f },
	};

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	//if (ownerDrawFlags != 0 && !CG_OwnerDrawVisible(ownerDrawFlags)) {
	//	return;
	//}

  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;

  switch (ownerDraw) {
	case CG_FIELD_HEALTH:
		value = cg.snap->ps.stats[STAT_HEALTH];
		if(value < 0)
			value = 0;
		CG_DrawField(rect.x, rect.y, 3, value);
		break;
	case CG_FIELD_MONEY:
		if(cg.snap->ps.stats[STAT_FLAGS] & SF_CANBUY)
			trap_R_SetColor(colorgold);
		else
			trap_R_SetColor(color2);
		CG_DrawMoneyField (rect.x, rect.y, 4, cg.snap->ps.stats[STAT_MONEY]);
		//trap_R_SetColor(NULL);
		break;
	case CG_FIELD_SCORE1:
		trap_R_SetColor(color2);
		if(cgs.gametype >= GT_TEAM) {
			score = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) ? cgs.scores1 : cgs.scores2;
		}
		else {
			score = cg.snap->ps.persistant[PERS_SCORE];
		}
		if(!(cgs.gametype == GT_DUEL && cg.snap->ps.pm_flags & PMF_FOLLOW &&
				cg.snap->ps.pm_type != PM_CHASECAM) && score != SCORE_NOT_PRESENT) {
			CG_DrawMoneyField (rect.x, rect.y, 2, score);
		}
		break;
	case CG_FIELD_SCORE2:
		if(cgs.gametype >= GT_TEAM) {
			score = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) ? cgs.scores2 : cgs.scores1;
		}
		else {
			score = (cg.snap->ps.persistant[PERS_SCORE] != cgs.scores1) ? cgs.scores1 : cgs.scores2;
		}
		if(!(cgs.gametype == GT_DUEL && cg.snap->ps.pm_flags & PMF_FOLLOW &&
				cg.snap->ps.pm_type != PM_CHASECAM) && score != SCORE_NOT_PRESENT) {
			CG_DrawMoneyField (rect.x, rect.y, 2, score);
		}
		trap_R_SetColor(NULL);
		break;
	case CG_HOLDABLES:
		CG_DrawHoldables(rect.x, rect.y);
		break;
	case CG_FIELD_AMMO:
		if(cg.snap->ps.ammo[bg_weaponlist[cg.snap->ps.weapon].clip] != -1 && bg_weaponlist[cg.snap->ps.weapon].clip)
			CG_DrawField(rect.x, rect.y, 2, cg.snap->ps.ammo[bg_weaponlist[cg.snap->ps.weapon].clip]);
		else if(cg.snap->ps.ammo[bg_weaponlist[cg.snap->ps.weapon].clip] != -1)
			CG_DrawField(rect.x, rect.y, 2, cg.snap->ps.ammo[cg.snap->ps.weapon]);
		else
			CG_DrawField(rect.x, rect.y, 2, 1);
		break;
	case CG_AMMO_WEAPON1:
		CG_DrawAmmoWeapon1(rect.x, rect.y);
		break;
	case CG_AMMO_WEAPON2:
		CG_DrawAmmoWeapon2(rect.x, rect.y);
		break;

	// buymenu
	case CG_BUTTON_PISTOLS:
	case CG_BUTTON_RIFLES:
	case CG_BUTTON_SHOTGUNS:
	case CG_BUTTON_MACHINEGUNS:
	case CG_BUTTON_MISC:
	case CG_BUTTON_PRIM_AMMO:
	case CG_BUTTON_SEC_AMMO:
		CG_DrawPic(0, rect.y, 128, 24, cgs.media.menu_tex);
		if(cg.menustat == ownerDraw - CG_BUTTON_PISTOLS + 1)
			CG_DrawPic(16, rect.y, 96, 24, cgs.media.hud_button_selected);

		if(ownerDraw == CG_BUTTON_PRIM_AMMO || ownerDraw == CG_BUTTON_SEC_AMMO){
			int belt = 1;
			int weapon = BG_FindPlayerWeapon(WP_WINCHESTER66, WP_DYNAMITE, &cg.snap->ps);
			if(cg.snap->ps.powerups[PW_BELT]) belt = 2;
			if(ownerDraw == CG_BUTTON_PRIM_AMMO) weapon = WP_REM58;

			if(cgs.gametype == GT_DUEL)
				Vector4Copy(colors[4], item->window.foreColor);
			else if(cg.snap->ps.ammo[bg_weaponlist[weapon].clip] >= bg_weaponlist[weapon].maxAmmo*belt)
				Vector4Copy(colors[2], item->window.foreColor);
			else if(cg.snap->ps.stats[STAT_MONEY] < 5)
				Vector4Copy(colors[1], item->window.foreColor);
			else
				Vector4Copy(colors[0], item->window.foreColor);
		}

		Item_Text_Paint(item);

		// draw the number
		trap_R_SetColor(color2);
		CG_DrawPic(0, rect.y+2, 16, 20,
			cgs.media.numbermoneyShaders[ownerDraw-CG_BUTTON_PISTOLS+1]);
		trap_R_SetColor(NULL);
		break;

	case CG_BUTTON1:
	case CG_BUTTON2:
	case CG_BUTTON3:
	case CG_BUTTON4:
	case CG_BUTTON5:
	case CG_BUTTON6:
		CG_DrawPic(128, rect.y, 192, 24, cgs.media.menu_tex);
		if(cg.menuitem == ownerDraw - CG_BUTTON1 + 1)
			CG_DrawPic(128+24, rect.y, 144, 24, cgs.media.hud_button_selected);

		item->text = menu_items[ownerDraw-CG_BUTTON1].string;

		if(menu_items[ownerDraw-CG_BUTTON1].inventory)
			Vector4Copy(colors[2], item->window.foreColor);
		else if(!menu_items[ownerDraw-CG_BUTTON1].money)
			Vector4Copy(colors[1], item->window.foreColor);
		else
			Vector4Copy(colors[0], item->window.foreColor);

		Item_Text_Paint(item);

		// draw the number
		trap_R_SetColor(color2);
		CG_DrawPic(128+4, rect.y+2, 16, 20,
			cgs.media.numbermoneyShaders[ownerDraw-CG_BUTTON1+1]);
		trap_R_SetColor(NULL);
		break;

	case CG_ITEMBOX:
		// first draw the info
		buyitem = bg_itemlist+1;

		for(j = 0; buyitem->classname; buyitem++){
			const int width = rect.w-2*26;
			int height;
			int newy;

			if ( buyitem->weapon_sort != cg.menustat )
				continue;
			j++;

			if( j != cg.menuitem)
				continue;

			height = width/buyitem->xyrelation;

			newy = y + 33;

			// first draw the background
			CG_FillRect(rect.x, rect.y, rect.w, height + 33 + 10 + 25 + 24, colors[3]);

			if(buyitem->giType == IT_WEAPON){
				CG_RegisterWeapon( buyitem->giTag );
				CG_DrawPic(x+26, newy, width, height, cg_weapons[buyitem->giTag].weaponIcon);
			} else {
				CG_RegisterItemVisuals(ITEM_INDEX(buyitem));
				CG_DrawPic(x+26, newy, width, height, cg_items[ITEM_INDEX(buyitem)].icon);
			}

			newy += height + 10;

			//different colors depending on money, the player has
			if(menu_items[cg.menuitem-1].inventory){
				CG_Text_Paint(x+35, newy+10, 0.28f, colors[2], va("cost: $%i", buyitem->prize), 0, -1, 0);
				CG_Text_Paint(x+35, newy+25, 0.28f, colors[2], "You own this item", 0, -1, 0);
			} else if(!menu_items[cg.menuitem-1].money) {
				CG_Text_Paint(x+35, newy+10, 0.28f, colors[1], va("cost: $%i", buyitem->prize), 0, -1, 0);
				CG_Text_Paint(x+35, newy+25, 0.28f, colors[1], "Not enough money", 0, -1, 0);
			} else {
				CG_Text_Paint(x+35, newy+10, 0.28f, colors[0], va("cost: $%i", buyitem->prize), 0, -1, 0);
				CG_Text_Paint(x+35, newy+25, 0.28f, colors[0], "Press MOUSE1 to buy", 0, -1, 0);
			}

			newy += 25 + 24;

			height = newy - rect.y;

			// then draw the box
			CG_DrawPic(rect.x, rect.y, rect.w, 28, cgs.media.menu_top);
			for(dyn_y=rect.y+28; dyn_y-rect.y+24 < height; dyn_y += 24){
				CG_DrawPic(rect.x, dyn_y, rect.w, 24, cgs.media.menu_tex);
			}
			CG_DrawPic(rect.x, dyn_y, rect.w, 24, cgs.media.menu_end);

			break;
		}

		break;
	case CG_ROUNDTIME:
		CG_DrawRoundtime(&rect, scale, color, shader, textStyle);
		break;

  case CG_PLAYER_ARMOR_ICON:
    CG_DrawPlayerArmorIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_ARMOR_ICON2D:
    CG_DrawPlayerArmorIcon(&rect, qtrue);
    break;
  case CG_PLAYER_ARMOR_VALUE:
    CG_DrawPlayerArmorValue(&rect, scale, color, shader, textStyle);
    break;
  case CG_PLAYER_AMMO_ICON:
    CG_DrawPlayerAmmoIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_AMMO_ICON2D:
    CG_DrawPlayerAmmoIcon(&rect, qtrue);
    break;
  case CG_PLAYER_AMMO_VALUE:
    CG_DrawPlayerAmmoValue(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_HEAD:
    CG_DrawSelectedPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY, qfalse);
    break;
  case CG_VOICE_HEAD:
    CG_DrawSelectedPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY, qtrue);
    break;
  case CG_VOICE_NAME:
    CG_DrawSelectedPlayerName(&rect, scale, color, qtrue, textStyle);
    break;
  case CG_SELECTEDPLAYER_STATUS:
    CG_DrawSelectedPlayerStatus(&rect);
    break;
  case CG_SELECTEDPLAYER_ARMOR:
    CG_DrawSelectedPlayerArmor(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_HEALTH:
    CG_DrawSelectedPlayerHealth(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_NAME:
    CG_DrawSelectedPlayerName(&rect, scale, color, qfalse, textStyle);
    break;
  case CG_SELECTEDPLAYER_LOCATION:
    CG_DrawSelectedPlayerLocation(&rect, scale, color, textStyle);
    break;
  case CG_SELECTEDPLAYER_WEAPON:
    CG_DrawSelectedPlayerWeapon(&rect);
    break;
  case CG_SELECTEDPLAYER_POWERUP:
    break;
  case CG_PLAYER_HEAD:
    CG_DrawPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_ITEM:
    CG_DrawPlayerItem(&rect, scale, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_SCORE:
    CG_DrawPlayerScore(&rect, scale, color, shader, textStyle);
    break;
  case CG_PLAYER_HEALTH:
    CG_DrawPlayerHealth(&rect, scale, color, shader, textStyle);
    break;
  case CG_RED_SCORE:
    CG_DrawRedScore(&rect, scale, color, shader, textStyle);
    break;
  case CG_BLUE_SCORE:
    CG_DrawBlueScore(&rect, scale, color, shader, textStyle);
    break;
  case CG_RED_NAME:
    CG_DrawRedName(&rect, scale, color, textStyle);
    break;
  case CG_BLUE_NAME:
    CG_DrawBlueName(&rect, scale, color, textStyle);
    break;
  case CG_BLUE_FLAGHEAD:
  case CG_BLUE_FLAGSTATUS:
  case CG_BLUE_FLAGNAME:
  case CG_RED_FLAGHEAD:
  case CG_RED_FLAGSTATUS:
  case CG_RED_FLAGNAME:
  case CG_HARVESTER_SKULLS:
  case CG_HARVESTER_SKULLS2D:
  case CG_ONEFLAG_STATUS:
    break;
  case CG_PLAYER_LOCATION:
    CG_DrawPlayerLocation(&rect, scale, color, textStyle);
    break;
  case CG_TEAM_COLOR:
    CG_DrawTeamColor(&rect, color);
    break;
  case CG_CTF_POWERUP:
  case CG_AREA_POWERUP:
    break;
  case CG_PLAYER_STATUS:
    CG_DrawPlayerStatus(&rect);
    break;
  case CG_PLAYER_HASFLAG:
    CG_DrawPlayerHasFlag(&rect, qfalse);
    break;
  case CG_PLAYER_HASFLAG2D:
    CG_DrawPlayerHasFlag(&rect, qtrue);
    break;
  case CG_AREA_SYSTEMCHAT:
    CG_DrawAreaSystemChat(&rect, scale, color, shader);
    break;
  case CG_AREA_TEAMCHAT:
    CG_DrawAreaTeamChat(&rect, scale, color, shader);
    break;
  case CG_AREA_CHAT:
    CG_DrawAreaChat(&rect, scale, color, shader);
    break;
  case CG_GAME_TYPE:
    CG_DrawGameType(&rect, scale, color, shader, textStyle);
    break;
  case CG_GAME_STATUS:
    CG_DrawGameStatus(&rect, scale, color, shader, textStyle);
		break;
  case CG_KILLER:
    CG_DrawKiller(&rect, scale, color, shader, textStyle);
		break;
	case CG_ACCURACY:
	case CG_ASSISTS:
	case CG_DEFEND:
	case CG_EXCELLENT:
	case CG_IMPRESSIVE:
	case CG_PERFECT:
	case CG_GAUNTLET:
	case CG_CAPTURES:
		break;
  case CG_SPECTATORS:
		CG_DrawTeamSpectators(&rect, scale, color, shader);
		break;
  case CG_TEAMINFO:
		break;
  case CG_CAPFRAGLIMIT:
    CG_DrawCapFragLimit(&rect, scale, color, shader, textStyle);
		break;
  case CG_1STPLACE:
    CG_Draw1stPlace(&rect, scale, color, shader, textStyle);
		break;
  case CG_2NDPLACE:
    CG_Draw2ndPlace(&rect, scale, color, shader, textStyle);
		break;
  default:
    break;
  }
}

void CG_MouseEvent(int x, int y) {
	int n;

	if ( ((cg.predictedPlayerState.pm_type == PM_NORMAL || cg.predictedPlayerState.pm_type == PM_SPECTATOR) && cg.showScores == qfalse)
		&& !cg.menu) {
		trap_Key_SetCatcher(0);
		return;
	}

	cgs.cursorX+= x;
	if (cgs.cursorX < 0)
		cgs.cursorX = 0;
	else if (cgs.cursorX > 640)
		cgs.cursorX = 640;

	cgs.cursorY += y;
	if (cgs.cursorY < 0)
		cgs.cursorY = 0;
	else if (cgs.cursorY > 480)
		cgs.cursorY = 480;

	n = Display_CursorType(cgs.cursorX, cgs.cursorY);
	cgs.activeCursor = 0;
	if (n == CURSOR_ARROW) {
		cgs.activeCursor = cgs.media.selectCursor;
	} else if (n == CURSOR_SIZER) {
		cgs.activeCursor = cgs.media.sizeCursor;
	}

  if (cgs.capturedItem) {
	  Display_MouseMove(cgs.capturedItem, x, y);
  } else {
	  Display_MouseMove(NULL, cgs.cursorX, cgs.cursorY);
  }

}

/*
==================
CG_HideTeamMenus
==================

*/
void CG_HideTeamMenu( void ) {
  Menus_CloseByName("teamMenu");
  Menus_CloseByName("getMenu");
}

/*
==================
CG_ShowTeamMenus
==================

*/
void CG_ShowTeamMenu( void ) {
  Menus_OpenByName("teamMenu");
}




/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
void CG_EventHandling(int type) {
	cgs.eventHandling = type;
  if (type == CGAME_EVENT_NONE) {
    CG_HideTeamMenu();
  } else if (type == CGAME_EVENT_TEAMMENU) {
    //CG_ShowTeamMenu();
  } else if (type == CGAME_EVENT_SCOREBOARD) {
  }

}



void CG_KeyEvent(int key, qboolean down) {
	if(cg.oldbutton && !down){
		cg.oldbutton = qfalse;
	}

	if (!down) {
		return;
	}

	if ( (cg.predictedPlayerState.pm_type == PM_NORMAL || (cg.predictedPlayerState.pm_type == PM_SPECTATOR && cg.showScores == qfalse))
		&& !cg.menu) {
		CG_EventHandling(CGAME_EVENT_NONE);
		trap_Key_SetCatcher(0);
		return;
	}

	//if (key == trap_Key_GetKey("teamMenu") || !Display_CaptureItem(cgs.cursorX, cgs.cursorY)) {
    // if we see this then we should always be visible
	//  CG_EventHandling(CGAME_EVENT_NONE);
	//  trap_Key_SetCatcher(0);
	//}



	Display_HandleKey(key, down, cgs.cursorX, cgs.cursorY);

	if (cgs.capturedItem) {
		cgs.capturedItem = NULL;
	}	else {
		if (key == K_MOUSE2 && down) {
			cgs.capturedItem = Display_CaptureItem(cgs.cursorX, cgs.cursorY);
		}
	}
}

int CG_ClientNumFromName(const char *p) {
  int i;
  for (i = 0; i < cgs.maxclients; i++) {
    if (cgs.clientinfo[i].infoValid && Q_stricmp(cgs.clientinfo[i].name, p) == 0) {
      return i;
    }
  }
  return -1;
}

void CG_ShowResponseHead(void) {
	Menus_OpenByName("voiceMenu");
	trap_Cvar_Set("cl_conXOffset", "72");
	cg.voiceTime = cg.time;
}

void CG_RunMenuScript(char **args) {
}


void CG_GetTeamColor(vec4_t *color) {
  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
    (*color)[0] = 1.0f;
    (*color)[3] = 0.25f;
    (*color)[1] = (*color)[2] = 0.0f;
  } else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
    (*color)[0] = (*color)[1] = 0.0f;
    (*color)[2] = 1.0f;
    (*color)[3] = 0.25f;
  } else {
    (*color)[0] = (*color)[2] = 0.0f;
    (*color)[1] = 0.17f;
    (*color)[3] = 0.25f;
	}
}

