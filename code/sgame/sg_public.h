#ifndef _SG_PUBLIC_
#define _SG_PUBLIC_

#pragma once

#ifdef QVM
#include "qvmstdlib.h"
#include "nomadlib.h"
#endif

typedef enum
{
    SYS_CON_PRINTF = 0,
    SYS_CON_ERROR,
    CON_FLUSH,
    G_PLAYSFX,
    G_GETTILEMAP,
<<<<<<< HEAD
    G_SAVEGAME,
    G_LOADGAME,
    G_GETEVENTS,
    CVAR_REGISTER,
    CVAR_GETVALUE,
    CVAR_SETVALUE,
    CVAR_REGISTERNAME,
=======
    N_SAVEGAME,
    N_LOADGAME,
    G_UPDATECONFIG,
>>>>>>> parent of 7803dbe (got sound working, engine almost don)

    NUM_SGAME_IMPORT
} sgameImport_t;

typedef enum
{
    SGAME_INIT,
    SGAME_SHUTDOWN,
    SGAME_RUNTIC,
    SGAME_STARTLEVEL,
    SGAME_ENDLEVEL,
} sgameExport_t;

void G_GetTilemap(sprite_t tilemap[MAP_MAX_Y][MAP_MAX_X]);
void G_SaveGame(uint32_t slot, const void *data, const uint64_t size);
void G_LoadGame(uint32_t slot, void *data, uint64_t *size);
void G_GetEvents(eventState_t* events);

#endif
