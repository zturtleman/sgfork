//Weapon handling functions for both QAGAME and CGAME QVMs

#define MAX_TRACES_COUNT		100
#define MAX_TRACE_LENGTH		8192*16
#define AIR_THICKNESS			100.0f

void BG_DoProjectile(const int weapon, float damage, const vec3_t muzzle, const vec3_t smallStep, vec3_t end, const int player_num);
void BG_GetProjectileTraceEnd(const vec3_t muzzle, const vec3_t forward, vec3_t right, vec3_t up, vec3_t traceEnd);
void BG_GetProjectileEndAndSmallStep(const float spread, int *seed, const vec3_t muzzle, const vec3_t traceEnd, const vec3_t right, const vec3_t up, vec3_t end, vec3_t smallStep);
