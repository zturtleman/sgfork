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
=============================
Weapon part
=============================
*/

#define DEFAULT_SHOTGUN_SPREAD	700
#define DEFAULT_SHOTGUN_COUNT	11

#define WINCH97_1_KSCALE 500
#define WINCH97_2_KSCALE 300

#define GATLING_MINS_PITCH -3
#define GATLING_MINS_YAW -3
#define GATLING_MINS_ROLL 0.0
#define GATLING_MAXS_PITCH 3
#define GATLING_MAXS_YAW 3
#define GATLING_MAXS_ROLL 35
#define GATLING_MINS2_PITCH -20
#define GATLING_MINS2_YAW -20
#define GATLING_MINS2_ROLL 0.0
#define GATLING_MAXS2_PITCH 20
#define GATLING_MAXS2_YAW 20
#define GATLING_MAXS2_ROLL 35
#define TRIPOD_TIME			750
#define	STAGE_TIME			500
#define GATLING_RANGE	20
#define	GATLING_DOWN	23.0
#define	GATLING_UP		23.0
#define GATLING_SIDE	70.0

#define DYNA_KSCALE 300
#define DYNA_DAMAGE (900+(rand()%200))
#define DYNA_SPLASHDAMAGE (600+(rand()%200))
#define DYNA_SPLASHRADIUS (200+(rand()%50))
#define DYNA_HEALTH 5
#define DYNA_SPEED 700

#define DYN_SPLASH_DAM			150
#define DYN_SPLASH_RAD			200
#define DYN_DAM					200

#define BELT_MAXAMMO 1.25f

#define MOLOTOV_DAMAGE 7
#define MOLOTOV_SPLASHDAMAGE 0
#define MOLOTOV_SPLASHRADIUS 0
#define MOLOTOV_SPEED 700
#define	MOLOTOV_BURNTIME		30000  // burning in hand
#define	WHISKEY_BURNTIME		15000  // whiskey pool
#define WHISKEY_SICKERTIME		60000  // time till whiskey sinks into the ground
#define ALC_COUNT				8 // number of "sprotzs" of alcohol

#define	SCOPE_TIME				400

/*
=============================
Player
=============================
*/
#define PLAYER_MIN_PITCH -14
#define PLAYER_MIN_YAW -14
#define PLAYER_MIN_HIT_PITCH -25
#define PLAYER_MIN_HIT_YAW -25

#define PLAYER_MAX_PITCH 14
#define PLAYER_MAX_YAW 14
#define PLAYER_MAX_HIT_PITCH 25
#define PLAYER_MAX_HIT_YAW 25

/*
=============================
System
=============================
*/
#define G_SPEED_DEF "200"
#define G_GRAVITY_DEF "900"
#define DEFAULT_GRAVITY 900

#define	GIB_HEALTH			-40
#define DEFAULT_REDTEAM_NAME		"Lawmen"
#define DEFAULT_BLUETEAM_NAME		"Outlaws"

#define	MINS_Z				-24
#define MAXS_Z_HIT			38
#define MAXS_Z				28
#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

#define	FALLOFF_RANGE	150
#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present
#define	MAX_DEATH_MESSAGES		4
#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define MAX_VOTE_ARGS 20

/*
=============================
Money system
=============================
*/

#define	START_MONEY				20.00
#define MAX_MONEY				200.00
#define	MIN_MONEY				20.00
#define DU_MIN_MONEY			20.00
#define SOCIAL_MONEY			28.00

#define MAX_REWARD				"20"
#define	LOSE_MONEY				7.00

#define	ROUND_WIN_MONEY			"16"
#define	ROUND_LOSE_MONEY		"10"

//buy properties
#define	BUY_TIME				60000

//money-system end

// Reward for player who rob the bank (added by Joe Kari)
#define MAX_POINT_ROBBER_REWARD			5
#define MIN_POINT_ROBBER_REWARD			0

/*
=============================
Misc
=============================
*/

#define BOILER_PLATE 60
#define	ARMOR_PROTECTION	0.66

#define	ITEM_RADIUS			15		// item sizes are needed for client side pickup detection
#define MEDAL_TIME 3000
#define ACTIVATE_RANGE 45

#define	MAX_ITEMS			256
#define	RANK_TIED_FLAG		0x4000
#define	MAX_BULLET_DISTANCE		800

/*
=============================
Duel stats
=============================
*/
#define DU_INTRO_CAM		7000.0f
#define DU_INTRO_DRAW		3000.0f
#define	DU_CROSSHAIR_START	1500.0f
#define DU_CROSSHAIR_FADE	4500.0f
#define DU_WP_STEP			2

/*
=============================
Administration part
=============================
*/

#define AP_MUTED 0x00000001

