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

/*
 * Note: These tables are based on weapon enums. If you modify them, be sure 
 * that you also modified the following enums : 
 *    - weapon_t
 *    - wp_anims_t
 *    - wp_sort_t
*/

#ifndef __BG_PARSE_H__
#define __BG_PARSE_H__

#define CONFIG_DIR "configs"
#define GRAVITY_FILE "gravity"

/*typedef struct psf_weapons_s {
  char *fileName;
  char *numNames;
} psf_weapons_t;

//typedef struct psf_weapons_s psf_weapons_t;
const char *psf_itemTypes[];
const char *psf_item_fileNames[];
const char *psf_weapon_sortNames[];
const char *psf_weapon_animName[];
const psf_weapons_t psf_weapons_config[];

void parse_config( int fileType, char *fileName, int wp_num );
char *Parse_FindFile( int num, int fileType );
void config_GetInfos( int fileType );
int BG_ItemNumByName( char *name );
int BG_AnimNumByName( char *anim );
int BG_WeaponNumByName( char *wp );
*/

const psf_weapons_t psf_weapons_config[] = {
  { "none.weapon", "WP_NONE", },
  { "knife.weapon", "WP_KNIFE", },
  { "rem58.weapon", "WP_REM58", },
  { "schofield.weapon", "WP_SCHOFIELD", },
  { "peacemaker.weapon", "WP_PEACEMAKER", },
  { "winchester66.weapon", "WP_WINECHESTER66", },
  { "lightning.weapon", "WP_LIGHTNING", },
  { "sharps.weapon", "WP_SHARPS", },
  { "remington_gauge.weapon", "WP_REMINGTON_GAUGE", },
  { "sawedoff.weapon", "WP_SAWEDOFF", },
  { "winch97.weapon", "WP_WINCH97", },
  { "gatling.weapon", "WP_GATLING", },
  { "dynamite.weapon", "WP_DYNAMITE", },
  { "molotov.weapon", "WP_MOLOTOV", },
  { NULL, "WP_NUM_WEAPONS" },
  { NULL, "WP_AKIMBO" },
  { NULL, "WP_BULLETS_CLIP" },
  { NULL, "WP_SHELLS_CLIP" },
  { NULL, "WP_CART_CLIP" },
  { NULL, "WP_GATLING_CLIP" },
  { NULL, "WP_SHARPS_CLIP" },
  { NULL, "WP_SEC_PISTOL" },
};

const char *psf_weapon_animName[] = {
  "WP_ANIM_CHANGE",
  "WP_ANIM_DROP",
  "WP_ANIM_IDLE",
  "WP_ANIM_FIRE",
  "WP_ANIM_ALT_FIRE",
  "WP_ANIM_RELOAD",
  "WP_ANIM_SPECIAL",
  "WP_ANIM_SPECIAL2",
};
int psf_weapon_numAnimName = sizeof(psf_weapon_animName)/sizeof(psf_weapon_animName[0]);

const char *psf_weapon_buyType[] = {
  "WS_NONE",
  "WS_PISTOL",
  "WS_RIFLE",
  "WS_SHOTGUN",
  "WS_MGUN",
  "WS_MISC",
};

const char *psf_weapon_sortNames[] = {
  "WPS_NONE",
  "WPS_MELEE",
  "WPS_PISTOL",
  "WPS_GUN",
  "WPS_OTHER",
};

const char *psf_item_fileNames[IT_NUM_ITEMS] = {
  "none",
  "item_boiler_plate",
  "pickup_money",
  "item_money",
  "item_scope",
  "item_belt",
  "weapon_winchester66",
  "weapon_lightning",
  "weapon_sharps",
  "weapon_gatling",
  "weapon_remington58",
  "weapon_schofield",
  "weapon_peacemaker",
  "weapon_dynamite",
  "weapon_molotov",
  "weapon_knife",
  "weapon_shotgun",
  "weapon_sawedoff",
  "weapon_winch97",
  "ammo_shells",
  "ammo_bullets",
  "ammo_dynamite",
  "ammo_molotov",
  "ammo_knives",
  "ammo_cartridges",
  "ammo_gatling",
  "ammo_sharps",
};

const char *psf_itemTypes[] = {
  "IT_BAD",
  "IT_WEAPON",
  "IT_AMMO",
  "IT_ARMOR",
  "IT_HEALTH",
  "IT_POWERUP",
};

#endif /* __BG_PARSE_H__ */
