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
void Svcmd_AddIP_f (void)
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
void Svcmd_RemoveIP_f (void)
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
void	Svcmd_EntityList_f (void) {
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
void	Svcmd_ForceTeam_f( void ) {
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

void Svcmd_KickBots_f(void){
	int i;
	gentity_t *ent;

	for(i=0;i<level.maxclients;i++){
		ent = &g_entities[i];

		if(ent->r.svFlags & SVF_BOT){
			trap_SendConsoleCommand( EXEC_INSERT, va("clientkick %i\n", i) );
		}
	}
}

void Svcmd_Mute( qbool mute )
{
  gclient_t *cl;
  char buf[MAX_TOKEN_CHARS];

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

void Svcmd_GameWeapon_f( void ) {
  char args[10][25];
  char buf[20000];
  int i, n, bsize;
  int argc = trap_Argc( );
  char *fileName;
  fileHandle_t f;
  wpinfo_t *wpinfo;
  animation_t *anim;


  if( !g_cheats.integer )
  {
    Com_Printf( "Svcmd_GameWeapon_f: Cheats off.\n" );
    return;
  }

  for( i = 0; i <= argc; i++ )
    trap_Argv( i, args[i], sizeof(args[i]) );

  bsize = sizeof(buf);

  if( !Q_stricmp( args[1], "help" ) ) {
    Com_Printf( "config:\n"
      "  help\n"
      "  weapons\n"
      "    save\n"
      "    load\n"
      "    modify\n"
      );
  }
  else if( !Q_stricmp( args[1], "weapons" ) ) {
    if( !Q_stricmp( args[2], "save" ) ) {
      for(i=1; i < WP_NUM_WEAPONS; i++ )
      {
        fileName = Weapons_FileForWeapon(i);
        trap_FS_FOpenFile( fileName, &f, FS_WRITE );

        Com_sprintf( buf, sizeof(buf), "// %s created automatically at %i:%i:%i on %i/%i/%i\n\n",
            fileName, level.qtime.tm_hour, level.qtime.tm_min, level.qtime.tm_sec,
            level.qtime.tm_mday, level.qtime.tm_mon, level.qtime.tm_year );

        wpinfo = &bg_weaponlist[i];

        for( n=WP_ANIM_CHANGE; n <= WP_ANIM_SPECIAL2; n++ ) {
          anim = &wpinfo->animations[n];
          Q_strcat( buf, bsize, va( "%s\n", weapon_animName[n] ) );
          Q_strcat( buf, bsize, va( "firstFrame = \"%i\"\n", anim->firstFrame ) );
          Q_strcat( buf, bsize, va( "numFrames = \"%i\"\n", anim->numFrames ) );
          Q_strcat( buf, bsize, va( "loopFrames = \"%i\"\n", anim->loopFrames ) );
          Q_strcat( buf, bsize, va( "frameLerp = \"%i\"\n", anim->frameLerp ) );
          Q_strcat( buf, bsize, va( "initialLerp = \"%i\"\n", anim->initialLerp ) );
          Q_strcat( buf, bsize, va( "reversed = \"%i\"\n", anim->reversed ) );
          Q_strcat( buf, bsize, va( "flipflop = \"%i\"\n", anim->flipflop ) );
          Q_strcat( buf, bsize, "\n" );
        }

        Q_strcat( buf, bsize, va( "spread = \"%f\"\n", wpinfo->spread ) );
        Q_strcat( buf, bsize, va( "damage = \"%f\"\n", wpinfo->damage ) );
        Q_strcat( buf, bsize, va( "range = \"%i\"\n", wpinfo->range ) );
        Q_strcat( buf, bsize, va( "addTime = \"%i\"\n", wpinfo->addTime ) );
        Q_strcat( buf, bsize, va( "count = \"%i\"\n", wpinfo->count ) );
        Q_strcat( buf, bsize, va( "clipAmmo = \"%i\"\n", wpinfo->clipAmmo ) );
        Q_strcat( buf, bsize, va( "clip = \"%s\"\n", weapon_numNames[ wpinfo->clip ] ) );
        Q_strcat( buf, bsize, va( "maxAmmo = \"%i\"\n", wpinfo->maxAmmo ) );
        Q_strcat( buf, bsize, va( "v_model = \"%s\"\n", wpinfo->v_model ) );
        Q_strcat( buf, bsize, va( "v_barrel = \"%s\"\n", wpinfo->v_barrel ) );
        Q_strcat( buf, bsize, va( "snd_fire = \"%s\"\n", wpinfo->snd_fire ) );
        Q_strcat( buf, bsize, va( "snd_reload = \"%s\"\n", wpinfo->snd_reload ) );
        Q_strcat( buf, bsize, va( "name = \"%s\"\n", wpinfo->name ) );
        Q_strcat( buf, bsize, va( "path = \"%s\"\n", wpinfo->path ) );
        Q_strcat( buf, bsize, va( "wp_sort = \"%s\"\n", weapon_sortNames[ wpinfo->wp_sort ] ) );

        trap_FS_Write( buf, strlen(buf), f );
        trap_FS_FCloseFile( f );
      }
    }
    else if( !Q_stricmp( args[2], "load" ) ) {
      G_LogPrintf( "Loading weapons' configuration files.\n" );
      Weapons_GetInfos( );
    }
    else if( !Q_stricmp( args[2], "modify" ) ) {
      if( !args[3] ) return;

      wpinfo = &bg_weaponlist[ BG_WeaponNumByName(args[3]) ];
      if( !Q_stricmp( args[4], "anim" ) ) {
        anim = &wpinfo->animations[ BG_AnimNumByName(args[5]) ];

        if( !Q_stricmp( args[6], "firstFrame" ) )
          anim->firstFrame = atoi(args[7]);
        else if( !Q_stricmp( args[6], "numFrames" ) )
          anim->numFrames = atoi(args[7]);
        else if( !Q_stricmp( args[6], "loopFrames" ) )
          anim->loopFrames = atoi(args[7]);
        else if( !Q_stricmp( args[6], "frameLerp" ) )
          anim->frameLerp = atoi(args[7]);
        else if( !Q_stricmp( args[6], "initialLerp" ) )
          anim->initialLerp = atoi(args[7]);
        else if( !Q_stricmp( args[6], "reversed" ) )
          anim->reversed = atoi(args[7]);
        else if( !Q_stricmp( args[6], "flipflop" ) )
          anim->flipflop = atoi(args[7]);
      }
      else if( !Q_stricmp( args[4], "spread" ) )
        wpinfo->spread = atof( args[5] );
      else if( !Q_stricmp( args[4], "damage" ) )
        wpinfo->damage = atof( args[5] );
      else if( !Q_stricmp( args[4], "range" ) )
        wpinfo->range = atoi( args[5] );
      else if( !Q_stricmp( args[4], "addTime" ) )
        wpinfo->addTime = atoi( args[5] );
      else if( !Q_stricmp( args[4], "count" ) )
        wpinfo->count = atoi( args[5] );
      else if( !Q_stricmp( args[4], "clipAmmo" ) )
        wpinfo->clipAmmo = atoi( args[5] );
      else if( !Q_stricmp( args[4], "clip" ) )
      {
        for(i=0; i <= WP_SEC_PISTOL; i++) {
          if( !Q_stricmp( weapon_numNames[i], args[5] ) )
            wpinfo->clip = i;
        }
      }
      else if( !Q_stricmp( args[4], "maxAmmo" ) )
        wpinfo->maxAmmo = atoi( args[5] );
      else if( !Q_stricmp( args[4], "v_model" ) )
        Com_sprintf( wpinfo->v_model, sizeof(wpinfo->v_model), args[5] );
      else if( !Q_stricmp( args[4], "v_barrel" ) )
        Com_sprintf( wpinfo->v_barrel, sizeof( wpinfo->v_barrel), args[5] );
      else if( !Q_stricmp( args[4], "snd_fire" ) )
        Com_sprintf( wpinfo->snd_fire, sizeof( wpinfo->snd_fire), args[5] );
      else if( !Q_stricmp( args[4], "snd_reload" ) )
        Com_sprintf( wpinfo->snd_reload, sizeof( wpinfo->snd_reload ), args[5] );
      else if( !Q_stricmp( args[4], "name" ) )
        Com_sprintf( wpinfo->name, sizeof( wpinfo->name ), args[5] );
      else if( !Q_stricmp( args[4], "path" ) )
        Com_sprintf( wpinfo->path, sizeof( wpinfo->path ), args[5] );
      else if( !Q_stricmp( args[4], "wp_sort" ) )
      {
        for( i = WPS_NONE; i < WPS_NUM_ITEMS; i++ ) {
          if( !Q_stricmp( weapon_sortNames[i], args[5] ) )
            wpinfo->wp_sort = i;
        }
      }
    }
  }
}

char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qbool	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

  if( !Q_stricmp( cmd, "mute" ) ) {
    Svcmd_Mute(qtrue);
    return qtrue;
  } else if( !Q_stricmp( cmd, "unmute" ) ) {
    Svcmd_Mute(qfalse);
    return qtrue;
  }

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

  if( !Q_stricmp( cmd, "config" ) ) {
    Svcmd_GameWeapon_f();
    return qtrue;
  }

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "kickbots") == 0) {
		Svcmd_KickBots_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(1) ) );
			return qtrue;
		}
		else if (Q_stricmp (cmd, "bigtext") == 0 || Q_stricmp (cmd, "cp") == 0) {
			const char *arg = ConcatArgs(1);
			if ( Q_strncmp ("-1", arg, strlen(arg)) == 0 )
				trap_SendServerCommand( -1, va("cp \"%s\"", ConcatArgs(2) ) );
			else if ( arg[0]<'0' || arg[0]>'9' )
				trap_SendServerCommand( -1, va("cp \"%s\"", arg ) );
			else if (ClientForString(arg)) {
				int clientNum = atoi(arg);
				trap_SendServerCommand( clientNum, va("cp \"%s\"", ConcatArgs(2) ) );
			}
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server: %s\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

