#include "n_shared.h"
#include <steam/steam_api.h>
#include "n_steam.h"

typedef struct {
    const char *pName;
    
} achievement_t;

class CSteamAchievementManager
{
public:
private:

};

CSteamManager::CSteamManager( void ) {
}

CSteamManager::~CSteamManager() {
}

void CSteamManager::LoadDLC( void )
{
    int32 i;

    m_nDlcCount = SteamApps()->GetDLCCount();
    m_pDlcList = (dlc_t *)Z_Malloc( sizeof(*m_pDlcList) * m_nDlcCount, TAG_STATIC );

    for ( i = 0; i < m_nDlcCount; i++ ) {
        if ( !SteamApps()->BGetDLCDataByIndex( i, &m_nAppId, &m_pDlcList[i].bAvailable, m_pDlcList[i].szName, sizeof(m_pDlcList[i].szName) ) ) {
            LogError( "failed to load dlc at %i!\n", i );
        }
    }
}

void CSteamManager::Init( void )
{    
    SteamAPI_Init();
}

void CSteamManager::Shutdown( void )
{
    SteamAPI_Shutdown();
}
