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
#include "bg_local.h"

//
// qvms' parse files
//

float parse_getFloat( char *s )
{
  char *t;

  t = COM_ParseExt( &s, qfalse );
  if( t[0] == '=' )
    t = COM_ParseExt( &s, qfalse );

  return atof(t);
}

int parse_getInt( char *s )
{
  char *t;

  t = COM_ParseExt( &s, qfalse );
  if( t[0] == '=' )
    t = COM_ParseExt( &s, qfalse );

  return atoi(t);
}

char *parse_getChar( char *s )
{
  char *t;

  t = COM_ParseExt( &s, qfalse );
  if( t[0] == '=' )
    t = COM_ParseExt( &s, qfalse );
  return t;
}

static void parse_TokToDest(char *str, char *s) {
  char buf[MAX_QPATH];

  Com_sprintf(buf, sizeof(buf), parse_getChar(s));
  if (!Q_stricmp(buf, "0") ||
	  !Q_stricmp(buf, "<NULL>") ||
	  !Q_stricmp(buf, "NULL"))
	*str=0;
  else
    memcpy(str, buf, MAX_QPATH);
}

void parse_config( int fileType, char *fileName, int num )
{
  int len;
  char *t;
  char buf[20000];
  char *s;
  int animNum=0;
  fileHandle_t f;
  int i;
  animation_t *anim;
  wpinfo_t *wpinfo;
  gitem_t *item;

  len = trap_FS_FOpenFile( fileName, &f, FS_READ );

  if( len < 0 || len > 20000 )
  {
    BG_Error( "parse_config: Couldn't open file %s\n", fileName );
    return;
  }

  trap_FS_Read( buf, len, f );
  s = buf;
  *(s+len) = '\0';
  trap_FS_FCloseFile( f );

  while( *(t=COM_Parse(&s)) )
  {
    //Com_Printf( "%s\n", t );
    switch( fileType )
    {
      case PFT_WEAPONS:
        wpinfo = &bg_weaponlist[num];

        for( i=0; i <= WP_ANIM_SPECIAL2; i++ ) {
          if( !Q_stricmp( t, psf_weapon_animName[i] ) ) {
            anim = &wpinfo->animations[i];
            animNum = i;
          }
        }
        if( !Q_stricmp( t, "firstFrame" ) )
          anim->firstFrame = parse_getInt( s );
        else if( !Q_stricmp( t, "numFrames" ) )
          anim->numFrames = parse_getInt( s );
        else if( !Q_stricmp( t, "loopFrames" ) )
          anim->loopFrames = parse_getInt( s );
        else if( !Q_stricmp( t, "frameLerp" ) )
          anim->frameLerp = parse_getInt( s );
        else if( !Q_stricmp( t, "initalLerp" ) )
          anim->initialLerp = parse_getInt( s );
        else if( !Q_stricmp( t, "reversed" ) )
          anim->reversed = parse_getInt( s );
        else if( !Q_stricmp( t, "flipflop" ) )
          anim->flipflop = parse_getInt( s );
        else if( !Q_stricmp( t, "spread" ) )
          wpinfo->spread = parse_getFloat( s );
        else if( !Q_stricmp( t, "damage" ) )
          wpinfo->damage = parse_getFloat( s );
        else if( !Q_stricmp( t, "range" ) )
          wpinfo->range = parse_getInt( s );
        else if( !Q_stricmp( t, "addTime" ) )
          wpinfo->addTime = parse_getInt( s );
        else if( !Q_stricmp( t, "count" ) )
          wpinfo->count = parse_getInt( s );
        else if( !Q_stricmp( t, "clipAmmo" ) )
          wpinfo->clipAmmo = parse_getInt( s );
        else if( !Q_stricmp( t, "clip" ) )
        {
          t = parse_getChar( s );
          if( !Q_stricmp( t, "=" ) )
            t = parse_getChar( s );

          if( !Q_stricmp( t, "0" ) )
            wpinfo->clip = 0;

          for(i=0; i <= WP_SEC_PISTOL; i++)
          {
            if( !Q_stricmp( psf_weapons_config[i].numNames, t ) )
              wpinfo->clip = i;
          }
        }
        else if( !Q_stricmp( t, "maxAmmo" ) )
          wpinfo->maxAmmo = parse_getInt( s );
        else if( !Q_stricmp( t, "v_model" ) )
          parse_TokToDest( wpinfo->v_model, s );
        else if( !Q_stricmp( t, "v_barrel" ) )
          parse_TokToDest( wpinfo->v_barrel, s );
        else if( !Q_stricmp( t, "snd_fire" ) )
          parse_TokToDest( wpinfo->snd_fire, s );
        else if( !Q_stricmp( t, "snd_reload" ) )
          parse_TokToDest( wpinfo->snd_reload, s );
        else if( !Q_stricmp( t, "name" ) )
          parse_TokToDest( wpinfo->name, s );
        else if( !Q_stricmp( t, "path" ) )
          parse_TokToDest( wpinfo->path, s );
        else if( !Q_stricmp( t, "wp_sort" ) )
        {
          t = parse_getChar(s);

          if( !Q_stricmp( t, "0" ) )
          {
            wpinfo->wp_sort = 0;
            break;
          }

          for( i=WPS_NONE; i < WPS_NUM_ITEMS; i++ )
          {
            if( !Q_stricmp( psf_weapon_sortNames[i], t ) )
              wpinfo->wp_sort = i;
          }
        }
        break;
      case PFT_ITEMS:
        item = &bg_itemlist[num];

        if( !Q_stricmp( t, "classname" ) )
          parse_TokToDest( item->classname, s );
        else if( !Q_stricmp( t, "pickup_sound" ) )
          parse_TokToDest( item->pickup_sound, s );
        else if( !Q_stricmp( t, "world_model" ) ) {
          COM_ParseExt(&s, qfalse);
          COM_ParseExt(&s, qfalse);
          parse_TokToDest( item->world_model[0], s );
          COM_ParseExt(&s, qfalse);
          parse_TokToDest( item->world_model[1], s );
          COM_ParseExt(&s, qfalse);
          parse_TokToDest( item->world_model[2], s );
          COM_ParseExt(&s, qfalse);
          parse_TokToDest( item->world_model[3], s );
        }
        else if( !Q_stricmp( t, "icon" ) )
          parse_TokToDest( item->icon, s );
        else if( !Q_stricmp( t, "xyrelation" ) )
          item->xyrelation = parse_getFloat( s );
        else if( !Q_stricmp( t, "pickup_name" ) )
          parse_TokToDest( item->pickup_name, s );
        else if( !Q_stricmp( t, "quantity" ) )
          item->quantity = parse_getInt(s);
        else if( !Q_stricmp( t, "giType" ) ) {
          t = parse_getChar(s);
          for(i=0; i <= IT_POWERUP; i++ ) {
            if( !Q_stricmp( psf_itemTypes[i], t ) )
              item->giType = i;
          }
        }
        else if( !Q_stricmp( t, "giTag" ) ) {
          t = parse_getChar(s);
          for(i=0; i <= WP_SEC_PISTOL; i++ ) {
            if( !Q_stricmp( psf_weapons_config[i].numNames, t ) )
              item->giTag = i;
          }
        }
        else if( !Q_stricmp( t, "prize" ) )
          item->prize = parse_getInt(s);
        else if( !Q_stricmp( t, "weapon_sort" ) ) {
          t = parse_getChar(s);
          for(i=0; i <= WS_MISC; i++ ) {
            if( !Q_stricmp( psf_weapon_buyType[i], t ) )
              item->weapon_sort = i;
          }
        }
        else if( !Q_stricmp( t, "precaches" ) )
          parse_TokToDest( item->precaches, s );
        else if( !Q_stricmp( t, "sounds" ) )
          parse_TokToDest( item->sounds, s );
        break;
#ifdef QAGAME
      case PFT_GRAVITY:
        if( !Q_stricmp( t, "pm_stopspeed" ) ) 
          pm_stopspeed = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_duckScale" ) )
          pm_duckScale = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_swimScale" ) )
          pm_swimScale = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_wadeScale" ) )
          pm_wadeScale = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_ladderScale" ) )
          pm_ladderScale = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_reloadScale" ) )
          pm_reloadScale = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_accelerate" ) )
          pm_accelerate = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_airaccelerate" ) )
          pm_airaccelerate = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_wateraccelerate" ) )
          pm_wateraccelerate = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_flyaccelerate" ) )
          pm_flyaccelerate = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_ladderAccelerate" ) )
          pm_ladderAccelerate = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_friction" ) )
          pm_friction = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_waterfriction" ) )
          pm_waterfriction = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_flightfriction" ) )
          pm_flightfriction = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_spectatorfriction" ) )
          pm_spectatorfriction = parse_getFloat(s);
        else if( !Q_stricmp( t, "pm_ladderfriction" ) )
          pm_ladderfriction = parse_getFloat(s);
        break;
#endif
    }
  }
}

int BG_WeaponNumByName( char *wp ) {
  int i;

  for(i=0; i <= WP_NUM_WEAPONS; i++ ) {
    if( !Q_stricmp( psf_weapons_config[i].numNames, wp ) )
      return i;
  }
  return 0;
}

int BG_AnimNumByName( char *anim ) {
  int i;

  for( i = 0; i <= WP_ANIM_SPECIAL2; i++ ) {
    if( !Q_stricmp( psf_weapon_animName[i], anim ) )
      return i;
  }
  return 0;
}

int BG_ItemNumByName( char *name )
{
  int i;

  for( i = 0; i <= IT_POWERUP; i++ ) {
    if( !Q_stricmp( psf_itemTypes[i], name ) )
      return i;
  }
  return 0;
}

int BG_WSNumByName( char *s )
{
  int i;

  for(i=0; i <= WS_MISC; i++ ) {
    if( !Q_stricmp( psf_weapon_buyType[i], s ) )
      return i;
  }
  return 0;
}

int BG_GitagNumByName( char *s )
{
  int i;
  for(i=0; i <= WP_SEC_PISTOL; i++ ) {
    if( !Q_stricmp( psf_weapons_config[i].numNames, s ) )
      return i;
  }
  return 0;
}

int BG_GitypeByName( char *s )
{
  int i;

  for(i=0; i <= IT_POWERUP; i++ ) {
    if( !Q_stricmp( psf_itemTypes[i], s ) )
      return i;
  }
  return 0;
}

void config_GetInfos( int fileType )
{
  int i;
  char buf[MAX_QPATH];

  if( fileType & PFT_WEAPONS ) {
    Com_Printf( "Loading weapons.\n" );
    for( i = WP_NONE; i < WP_NUM_WEAPONS; i++ )
      parse_config( PFT_WEAPONS, Parse_FindFile(buf, sizeof(buf), i, PFT_WEAPONS), i );
  } 
  if( fileType & PFT_ITEMS ) {
    Com_Printf( "Loading items.\n" );
    for( i = 0; i < IT_NUM_ITEMS; i++ ) {
      parse_config( PFT_ITEMS, Parse_FindFile(buf, sizeof(buf), i, PFT_ITEMS), i );
    }
  }
#ifdef QAGAME
  if( fileType & PFT_GRAVITY ) {
    Com_Printf( "Loading gravity.\n" );
    parse_config( PFT_GRAVITY, Parse_FindFile(buf, sizeof(buf), 0, PFT_GRAVITY), 0 );
  }
#endif
}

#define CONFIG_DIR "configs"
char *Parse_FindFile(char *filename_out, int filename_out_size, int num, int fileType) {
  switch(fileType) {
    case PFT_WEAPONS:
      if (num >= 0 && num < WP_NUM_WEAPONS)
        Com_sprintf(filename_out, filename_out_size, "%s/%s", CONFIG_DIR, psf_weapons_config[num].fileName);
      else
        BG_Error("Parse_FindFile: Invalid weapon number %i\n", num);
      break;
    case PFT_ITEMS:
      if (num >= 0 && num < IT_NUM_ITEMS)
        Com_sprintf(filename_out, filename_out_size, "%s/%s.item", CONFIG_DIR, psf_item_fileNames[num]);
      else
        BG_Error("Parse_FindFile: Invalid item number %i\n", num);
      break;
#ifdef QAGAME
    case PFT_GRAVITY:
      Com_sprintf(filename_out, filename_out_size, "%s/%s.cfg", CONFIG_DIR, GRAVITY_FILE);
      break;
#endif
	default:
      BG_Error("Parse_FindFile: Invalid file type number %i\n", fileType);
      break;
  }
  return filename_out;
}

int BG_WeaponListChange(char *weapon, char *an, char *dest, char *value) {
  int i;
  wpinfo_t *wp = NULL;
  animation_t *anim = NULL;

  for (i = WP_NONE; i <= WP_MOLOTOV; i++)
    if (!Q_stricmp(psf_weapons_config[i].numNames, weapon)) {
      wp = &bg_weaponlist[i];
      break;
    }
  if (!wp) {
    Com_Printf("^3Warning: weapon \'%s\' doesn't exists.\n", weapon);
    return 1;
  }

  if (an) {
    for (i = 0; i <= psf_weapon_numAnimName; i++)
      if (!Q_stricmp(psf_weapon_animName[i], an)) {
        anim = &wp->animations[i];
        break;
	  }
    if (!anim) {
      Com_Printf("^3Warning: animation \'%s\' doesn't exists.\n", an);
      return 1;
	}
  }

  if (!Q_stricmp("firstFrame", dest))
    anim->firstFrame = atoi(value);
  else if (!Q_stricmp("numFrames", dest))
    anim->numFrames = atoi(value);
  else if (!Q_stricmp("loopFrames", dest))
    anim->loopFrames = atoi(value);
  else if (!Q_stricmp("frameLerp", dest))
    anim->frameLerp = atoi(value);
  else if (!Q_stricmp("initialLerp", dest))
    anim->initialLerp = atoi(value);
  else if (!Q_stricmp("reversed", dest))
    anim->reversed = atoi(value);
  else if (!Q_stricmp("flipflop", dest))
    anim->flipflop = atoi(value);
  else if (!Q_stricmp("spread", dest))
    wp->spread = atof(value);
  else if (!Q_stricmp("damage", dest))
    wp->damage = atof(value);
  else if (!Q_stricmp("range", dest))
    wp->range = atoi(value);
  else if (!Q_stricmp("addTime", dest))
    wp->addTime = atoi(value);
  else if (!Q_stricmp("count", dest))
    wp->count = atoi(value);
  else if (!Q_stricmp("clipAmmo", dest))
    wp->clipAmmo = atoi(value);
  else if (!Q_stricmp("clip", dest)) {
    if (!Q_stricmp(value, "0"))
      wp->clip = 0;
	else {
      for (i = WP_NONE; i <= WP_SEC_PISTOL; i++)
		  if (!Q_stricmp(psf_weapons_config[i].numNames, value)) {
            wp->clip = i;
			break;
          }
    }
  }
  else if (!Q_stricmp("maxAmmo", dest))
    wp->maxAmmo = atoi(value);
  else if (!Q_stricmp("v_model", dest))
    memcpy(wp->v_model, value, sizeof(wp->v_model));
  else if (!Q_stricmp("v_barrel", dest))
    memcpy(wp->v_barrel, value, sizeof(wp->v_barrel));
  else if (!Q_stricmp("snd_fire", dest))
    memcpy(wp->snd_fire, value, sizeof(wp->snd_fire));
  else if (!Q_stricmp("snd_reload", dest))
    memcpy(wp->snd_reload, value, sizeof(wp->snd_reload));
  else if (!Q_stricmp("name", dest))
    memcpy(wp->name, value, sizeof(wp->name));
  else if (!Q_stricmp("path", dest))
    memcpy(wp->path, value, sizeof(wp->path));
  else if (!Q_stricmp("wp_sort", dest)) {
    if (!Q_stricmp(value, "0"))
      wp->wp_sort = 0;
	else {
      for (i = WPS_NONE; i < WPS_NUM_ITEMS; i++)
        if (!Q_stricmp(psf_weapon_sortNames[i], value)) {
          wp->wp_sort = i;
          break;
        }
    }
  }

  return 0;
}

int BG_ItemListChange( char *item, char *dest, char *value )
{
  int i;
  gitem_t *it = NULL;

  for (i = 0; i < IT_NUM_ITEMS; i++)
    if (!Q_stricmp(psf_item_fileNames[i], item)) {
      it = &bg_itemlist[i];
      break;
	}
  if (!it) {
    Com_Printf("^3Warning: item \'%s\' doesn't exists.\n", item);
    return 1;
  }

  if (!Q_stricmp("classname", dest))
    memcpy(it->classname, value, sizeof(it->classname));
  else if (!Q_stricmp("pickup_sound", dest))
    memcpy(it->pickup_sound, value, sizeof(it->pickup_sound));
  else if (!Q_stricmp("world_model_0", dest))
    memcpy(it->world_model[0], value, sizeof(it->world_model[0]));
  else if (!Q_stricmp("world_model_1", dest))
    memcpy(it->world_model[1], value, sizeof(it->world_model[1]));
  else if (!Q_stricmp("world_model_2", dest))
    memcpy(it->world_model[2], value, sizeof(it->world_model[2]));
  else if (!Q_stricmp("world_model_3", dest))
    memcpy(it->world_model[3], value, sizeof(it->world_model[3]));
  else if (!Q_stricmp("icon", dest))
    memcpy(it->icon, value, sizeof(it->icon));
  else if (!Q_stricmp("xyrelation", dest))
    it->xyrelation = atof(value);
  else if (!Q_stricmp("quantity", dest))
    it->quantity = atoi(value);
  else if (!Q_stricmp("giType", dest)) {
    for (i=IT_BAD; i <= IT_POWERUP; i++)
      if (!Q_stricmp(psf_itemTypes[i], value)) {
        it->giType = i;
        break;
      }
  }
  else if (!Q_stricmp("giTag", dest)) {
    for (i=WP_NONE; i <= WP_SEC_PISTOL; i++)
      if (!Q_stricmp(psf_weapons_config[i].numNames, value)) {
        it->giTag = i;
        break;
      }
  }
  else if (!Q_stricmp("prize", dest))
    it->prize = atoi(value);
  else if (!Q_stricmp("weapon_sort", dest)) {
    if (!Q_stricmp(value, "0"))
      it->weapon_sort = 0;
	else {
      for (i=WPS_NONE; i < WPS_NUM_ITEMS; i++)
        if (!Q_stricmp(psf_weapon_sortNames[i], value)) {
          it->weapon_sort = i;
          break;
        }
    }
  }
  else if (!Q_stricmp("precaches", dest))
    memcpy(it->precaches, value, sizeof(it->precaches));
  else if (!Q_stricmp("sounds", dest))
    memcpy(it->sounds, value, sizeof(it->sounds));

  return 0;
}
