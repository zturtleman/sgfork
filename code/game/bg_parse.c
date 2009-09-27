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

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_parse.h"

//
// Common parse files
//

float parse_getNum( char *s )
{
  char *t;

  t = COM_ParseExt( &s, qfalse );
  if( !Q_stricmp( t, "=" ) )
    t = COM_ParseExt( &s, qfalse );

  return atof(t);
}

char *parse_getChar( char *s )
{
  char *t;

  t = COM_ParseExt( &s, qfalse );
  if( !Q_stricmp( t, "=" ) )
    t = COM_ParseExt( &s, qfalse );
  return t;
}

static void parse_TokToDest( char *str, char *s )
{
  char buf[MAX_QPATH];

  Com_sprintf( buf, sizeof(buf), parse_getChar(s) );

  if( !Q_stricmp( s, "0" ) || !Q_stricmp( s, "<NULL>" ) )
    str = 0;
  else
    memcpy( str, buf, sizeof(buf) );
}

void Weapons_ParseFile( char *fileName, int wp_num )
{
  int len;
  char *t;
  char ach[20000];
  char *s;
  int animNum=0;
  fileHandle_t f;
  int i;

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );

  if( len < 0 || len > 20000 )
  {
    BG_Error( ERR_FATAL, "Weapons_ParseFile: Couldn't open file %s\n", fileName );
    return;
  }

  trap_FS_Read( ach, len, f );
  s = ach;
  *( s + len ) = '\0';
  trap_FS_FCloseFile( f );

  while( *(t=COM_Parse(&s)) )
  {
    for( i=0; i <= WP_ANIM_SPECIAL2; i++ ) {
      if( !Q_stricmp( t, weapon_animName[i] ) )
        animNum = i;
    }
    if( !Q_stricmp( t, "firstFrame" ) )
      bg_weaponlist[wp_num].animations[animNum].firstFrame = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "numFrames" ) )
      bg_weaponlist[wp_num].animations[animNum].numFrames = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "loopFrames" ) )
      bg_weaponlist[wp_num].animations[animNum].loopFrames = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "frameLerp" ) )
      bg_weaponlist[wp_num].animations[animNum].frameLerp = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "initalLerp" ) )
      bg_weaponlist[wp_num].animations[animNum].initialLerp = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "reversed" ) )
      bg_weaponlist[wp_num].animations[animNum].reversed = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "flipflop" ) )
      bg_weaponlist[wp_num].animations[animNum].flipflop = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "spread" ) )
      bg_weaponlist[wp_num].spread = parse_getNum( s );
    else if( !Q_stricmp( t, "damage" ) )
      bg_weaponlist[wp_num].damage = parse_getNum( s );
    else if( !Q_stricmp( t, "range" ) )
      bg_weaponlist[wp_num].range = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "addTime" ) )
      bg_weaponlist[wp_num].addTime = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "count" ) )
      bg_weaponlist[wp_num].count = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "clipAmmo" ) )
      bg_weaponlist[wp_num].clipAmmo = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "clip" ) )
    {
      t = parse_getChar( s );
      if( !Q_stricmp( t, "=" ) )
        t = parse_getChar( s );

      if( !Q_stricmp( t, "0" ) )
        bg_weaponlist[wp_num].clip = 0;

      for(i=0; i <= WP_SEC_PISTOL; i++)
      {
        if( !Q_stricmp( weapon_numNames[i], t ) )
          bg_weaponlist[wp_num].clip = i;
      }
    }
    else if( !Q_stricmp( t, "maxAmmo" ) )
      bg_weaponlist[wp_num].maxAmmo = (int)parse_getNum( s );
    else if( !Q_stricmp( t, "v_model" ) )
      parse_TokToDest( bg_weaponlist[wp_num].v_model, s );
    else if( !Q_stricmp( t, "v_barrel" ) )
      parse_TokToDest( bg_weaponlist[wp_num].v_barrel, s );
    else if( !Q_stricmp( t, "snd_fire" ) )
      parse_TokToDest( bg_weaponlist[wp_num].snd_fire, s );
    else if( !Q_stricmp( t, "snd_reload" ) )
      parse_TokToDest( bg_weaponlist[wp_num].snd_reload, s );
    else if( !Q_stricmp( t, "name" ) )
      parse_TokToDest( bg_weaponlist[wp_num].name, s );
    else if( !Q_stricmp( t, "path" ) )
      parse_TokToDest( bg_weaponlist[wp_num].path, s );
    else if( !Q_stricmp( t, "wp_sort" ) )
    {
      t = parse_getChar(s);

      if( !Q_stricmp( t, "0" ) )
      {
        bg_weaponlist[wp_num].wp_sort = 0;
        break;
      }

      for( i=WPS_NONE; i < WPS_NUM_ITEMS; i++ )
      {
        if( !Q_stricmp( weapon_sortNames[i], t ) )
          bg_weaponlist[wp_num].wp_sort = i;
      }
    }
  }
}

int BG_WeaponNumByName( char *wp ) {
  int i;

  for(i=0; i <= WP_NUM_WEAPONS; i++ ) {
    if( !Q_stricmp( weapon_numNames[i], wp ) )
      return i;
  }
  return 0;
}

int BG_AnimNumByName( char *anim ) {
  int i;

  for( i = 0; i <= WP_ANIM_SPECIAL2; i++ ) {
    if( !Q_stricmp( weapon_animName[i], anim ) )
      return i;
  }
  return 0;
}

void Weapons_GetInfos( void )
{
  int i;

  for( i= WP_NONE; i < WP_NUM_WEAPONS; i++ )
    Weapons_ParseFile( Weapons_FileForWeapon(i), i );
}

#define CONFIG_DIR "configs"
char *Weapons_FileForWeapon( int wp_num )
{
  char f[MAX_QPATH];

  if( wp_num >= 0 && wp_num < WP_NUM_WEAPONS )
    Com_sprintf( f, sizeof(f), "%s/%s", CONFIG_DIR, wp_fileNames[wp_num] );
  else
    BG_Error( "Weapons_FileForWeapon: Invalid weapon number %i\n", wp_num );

  return f;
}

