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
#define PMOVE_CLAMP_BORDER_HORZ		-0.2f
#define PMOVE_CLAMP_BORDER_VERT		0.0f

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
#define PMOVE_MAXSPEED 10

typedef struct {
	vec3_t vel;
	vec3_t grapplePoint;
	float forward, backward, right, left;
	qboolean rightmove;
	qboolean leftmove;
	qboolean backwardmove;
	qboolean forwardmove;
	int waterlevel;
	int velDir;
	int velDirInverse;
	int frametime;
	int wallTime;
	int flags;
	dirtype_t movementDir;
	qboolean wallHook;
	qboolean groundPlane;
	qboolean walking;
} pmove_t;

qboolean P_GiveWeapon( weapontype_t type )
{
    int slot;

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

static int key_dash;
static int key_melee;

static void P_SetLegsAnim( int anim )
{
	sg.playr.foot_frame = anim;
}

static void PM_Friction( pmove_t *pm )
{
	float speed;
	vec3_t v;

	VectorCopy( v, pm->vel );

	speed = VectorLength( &v );
	if ( speed <= 0.0f ) {
		return;
	}

	if ( pm->vel.x < 0 ) {
		pm->vel.x = MIN( 0, pm->vel.x + pm_groundFriction.f );
	} else if ( pm->vel.x > 0 ) {
		pm->vel.x = MAX( 0, pm->vel.x - pm_groundFriction.f );
	}

	if ( pm->vel.y < 0 ) {
		pm->vel.y = MIN( 0, pm->vel.y + pm_groundFriction.f );
	} else if ( pm->vel.x > 0 ) {
		pm->vel.y = MAX( 0, pm->vel.y - pm_groundFriction.f );
	}

	pm->forward = MAX( 0, pm->forward - pm_groundFriction.f );
	pm->backward = MAX( 0, pm->backward - pm_groundFriction.f );
	pm->left = MAX( 0, pm->left - pm_groundFriction.f );
	pm->right = MAX( 0, pm->right - pm_groundFriction.f );
}

static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel, pmove_t *pm )
{
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

	pm->vel.x += accelspeed * wishdir.x;
	pm->vel.y += accelspeed * wishdir.y;
	pm->vel.z += accelspeed * wishdir.z;
}

static void PM_ClipVelocity( vec3_t vel, float *forward, float *left, float *right, float *backward )
{
	if ( vel.x > PMOVE_MAXSPEED ) {
		vel.x = PMOVE_MAXSPEED;
	} else if ( vel.x < -PMOVE_MAXSPEED ) {
		vel.x = -PMOVE_MAXSPEED;
	}

	if ( vel.y > PMOVE_MAXSPEED ) {
		vel.y = PMOVE_MAXSPEED;
	} else if ( vel.y < -PMOVE_MAXSPEED ) {
		vel.y = -PMOVE_MAXSPEED;
	}

	if ( vel.z > 10 ) {
		vel.z = 10;
	}

	*forward = MIN( *forward, PMOVE_MAXSPEED );
	*backward = MIN( *backward, PMOVE_MAXSPEED );
	*left = MIN( *left, PMOVE_MAXSPEED );
	*right = MIN( *right, PMOVE_MAXSPEED );
}

void P_MeleeThink( sgentity_t *self )
{
	bbox_t bounds;
	sgentity_t *ent;
	
	SG_BuildBounds( &bounds, PLAYR_WIDTH, MELEE_RANGE, &ent->origin );
	
	ent = Ent_CheckEntityCollision( self );
	if ( !ent ) {
		return;
	}
	
	//
	// check for parry
	//
	
	if ( ent->flags & EF_PARRY ) {
		// its a projectile
		Snd_PlaySfx( sg.media.player_parry );
		Ent_SetState( sg.playr.ent, S_PLAYR_PARRY );
	} else if ( ent->flags & EF_FIGHTING && ent->flags & EF_PARRY ) {
		// its an attack
		Snd_PlaySfx( sg.media.player_parry );
		Ent_SetState( sg.playr.ent, S_PLAYR_PARRY );
	}
}

/*
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
*/

static pmove_t pm;

/*
* P_ClipOrigin: returns qtrue if the player's origin was clipped
*/
static qboolean P_ClipOrigin( sgentity_t *self )
{
	vec3_t origin;
	sgentity_t *ent;

	VectorCopy( origin, self->origin );

	if ( origin.x > sg.mapInfo.width - 1 ) {
		origin.x = sg.mapInfo.width - 1;
	} else if ( origin.x < PMOVE_CLAMP_BORDER_HORZ ) {
		origin.x = PMOVE_CLAMP_BORDER_HORZ;
	}

	if ( origin.y > sg.mapInfo.height - 1 ) {
		origin.y = sg.mapInfo.height - 1;
	} else if ( origin.y < PMOVE_CLAMP_BORDER_VERT ) {
		origin.y = PMOVE_CLAMP_BORDER_VERT;
	}

	if ( origin.z > 10 ) {
		origin.z = 10;
	}

	if ( !VectorCompare( &self->origin, &origin ) ) { // clip it at map boundaries
		VectorCopy( self->origin, origin );
		VectorClear( pm.vel );
		return qtrue;
	} else if ( Ent_CheckWallCollision( self ) || Ent_CheckEntityCollision( self ) ) { // hit a solid entity
		VectorCopy( self->origin, origin );
		VectorClear( pm.vel );
		return qtrue;
	}

	return qfalse;
}

static void Pmove( sgentity_t *self )
{
	int i;
	float tmp;

	if ( pm.left > pm.right ) {
		self->facing = 1;
	} else if ( pm.right > pm.left ) {
		self->facing = 0;
	}

	pm.vel.x += pm.right;
	pm.vel.x -= pm.left;

	pm.vel.y -= pm.forward;
	pm.vel.y += pm.backward;

	pm.backwardmove = pm.backward == 0;
	pm.forwardmove = pm.forward == 0;
	pm.leftmove = pm.left == 0;
	pm.rightmove = pm.right == 0;

	PM_ClipVelocity( pm.vel, &pm.forward, &pm.left, &pm.right, &pm.backward );
	PM_Friction( &pm );

	self->origin.x += pm.vel.x;
	self->origin.y += pm.vel.y;

	if ( P_ClipOrigin( sg.playr.ent ) ) {
		VectorClear( pm.vel );
	}

	sg.cameraPos.x = self->origin.x - ( sg.cameraPos.x / 2 );
	sg.cameraPos.y = -self->origin.y;
}

void P_Thinker( sgentity_t *self )
{
	int i;
	float f;

	Pmove( self );

	ImGui_BeginWindow( "Player Move Metrics", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize );

	for ( i = 0; i < 3; i++ ) {
		f = vec3_get( &self->origin, i );
		ImGui_Text( "self->origin[%i]: %f", i, f );
	}

	ImGui_Text( "pm.forward: %f", pm.forward );
	ImGui_Text( "pm.backward: %f", pm.backward );
	ImGui_Text( "pm.left: %f", pm.left );
	ImGui_Text( "pm.right: %f", pm.right );

	ImGui_EndWindow();
}

void SG_InitPlayer( void )
{    
    // initialize player state
    memset( &sg.playr, 0, sizeof(sg.playr) );

	sg.playr.foot_frame = 0;
	sg.playr.foot_sprite = SPR_PLAYR_LEGS0_7_R;

    // mark as allocated
    sg.playrReady = qtrue;
}

void SG_KeyEvent( int key, qboolean down )
{
	if ( down ) {
		switch ( key ) {
		case KEY_W:
			pm.forward += pm_baseSpeed.f * pm_baseAccel.f;
			break;
		case KEY_S:
			pm.backward += pm_baseSpeed.f * pm_baseAccel.f;
			break;
		case KEY_A:
			pm.left += pm_baseSpeed.f * pm_baseAccel.f;
			break;
		case KEY_D:
			pm.right += pm_baseSpeed.f * pm_baseAccel.f;
			break;
		};
	}
}
