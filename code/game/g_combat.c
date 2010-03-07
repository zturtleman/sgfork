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
// g_combat.c

#include "g_local.h"


qbool LogAccuracyHit(const gentity_t *target, const gentity_t *attacker) {
	if (!target->takedamage || target == attacker || !target->client
		|| !attacker->client || target->client->ps.stats[STAT_HEALTH] <= 0
		|| OnSameTeam(target, attacker))
		return qfalse;

	return qtrue;
}

/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	if ( !ent->client || level.warmupTime )
		return;

	// show score plum
	ScorePlum(ent, origin, score);
	//
	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( g_gametype.integer == GT_TEAM )
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

void AddScoreRTPTeam( int team, int score ) {
	// no scoring during pre-match warmup
	if ( level.warmupTime)
		return;

	level.teamScores[ team] += score;
	CalculateRanks();
}

void AddScoreRTP( gentity_t *ent, int score ) {
	if ( !ent->client )
		return;

	// no scoring during pre-match warmup
	if ( level.warmupTime)
		return;

//	ent->client->ps.persistant[PERS_SCORE] += score;
//	if (g_gametype.integer == GT_TEAM)
	level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	float		angle;
	int			i;
	gentity_t	*drop;
	gentity_t	*sec_pistol_drop;

	// in duel, no items will be  dropped
	if(g_gametype.integer == GT_DUEL)
		return;

	angle = 22.5f;
	for(i = WP_KNIFE; i < WP_NUM_WEAPONS; i++){
		if(!(self->client->ps.stats[STAT_WEAPONS] & ( 1 << i)))
			continue;

		if( i == WP_GATLING && self->client->ps.stats[STAT_GATLING_MODE] &&
			!(self->client->ps.stats[STAT_FLAGS] & SF_GAT_CARRY))
			continue;

		// find the item type for this weapon
		item = BG_ItemForWeapon( i );
		angle += 66.6f;

		// spawn the item
		drop = Drop_Item( self, item, angle );

		// Check if the player carries 2 pistols of the same kind
		// Pistols _should_ be between knife and winchester66 (see bg_public.h)
		if (i > WP_KNIFE && i < WP_WINCHESTER66) {
			if((self->client->ps.stats[STAT_FLAGS] & SF_SEC_PISTOL)) {
				angle += 66.6f;

				// spawn the item
				sec_pistol_drop = Drop_Item( self, item, angle );

				// Since the second pistol is dropped (the one on the left holster),
				// the correct ammo must be set in the dropped weapon
				if (self->client->ps.ammo[WP_AKIMBO] > bg_weaponlist[i].clipAmmo)
					sec_pistol_drop->count = bg_weaponlist[i].clipAmmo + 1;
				else
					sec_pistol_drop->count = self->client->ps.ammo[WP_AKIMBO] + 1;

				self->client->ps.ammo[WP_AKIMBO] = 0;
				self->client->ps.stats[STAT_FLAGS] &= ~SF_SEC_PISTOL;
			}
		}

		if(item->giType == IT_WEAPON) {
			if (item->giTag == WP_DYNAMITE || item->giTag == WP_KNIFE ||
			                                  item->giTag == WP_MOLOTOV) {
				drop->count = self->client->ps.ammo[i] + 1;
			}
			else {
				if(self->client->ps.ammo[i] > bg_weaponlist[item->giTag].clipAmmo)
					drop->count = bg_weaponlist[item->giTag].clipAmmo + 1;
				else
					drop->count = self->client->ps.ammo[i] + 1;
			}
		}

		// Player no longer possessed the given weapon.
		// This prevent the player to drop the same weapon, several times.
		self->client->ps.ammo[i] = 0;
		self->client->ps.stats[STAT_WEAPONS] &= ~( 1 << i );
	}

	// drop all the powerups if not in teamplay
		angle = 45;
		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
			if(i >= DM_HEAD_1 && i <= DM_LEGS_2)
				continue;

			// Don't drop the belt, nobody wants to pick up an empty belt
			if ( i == PW_BELT )
				continue;

			if ( self->client->ps.powerups[ i ] ) {
				if(i == PW_GOLD)
					item = BG_ItemByClassname("item_money");
				else
					continue;
				if ( !item )
					continue;

				drop = Drop_Item( self, item, angle );
				// decide how many seconds it has left
				drop->count = 1;
				angle += 45;

				// remove the powerup
				self->client->ps.powerups[i] = 0;
			}
		}
	// drop armor
	if( self->client->ps.stats[STAT_ARMOR]){
		item = BG_Item("Boiler Plate");

		drop = Drop_Item(self, item, rand()%360);

		drop->count = self->client->ps.stats[STAT_ARMOR];

		// Player no longer possessed the boiler plate.
		// This prevent the player to drop another boiler plate, several times.
		self->client->ps.stats[STAT_ARMOR] = 0;
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, const gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self )
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
  else
		return; //if client doesnt have killer

	//small hack. grapplePoint isnt used when player dead, so we use it to tell the client
	//where to turn. A better solution would be if we could add a new variable to playerstate
	//which is communicated between the client and the server.
	vectoangles(dir, angles);
	self->client->ps.grapplePoint[YAW]   = angles[YAW];
	self->client->ps.grapplePoint[PITCH] = angles[PITCH];
	self->client->ps.grapplePoint[ROLL]  = 0;
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	gentity_t *ent;
	int i;

	//if this entity still has kamikaze
	if (self->s.eFlags & EF_KAMIKAZE) {
		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < MAX_GENTITIES; i++) {
			ent = &g_entities[i];
			if (!ent->inuse)
				continue;
			if (ent->activator != self)
				continue;
			if (strcmp(ent->classname, "kamikaze timer"))
				continue;
			G_FreeEntity(ent);
			break;
		}
	}
	G_AddEvent( self, EV_GIB_PLAYER, killer );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

// these are just for logging, the client prints its own messages
// Tequila comment: enum should match meansOfDeath_t in bg_public.h
char	*modNames[] = {
	"MOD_UNKNOWN",
	//melee
	"MOD_KNIFE",

	//pistols
	"MOD_REM58",
	"MOD_SCHOFIELD",
	"MOD_PEACEMAKER",

	//rifles
	"MOD_WINCHESTER66",
	"MOD_LIGHTNING",
	"MOD_SHARPS",

	//shotguns
	"MOD_REMINGTON_GAUGE",
	"MOD_SAWEDOFF",
	"MOD_WINCH97",

	//automatics
	"MOD_GATLING",

	//explosives
	"MOD_DYNAMITE",
	"MOD_MOLOTOV",

	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_WORLD_DAMAGE",
	"MOD_TRIGGER_HURT",
	"MOD_BOILER"
};

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;
	int fallanim = 0;

	if ( self->client->ps.pm_type == PM_DEAD )
		return;

	if ( level.intermissiontime )
		return;

//unlagged - backward reconciliation #2
	// make sure the body shows up in the client's current position
	G_UnTimeShiftClient( self );
//unlagged - backward reconciliation #2

	// check for an almost capture
	self->client->ps.pm_type = PM_DEAD;

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else
			killerName = "<non-client>";
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else
		obit = modNames[meansOfDeath];

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n",
		killer, self->s.number, meansOfDeath, killerName,
		self->client->pers.netname, obit );

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

	if (attacker && attacker->client) {
		if ( attacker == self || OnSameTeam (self, attacker ) )
			AddScore( attacker, self->r.currentOrigin, -1 );
		else
			AddScore( attacker, self->r.currentOrigin, 1 );
	} else
		AddScore( self, self->r.currentOrigin, -1 );

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP ))
		TossClientItems( self );

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED
        || client->sess.sessionTeam < TEAM_SPECTATOR )
			continue;

		if ( client->sess.spectatorClient == self->s.number )
			Cmd_Score_f( g_entities + i );
	}

	self->takedamage = qfalse;	// cannot be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = 0;
	// reset events (important for fly)
	self->s.event = 0;
	self->client->ps.externalEvent = 0;

	//decide player is killed himself or not
	if ((attacker && attacker == self) || (inflictor && inflictor == self)
		|| (meansOfDeath >= MOD_WATER && meansOfDeath <= MOD_TRIGGER_HURT )) {

		self->client->ps.pm_flags|=PMF_SUICIDE;
	}

	self->s.loopSound = 0;
	self->r.maxs[2] = -8;
	self->client->respawnTime = level.time + 3000;

	// check if we can play a special falloff animation
	if(self->client->ps.groundEntityNum != ENTITYNUM_NONE){
		vec3_t axis[3];
		trace_t trace;
		vec3_t mins, maxs, pos;
		vec3_t mins2, maxs2;
		vec3_t angles;

		VectorClear(angles);
		angles[YAW] = self->client->ps.viewangles[YAW];
		AnglesToAxis(angles, axis);
		VectorSet(mins, -20, -20, -180);
		VectorSet(maxs, 20, 20, 30);
		VectorSet(maxs2, 20, 20, 20);
		VectorSet(mins2, -20, -20, -19);

		// calculate position
		VectorMA(self->client->ps.origin, FALLOFF_RANGE, axis[0], pos);

		// see if there is space behind the player to fall down
		trap_Trace(&trace, pos, mins, maxs, pos, self->client->ps.clientNum, MASK_SHOT);

		if(!trace.allsolid && !trace.startsolid){
			fallanim = BOTH_FALL;
		} else {
			// do another trace, this time try to fall off backwards

			// calculate position
			VectorMA(self->client->ps.origin, -FALLOFF_RANGE, axis[0], pos);

			// see if there is space behind the player to fall down
			trap_Trace(&trace, pos, mins, maxs, pos, self->client->ps.clientNum, MASK_SHOT);

			if(!trace.allsolid && !trace.startsolid)
				fallanim = BOTH_FALL_BACK;
		}
		// at last check if we can get a direct line to the falling-down-position
		trap_Trace(&trace, self->client->ps.origin, mins2, maxs2, pos, -1, MASK_SHOT);

		if(trace.fraction != 1.0)
			fallanim = 0;
	}
	if(fallanim)
		self->client->lasthurt_anim = fallanim;

	anim = self->client->lasthurt_anim;

	// for the no-blood option, we need to prevent the health
	// from going to gib level
	if ( self->health <= GIB_HEALTH )
		self->health = GIB_HEALTH+1;

	self->client->ps.legsAnim =
		( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
	self->client->ps.torsoAnim =
		( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	G_AddEvent( self, EV_DEATH_DEFAULT + anim/2, killer );

	// the body can't be gibbed anymore
	self->die = 0;

	trap_LinkEntity (self);

	if(self->s.number != ENTITYNUM_NONE && self->s.number != ENTITYNUM_WORLD &&
		killer != ENTITYNUM_NONE && killer != ENTITYNUM_WORLD){
		int rank, amount;
		float	f_amount;
		gentity_t *assaulter = &g_entities[killer];
		gentity_t *victim = &g_entities[self->s.number];

		if(self->client->ps.clientNum == assaulter->client->ps.clientNum)
			return;

		//the attacker gets the reward ;) by Spoon
		rank = 1;

		//if he didn't kill a mate ;)
		if(victim->client->sess.sessionTeam == assaulter->client->sess.sessionTeam && g_gametype.integer >= GT_TEAM) {

			assaulter->client->ps.stats[STAT_MONEY] -= 2*g_moneyLose.integer;

			if(assaulter->client->ps.stats[STAT_MONEY] < 0){
				if (g_moneyRespawnMode.integer == 1) {
					// Joe Kari: in the new money system, the teamkiller don't waste its money, 
					// what he loses is a gift to his mate
					victim->client->ps.stats[STAT_MONEY] += 2*g_moneyLose.integer + assaulter->client->ps.stats[STAT_MONEY];
				}
				assaulter->client->ps.stats[STAT_MONEY] = 0;
			} else if (g_moneyRespawnMode.integer == 1) {
				// Joe Kari: in the new money system, the teamkiller don't waste its money, 
				// what he loses is a gift to his mate
				victim->client->ps.stats[STAT_MONEY] += 2*g_moneyLose.integer;
			}
			return;
		}

		//calculate rank of the inflictor
		for(i=0; i<level.maxclients; i++){

			if(level.clients[i].ps.clientNum == victim->client->ps.clientNum)
				continue;

			if(level.clients[i].ps.persistant[PERS_SCORE] > victim->client->ps.persistant[PERS_SCORE])
				rank ++;
		}

		//give the attacker the money
		if(assaulter->client->ps.clientNum != victim->client->ps.clientNum){
			const int	players = TeamCount(-1, TEAM_FREE) + TeamCount(-1, TEAM_RED) +
				TeamCount(-1, TEAM_BLUE) + TeamCount(-1, TEAM_RED_SPECTATOR) +
				TeamCount(-1, TEAM_BLUE_SPECTATOR) +
				(g_gametype.integer == GT_DUEL ? TeamCount( -1, TEAM_SPECTATOR) : 0);
			const float	factor = (float)players/4;
			float	factor2 = factor -1;

			if(factor2<0)
				factor2 = 1;
			else {
				factor2 = factor2/5;
				factor2 += 1;
			}
			//G_Printf("amount: %i %f", rank, factor2);


			f_amount = sqrt(sqrt(1.0/rank)); // take (1/rank)exp 1/4
			f_amount = (float)(f_amount*f_amount*f_amount) * (float)(g_moneyMaxReward.integer * factor2); // here exponent is 3

			if(g_gametype.integer == GT_DUEL)
				f_amount *= 1.75f;

			// overall exponent is 3/4
			amount = (int)f_amount;

			assaulter->client->ps.stats[STAT_MONEY] += amount;

			if(assaulter->client->ps.stats[STAT_MONEY] > g_moneyMax.integer)
				assaulter->client->ps.stats[STAT_MONEY] = g_moneyMax.integer;
		}

		victim->client->deaths++;

		if(victim->client->deaths >= g_countSocialHelp.integer){
			victim ->client->deaths = 0;
			victim->client->ps.stats[STAT_MONEY] += g_moneySocialHelp.integer;
		}

		if(victim->client->ps.stats[STAT_MONEY] > g_moneyMax.integer)
			victim->client->ps.stats[STAT_MONEY] = g_moneyMax.integer;

		if(0)
			G_Printf("Rank: %i, Amount: %i\n", rank, amount);
	}

	//G_Printf("attacker: %i inflictor: %i\n", attacker->client->ps.persistant[PERS_SCORE],
	//	inflictor->client->ps.persistant[PERS_SCORE]);
	//G_Printf("attacker: %i inflictor: %i\n", attacker->client->ps.stats[STAT_MONEY],
	//  inflictor->client->ps.stats[STAT_MONEY]);
}

static qbool CanDirectlyDamage(const gentity_t *targ, const vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin as 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	trap_Trace_New ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection,
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace_New ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace_New ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace_New ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace_New ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	return qfalse;
}


qbool G_RadiusDamage(vec3_t origin, gentity_t *attacker, const float initialDamage, const float radius,
					 gentity_t *ignore, const meansOfDeath_t mod) {
	float		damage;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	int			i, e;
	qbool		hitClient = qfalse;

	for (i = 0; i < 3; i++) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

	for (e = 0; e < numListedEntities; e++) {
		if (&g_entities[entityList[e]] == ignore || !g_entities[entityList[e]].takedamage)
			continue;

		if (CanDirectlyDamage(&g_entities[entityList[e]], origin)) {
			vec3_t		dir;

			if (!hitClient)
				hitClient = LogAccuracyHit(&g_entities[entityList[e]], attacker);

			// Find the distance from the edge of the bounding box
			// Can't use entity's origin because bmodels for most of entities (especially breakables)
			// have their origin as 0,0,0
			// And most of them are really wide or tall which makes distance too long comparing to radius
			for (i = 0; i < 3; i++)
				if (origin[i] < g_entities[entityList[e]].r.absmin[i])
					dir[i] = g_entities[entityList[e]].r.absmin[i] - origin[i];
				else if (origin[i] > g_entities[entityList[e]].r.absmax[i])
					dir[i] = origin[i] - g_entities[entityList[e]].r.absmax[i];
				else
					dir[i] = 0;
/*DEBUG_MOD
	{
		gentity_t	*tEnt = G_TempEntity(g_entities[entityList[e]].r.currentOrigin, EV_RAILTRAIL);
		tEnt->s.torsoAnim = rand() % Q_COLORS_COUNT;
		VectorCopy(origin, tEnt->s.origin2);
	}
*/
			damage = initialDamage * (1.0f - VectorLength(dir) / radius);
			if (damage < 1)
				continue;

			// push the center of mass higher than the origin so players
			// get knocked into the air more
			VectorSubtract(g_entities[entityList[e]].r.currentOrigin, origin, dir);
			dir[2] += UP_KNOCK_ADDENDUM;
			G_Damage(&g_entities[entityList[e]], NULL, attacker, dir, origin, damage, 0, mod);
		}
	}

	return hitClient;
}

static int G_GetHitLocation(const gentity_t *targ, const vec3_t point) {
	int	unitsBelowPlayerTop = targ->r.currentOrigin[2] + targ->r.maxs[2] - point[2];	

	if (unitsBelowPlayerTop < PLAYER_HEAD_Z)
		return HIT_LOCATION_HEAD;
	else if (unitsBelowPlayerTop < PLAYER_BODY_Z + PLAYER_HEAD_Z) {
		vec3_t	bulletAngle;
		int		impactRotation;

		VectorSubtract(targ->r.currentOrigin, point, bulletAngle);
		vectoangles(bulletAngle, bulletAngle);

		impactRotation = abs(360 + 45 + targ->client->ps.viewangles[YAW] - bulletAngle[YAW]) % 360;

		if (impactRotation < 90)
			return HIT_LOCATION_BACK;
		else if (impactRotation < 180)
			return HIT_LOCATION_LEFT;
		else if (impactRotation < 270)
			return HIT_LOCATION_FRONT;
		else
			return HIT_LOCATION_RIGHT;
	} else
		return HIT_LOCATION_LEGS;
}

static float G_DamageProjectileHitPlayer(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			  const vec3_t dir, const vec3_t point, float damage, const meansOfDeath_t mod) {
	int		hitLocation = HIT_LOCATION_NONE;

	if ((attacker->client && !g_friendlyFire.integer
		&& OnSameTeam(targ, attacker))
		|| targ->flags & FL_GODMODE)
		return damage;

	if (point) {
		hitLocation = G_GetHitLocation(targ, point);

		if (hitLocation == HIT_LOCATION_RIGHT
			&& targ->client->ps.weapon == WP_DYNAMITE) {
			G_InstantExplode(point, attacker);
			return damage;
		}
	}

	if (attacker->client) {
		targ->client->lasthurt_client = targ->client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		targ->client->damage_fromWorld = qfalse;
		targ->client->lasthurt_mod = mod;

		// reduce damage by the attacker's handicap value
		if ((targ->client->damage_blood = damage * attacker->client->ps.stats[STAT_MAX_HEALTH] / 100.0f) < 1)
			targ->client->damage_blood = 1;
	} else {
		targ->client->damage_fromWorld = qtrue;
		targ->client->lasthurt_mod = mod;
		targ->client->damage_blood = damage;
	}

	if (point) {
		if (targ->client->ps.stats[STAT_ARMOR] && mod != WP_SHARPS
			&& (hitLocation & (HIT_LOCATION_BACK | HIT_LOCATION_FRONT))) {
			targ->client->damage_armor = targ->client->damage_blood;
			damage = targ->client->damage_blood = 0;
			if ((targ->client->ps.stats[STAT_ARMOR] -= targ->client->damage_armor) < 0)
				targ->client->ps.stats[STAT_ARMOR] = 0;	
		}

		if (hitLocation == HIT_LOCATION_HEAD) {
			targ->client->lasthurt_anim = BOTH_DEATH_HEAD;
			targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood * HIT_LOCATION_MULTIPLIER_HEAD;
			targ->client->ps.powerups[DM_HEAD_2] = (targ->health < HEALTH_INJURED) ? 1 : 0;
			targ->client->ps.powerups[DM_HEAD_1] = (targ->health < HEALTH_USUAL) ? 1 : 0;
		} else if (hitLocation == HIT_LOCATION_LEGS) {
			targ->client->ps.stats[STAT_KNOCKTIME] = -1;
			VectorClear(targ->client->ps.velocity);
			targ->client->lasthurt_anim = (rand() % 2)? BOTH_DEATH_LEG_L : BOTH_DEATH_LEG_R;
			targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood * HIT_LOCATION_MULTIPLIER_LEGS;
			targ->client->ps.powerups[DM_LEGS_2] = (targ->health < HEALTH_INJURED) ? 1 : 0;
			targ->client->ps.powerups[DM_LEGS_1] = (targ->health < HEALTH_USUAL) ? 1 : 0;
		} else {
			if (hitLocation == HIT_LOCATION_LEFT) {
				targ->client->lasthurt_anim = BOTH_DEATH_ARM_L;
				targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood * HIT_LOCATION_MULTIPLIER_LEFT;
			} else if (hitLocation == HIT_LOCATION_RIGHT) {
				targ->client->lasthurt_anim = BOTH_DEATH_ARM_R;
				targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood * HIT_LOCATION_MULTIPLIER_RIGHT;
			} else if (hitLocation == HIT_LOCATION_FRONT) {
				targ->client->lasthurt_anim = (rand() % 2)? BOTH_DEATH_CHEST : BOTH_DEATH_STOMACH;
				targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood * HIT_LOCATION_MULTIPLIER_FRONT;
			} else if (hitLocation == HIT_LOCATION_BACK) {
				targ->client->lasthurt_anim = BOTH_DEATH_DEFAULT;
				targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood * HIT_LOCATION_MULTIPLIER_BACK;
			}
			targ->client->ps.powerups[DM_TORSO_2] = (targ->health < HEALTH_INJURED) ? 1 : 0;
			targ->client->ps.powerups[DM_TORSO_1] = (targ->health < HEALTH_USUAL) ? 1 : 0;
		}
	} else {
		targ->client->ps.stats[STAT_HEALTH] = targ->health -= targ->client->damage_blood;
	}

	if (attacker->client && !targ->client->tEntFireWeapon) {
		vec3_t	normal;
		targ->client->tEntFireWeapon = G_TempEntity(point, EV_PLAYER_HIT);
		targ->client->tEntFireWeapon->r.svFlags |= SVF_BROADCAST;
		targ->client->tEntFireWeapon->s.clientNum = targ->s.number;
		VectorCopy(dir, normal);
		VectorScale(normal, -1, normal);
		targ->client->tEntFireWeapon->s.torsoAnim = DirToByte(normal);
		targ->client->tEntFireWeapon->s.otherEntityNum = attacker->s.number;
	}
	targ->client->tEntFireWeapon->s.eventParm |= hitLocation;

	if (dir && mod < WP_NUM_WEAPONS) {
		vec3_t	knockbackVelocity;
		float	damageRatio = damage / bg_weaponlist[mod].damage;

		VectorCopy(dir, targ->client->damage_from);
		VectorNormalize(targ->client->damage_from);
		VectorScale(targ->client->damage_from, bg_weaponlist[mod].knockback * damageRatio, knockbackVelocity);
		VectorAdd(targ->client->ps.velocity, knockbackVelocity, targ->client->ps.velocity);
		
		damageRatio *= MAX_KNOCKBACKTIME;
		if (damageRatio > targ->client->ps.pm_time) {
			targ->client->ps.pm_time = (damageRatio < MIN_KNOCKBACKTIME) ? MIN_KNOCKBACKTIME : damageRatio;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	if (targ->health <= 0) {
		targ->health = 0;
		targ->enemy = attacker;
		targ->die(targ, inflictor, attacker, targ->client->damage_armor + targ->client->damage_blood, mod);
	}

	return damage * DAMAGE_LOSE_ON_PLAYER;
}


float G_Damage(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   const vec3_t dir, const vec3_t point, const float damage, const int dflags, const meansOfDeath_t mod) {
	if (!targ->takedamage || level.intermissionQueued || targ->health <= 0)
		return damage;

	if (targ->client && attacker && !(dflags & DAMAGE_INSTANT_KILL))
		return G_DamageProjectileHitPlayer(targ, inflictor, attacker, dir, point, damage, mod);

	// shootable doors / buttons don't actually have any health
	if (targ->s.eType == ET_MOVER) {
		if (targ->use && targ->moverState == MOVER_POS1)
			targ->use(targ, inflictor, attacker);
		return damage;
	}

	//if the breakable only accepts damage of specific weapon(s)
	if (targ->s.eType == ET_BREAKABLE
		&& targ->s.weapon && mod != targ->s.weapon)
			return damage;

	if (dflags & DAMAGE_INSTANT_KILL) {
		targ->health = 0;
		if (targ->client)
			targ->client->ps.stats[STAT_HEALTH] = 0;
	} else
		targ->health -= damage;

	if (targ->health <= 0) {
		targ->health = 0;
		targ->enemy = attacker;
		targ->die(targ, inflictor, attacker, damage, mod);
	}

	return damage;
}
