#include "../src/n_shared.h"
#include "sg_local.h"
#include "sg_playr.h"

static playr_t* p;

void P_Teleport()
{

}

void P_Friction(void)
{
    vec2_t vec;
    float *vel;
    float speed, newspeed, control;
    float drop;

    vel = p->thrust;

    VectorCopy(vel, vec);
    if (p->state == S_PLAYR_MOVE_WALKING) {

    }
}

/* 2d-styled strafe-jumping */
void P_Accelerate(vec2_t wishdir, float wishspeed, float accel)
{
    int i;
    float addspeed, accelspeed, currentspeed;

    currentspeed = DotProduct(p->thrust, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0) {
        return;
    }
    accelspeed = accel*wishspeed;
    if (accelspeed > addspeed) {
        accelspeed = addspeed;
    }

    for (i = 0; i < 2; i++) {
        p->thrust[i] += accelspeed * wishdir[i];
    }
}

static void P_GrappleMove(void)
{
    vec2_t vel, v;
    float vlen;

    VectorScale()
}

void P_Move()
{

}