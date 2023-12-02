#include "sg_local.h"

static playr_t* playr;

#define WEAPON_SLOT_MELEE   0
#define WEAPON_SLOT_SHOTGUN 1
#define WEAPON_SLOT_RIFLE   2
#define WEAPON_SLOT_ARM     3

void P_Thinker( sgentity_t *self )
{

}

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
}

void SG_InitPlayer( void )
{
    sgentity_t *ent;

    G_Printf ( "Allocating player...\n" );

    ent = SG_AllocEntity( ET_PLAYR );

    playr = &sg.playr;
    
    // initialize player state
    memset( playr, 0, sizeof(*playr) );
    
    ent->stateOffset = ST_PLAYR_IDLE;
    ent->hShader = sg.media.raio_shader;

    // mark as allocated
    sg.playrReady = qtrue;
}
