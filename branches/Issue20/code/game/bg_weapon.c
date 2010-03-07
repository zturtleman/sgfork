//Shared weapon handling functions for both QAGAME and CGAME QVMs

#if defined QAGAME
#include "g_local.h"
#define BG_Trace trap_Trace_New2
#elif defined CGAME
#include "../cgame/cg_local.h"
#define BG_Trace CG_Trace_New
#endif
#include "bg_weapon.h"

void BG_GetProjectileTraceEnd(const vec3_t muzzle, const vec3_t forward, vec3_t right, vec3_t up, vec3_t traceEnd) {
	VectorNormalizeClearOutOnZeroLength(forward, traceEnd);
	PerpendicularVector(right, traceEnd);
	CrossProduct(traceEnd, right, up);
	VectorMA(muzzle, MAX_TRACE_LENGTH, traceEnd, traceEnd);
}

void BG_GetProjectileEndAndSmallStep(const float spread, int *seed, const vec3_t muzzle, const vec3_t traceEnd, const vec3_t right, const vec3_t up, vec3_t end, vec3_t smallStep) {
	float		r;	//End point right offset (spread dependent)
	float		u;	//End point upper offset (spread dependent)
	r = Q_random(seed) * M_PI * 2.0f;
	u = sin(r) * Q_crandom(seed) * spread * 16;
	r = cos(r) * Q_crandom(seed) * spread * 16;
	VectorMA(traceEnd, r, right, end);
	VectorMA(end, u, up, end);
	VectorSubtract(end, muzzle, smallStep);
	VectorNormalizeFast(smallStep); //We don't need much of precision, ~1 unit is enough
}

void BG_DoProjectile(const int weapon, float damage, const vec3_t muzzle, const vec3_t smallStep, vec3_t end, const int player_num) {
	trace_t		tr;				//Trace object
	int			tracesCount;	//Number of passed traces
#ifdef QAGAME
	int			passent = player_num | PASS_ONLY_THIS;	//Entity to be skiped by trace function
#else
	int			passent = player_num;	//Entity to be skiped by trace function
	// We don't set flag for CGAME traces because they don't check for relations
	// (CG_ClipMoveToEntities)
	// But we have to set it for QAGAME to be able to shot own missiles
	// (SV_ClipMoveToEntities)
#endif
	int			shaderNum;		//Material number in prefixInfo array
	vec3_t		currentOrigin;	//Muzzle origin for the current trace

	VectorCopy(muzzle, currentOrigin);

	for (tracesCount = 0; tracesCount < MAX_TRACES_COUNT; tracesCount++) {
		shaderNum = BG_Trace(&tr, currentOrigin, NULL, NULL, end, passent, MASK_SHOT);

		if (tr.fraction == 1.0f || tr.allsolid)
			break;

#ifdef CGAME
		CG_Do_Bubbles(currentOrigin, tr.endpos, smallStep, weapon);
#endif

		if (tr.surfaceFlags & SURF_NOIMPACT)
			break;

		damage -= bg_weaponlist[weapon].damage / bg_weaponlist[weapon].range * Distance(tr.endpos, currentOrigin) / AIR_THICKNESS;
		if (damage < 1)
			break;

		VectorAdd(tr.endpos, smallStep, currentOrigin);

#if defined QAGAME
		if (g_entities[tr.entityNum].client) {
			damage = G_Damage(&g_entities[tr.entityNum], &g_entities[player_num], &g_entities[player_num], smallStep, tr.endpos, damage, 0, weapon);
#elif defined CGAME
		if (cg_entities[tr.entityNum].currentState.eType == ET_PLAYER) {
#endif
			continue;
#if defined QAGAME
		}

		if (g_entities[tr.entityNum].takedamage)
			damage = G_Damage(&g_entities[tr.entityNum], &g_entities[player_num], &g_entities[player_num], smallStep, tr.endpos, damage, 0, weapon);

		if (!g_entities[tr.entityNum].takedamage || g_entities[tr.entityNum].health > 0) {
#elif defined CGAME
		} else {
#endif
			float	thickness = 0.0f;	//Wall thickness
			trace_t	innerTrace;			//Internal trace object
			int		i;
#ifdef CGAME
			CG_ProjectileHitWall(weapon, tr.endpos, tr.plane.normal, tr.surfaceFlags, shaderNum, tr.entityNum);
#endif
			for (i = 0; i < NUM_PREFIXINFO; i++)
				if (tr.surfaceFlags & prefixInfo[i].surfaceFlags) {
					thickness = prefixInfo[i].thickness;
					break;
				}

			if (thickness == 0.0f)
				break;

			BG_Trace(&innerTrace, currentOrigin, NULL, NULL, end, passent, MASK_SHOT);

			if (innerTrace.fraction == 1.0f || innerTrace.allsolid)
				break;

			BG_Trace(&innerTrace, innerTrace.endpos, NULL, NULL, currentOrigin, passent, MASK_SHOT);

			damage -= bg_weaponlist[weapon].damage / bg_weaponlist[weapon].range * Distance(innerTrace.endpos, currentOrigin) / thickness;
			if (damage < 1)
				break;
#ifdef CGAME
			CG_ProjectileHitWall(weapon, innerTrace.endpos, innerTrace.plane.normal, innerTrace.surfaceFlags, shaderNum, innerTrace.entityNum);
#endif
		}
	}
#ifdef CGAME
	if (tr.fraction == 1.0f || tr.allsolid)
		VectorCopy(currentOrigin, end);
	else
		VectorCopy(tr.endpos, end);
#endif
}
