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
// g_weapon.c
// perform the server side effects of a weapon firing

#include "g_local.h"

static	vec3_t	forward, right, up;
static	vec3_t	muzzle;
static	vec3_t	endpos; // used for shoot thru walls code
static	qboolean shootthru;

#define NUM_NAILSHOTS 10

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET(KNIFE)

======================================================================
*/

void Weapon_Gauntlet( gentity_t *ent ) {

}

void weapon_knife_fire (gentity_t *ent) {
	gentity_t	*m;

	VectorNormalize( forward );

	m = fire_knife (ent, muzzle, forward, 1200);

	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}

/*
===============
CheckKnifeAttack
===============
*/
qboolean CheckKnifeAttack( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	gentity_t	*traceEnt;
	int			damage = bg_weaponlist[WP_KNIFE].damage;
	vec3_t		mins,maxs;
	int shaderNum;
	int i;
	usercmd_t *ucmd;

	if(ent->client->ps.stats[STAT_FLAGS] & SF_WP_CHOOSE){
		return qfalse;
	}

	ucmd = &ent->client->pers.cmd;
	// If reload button is pressed at the same time as the attack button,
	// disable knife action.
	// The same applies to any "talk action"
	// (say, say_team, opening console and buy choosing action)
	if (ucmd->buttons & (BUTTON_RELOAD | BUTTON_TALK))
		return qfalse;

#define BOX 20
	VectorSet(mins, -BOX, -BOX, -BOX);
	VectorSet(maxs, BOX, BOX, BOX);

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

	VectorMA (muzzle, 10, forward, end);

	// don't know, but this has to be donw
	for(i=0; i<MAX_CLIENTS; i++){
		gentity_t *t;

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR)
			continue;

		t = &g_entities[i];

		trap_LinkEntity(t);
	}
	trap_Trace_New (&tr, muzzle, mins, maxs, end, ent->s.number, MASK_SHOT);

	traceEnt = &g_entities[ tr.entityNum ];

	// slit the wall
	if ( (tr.surfaceFlags & SURF_NOIMPACT || tr.startsolid || tr.allsolid) &&
		!traceEnt->client) {

		// do another trace
		CalcMuzzlePoint ( ent, forward, right, up, muzzle );
		VectorMA (muzzle, 32, forward, end);
		shaderNum = trap_Trace_New2 (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);

		traceEnt = &g_entities[ tr.entityNum ];

		if(tr.contents & CONTENTS_SOLID){
			gentity_t *tent;
			vec3_t dir;

			VectorAdd(tr.plane.normal, traceEnt->s.pos.trDelta, dir);
			VectorNormalize(dir);

			tent = G_TempEntity( tr.endpos, EV_KNIFEHIT );
			// solid was hit
			tent->s.eventParm = DirToByte( dir );
			tent->s.time2 = tr.surfaceFlags;
			tent->s.torsoAnim = shaderNum;
			tent->r.svFlags |= SVF_BROADCAST;
		}

		if(traceEnt->takedamage){
			goto wall;
		}
		return qfalse;
	}

	// check if really hit
	if ( traceEnt->takedamage && traceEnt->client ) {

		ent->client->lasthurt_part = PART_UPPER;

	}

wall:
	if ( !traceEnt->takedamage) {
		return qfalse;
	}

	if(damage){
		gentity_t *tent;
		vec3_t dir;

		VectorAdd(tr.plane.normal, traceEnt->s.pos.trDelta, dir);
		VectorNormalize(dir);

		tent = G_TempEntity( tr.endpos, EV_KNIFEHIT );
		// no wall was hit
		tent->s.torsoAnim = -1;
		tent->s.weapon = traceEnt->s.number;
		tent->s.eventParm = DirToByte( dir );
		tent->r.svFlags |= SVF_BROADCAST;

		if(tr.contents & CONTENTS_SOLID && traceEnt->s.eType == ET_BREAKABLE &&
			!(traceEnt->flags & FL_BREAKABLE_INIT)){
			shaderNum = trap_Trace_New2 (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);

			if(tr.entityNum == traceEnt->s.number){
				G_BreakablePrepare(traceEnt, shaderNum);
			}
		}
	}

	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, 0, MOD_KNIFE );

	return qtrue;
}


/*
=================
//modify the damage by calculating the distance
=================
*/
float Modify_BulletDamage(float damage, int weapon, vec3_t start, vec3_t end){
	float distance;
	//X+Y-Axis
	distance = sqrt((start[0]-end[0])*(start[0]-end[0]) +
		(start[1]-end[1])*(start[1]-end[1]));
	//Z-Axis
	distance = sqrt(distance*distance + (start[2]-end[2])*
		(start[2]-end[2]));

	if(distance > bg_weaponlist[weapon].range){
		damage -= (1/sqrt(bg_weaponlist[weapon].range/distance))*2;
	}

	//G_Printf("distance:%f, damage: %f org. damage: %f\n", distance, damage,
	//	bg_weaponlist[weapon].damage);

	return damage > 0 ? damage : 0 ;
}

/*
=====================
G_BreakablePrepare
by: Spoon
3.8.2001

Checks if a ET_BREAKABLE was hit->save surfacetype
======================
*/
void G_BreakablePrepare(gentity_t *ent, int shaderNum){

	if(ent->s.eType != ET_BREAKABLE)
		return;

	//save surfaceFlags
	if(ent->count != -1 && shaderNum == -1)
		return;

	ent->count = shaderNum;
}

#define MACHINEGUN_SPREAD	200
#define	MACHINEGUN_DAMAGE	7
#define	MACHINEGUN_TEAM_DAMAGE	7		// wimpier MG in teamplay

void Bullet_Fire (gentity_t *ent, float spread, float damage, const int weapon ) {
	trace_t		tr;
	vec3_t		end;
	float		r;
	float		u;
	gentity_t	*tent = NULL, *tent2;
	gentity_t	*traceEnt;
	int			passent;
	int			shaderNum, shaderNum2; //shaderNum2 used for shoot-thru
	int			location;
	vec3_t		minsback[MAX_CLIENTS], maxsback[MAX_CLIENTS];
	int i, shootcount = 0;

//unlagged - backward reconciliation #2
		// backward-reconcile the other clients
		G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	for(i=0; i<MAX_CLIENTS; i++){
		gentity_t *t;

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR)
			continue;

		t = &g_entities[i];

		VectorCopy(t->r.mins, minsback[i]);
		VectorCopy(t->r.maxs, maxsback[i]);
		VectorCopy(playerMins_hit, t->r.mins);
		VectorCopy(playerMaxs_hit, t->r.maxs);
		trap_LinkEntity(t);
	}

		//smoke puff
	if(weapon > WP_KNIFE &&
		weapon < WP_WINCHESTER66){
		tent2 = G_TempEntity( muzzle, EV_SMOKE );
		VectorScale( forward, 4096, tent2->s.origin2 );
		SnapVector( tent2->s.origin2 );
		// save client
		tent2->s.frame = ent->s.number;
	}

	r = random() * M_PI * 2.0f;
	u = sin(r) * crandom() * spread * 16;
	r = cos(r) * crandom() * spread * 16;

pistolfire:
	shootthru = qfalse;
	VectorMA (muzzle, 8192*16, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	passent = ENTITYNUM_NONE;//you should be able to shoot your own missiles...
	// unlink player for trace

	trap_UnlinkEntity(ent);
	shaderNum = trap_Trace_New2(&tr, muzzle, NULL, NULL, end, passent, MASK_SHOT);
	trap_LinkEntity(ent);

	//check if water was hit
	// check done in cgame now G_WaterWasHit(muzzle, end, passent);

	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		//NT - make sure we un-time-shift the clients
        goto untimeshift;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	if ( traceEnt->takedamage){
		damage = Modify_BulletDamage(damage, weapon, ent->r.currentOrigin, tr.endpos);

		//check if type is breakable
		G_BreakablePrepare(traceEnt, shaderNum);

		if(traceEnt->client){
			vec3_t dir;
			gclient_t	*client = traceEnt->client;
			int count = 0;

			location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
				client->legs_angles, client->torso_angles, client->ps.viewangles,
				&client->torso, &client->legs, muzzle, tr.endpos);


			// check as long a model and a wall was not hit
			while(location == -1){
				count++;

				if(count >= 100){
					G_Printf("Error: Too many traces\n");
					G_Printf("Error: %.1f pistol pellet damage expected on entity %d (%s)\n",
						damage, tr.entityNum, client->pers.netname );
					G_Printf("Error: start entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						ent->s.number, ent->client->pers.netname,
						ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
					G_Printf("Error: end entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						tr.entityNum, client->pers.netname, tr.endpos[0], tr.endpos[1], tr.endpos[2] );
					if (count > 100)
						goto untimeshift;
				}

				//do another trace
				VectorMA (tr.endpos, 8192*16, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);

				shaderNum = trap_Trace_New2(&tr, tr.endpos, NULL, NULL, end, tr.entityNum, MASK_SHOT);

				damage = Modify_BulletDamage(damage, weapon, ent->r.currentOrigin, tr.endpos);

				if ( damage < 1.0f ) {
					// Tequila comment: Stop anyway when no damage can be applied
					goto untimeshift;
				}

				if ( tr.surfaceFlags & SURF_NOIMPACT ) {
					//NT - make sure we un-time-shift the clients
					goto untimeshift;
				}

				traceEnt = &g_entities[ tr.entityNum ];

				//if its an damageable entity, but no player
				if(!traceEnt->client){
					if(traceEnt->takedamage){
						G_BreakablePrepare(traceEnt, shaderNum);

						G_Damage( traceEnt, ent, ent, forward, tr.endpos,
							damage, 0, weapon);
					}
					goto wall;
				}

				client = traceEnt->client;

				location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
					client->legs_angles, client->torso_angles, client->ps.viewangles,
					&client->torso, &client->legs, muzzle, tr.endpos);
			}

			// snap the endpos to integers, but nudged towards the line
			SnapVectorTowards( tr.endpos, muzzle );

			client->lasthurt_location = location;
			client->lasthurt_part = hit_info[location].hit_part;

			VectorAdd(tr.plane.normal, traceEnt->s.pos.trDelta, dir);
			VectorNormalize(dir);

			tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_FLESH );
			tent->s.eventParm = traceEnt->s.number;
			tent->s.eventParm = DirToByte( dir );

			G_Damage( traceEnt, ent, ent, forward, tr.endpos,
				damage, 0, weapon);
		} else {
			G_Damage( traceEnt, ent, ent, forward, tr.endpos,
				damage, 0, weapon);
		}
	}

//mark
wall:
	// snap the endpos to integers, but nudged towards the line
	if(g_entities[tr.entityNum].s.eType != ET_BREAKABLE)
		SnapVectorTowards( tr.endpos, muzzle );

	// send bullet impact
	if(traceEnt->client && traceEnt->client->lasthurt_mod == MOD_BOILER){
		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_BOILER );
		tent->s.eventParm = DirToByte( tr.plane.normal );
	} else if ( traceEnt->takedamage && traceEnt->client ) {
	} else {

		tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.time2 = tr.surfaceFlags;
		tent->s.otherEntityNum2 = tr.entityNum;

		// if we hit a breakable
		if(g_entities[tr.entityNum].s.eType == ET_BREAKABLE)
			tent->s.time2 |= SURF_BREAKABLE;

		tent->s.torsoAnim = shaderNum;

		// look if the weapon is able to shoot through the wall
		shootthru = BG_ShootThruWall(&damage, tr.endpos, muzzle, tr.surfaceFlags, endpos,
			trap_Trace_New);
	}
	// Avoid compilation warning
	if (tent) {
		tent->r.svFlags |= SVF_BROADCAST;
		tent->s.time = weapon;
		tent->s.otherEntityNum = ent->s.number;
	}

	if(shootthru && (!traceEnt->takedamage || traceEnt->health > 0)){
		// do another mark on the other side of the wall, but only if it still exists
		gentity_t *t;

		// get information of wall
		shaderNum2 = trap_Trace_New2( &tr, endpos, NULL, NULL, muzzle, passent, (MASK_SOLID|CONTENTS_BODY));

		t = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
		t->s.eventParm = DirToByte( tr.plane.normal );
		t->s.time2 = tr.surfaceFlags;
		t->s.torsoAnim = shaderNum2;
		t->s.otherEntityNum2 = tr.entityNum;

		// if we hit a breakable
		if(g_entities[tr.entityNum].s.eType == ET_BREAKABLE){
			t->s.time2 |= SURF_BREAKABLE;
		}

		t->r.svFlags |= SVF_BROADCAST;
		t->s.time = weapon;
		t->s.otherEntityNum = ent->s.number;
	}

	if(shootthru){
		shootcount++;
		if(shootcount < 10){
			VectorCopy(endpos, muzzle);
			goto pistolfire;
		}
	}

untimeshift:
	for(i=0; i<MAX_CLIENTS; i++){
		gentity_t *t;

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR)
			continue;

		t = &g_entities[i];

		VectorCopy(minsback[i], t->r.mins);
		VectorCopy(maxsback[i], t->r.maxs);
		trap_LinkEntity(t);
	}

//unlagged - backward reconciliation #2
		// put them back
		G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2
}


/*
======================================================================

SHOTGUN

======================================================================
*/

// DEFAULT_SHOTGUN_SPREAD and DEFAULT_SHOTGUN_COUNT	are in bg_public.h, because
// client predicts same spreads
#define	DEFAULT_SHOTGUN_DAMAGE	10

qboolean ShotgunPellet( float r, float u, vec3_t start, vec3_t end, gentity_t *ent) {
	trace_t		tr;
	float		damage = bg_weaponlist[ent->client->ps.weapon].damage;
	int			passent;
	gentity_t	*traceEnt;
	gentity_t	*tent;
	vec3_t		tr_start, tr_end;
	int			shaderNum;
	int			shootcount = 0;

	passent = ENTITYNUM_NONE; // you should be able to shoot your own missile
	VectorCopy( start, tr_start );
	VectorCopy( end, tr_end );

shotgunfire:
	shootthru = qfalse;

	shaderNum = trap_Trace_New2 (&tr, tr_start, NULL, NULL, tr_end, passent, MASK_SHOT);
	traceEnt = &g_entities[ tr.entityNum ];

	// send bullet impact
	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	if ( traceEnt->takedamage) {
		int location;
		gclient_t *client = traceEnt->client;
		float	olddamage = damage;

		damage = Modify_BulletDamage(damage, ent->client->ps.weapon, ent->r.currentOrigin,
			tr.endpos);

		damage = (damage+olddamage)/2;

		//if it hit a player
		if(traceEnt->client){
			int i;
			int count = 0;

			location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
				client->legs_angles, client->torso_angles, client->ps.viewangles,
				&client->torso, &client->legs, muzzle, tr.endpos);

			// check as long a model was not hit or a wall was not hit
			while(location == -1){
				vec3_t	dir;
				count++;

				if(count >= 100){
					G_Printf("Error: Too many traces\n");
					G_Printf("Error: %.1f shotgun pellet damage expected on entity %d (%s)\n",
						damage, tr.entityNum, client->pers.netname );
					G_Printf("Error: start entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						ent->s.number, ent->client->pers.netname,
						ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
					G_Printf("Error: end entity %d (%s) at [%.1f;%.1f;%.1f]\n",
						tr.entityNum, client->pers.netname, tr.endpos[0], tr.endpos[1], tr.endpos[2] );
					if (count > 100)
						return qfalse;
				}

				VectorSubtract(tr_end, tr_start, dir);
				VectorNormalize(dir);

				//do another trace
				VectorMA (tr.endpos, 8192*16, dir, tr_end);

//				G_LogPrintf("shooting through a player\n");

				shaderNum = trap_Trace_New2(&tr, tr.endpos, NULL, NULL, tr_end, tr.entityNum, MASK_SHOT);

				damage = Modify_BulletDamage(damage, ent->client->ps.weapon, ent->r.currentOrigin, tr.endpos);

				if ( damage < 1.0f ) {
					// Tequila comment: Stop when no damage can be applied
					return qfalse;
				}

				if ( tr.surfaceFlags & SURF_NOIMPACT ) {
					return qfalse;
				}

				traceEnt = &g_entities[ tr.entityNum ];

				//if its an damageable entity, but no player
				if(!traceEnt->client){
					if(traceEnt->takedamage){
//						G_LogPrintf("hit something damageable\n");

						//check if type is breakable
						G_BreakablePrepare(traceEnt, shaderNum);

						G_Damage( traceEnt, ent, ent, forward, tr.endpos,
							damage, 0, ent->client->ps.weapon);
					}
					goto wall;
				}

				client = traceEnt->client;

				location = G_HitModelCheck(&hit_data, traceEnt->s.pos.trBase,
					client->legs_angles, client->torso_angles, client->ps.viewangles,
					&client->torso, &client->legs, muzzle, tr.endpos);
			}

			client->lasthurt_location = location;
			client->lasthurt_part = hit_info[location].hit_part;

			if(traceEnt->client && traceEnt->client->lasthurt_mod == MOD_BOILER){
				tent = G_TempEntity( tr.endpos, EV_BOILER_HIT );
				tent->s.eventParm = traceEnt->s.number;
				tent->r.svFlags |= SVF_BROADCAST;
			}
			ent->client->lasthurt_client = tr.entityNum;

			// don't send a hit-message for each pellet, so store 3 locations
			for(i=0;i<3;i++){
				//  this location is already recognized
				if(ent->s.angles2[i] != -1 && (
					!Q_stricmp(hit_info[location].backname,
					hit_info[(int)ent->s.angles2[i]].backname) ||
					!Q_stricmp(hit_info[location].forename,
					hit_info[(int)ent->s.angles2[i]].forename)))
					break;

				if(ent->s.angles2[i] != -1)
					continue;

				ent->s.angles2[i] = location;
				break;
			}
			ent->s.eFlags |= EF_HIT_MESSAGE;

			G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, ent->client->ps.weapon);
			return qtrue;
		}

		//check if type is breakable
		G_BreakablePrepare(traceEnt, shaderNum);

		G_Damage( traceEnt, ent, ent, forward, tr.endpos, damage, 0, ent->client->ps.weapon);

wall:
		if(traceEnt->client && traceEnt->client->lasthurt_mod == MOD_BOILER){
			tent = G_TempEntity( tr.endpos, EV_BOILER_HIT );
			tent->s.eventParm = traceEnt->s.number;
			tent->r.svFlags |= SVF_BROADCAST;
		}
	}

	if ( traceEnt->takedamage && traceEnt->client ) {
	} else { //wall was hit

#define DAM_FACTOR 0.5f
		// modify damage for a short time, cause shotguns have particles with low damage
		damage *= DAM_FACTOR;

		// look if the weapon is able to shoot through the wall
		shootthru = BG_ShootThruWall(&damage, tr.endpos, tr_start, tr.surfaceFlags, endpos,
			trap_Trace);

		damage /= DAM_FACTOR;

		if(shootthru){
//			G_LogPrintf("shooting thru wall\n");
			VectorCopy(endpos, tr_start);
			VectorMA( tr_start, 8192 * 16, forward, tr_end);
			VectorMA (tr_end, r, right, tr_end);
			VectorMA (tr_end, u, up, tr_end);

			shootcount++;
			if(shootcount < 10)
				goto shotgunfire;
		}
	}
	return qfalse;
}

// this should match CG_ShotgunPattern
int ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, gentity_t *ent, qboolean altfire ) {
	int		i;
	float		r, u;
	float           spread_dist , spread_angle , angle_shift , current_angle_shift ;
	float           max_spread_circle , current_spread_circle , extra_circle ;
	int             current_spread_cell , pellet_per_circle , extra_center_pellet , current_pellet_per_circle ;
	vec3_t		end;
	vec3_t		forward, right, up;
	int			count = bg_weaponlist[ent->client->ps.weapon].count;
	int			playerhitcount = 0;
	gentity_t	*tent;
	vec3_t		minsback[MAX_CLIENTS], maxsback[MAX_CLIENTS];

//unlagged - backward reconciliation #2
	// backward-reconcile the other clients
	G_DoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	for(i=0; i<MAX_CLIENTS; i++){
		gentity_t *t;

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR)
			continue;

		t = &g_entities[i];

		VectorCopy(t->r.mins, minsback[i]);
		VectorCopy(t->r.maxs, maxsback[i]);
		VectorCopy(playerMins_hit, t->r.mins);
		VectorCopy(playerMaxs_hit, t->r.maxs);
		trap_LinkEntity(t);
	}

	if(altfire)
		count *= 2;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// clear hit-locations
	for( i=0; i<3; i++){
		ent->s.angles2[i] = -1;
	}

	// unlink player first
	trap_UnlinkEntity(ent);
	
	if ( g_newShotgunPattern.integer ) {
		
		// Joe Kari: new experimental shotgun pattern //
		
		// generate the "random" spread pattern
		
		switch ( count )  {
			case 14 :
			case 28 :
				pellet_per_circle = 7 ;
				extra_center_pellet = 0 ;
				break ;
			case 6 :
				pellet_per_circle = 5 ;
				extra_center_pellet = 1 ;
				break ;
			default :
				pellet_per_circle = 6 ;
				extra_center_pellet = 0 ;
		}
		max_spread_circle = count / pellet_per_circle ;
		if ( max_spread_circle < 1 )  max_spread_circle = 1 ;
		angle_shift = Q_random( &seed ) * M_PI * 2.0f ;
		if ( extra_center_pellet > 0 )  {
			extra_circle = (float)extra_center_pellet / (float)pellet_per_circle ;
			max_spread_circle += extra_circle ;
		}
		
		for ( i = - extra_center_pellet ; i < count - extra_center_pellet ; i++ ) {
			
			if ( extra_center_pellet > 0 )  {
				if ( i < 0 )  {
					current_spread_circle = 0 ;
					current_pellet_per_circle = extra_center_pellet ;
				}
				else  {
					current_spread_circle = extra_circle + i / pellet_per_circle ;
					current_pellet_per_circle = pellet_per_circle ;
				}
				current_spread_cell = i - current_spread_circle * current_pellet_per_circle ;
			}
			else  {
				current_spread_circle = i / pellet_per_circle ;
				current_pellet_per_circle = pellet_per_circle ;
				current_spread_cell = i - current_spread_circle * current_pellet_per_circle ;
			}
			current_angle_shift = angle_shift + current_spread_circle * M_PI / (float)current_pellet_per_circle ;
			
			spread_dist = ( current_spread_circle + Q_random( &seed ) ) / max_spread_circle * bg_weaponlist[ent->client->ps.weapon].spread * 16 ;
			// spread adjustement to keep the same spread feeling:
			spread_dist *= 1.4f ;
			
			spread_angle = current_angle_shift + ( (float)current_spread_cell + Q_random( &seed ) ) * M_PI * 2.0f / (float)current_pellet_per_circle ;
			
			r = sin( spread_angle ) * spread_dist ;
			u = cos( spread_angle ) * spread_dist ;
			
			VectorMA( origin, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);


			if( ShotgunPellet( r, u, origin, end, ent)){
				if((i+1) < 16)
					playerhitcount |= (1 << (i+1));
			}

		}
		
		// End (Joe Kari) //
		
	} else {
		// generate the "random" spread pattern
		for ( i = 0 ; i < count ; i++ ) {

			r = Q_crandom( &seed ) * bg_weaponlist[ent->client->ps.weapon].spread * 16;
			u = Q_crandom( &seed ) * bg_weaponlist[ent->client->ps.weapon].spread * 16;
			VectorMA( origin, 8192 * 16, forward, end);
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);
			
			if( ShotgunPellet( r, u, origin, end, ent)){
				if((i+1) < 16)
					playerhitcount |= (1 << (i+1));
			}
		
		}
	}
	
	
	trap_LinkEntity(ent);

	if(ent->s.angles2[0] != -1 &&
		(ent->s.eFlags & EF_HIT_MESSAGE)){
		// send the hit message
		tent = G_TempEntity( vec3_origin, EV_HIT_MESSAGE );

		if(ent->client->lasthurt_direction == LOCATION_BACK)
			tent->s.weapon = 0;
		else
			tent->s.weapon = 1;

		tent->s.frame = -1;
		tent->s.otherEntityNum = ent->client->lasthurt_victim;
		tent->s.otherEntityNum2 = ent->s.number;
		tent->r.svFlags |= SVF_BROADCAST;
		tent->s.angles2[0] = ent->s.angles2[0];
		tent->s.angles2[1] = ent->s.angles2[1];
		tent->s.angles2[2] = ent->s.angles2[2];
	}

	for(i=0; i<MAX_CLIENTS; i++){
		gentity_t *t;

		if(level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		if(level.clients[i].sess.sessionTeam >= TEAM_SPECTATOR)
			continue;

		t = &g_entities[i];

		VectorCopy(minsback[i], t->r.mins);
		VectorCopy(maxsback[i], t->r.maxs);
		trap_LinkEntity(t);
	}

//unlagged - backward reconciliation #2
	// put them back
	G_UndoTimeShiftFor( ent );
//unlagged - backward reconciliation #2

	ent->s.eFlags &= ~EF_HIT_MESSAGE;

	return playerhitcount;
}

void weapon_supershotgun_fire (gentity_t *ent, qboolean altfire) {
	gentity_t		*tent;
	int				temp = ent->client->lasthurt_client;
	int				playerhitcount;

	// send shotgun blast
	tent = G_TempEntity( muzzle, EV_SHOTGUN );
	VectorScale( forward, 4096, tent->s.origin2 );
	SnapVector( tent->s.origin2 );
	tent->s.eventParm = rand() & 255;		// seed for spread pattern
	tent->s.otherEntityNum = ent->s.number;

	tent->s.weapon = ent->client->ps.weapon;
	tent->s.angles[0] = altfire;

//	G_LogPrintf("\nShotgun Fired Start\n");

	playerhitcount = ShotgunPattern( tent->s.pos.trBase, tent->s.origin2, tent->s.eventParm, ent, altfire );

	tent->s.clientNum = ent->client->lasthurt_client;
	tent->s.angles[1] = playerhitcount;

	ent->client->lasthurt_client = temp;

//	G_LogPrintf("Shotgun Fired End\n\n");
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (gentity_t *ent) {
	gentity_t	*m;
	trace_t		tr;
	vec3_t		mins, maxs;

	// extra vertical velocity
	forward[2] += 0.1f;
	VectorNormalize( forward );

	//test if it is starting in solid
	VectorSet(mins, -8, -8 , -8);
	VectorSet(maxs, 8, 8, 8);

	do {
		trap_Trace_New( &tr, muzzle, mins, maxs, muzzle, -1, MASK_SOLID );

		if(tr.startsolid){
			// A solid is detected, move forward the starting vector position
			// by a unit of 0.3 until the weapon do not collide a solid
			// This happened especially when we throw the object, the back on a wall !
			VectorMA(muzzle, 0.3f, forward, muzzle);
		}

	} while(tr.startsolid);

	m = fire_dynamite (ent, muzzle, forward, DYNA_SPEED);
}

/*
====================
Molotov Cocktail
====================
*/

void weapon_molotov_fire (gentity_t *ent) {
	gentity_t	*m;
	trace_t		tr;
	vec3_t		mins, maxs;

	// extra vertical velocity
	forward[2] += 0.1f;
	VectorNormalize( forward );

	//test if it is starting in solid
	VectorSet(mins, -8, -8 , -8);
	VectorSet(maxs, 8, 8, 8);

	do {
		trap_Trace_New( &tr, muzzle, mins, maxs, muzzle, -1, MASK_SOLID );

		if(tr.startsolid){
			// A solid is detected, move forward the starting vector position
			// by a unit of 0.3 until the weapon do not collide a solid
			// This happened especially when we throw the object, the back on a wall !
			VectorMA(muzzle, 0.3f, forward, muzzle);
		}

	} while(tr.startsolid);

	m = fire_molotov (ent, muzzle, forward, MOLOTOV_SPEED);
}

//======================================================================

/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->client->ps.origin, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;

	// new eye system, makes it possible to show the legs
	G_ModifyEyeAngles(muzzlePoint, ent->client->ps.viewangles, qfalse);

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->client->ps.origin, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;

	// new eye system, makes it possible to show the legs
	G_ModifyEyeAngles(muzzlePoint, ent->client->ps.viewangles, qfalse);
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}



/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent, qboolean altfire, int weapon ) {
	float spread = bg_weaponlist[weapon].spread;
	float damage = bg_weaponlist[weapon].damage;

	if(ent->client->movestate & MS_WALK){
		spread *= 1.4f;
	}

	if(ent->client->movestate & MS_CROUCHED) {
		spread *= 0.65f;
	} else if(ent->client->movestate & MS_JUMP) {
		spread *= 5.0f;
	}

	// adjust spread/damage for gametypes
	if(g_gametype.integer == GT_DUEL){
		if(weapon != WP_PEACEMAKER)
			spread *= 0.8f;
		if(weapon == WP_PEACEMAKER)
			damage *= 0.9f;
	} else if(weapon != WP_NONE){
		gitem_t *item = BG_FindItemForWeapon(weapon);

		spread *= 0.8f;

		if(item->weapon_sort != WS_SHOTGUN){
			spread *= 0.55f;
		}
	}

	// if playing duel, change accuracy at the beginning of the round
	if(g_gametype.integer && du_introend - DU_INTRO_DRAW - DU_INTRO_CAM <= level.time
		&& du_introend + DU_CROSSHAIR_FADE >= level.time){
		float factor = 1.0f;

		if(du_introend + DU_CROSSHAIR_START <= level.time){
			factor = (float)(level.time - du_introend - DU_CROSSHAIR_START)/DU_CROSSHAIR_FADE;
			factor = 1-factor;
		}

		spread += factor*2000;
	}

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );

	// fire the specific weapon
	switch( weapon ) {
	case WP_KNIFE:
		if(ent->client->ps.stats[STAT_WP_MODE])
			weapon_knife_fire( ent );
		break;
	case WP_SAWEDOFF:
	case WP_REMINGTON_GAUGE:
	case WP_WINCH97:
		weapon_supershotgun_fire( ent, altfire);
		break;
	case WP_WINCHESTER66:
	case WP_LIGHTNING:
	case WP_SHARPS:
	case WP_GATLING:
		if(ent->client->ps.powerups[PW_SCOPE] == 2 && ent->client->ps.stats[STAT_WP_MODE] != 1)
			spread *= 2;
		else if(ent->client->ps.powerups[PW_SCOPE] == 2 && ent->client->ps.stats[STAT_WP_MODE] == 1)
			spread /= 2;

		Bullet_Fire( ent, spread, damage, weapon );
		break;
	case WP_PEACEMAKER:
	case WP_REM58:
	case WP_SCHOFIELD:
		if(altfire)
			Bullet_Fire( ent, spread*3, damage, weapon );
		else
			Bullet_Fire( ent, spread, damage, weapon );
		break;
	case WP_DYNAMITE:
		weapon_grenadelauncher_fire( ent );
		ent->client->ps.stats[STAT_WP_MODE] = 0;
		break;
	case WP_MOLOTOV:
		weapon_molotov_fire( ent);
		ent->client->ps.stats[STAT_WP_MODE] = 0;
		break;
	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}