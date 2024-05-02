// unix_crashwindow.cpp - a specialized crash window similar to the one used in Bannerlord, but only
// for whenever a module crashes, not the engine

#include "../engine/n_shared.h"
#include "../game/g_game.h"

void Sys_CrashWindow( void )
{
    const UtlVector<CModuleInfo *>& loadList = g_pModuleLib->GetLoadList();
    SDL_MessageBoxData boxData;
    SDL_MessageBoxButtonData *buttonData;
}
