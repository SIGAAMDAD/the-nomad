typedef enum
{
	ST_NULL = 0,

    // these states are not actually valid
    ST_IDLE,
    ST_KNOCKBACK,
    ST_DEAD,

    // player states
    ST_PLAYR_IDLE,
    ST_PLAYR_MOVE,
    ST_PLAYR_KNOCKBACK,
    ST_PLAYR_DEAD,

    // shotty states
    ST_SHOTTY_IDLE,
    ST_SHOTTY_WANDER,
    ST_SHOTTY_KNOCKBACK,
    ST_SHOTTY_FIGHT,
    ST_SHOTTY_DEAD,

    // grunt states
    ST_GRUNT_IDLE,
    ST_GRUNT_WANDER,
    ST_GRUNT_KNOCKBACK,
    ST_GRUNT_FIGHT,
    ST_GRUNT_DEAD,

    // hulk states

	NUMSTATES
} statenum_t;