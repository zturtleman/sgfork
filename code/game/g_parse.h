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

#include "g_local.h"

/*
 * Note: These tables are based on weapon enums. If you modify them, be sure 
 * that you also modified the following enums : 
 *    - weapon_t
 *    - wp_anims_t
 *    - wp_sort_t
*/

const char *wp_fileNames[] = {
  "none.weapon",
  "knife.weapon",
  "rem58.weapon",
  "schofield.weapon",
  "peacemaker.weapon",
  "winchester66.weapon",
  "lightning.weapon",
  "sharps.weapon",
  "remington_gauge.weapon",
  "sawedoff.weapon",
  "winch97.weapon",
  "gatling.weapon",
  "dynamite.weapon",
  "molotov.weapon",
};

const char *weapon_numNames[] = {
  "WP_NONE",
  "WP_KNIFE",
  "WP_REM58",
  "WP_SCHOFIELD",
  "WP_PEACEMAKER",
  "WP_WINCHESTER66",
  "WP_LIGHTNING",
  "WP_SHARPS",
  "WP_REMINGTON_GAUGE",
  "WP_SAWEDOFF",
  "WP_WINCH97",
  "WP_GATLING",
  "WP_DYNAMITE",
  "WP_MOLOTOV",
  "WP_NUM_WEAPONS",
  "WP_AKIMBO",
  "WP_BULLETS_CLIP",
  "WP_SHELLS_CLIP",
  "WP_CART_CLIP",
  "WP_GATLING_CLIP",
  "WP_SHARPS_CLIP",

  "WP_SEC_PISTOL",
};

const char *weapon_animName[] = {
  "WP_ANIM_CHANGE",
  "WP_ANIM_DROP",
  "WP_ANIM_IDLE",
  "WP_ANIM_FIRE",
  "WP_ANIM_ALT_FIRE",
  "WP_ANIM_RELOAD",
  "WP_ANIM_SPECIAL",
  "WP_ANIM_SPECIAL2",
};

const char *weapon_sortNames[] = {
  "WPS_NONE",
  "WPS_MELEE",
  "WPS_PISTOL",
  "WPS_GUN",
  "WPS_OTHER",
};

