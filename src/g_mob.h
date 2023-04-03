#ifndef _G_MOB_
#define _G_MOB_

#pragma once

// mob flags
enum : uint32_t
{
    MF_HAS_MELEE = 0xa1,
    MF_HAS_HITSCAN = 0xa2,
    MF_HAS_PROJECTILE = 0xa3,
    
    // if the mob in question has additional buffs
    MF_IS_LEADER = 0x55f,
    
    // if the mob in question is dead
    MF_DEAD = 0xDEAD,

    // if the mob in question cannot move (essentially defender inspired creature type from MTG)
    MF_SENTRY = 0xad34,

    NUMMOBFLAGS
};

typedef struct mobj_s
{
    const char name[80]={0};
    int16_t health;
    uint32_t flags;

    inline mobj_s() = default;
    inline mobj_s(const mobj_s &) = default;
    inline mobj_s(mobj_s &&) = default;
    inline ~mobj_s() = default;

    inline mobj_s& operator=(const mobj_s &m) {
        N_memcpy((void *)this, &m, sizeof(mobj_s));
        return *this;
    }
} mobj_t;

enum
{
    MT_HULK,
    MT_RAVAGER,
    MT_GRUNT,
    MT_SOLDIER,
    MT_SHOTTY,
    MT_GUNNER,

    NUMMOBS
};

extern mobj_t mobinfo[NUMMOBS];

class Mob
{
public:
    mobj_t c_mob;

    int16_t health;
    uint32_t flags;
    coord_t mpos;
    uint8_t mdir;
public:
    Mob();
    Mob(const Mob &) = delete;
    Mob(Mob &&) = default;
    ~Mob() = default;

    inline Mob& operator=(const mobj_t& m) {
        c_mob = m;
        health = m.health;
        flags = m.flags;
        return *this;
    }
    inline Mob& operator=(const Mob &m) {
        flags = m.flags;
        health = m.health;
        mpos = m.mpos;
        mdir = m.mdir;
        c_mob = m.c_mob;
        return *this;
    }

    inline void gendir(void)
    { mdir = P_Random() & 3; }
    inline uint32_t getmobjindex() const
    {
        for (uint32_t i = 0; i < NUMMOBS; ++i) {
            if (N_memcmp(&mobinfo[i], &c_mob, sizeof(mobj_t))) {
                return i;
                break;
            }
        }
        LOG_WARN("Mob::getmobjindex: mob has invalid mobj_t type, returning 0");
        return 0;
    }
};

void M_RunThinker(linked_list<Mob*>::iterator it);

#endif