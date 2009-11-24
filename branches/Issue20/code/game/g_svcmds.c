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
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "bg_local.h"


/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static qbool StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];

	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}

	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
					break;
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}

		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		m[i] = 255;

		if (!*s)
			break;
		s++;
	}

	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	byte	m[4];
	int		i,j;
	char	iplist_final[MAX_CVAR_VALUE_STRING];
	char	ip[64];

	*iplist_final = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		*(unsigned *)m = ipFilters[i].mask;
		*ip = 0;
		for (j = 0 ; j < 4 ; j++)
		{
			if (m[j]!=255)
				Q_strcat(ip, sizeof(ip), "*");
			else
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			Q_strcat(ip, sizeof(ip), (j<3) ? "." : " ");
		}		
		if (strlen(iplist_final)+strlen(ip) < MAX_CVAR_VALUE_STRING)
		{
			Q_strcat( iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf("g_banIPs overflowed at MAX_CVAR_VALUE_STRING\n");
			break;
		}
	}

	trap_Cvar_Set( "g_banIPs", iplist_final );
}

/*
=================
G_FilterPacket
=================
*/
qbool G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}

	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}

	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void)
{
	char *s, *t;
	char		str[MAX_CVAR_VALUE_STRING];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f(char *cmd)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f(char *cmd)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f(char *cmd) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BREAKABLE:
			G_Printf("ET_BREAKABLE		  ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( char *cmd ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}

void Svcmd_KickBots_f(char *cmd){
	int i;
	gentity_t *ent;

	for(i=0;i<level.maxclients;i++){
		ent = &g_entities[i];

		if(ent->r.svFlags & SVF_BOT){
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick %i\n", i) );
		}
	}
}

void Svcmd_Mute( char *cmd )
{
  gclient_t *cl;
  char buf[MAX_TOKEN_CHARS];
  qbool mute;

  mute = ( !Q_stricmp( cmd, "mute" ) ) ? qtrue : qfalse;

  trap_Argv( 1, buf, sizeof(buf) );
  cl = ClientForString( buf );

  if( !cl )
    return;

  if( mute && (cl->pers.punished & AP_MUTED) )
  {
    G_LogPrintf( "%s is already muted\n" );
    return;
  }
  else if( !mute && !(cl->pers.punished & AP_MUTED) )
  {
    G_LogPrintf( "%s is not muted\n" );
    return;
  }

  if( !mute )
    cl->pers.punished &= ~AP_MUTED;
  else
    cl->pers.punished |= AP_MUTED;

  trap_SendServerCommand( cl->ps.clientNum,
      va( "print \"You have been ^1%s\n\"", (mute) ? "muted" : "unmuted" ) );
  G_LogPrintf( "%s has been %s\n", cl->pers.netname,
      (mute) ? "muted" : "unmuted" );
}

#define MAX_ARGC_CONFIG 7
static void Svcmd_GameConfig_f(char *cmd) {
  char args[MAX_ARGC_CONFIG][MAX_QPATH];
  char buf[MAX_STRING_CHARS];
  char filename_buf[MAX_QPATH];
  int i;
  int argc = trap_Argc();
  fileHandle_t f;
  const char* usage = "Usage:\n"
    "config [weapons|items|gravity] save\n"
    "config [weapons|items|gravity] load\n"
    "config gravity modify <variable> <value>\n"
    "config items modify <item file name> <variable> <value>\n"
    "config weapons modify <weapon name> <variable> <value>\n"
    "config weapons modify <weapon name> anim <variable> <value>\n"
    "weapon name is like WP_KNIFE\n"
    "item file name is like item_boiler_plate\n";

  if (!g_cheats.integer) {
    Com_Printf("Svcmd_GameConfig_f: Cheats off.\nTurn cheats on if you want to use this command\n");
    return;
  }

  if (argc < 2 || argc > MAX_ARGC_CONFIG)
    Com_Printf(usage);

  for (i = 0; i < argc; i++)
    trap_Argv(i, args[i], sizeof(args[i]));

  if (!Q_stricmp(args[1], "gravity")) {
    if (!Q_stricmp(args[2], "save")) {
      Com_Printf("Savig gravity config...\n");
      trap_FS_FOpenFile(Parse_FindFile(filename_buf, sizeof(filename_buf), 0, PFT_GRAVITY), &f, FS_WRITE);
      Com_sprintf(buf, sizeof(buf), "// %s created automatically at %i:%i:%i on %i/%i/%i\n\n",
          filename_buf, level.qtime.tm_hour, level.qtime.tm_min, level.qtime.tm_sec,
          level.qtime.tm_mday, level.qtime.tm_mon, level.qtime.tm_year);
      trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_stopspeed = \"%f\"\n", pm_stopspeed));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_duckScale = \"%f\"\n", pm_duckScale));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_swimScale = \"%f\"\n", pm_swimScale));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_wadeScale = \"%f\"\n", pm_wadeScale));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_ladderScale = \"%f\"\n", pm_ladderScale));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_reloadScale = \"%f\"\n", pm_reloadScale));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_accelerate = \"%f\"\n", pm_accelerate));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_airaccelerate = \"%f\"\n", pm_airaccelerate));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_wateraccelerate = \"%f\"\n", pm_wateraccelerate));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_flyaccelerate = \"%f\"\n", pm_flyaccelerate));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_ladderAccelerate = \"%f\"\n", pm_ladderAccelerate));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_friction = \"%f\"\n", pm_friction));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_waterfriction = \"%f\"\n", pm_waterfriction));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_flightfriction = \"%f\"\n", pm_flightfriction));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_spectatorfriction = \"%f\"\n", pm_spectatorfriction));
	  trap_FS_Write(buf, strlen(buf), f);
      Com_sprintf(buf, sizeof(buf), va("pm_ladderfriction = \"%f\"\n", pm_ladderfriction));
	  trap_FS_Write(buf, strlen(buf), f);
      trap_FS_FCloseFile(f);
      Com_Printf("Savig gravity config...Done\n");
    } else if (!Q_stricmp(args[2], "modify")) {
      if (argc < 4) {
        Com_Printf(usage);
		return;
      }
      if (!Q_stricmp(args[3], "pm_stopspeed")) 
        pm_stopspeed = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_duckScale"))
        pm_duckScale = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_swimScale"))
        pm_swimScale = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_wadeScale"))
        pm_wadeScale = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_ladderScale"))
        pm_ladderScale = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_reloadScale"))
        pm_reloadScale = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_accelerate"))
        pm_accelerate = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_airaccelerate"))
        pm_airaccelerate = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_wateraccelerate"))
        pm_wateraccelerate = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_flyaccelerate"))
        pm_flyaccelerate = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_ladderAccelerate"))
        pm_ladderAccelerate = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_friction"))
        pm_friction = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_waterfriction"))
        pm_waterfriction = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_flightfriction"))
        pm_flightfriction = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_spectatorfriction"))
        pm_spectatorfriction = atof(args[4]);
      else if (!Q_stricmp(args[3], "pm_ladderfriction"))
        pm_ladderfriction = atof(args[4]);
	  else
        Com_Printf("No such variable to modify in garvity - %\n", args[3]);
    } else if (!Q_stricmp(args[2], "load")) {
      Com_Printf("Loading gravity config...\n");
      config_GetInfos(PFT_GRAVITY);
      Com_Printf("Loading gravity config...Done\n");
    }
  } else if (!Q_stricmp(args[1], "items")) {
    if (!Q_stricmp(args[2], "save")) {
      gitem_t *item;
      Com_Printf("Savig items config...\n");
      for (item = &bg_itemlist[1]; ITEM_INDEX(item) < IT_NUM_ITEMS; item++) {
        trap_FS_FOpenFile(Parse_FindFile(filename_buf, sizeof(filename_buf), ITEM_INDEX(item), PFT_ITEMS), &f, FS_WRITE);
        Com_sprintf(buf, sizeof(buf), "// %s created automatically at %i:%i:%i on %i/%i/%i\n\n",
            filename_buf, level.qtime.tm_hour, level.qtime.tm_min, level.qtime.tm_sec,
            level.qtime.tm_mday, level.qtime.tm_mon, level.qtime.tm_year);
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("classname = \"%s\"\n", item->classname));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("pickup_sound = \"%s\"\n", item->pickup_sound));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("world_model = { \"%s\" \"%s\" \"%s\" \"%s\" }\n",
              item->world_model[0], item->world_model[1], item->world_model[2], item->world_model[3]));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("icon = \"%s\"\n", item->icon));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("xyrelation = \"%f\"\n", item->xyrelation));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("pickup_name = \"%s\"\n", item->pickup_name));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("quantity = \"%i\"\n", item->quantity));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("giType = \"%s\"\n", psf_itemTypes[item->giType]));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("giTag = \"%s\"\n", psf_weapons_config[item->giTag].numNames));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("prize = \"%i\"\n", item->prize));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("weapon_sort = \"%s\"\n", psf_weapon_buyType[item->weapon_sort]));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("precaches = \"%s\"\n", item->precaches));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("sounds = \"%s\"\n", item->sounds));
        trap_FS_Write(buf, strlen(buf), f);
        trap_FS_FCloseFile(f);
      }
      Com_Printf("Savig items config...Done\n");
	} else if (!Q_stricmp(args[2], "load")) {
      Com_Printf("Loading items config...\n");
      config_GetInfos(PFT_ITEMS);
      Com_Printf("Loading items config...Done\n");
	} else if (!Q_stricmp(args[2], "modify")) {
      if (argc < 5) {
        Com_Printf(usage);
		return;
      }
      BG_ItemListChange(args[3], args[4], args[5]);
      trap_SendItemsChanges(args[3],args[4],args[5]);
	}
  }	else if (!Q_stricmp(args[1], "weapons")) {
    if (!Q_stricmp(args[2], "save")) {
      Com_Printf("Savig weapons config...\n");
      for (i = 1; i < WP_NUM_WEAPONS; i++) {
        wpinfo_t *wpinfo = &bg_weaponlist[i];
        int n;
        trap_FS_FOpenFile(Parse_FindFile(filename_buf, sizeof(filename_buf), i, PFT_WEAPONS), &f, FS_WRITE);
        Com_sprintf(buf, sizeof(buf), "// %s created automatically at %i:%i:%i on %i/%i/%i\n\n",
            filename_buf, level.qtime.tm_hour, level.qtime.tm_min, level.qtime.tm_sec,
            level.qtime.tm_mday, level.qtime.tm_mon, level.qtime.tm_year);
        trap_FS_Write(buf, strlen(buf), f);
  
        for (n=WP_ANIM_CHANGE; n <= WP_ANIM_SPECIAL2; n++) {
          animation_t *anim = &wpinfo->animations[n];
          Com_sprintf(buf, sizeof(buf), va("%s\n", psf_weapon_animName[n]));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("firstFrame = \"%i\"\n", anim->firstFrame));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("numFrames = \"%i\"\n", anim->numFrames));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("loopFrames = \"%i\"\n", anim->loopFrames));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("frameLerp = \"%i\"\n", anim->frameLerp));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("initialLerp = \"%i\"\n", anim->initialLerp));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("reversed = \"%i\"\n", anim->reversed));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), va("flipflop = \"%i\"\n", anim->flipflop));
          trap_FS_Write(buf, strlen(buf), f);
          Com_sprintf(buf, sizeof(buf), "\n");
          trap_FS_Write(buf, strlen(buf), f);
        }
  
        Com_sprintf(buf, sizeof(buf), va("spread = \"%f\"\n", wpinfo->spread));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("damage = \"%f\"\n", wpinfo->damage));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("range = \"%i\"\n", wpinfo->range));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("addTime = \"%i\"\n", wpinfo->addTime));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("count = \"%i\"\n", wpinfo->count));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("clipAmmo = \"%i\"\n", wpinfo->clipAmmo));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("clip = \"%s\"\n", psf_weapons_config[wpinfo->clip].numNames));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("maxAmmo = \"%i\"\n", wpinfo->maxAmmo));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("v_model = \"%s\"\n", wpinfo->v_model));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("v_barrel = \"%s\"\n", wpinfo->v_barrel));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("snd_fire = \"%s\"\n", wpinfo->snd_fire));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("snd_reload = \"%s\"\n", wpinfo->snd_reload));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("name = \"%s\"\n", wpinfo->name));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("path = \"%s\"\n", wpinfo->path));
        trap_FS_Write(buf, strlen(buf), f);
        Com_sprintf(buf, sizeof(buf), va("wp_sort = \"%s\"\n", psf_weapon_sortNames[ wpinfo->wp_sort ]));
        trap_FS_Write(buf, strlen(buf), f);
        trap_FS_FCloseFile(f);
      }
      Com_Printf("Savig weapons config...Done\n");
    } else if (!Q_stricmp(args[2], "load")) {
      Com_Printf("Loading weapons config...\n");
      config_GetInfos(PFT_WEAPONS);
      Com_Printf("Loading weapons config...Done\n");
    } else if (!Q_stricmp(args[2], "modify")) {
      if (argc < 5) {
        Com_Printf(usage);
        return;
      }
      if (!Q_stricmp(args[4], "anim")) {
        if (argc < 7) {
          Com_Printf(usage);
          return;
        }
        BG_WeaponListChange(args[3], args[5], args[6], args[7]);
        trap_SendWeaponsChanges(args[3], args[5], args[6], args[7]);
      } else {
        BG_WeaponListChange(args[3], NULL, args[4], args[5]);
        trap_SendWeaponsChanges(args[3], NULL, args[4], args[5]);
      }
    }
  } else {
    Com_Printf(usage);
  }
}

void Svcmd_ListIPs_f( char *cmd ) {
  trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
}

char	*ConcatArgs( int start );

void Svcmd_Say_f( char *cmd ) {
  if (g_dedicated.integer) {
    if( !Q_stricmp (cmd, "say") )
      trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(1) ) );
      return;
    }
    else if( !Q_stricmp (cmd, "bigtext") || !Q_stricmp (cmd, "cp") ) {
      const char *arg = ConcatArgs(1);
      if ( Q_strncmp ("-1", arg, strlen(arg)) == 0 )
        trap_SendServerCommand( -1, va("cp \"%s\"", ConcatArgs(2) ) );
      else if ( arg[0]<'0' || arg[0]>'9' )
        trap_SendServerCommand( -1, va("cp \"%s\"", arg ) );
      else if (ClientForString(arg)) {
        int clientNum = atoi(arg);
        trap_SendServerCommand( clientNum, va("cp \"%s\"", ConcatArgs(2) ) );
      }
      return;
    }
    // everything else will also be printed as a say command
    trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(0) ) );
    return;
}

/*
=================
ConsoleCommand

=================
*/
typedef struct g_svcmds_gameCmds_s {
  char *name;
  void (*cmdHandler)(char *cmd);
} g_svcmds_gameCmds_t;

static g_svcmds_gameCmds_t consoleCmdsTable[] = {
  { "entitylist", Svcmd_EntityList_f },
  { "forceteam", Svcmd_ForceTeam_f },
  { "mute", Svcmd_Mute },
  { "unmute", Svcmd_Mute },
  { "config", Svcmd_GameConfig_f },
  { "addbot", Svcmd_AddBot_f },
  { "kickbots", Svcmd_KickBots_f },
  { "botlist", Svcmd_BotList_f },
  { "abort_podium", Svcmd_AbortPodium_f },
  { "addip", Svcmd_AddIP_f },
  { "removeip", Svcmd_RemoveIP_f },
  { "listip", Svcmd_ListIPs_f },
  { "say", Svcmd_Say_f },
};

int numConsoleCmds = sizeof( consoleCmdsTable ) / sizeof( consoleCmdsTable[0] );

qbool ConsoleCommand(void) {
	char	cmd[MAX_TOKEN_CHARS];
	int		i;

	trap_Argv(0, cmd, sizeof(cmd));

	for (i=0; i <= numConsoleCmds; i++)
		if(!Q_stricmp( consoleCmdsTable[i].name, cmd)) {
			consoleCmdsTable[i].cmdHandler(cmd);
			return qtrue;
		}

	return qfalse;
}

