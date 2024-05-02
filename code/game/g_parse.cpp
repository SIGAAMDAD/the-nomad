#include "../engine/n_shared.h"
#include "g_game.h"

void G_SystemInfoChanged( void )
{
    const char *systemInfo;
    const char *s, *t;
    char key[BIG_INFO_KEY];
    char value[BIG_INFO_VALUE];

    systemInfo = gi.gameState.stringData + gi.gameState.stringOffsets[ CS_SYSTEMINFO ];

    // parse/update fs_game in first place
    s = Info_ValueForKey( systemInfo, "fs_game" );
    if ( FS_InvalidGameDir( s ) ) {
		Con_Printf( COLOR_YELLOW "WARNING: invalid fs_game value %s\n", s );
	} else {
		Cvar_Set( "fs_game", s );
	}
}

void G_InitUserInfo( void )
{
    char *userInfo;

    userInfo = gi.gameState.stringData;
    Info_SetValueForKey( userInfo );
}

void G_ParseGamestate( void )
{

}

qboolean G_GameSwitch( void ) {
    return ( gi.gameSwitch && !com_errorEntered );
}
