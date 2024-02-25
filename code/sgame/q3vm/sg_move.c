#include "sg_local.h"

typedef struct {
    int downtime[2];
    int msec;
    qboolean active;
} kbind_t;

typedef struct {
    vec3_t vel;
    int northmove;
    int southmove;
    int westmove;
    int eastmove;

    int wallTime;
    qboolean wallHook;

    qboolean groundPlane;
} pmove_t;

static kbind_t key_MoveNorth, key_MoveSouth, key_MoveEast, key_MoveWest;
static pmove_t pm;

static void PM_Friction( void )
{

}

//
// PM_WallMove: handles wall bouncingm,
//  
static void PM_WallMove( void )
{

}

static void PM_AirMove( void )
{
    if ( pm.groundPlane ) {
        return;
    }

    
}

static void PM_GroundMove( void )
{
    if ( !pm.groundPlane ) {
        return;
    }
}

static void PM_CalcMoveFrame( void )
{
    pm.northmove;
    pm.southmove;
}

void Pmove( void )
{
    memset( &pm, 0, sizeof(pm) );
}

void SG_MoveNorth_Up_f( void ) {
}

void SG_MoveNorth_Down_f( void ) {
}

void SG_MoveSouth_Up_f( void ) {
}

void SG_MoveSouth_Down_f( void ) {
}

void SG_MoveWest_Up_f( void ) {
}

void SG_MoveWest_Down_f( void ) {
}

void SG_MoveEast_Up_f( void ) {
}

void SG_MoveEast_Down_f( void ) {
}
