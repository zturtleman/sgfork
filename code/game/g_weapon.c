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
#include "bg_weapon.h"

void CheckCloseCombatKnifeAttack(gentity_t *ent) {
	trace_t	tr;				//Trace object
	int		shaderNum;		//Material number in prefixInfo array
	vec3_t	currentOrigin;	//Muzzle origin for the current trace
	vec3_t	end;			//Trace end point
	vec3_t	forward;		//Forward direction of the hitting player

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	CalcMuzzlePoint(ent, currentOrigin);
	VectorMA(currentOrigin, KNIFE_MAX_DISTANCE_HIT_UNITS, forward, end);

	shaderNum = trap_Trace_New2(&tr, currentOrigin, NULL, NULL, end, ent->s.number, MASK_SHOT);
/*DEBUG_MOD
	{
		gentity_t	*tEnt = G_TempEntity(tr.endpos, EV_RAILTRAIL);
		tEnt->s.torsoAnim = 1;//rand() % Q_COLORS_COUNT;
		VectorCopy(currentOrigin, tEnt->s.origin2);
	}

	{
		gentity_t	*tEnt = G_TempEntity(ent->client->ps.origin, EV_RAILTRAIL);
		tEnt->s.torsoAnim = 2;//rand() % Q_COLORS_COUNT;
		VectorCopy(currentOrigin, tEnt->s.origin2);
	}
*/
	if ((tr.fraction == 1.0f) || (tr.surfaceFlags & SURF_NOIMPACT))
		return;

	if (g_entities[tr.entityNum].client) {
		G_Damage(&g_entities[tr.entityNum], ent, ent, forward, tr.endpos, bg_weaponlist[WP_KNIFE].damage, 0, WP_KNIFE);
		return;
	}

	if (g_entities[tr.entityNum].takedamage) {
		G_BreakablePrepare(&g_entities[tr.entityNum], shaderNum);
		G_Damage(&g_entities[tr.entityNum], ent, ent, forward, tr.endpos, bg_weaponlist[WP_KNIFE].damage, 0, WP_KNIFE);
	}

	if (!g_entities[tr.entityNum].takedamage || g_entities[tr.entityNum].health > 0) {
		gentity_t	*tEnt = G_TempEntity(tr.endpos, EV_KNIFEHIT);
		tEnt->s.eventParm = DirToByte(tr.plane.normal);
		tEnt->s.torsoAnim = tr.surfaceFlags;
		tEnt->s.otherEntityNum = tr.entityNum;
		tEnt->s.weapon = shaderNum;
		tEnt->r.svFlags |= SVF_BROADCAST;
	}
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

static void BulletFire(const gentity_t *ent, const float spread, const int weapon, const vec3_t forward, const vec3_t muzzle) {
	gentity_t	*tEnt = G_TempEntity(muzzle, EV_BULLET);
	float		initialDamage = (g_gametype.integer == GT_DUEL && weapon == WP_PEACEMAKER) ? bg_weaponlist[weapon].damage * 0.9f : bg_weaponlist[weapon].damage;
	int			seed = rand() & 255;	//Seed for spread pattern
	vec3_t		smallStep;				//Small step (~1 unit) in the fire direction
	vec3_t		traceEnd;				//Trace end point at the maximum allowed distance
	vec3_t		end;					//Trace end point with spread taken into account
	vec3_t		right, up;

	VectorScale(forward, 4096, tEnt->s.origin2);
	SnapVector(tEnt->s.origin2);
	tEnt->s.eventParm = seed;
	tEnt->s.otherEntityNum = ent->s.number;
	tEnt->s.weapon = weapon;
	tEnt->s.angles[0] = spread;
	tEnt->s.angles[1] = initialDamage;

	BG_GetProjectileTraceEnd(muzzle, tEnt->s.origin2, right, up, traceEnd);
	BG_GetProjectileEndAndSmallStep(spread, &seed, muzzle, traceEnd, right, up, end, smallStep);
	BG_DoProjectile(weapon, initialDamage, muzzle, smallStep, end, ent->s.number);
}

static void ShotgunFire(const gentity_t *ent, const float spread, const int weapon,  qbool altfire, const vec3_t forward, const vec3_t muzzle) {
	gentity_t	*tEnt = G_TempEntity(muzzle, EV_SHOTGUN);
	float		initialDamage = bg_weaponlist[weapon].damage;
	int			pelletsCount = (altfire) ? bg_weaponlist[ent->client->ps.weapon].count * 2 : bg_weaponlist[ent->client->ps.weapon].count;
	int			seed = rand() & 255;	//Seed for spread pattern
	vec3_t		smallStep;				//Small step (~1 unit) in the fire direction
	vec3_t		traceEnd;				//Trace end point at the maximum allowed distance
	vec3_t		end;					//Trace end point with spread taken into account
	vec3_t		right, up;
	int			i;

	VectorScale(forward, 4096, tEnt->s.origin2);
	SnapVector(tEnt->s.origin2);
	tEnt->s.eventParm = seed;
	tEnt->s.otherEntityNum = ent->s.number;
	tEnt->s.otherEntityNum2 = pelletsCount;
	tEnt->s.weapon = weapon;
	tEnt->s.angles[0] = spread;
	tEnt->s.angles[1] = initialDamage;

	BG_GetProjectileTraceEnd(muzzle, tEnt->s.origin2, right, up, traceEnd);

	for (i = 0; i < pelletsCount; i++) {
		BG_GetProjectileEndAndSmallStep(spread, &seed, muzzle, traceEnd, right, up, end, smallStep);
		BG_DoProjectile(weapon, initialDamage, muzzle, smallStep, end, ent->s.number);
	}
}

/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t muzzlePoint ) {
	VectorCopy( ent->client->ps.origin, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;

	// new eye system, makes it possible to show the legs
	G_ModifyEyeAngles(muzzlePoint, ent->client->ps.viewangles, qfalse);

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}

void FireWeapon(gentity_t *ent, qbool altfire, weapon_t weapon) {
	vec3_t	muzzle;
	vec3_t	forward;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	CalcMuzzlePoint(ent, muzzle);

	if (weapon == WP_DYNAMITE || weapon == WP_MOLOTOV 
		|| (weapon == WP_KNIFE && ent->client->ps.stats[STAT_WP_MODE])) {
		FireThrowMissle(ent, muzzle, forward, weapon, bg_weaponlist[weapon].spread);
		return;
	} else {
		float	spread = bg_weaponlist[weapon].spread;
		int		i;

		if (ent->client->movestate & MS_WALK)
			spread *= 1.4f;

		if (ent->client->movestate & MS_CROUCHED)
			spread *= 0.65f;
		else if (ent->client->movestate & MS_JUMP)
			spread *= 5.0f;

		// adjust spread/damage for gametypes
		if (g_gametype.integer == GT_DUEL && weapon != WP_PEACEMAKER)
			spread *= 0.8f;
		else if (weapon != WP_NONE) {
			gitem_t	*item = BG_ItemForWeapon(weapon);
			spread *= 0.8f;

			if (item->weapon_sort != WS_SHOTGUN) {
				spread *= 0.55f;
			}
		}

		// if playing duel, change accuracy at the beginning of the round
		if (g_gametype.integer == GT_DUEL && du_introend - DU_INTRO_DRAW - DU_INTRO_CAM <= level.time
			&& du_introend + DU_CROSSHAIR_FADE >= level.time) {
			float factor = 1.0f;

			if (du_introend + DU_CROSSHAIR_START <= level.time) {
				factor = (float)(level.time - du_introend - DU_CROSSHAIR_START)/DU_CROSSHAIR_FADE;
				factor = 1-factor;
			}

			spread += factor*2000.0f;
		}

		//Reset tEntFireWeapon entities for all avalaible clients:
		for (i = 0; i < MAX_CLIENTS; i++)
			if (g_entities[i].client)
				g_entities[i].client->tEntFireWeapon = NULL;

		//unlagged - backward reconciliation #2
		// backward-reconcile the other clients
		G_DoTimeShiftFor(ent);
		//unlagged - backward reconciliation #2

		if (weapon == WP_SAWEDOFF || weapon == WP_REMINGTON_GAUGE || weapon == WP_WINCH97)
			ShotgunFire(ent, spread, weapon, altfire, forward, muzzle);
		else {
			if ((weapon == WP_PEACEMAKER || weapon == WP_REM58 || weapon == WP_SCHOFIELD)
				&& altfire)
				spread *= 3;
			BulletFire(ent, spread, weapon, forward, muzzle);
		}

		//unlagged - backward reconciliation #2
		// put them back
		G_UndoTimeShiftFor(ent);
		//unlagged - backward reconciliation #2
	}
}
