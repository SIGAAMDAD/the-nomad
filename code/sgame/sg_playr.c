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

// DO NOT CHANGE THESE, THESE VALUES ARE USED FOR CAMERA MOVEMENT!!!!!!!!!!!!!!!
#define PMOVE_VELSCALE_VERTICLE		0.329f
#define PMOVE_VELSCALE_HORIZONTAL	0.4467f
#define PMOVE_CAMERA_SPEED			0.079f

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
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	float accelerate;
	float vel;

	if ( PM_CheckJump( pm ) ) {
		// jumped away
		PM_AirMove( pm );
	}

	PM_Friction( pm );

	fmove = pm->forwardmove;
	smove = pm->rightmove;

	scale = PM_CmdScale( pm );

	PM_ClipVelocity( pm->forward );
	PM_ClipVelocity( pm->right );

	VectorNormalize( pm->forward );
	VectorNormalize( pm->forward );

	for ( i = 0; i < 3; i++ ) {
		wishvel[i] = pm->forward[i]*fmove + pm->right[i]*smove;
	}

	VectorCopy( wishvel, wishdir );
	wishspeed = VectorNormalize( wishdir );
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, accelerate, pm );

	vel = VectorLength( pm->vel );

	PM_ClipVelocity( pm->vel );

	VectorNormalize( pm->vel );
	VectorScale( pm->vel, vel, pm->vel );

	// don't do anything if standing still
	if ( !pm->vel[0] && !pm->vel[1] ) {
		return;
	}

	pm->speed = vel;

	VectorCopy( sg.playr.ent->vel, pm->vel );
}

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

static void P_SetMovementDir( pmove_t *pm )
{
	if ( pm->rightmove > pm->forwardmove ) {
		pm->velDir = 0;
		pm->velDirInverse = 1;
	} else if ( pm->rightmove < pm->forwardmove ) {
		pm->velDir = 1;
		pm->velDirInverse = 0;
	} else {
		pm->velDir = 0;
	}
	
	if ( pm->rightmove < 0 && pm->forwardmove > 0 ) {
		pm->movementDir = DIR_NORTH_WEST;
	} else if ( pm->rightmove == 0 && pm->forwardmove > 0 ) {
		pm->movementDir = DIR_NORTH;
	} else if ( pm->rightmove > 0 && pm->forwardmove > 0 ) {
		pm->movementDir = DIR_NORTH_EAST;
	} else if ( pm->rightmove > 0 && pm->forwardmove == 0 ) {
		pm->movementDir = DIR_EAST;
	} else if ( pm->rightmove > 0 && pm->forwardmove < 0 ) {
		pm->movementDir = DIR_SOUTH_EAST;
	} else if ( pm->rightmove == 0 && pm->forwardmove < 0 ) {
		pm->movementDir = DIR_SOUTH;
	} else if ( pm->rightmove < 0 && pm->forwardmove < 0 ) {
		pm->movementDir = DIR_SOUTH_WEST;
	} else if ( pm->rightmove < 0 && pm->forwardmove == 0 ) {
		pm->movementDir = DIR_WEST;
	}
}

/*
* P_ClipOrigin: returns qtrue if the player's origin was clipped
*/
static qboolean P_ClipOrigin( sgentity_t *self )
{
	vec3_t origin;
	sgentity_t *ent;

	VectorCopy( origin, self->origin );

	if ( origin[0] > sg.mapInfo.width - 1 ) {
		origin[0] = sg.mapInfo.width - 1;
	} else if ( origin[0] < 0 ) {
		origin[0] = 0;
	}

	if ( origin[1] > sg.mapInfo.height - 1 ) {
		origin[1] = sg.mapInfo.height - 1;
	} else if ( origin[1] < 0 ) {
		origin[1] = 0;
	}

	if ( !VectorCompare( self->origin, origin ) ) { // clip it at map boundaries
		VectorCopy( self->origin, origin );
		return qtrue;
	} else if ( Ent_CheckWallCollision( self ) || Ent_CheckEntityCollision( self ) ) { // hit a solid entity
		VectorCopy( self->origin, origin );
		return qtrue;
	}

	return qfalse;
}

static pmove_t pm;

void P_Thinker( sgentity_t *self )
{
	int i;
	ImGuiWindow window;

	self->facing = pm.velDir;

	window.m_bClosable = qfalse;
	window.m_bOpen = qtrue;
	window.m_Flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize;
	window.m_pTitle = "Player Move Metrics";

	ImGui_BeginWindow( &window );
	ImGui_SetWindowPos( 0, 0 );

	ImGui_Text( "pm.forwardmove: %i", pm.forwardmove );
	ImGui_Text( "pm.rightmove: %i", pm.rightmove );
	ImGui_Text( "pm.upmove: %i", pm.upmove );

	for ( i = 0; i < 3; i++ ) {
		ImGui_Text( "pm.vel[%i]: %f", i, pm.vel[i] );
	}

	ImGui_EndWindow();

	memset( &pm, 0, sizeof(pm) );
}

void SG_SendUserCmd( int rightmove, int forwardmove, int upmove ) {
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
	ent->health = 100;

	ent->width = 0.5f;
	ent->height = 0.5f;

    Ent_SetState( ent, S_PLAYR_IDLE );
	Ent_BuildBounds( ent );

    // mark as allocated
    sg.playrReady = qtrue;
}

static void SG_KeyDown( uint32_t key )
{
	sgentity_t *ent;
	float add, *velPoint;
}

void SG_KeyEvent( uint32_t key, qboolean down )
{
	if ( down ) {
		vec2_t cameraPos;
		VectorCopy2( cameraPos, sg.cameraPos );

		switch ( key ) {
		case KEY_W:
			pm.forwardmove++;
			sg.playr.ent->origin[1] -= PMOVE_VELSCALE_VERTICLE;
			sg.cameraPos[1] += PMOVE_CAMERA_SPEED;
			break;
		case KEY_S:
			pm.forwardmove--;
			sg.playr.ent->origin[1] += PMOVE_VELSCALE_VERTICLE;
			sg.cameraPos[1] -= PMOVE_CAMERA_SPEED;
			break;
		case KEY_A:
			pm.rightmove--;
			sg.playr.ent->origin[0] -= PMOVE_VELSCALE_HORIZONTAL;
			sg.cameraPos[0] -= PMOVE_CAMERA_SPEED;
			sg.playr.ent->facing = 1;
			break;
		case KEY_D:
			pm.rightmove++;
			sg.playr.ent->origin[0] += PMOVE_VELSCALE_HORIZONTAL;
			sg.cameraPos[0] += PMOVE_CAMERA_SPEED;
			sg.playr.ent->facing = 0;
			break;
		};

		// clip the origin so the camera doesn't detach from the player
		if ( P_ClipOrigin( sg.playr.ent ) ) {
			VectorCopy2( sg.cameraPos, cameraPos );
		}
	}
}
