#include "sg_local.h"

static int Cheat_GetToggle( const char *cheatname )
{
    char num[64];

    if ( trap_Argc() != 2 ) {
        G_Printf( "usage: %s <1|on|toggleon> or <0|off|toggleoff>\n", cheatname );
    }
    
    trap_Argv( 1, num, sizeof(num) );

    if ( num[0] == '1' || !N_stricmp( num, "on" ) || !N_stricmp( num, "toggleon" ) ) {
        G_Printf( "Cheat '%s' toggled on.\n", cheatname );
        return qtrue;
    } else if ( num[0] == '0' || !N_stricmp( num, "off" ) || !N_stricmp( num, "toggleoff" ) ) {
        G_Printf( "Cheat '%s' toggled off.\n", cheatname );
        return qfalse;
    }
    
    G_Printf( "WARNING: unknown option '%s' for cheat command\n", num );
    return qfalse;
}

static void Cheat_InfiniteHealth_f( void ) {
    Cvar_Set( "sgc_infiniteHealth", va( "%i", Cheat_GetToggle( "i_am_built_different" ) ) );
}

static void Cheat_InfiniteAmmo_f( void ) {
    Cvar_Set( "sgc_infiniteAmmo", va( "%i", Cheat_GetToggle( "i_need_more_BOOLETS" ) ) );
}

static void Cheat_InfiniteRage_f( void ) {
    Cvar_Set( "sgc_infiniteRage", va( "%i", Cheat_GetToggle( "too_angry_to_die" ) ) );
}

static void Cheat_GodMode_f( void ) {
    Cvar_Set( "sgc_godmode", va( "%i", Cheat_GetToggle( "godmode" ) ) );
}

static void Cheats_Set( int toggle ) {
    const char *str;
    str = va( "%i", toggle );

    Cvar_Set( "sgc_infiniteHealth", str );
    Cvar_Set( "sgc_infiniteAmmo", str );
    Cvar_Set( "sgc_infiniteRage", str );
    Cvar_Set( "sgc_godmode", str );
    Cvar_Set( "sgc_blindMobs", str );
    Cvar_Set( "sgc_deafMobs", str );
}

static void Cheat_EnableAll_f( void ) {
    Con_Printf( "Enabling all cheats" );
    Cvar_Set( "sg_cheatsOn", "1" );
}

static void Cheat_DisableAll_f( void ) {
    Con_Printf( "Disabling cheats.\n" );
    Cvar_Set( "sg_cheatsOn", "0" );
}

static void Cheat_BlindMobs_f( void ) {
    Cvar_Set( "sgc_blindMobs", va( "%i", Cheat_GetToggle( "blindmobs" ) ) );
}

static void Cheat_DeafMobs_f( void ) {
    Cvar_Set( "sgc_deafMobs", va( "%i", Cheat_GetToggle( "deafmobs" ) ) );
}

typedef struct {
    const char *name;
    void (*fn)( void );
} cmd_t;

static const cmd_t commands[] = {
    { "i_am_built_different", Cheat_InfiniteHealth_f },
    { "gmathitw", Cheat_InfiniteHealth_f },
    { "i_need_more_BOOLETS", Cheat_InfiniteAmmo_f },
    { "gmataitw", Cheat_InfiniteAmmo_f },
    { "godmode", Cheat_GodMode_f },
    { "iwtbag", Cheat_GodMode_f },
    { "blindmobs", Cheat_BlindMobs_f },
    { "deafmobs", Cheat_DeafMobs_f },
    { "iamacheater", Cheat_EnableAll_f },
    { "iamapussy", Cheat_EnableAll_f },
    { "iamnotacheater", Cheat_DisableAll_f },
    { "ihavetheballs", Cheat_DisableAll_f },
    { "too_angry_to_die", Cheat_InfiniteRage_f },
    { "iambatman", Cheat_InfiniteRage_f }
};

void SGameCommand( void )
{
    char cmd[MAX_TOKEN_CHARS];
    int i;

    trap_Argv( 0, cmd, sizeof(cmd) );

    for ( i = 0; i < arraylen( commands ); i++ ) {
        if ( !N_stricmp( commands[i].name, cmd ) ) {
            commands[i].fn();
        }
    }
}

void SG_InitCommands( void )
{
    int i;

    for ( i = 0; i < arraylen( commands ); i++ ) {
        trap_AddCommand( commands[i].name );
    }
}

void SG_ShutdownCommands( void )
{
    int i;

    for ( i = 0; i < arraylen( commands ); i++ ) {
        trap_RemoveCommand( commands[i].name );
    }
}
