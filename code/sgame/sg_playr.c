#include "sg_local.h"

#define LEGS_JUMP 0
#define LEGS_JUMPB 1
#define JUMP_VELOCITY 4.0f
#define PLAYR_WIDTH 1.15
#define MELEE_RANGE 5

#define WEAPON_SLOT_MELEE   0
#define WEAPON_SLOT_SHOTGUN 1
#define WEAPON_SLOT_RIFLE   2
#define WEAPON_SLOT_ARM     3

typedef enum {
    kbMelee,
    kbDash,
    kbGrenade,
    kbCrouch,
    kbAltFire,
} bindnum_t;

#define PMF_WALKING         (unsigned)0x0001
#define PMF_SLIDING         (unsigned)0x0002
#define PMF_JUMP_HELD       (unsigned)0x0004
#define PMF_BACKWARDS_JUMP  (unsigned)0x0008
#define PMOVE_MAXSPEED 80

typedef struct {
    vec3_t vel;
	vec3_t grapplePoint;
	vec3_t forward, right;
	float addspeed;
    float speed;
	int waterlevel;
	int velDir;
	int velDirInverse;
	int frametime;
	int rightmove;
	int upmove;
	int forwardmove;
	int wallTime;
	uint32_t flags;
	dirtype_t movementDir;
	qboolean wallHook;
	qboolean groundPlane;
	qboolean walking;
} pmove_t;

qboolean P_GiveWeapon( weapontype_t type )
{
    uint32_t slot;

    switch (type) {
    case WT_SHOTTY_3B_PUMP:
    case WT_SHOTTY_AUTO:
    case WT_SHOTTY_DB:
        slot = WEAPON_SLOT_SHOTGUN;
        break;
    case WT_MELEE_BO:
    case WT_MELEE_BS:
    case WT_MELEE_FISTS:
    case WT_MELEE_KANTANA:
    case WT_MELEE_STAR:
        slot = WEAPON_SLOT_MELEE;
        break;
    case WT_ARM_BLADE:
    case WT_ARM_FT:
    case WT_ARM_GRAPPLE:
    case WT_ARM_HANDCANNON:
    case WT_ARM_MP:
    case WT_ARM_SD:
    case WT_ARM_TT:
        slot = WEAPON_SLOT_ARM;
        break;
    };

    return qtrue;
}

static uint32_t key_dash;
static uint32_t key_melee;

static void P_SetLegsAnim( int anim )
{
	sg.playr.foot_frame = anim;
}

static void PM_Friction( pmove_t *pm )
{
	float speed;
	
	speed = VectorLength( pm->vel );
	if ( speed <= 0.0f ) {
		return;
	}
	

}

static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel, pmove_t *pm )
{
	int i;
	float addspeed, accelspeed, speed;
	
	speed = DotProduct( pm->vel, wishdir );
	addspeed = wishspeed - speed;
	if ( addspeed <= 0 ) {
		return;
	}
	accelspeed = accel * pm->frametime * wishspeed;
	if ( accelspeed > addspeed ) {
		accelspeed = addspeed;
	}
	
	for ( i = 0; i < 3; i++ ) {
		pm->vel[i] += accelspeed * wishdir[i];
	}
}

static void PM_ClipVelocity( vec3_t vel )
{
	vel[0] = MIN( vel[0], PMOVE_MAXSPEED );
	vel[1] = MIN( vel[1], PMOVE_MAXSPEED );
}

static float PM_CmdScale( pmove_t *pm )
{
	int max;
	float total, scale;
	
	max = abs( pm->forwardmove );
	if ( abs( pm->rightmove ) > max ) {
		max = abs( pm->rightmove );
	}
	if ( abs( pm->upmove ) > max ) {
		max = abs( pm->upmove );
	}
	if ( !max ) {
		return 0;
	}
	
	total = sqrt( pm->forwardmove * pm->forwardmove + pm->rightmove * pm->rightmove + pm->upmove * pm->upmove );
	scale = (float)pm->speed * max / ( 127.0f * total );
	
	return scale;
}

static void PM_AirMove( pmove_t *pm )
{
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	
	PM_Friction( pm );
	
	fmove = pm->forwardmove;
	smove = pm->rightmove;
	
	scale = PM_CmdScale( pm );
	
	for ( i = 0; i < 2; i++ ) {
		wishvel[i] = pm->forward[i]*fmove + pm->right[i]*smove;
	}
	
	VectorCopy( wishdir, wishvel );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;
	
	// not on ground, so have a little effect on velocity
	PM_Accelerate( wishdir, wishspeed, pm_airAccel.f, pm );
}

static void PM_GrappleMove( pmove_t *pm )
{
	vec3_t vel, v;
	float vlen;
	
	VectorScale( pm->forward, -16, v );
	VectorAdd( pm->grapplePoint, v, v );
	VectorSubtract( v, sg.playr.ent->origin, vel );
	vlen = VectorLength( vel );
	VectorNormalize( vel );
	
	if ( vlen <= 100 ) {
		VectorScale( vel, 10 * vlen, vel );
	} else {
		VectorScale( vel, 800, vel );
	}
	
	VectorCopy( vel, pm->vel );
	
	pm->groundPlane = qfalse;
}

static void PM_WallJumpAccel( pmove_t *pm )
{
	if ( pm->wallTime == 1 ) {
		pm->vel[pm->velDir] = MAX( pm->vel[pm->velDir], 8 );
		pm->vel[2] += 0.75f; // add a bit of height
	}
}

static qboolean PM_CheckJump( pmove_t *pm )
{
	if ( pm->upmove < 10 ) {
		// not holding jump
		return qfalse;
	}
	
	// must wait for jump to be released
	if ( pm->flags & PMF_JUMP_HELD ) {
		// clear upmove to cmdscale doesn't lower running speed
		pm->upmove = 0;
		return qfalse;
	}
	
	if ( pm->wallTime == 1 ) { // we were on the wall for just a single tic, wall bounce
		
	} else if ( pm->wallTime > 1 ) { // we were wall running, scale up the vel
		pm->vel[0] = pm->vel[0] + pm_wallrunAccelMove.f;
		pm->vel[1] = pm->vel[1] + pm_wallrunAccelMove.f;
		pm->vel[2] = pm->vel[2] + pm_wallrunAccelVertical.f;
	}
	
	pm->groundPlane = qfalse; // jumping away
	pm->walking = qfalse;
	pm->flags |= PMF_JUMP_HELD;
	
	pm->vel[2] = JUMP_VELOCITY;
	
	if ( pm->forwardmove >= 0 ) {
		P_SetLegsAnim( LEGS_JUMP );
		pm->flags &= ~PMF_BACKWARDS_JUMP;
	} else {
		P_SetLegsAnim( LEGS_JUMPB );
		pm->flags |= PMF_BACKWARDS_JUMP;
	}
	
	return qtrue;
}

static qboolean PM_CheckWaterJump( pmove_t *pm )
{
	vec3_t spot;
	int cont;
	vec3_t flatforward;
	
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}
	
	
	
	return qtrue;
}

static void PM_WallMove( pmove_t *pm )
{
	if ( pm->waterlevel ) {
		
	}
	
	if ( !Ent_CheckWallCollision( sg.playr.ent ) || !pm->groundPlane ) {
		return; // cannot wall run
	}
	
	if ( pm->wallTime >= pm_wallTime.i ) { // force the player off the wall
		pm->wallTime = 0;
		pm->wallHook = qfalse;
		return;
	}
	
//	pm->vel[0] ; // increase their speed just a bit
	
	pm->wallHook = qtrue;
	pm->wallTime++;
}

static void PM_WalkMove( pmove_t *pm )
{
}

static pmove_t pm;

void P_MeleeThink( sgentity_t *self )
{
	bbox_t bounds;
	sgentity_t *ent;
	
	SG_BuildBounds( &bounds, PLAYR_WIDTH, MELEE_RANGE, ent->origin );
	
	ent = Ent_CheckEntityCollision( self );
	if ( !ent ) {
		return;
	}
	
	//
	// check for parry
	//
	
	if ( ent->flags & EF_PARRY ) {
		// its a projectile
		trap_Snd_PlaySfx( sg.media.player_parry );
		Ent_SetState( sg.playr.ent, S_PLAYR_PARRY );
	} else if ( ent->flags & EF_FIGHTING && ent->flags & EF_PARRY ) {
		// its an attack
		trap_Snd_PlaySfx( sg.media.player_parry );
		Ent_SetState( sg.playr.ent, S_PLAYR_PARRY );
	}
}

static void P_SetMovementDir( int rightmove, int forwardmove )
{
	if ( rightmove > forwardmove ) {
		pm.velDir = 0;
		pm.velDirInverse = 1;
	} else if ( rightmove < forwardmove ) {
		pm.velDir = 1;
		pm.velDirInverse = 0;
	}
	
	if ( rightmove < 0 && forwardmove > 0 ) {
		pm.movementDir = DIR_NORTH_WEST;
	} else if ( rightmove == 0 && forwardmove > 0 ) {
		pm.movementDir = DIR_NORTH;
	} else if ( rightmove > 0 && forwardmove > 0 ) {
		pm.movementDir = DIR_NORTH_EAST;
	} else if ( rightmove > 0 && forwardmove == 0 ) {
		pm.movementDir = DIR_EAST;
	} else if ( rightmove > 0 && forwardmove < 0 ) {
		pm.movementDir = DIR_SOUTH_EAST;
	} else if ( rightmove == 0 && forwardmove < 0 ) {
		pm.movementDir = DIR_SOUTH;
	} else if ( rightmove < 0 && forwardmove < 0 ) {
		pm.movementDir = DIR_SOUTH_WEST;
	} else if ( rightmove < 0 && forwardmove == 0 ) {
		pm.movementDir = DIR_WEST;
	}
}

void P_Thinker( sgentity_t *self )
{

}

void SG_SendUserCmd( int forwardmove, int rightmove, int upmove, uint32_t buttons )
{
    P_SetMovementDir( rightmove, forwardmove );
	
	pm.upmove = upmove;
	pm.forwardmove = forwardmove;
	pm.rightmove = rightmove;
	
	VectorScale( pm.forward, forwardmove, pm.forward );
	VectorScale( pm.right, rightmove, pm.right );
}

void SG_InitPlayer( void )
{
    sgentity_t *ent;

    ent = SG_AllocEntity( ET_PLAYR );
    
    // initialize player state
    memset( &sg.playr, 0, sizeof(sg.playr) );

	sg.playr.foot_frame = 0;
	sg.playr.foot_sprite = SPR_PLAYR_LEGS0_7_R;
	sg.playr.ent = ent;

	ent->facing = 0;
    ent->hShader = sg.media.raio_shader;
	ent->hSpriteSheet = sg.media.raio_sprites;
	ent->sprite = SPR_PLAYR_IDLE_R;
	ent->frame = 0;
    ent->entPtr = &sg.playr;

	ent->width = 0.5f;
	ent->height = 0.5f;

    Ent_SetState( ent, S_PLAYR_IDLE );
	Ent_BuildBounds( ent );

    // mark as allocated
    sg.playrReady = qtrue;
}
