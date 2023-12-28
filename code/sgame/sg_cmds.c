#include "sg_local.h"


void SGameCommand( void )
{
    char cmd[MAX_TOKEN_CHARS];

    trap_Argv( 0, cmd, sizeof(cmd) );
}
