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

#define DYNA_HEALTH 5

#define BELT_MAXAMMO 1.25f

#define	MOLOTOV_BURNTIME		30000  // burning in hand
#define	WHISKEY_BURNTIME		15000  // whiskey pool
#define WHISKEY_SICKERTIME		60000  // time till whiskey sinks into the ground
#define ALC_COUNT				8 // number of "sprotzs" of alcohol

#define	SCOPE_TIME				400

#define ITEM_DROP_SPEED					200 // Speed for dropped items

#define KNIFE_MAX_DISTANCE_HIT_UNITS	32	// Length of knife's trace during close combat

#define MAX_KNOCKBACKTIME				200
#define MIN_KNOCKBACKTIME				50

#define DAMAGE_LOSE_ON_PLAYER			0.5f // Part of damage to be lost when going through player

#define UP_KNOCK_ADDENDUM				24; // Used to knock players a bit higher on radius damage

/*
=============================
Player
=============================
*/
#define PLAYER_HEAD_Z		8
#define PLAYER_BODY_Z		16
#define PLAYER_MIN_PITCH	-14
#define PLAYER_MIN_YAW		-14
#define PLAYER_MAX_PITCH	14
#define PLAYER_MAX_YAW		14
#define	PLAYER_MINS_Z		-24
#define PLAYER_MAXS_Z		8 + PLAYER_BODY_Z + PLAYER_HEAD_Z

//Locational damage info
#define HIT_LOCATION_NONE	0x00000000
#define	HIT_LOCATION_HEAD	0x00000001
#define HIT_LOCATION_FRONT	0x00000002
#define HIT_LOCATION_BACK	0x00000004
#define HIT_LOCATION_LEFT	0x00000008
#define HIT_LOCATION_RIGHT	0x00000010
#define HIT_LOCATION_LEGS	0x00000020

#define HIT_LOCATION_NAME_NONE	"error!"
#define	HIT_LOCATION_NAME_HEAD	"^1head"
#define HIT_LOCATION_NAME_FRONT	"^4chest"
#define HIT_LOCATION_NAME_BACK	"^4back"
#define HIT_LOCATION_NAME_LEFT	"^4left arm"
#define HIT_LOCATION_NAME_RIGHT	"^4right arm"
#define HIT_LOCATION_NAME_LEGS	"^2leg"

#define	HIT_LOCATION_MULTIPLIER_HEAD	1.5f
#define HIT_LOCATION_MULTIPLIER_FRONT	1.0f
#define HIT_LOCATION_MULTIPLIER_BACK	1.0f
#define HIT_LOCATION_MULTIPLIER_LEFT	1.0f
#define HIT_LOCATION_MULTIPLIER_RIGHT	1.0f
#define HIT_LOCATION_MULTIPLIER_LEGS	1.0f

//Health stages
#define HEALTH_USUAL	80
#define HEALTH_INJURED	49

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

#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

#define	FALLOFF_RANGE	150
#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present
#define	MAX_DEATH_MESSAGES		4
#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define MAX_VOTE_ARGS 20

// spawnflags for doors, dont forget to change for ClearItems
#define DOOR_RETURN 8
#define TRIGGER_DOOR 16

// only for rotating doors
#define DOOR_ROTATING_X_AXIS 32
#define DOOR_ROTATING_Y_AXIS 64
#define DOOR_ROTATING_ONE_WAY 128

#define MAX_FREESPAWN_ZONE_DISTANCE_SQUARE	490000

// damage flags
#define DAMAGE_INSTANT_KILL			0x00000001

// trace system flags
#define PASS_ONLY_THIS				0x10000000 // Flag for trace functions to skip child/parent check. Have to be larger than MAX_ENTITIES

/*
=============================
Money system
=============================
*/

//buy properties
#define	BUY_TIME						60000

// Reward for player who rob the bank (added by Joe Kari)
#define MAX_POINT_ROBBER_REWARD			5
#define MIN_POINT_ROBBER_REWARD			0

// Square maximum distance to spawn point where player is allowed to buy
#define MAX_BUY_DISTANCE_SQUARE			90000

#define COINS 7
#define	BILLS 15

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

/*
=============================
Duel stats
=============================
*/
#define DU_INTRO_CAM		7000
#define DU_INTRO_DRAW		3000
#define	DU_CROSSHAIR_START	1500
#define DU_CROSSHAIR_FADE	4500
#define DU_WP_STEP			2

/*
=============================
Administration part
=============================
*/

#define AP_MUTED 0x00000001

