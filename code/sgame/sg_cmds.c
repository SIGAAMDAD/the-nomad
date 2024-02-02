#include "sg_local.h"


void SGameCommand( void )
{
    char cmd[MAX_TOKEN_CHARS];

    trap_Argv( 0, cmd, sizeof(cmd) );

    //
    // cheat codes
    //

    // infinite health
    if ( !N_stricmp( cmd, "gimme_more" ) || !N_stricmp( cmd, "gmathitw" ) ) {
    }
    // god mode
    else if ( !N_stricmp( cmd, "godmode" ) || !N_stricmp( cmd, "iwtbag" ) ) {

    }
    // make all enemies blind
    else if ( !N_stricmp( cmd, "blindmobs" ) ) {

    }
    // make all enemies deaf
    else if ( !N_stricmp( cmd, "deafmobs" ) ) {

    }
    // enable all cheats
    else if ( !N_stricmp( cmd, "iamacheater" ) || !N_stricmp( cmd, "iamapussy" ) ) {

    }
    // disable all cheats
    else if ( !N_stricmp( cmd, "iamnotacheater" ) || !N_stricmp( cmd, "ihavetheballs" ) ) {

    }
}
