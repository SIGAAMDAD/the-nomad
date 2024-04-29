#include "n_shared.h"
#include <steam/steam_api.h>
#include "n_steam.h"
#include "n_common.h"
#include "n_cvar.h"

// define this only in release builds, but of course if we're debugging steam tho
//#define NOMAD_STEAM_APP

#ifndef NOMAD_STEAM_APP

void SteamApp_Init( void ) {
}

void SteamApp_Shutdown( void ) {
}

#else
// shared with MAX_MODULE_NAME
#define MAX_DLC_NAME MAX_NPATH

typedef struct {
    char szName[MAX_DLC_NAME];
    qboolean bAvailable;
} dlc_t;

class CSteamManager
{
public:
    CSteamManager( void );
    ~CSteamManager();

    void GDR_DECL LogInfo( const char *fmt, ... ) GDR_ATTRIBUTE((format( printf, 2, 3 )));
    void GDR_DECL LogError( const char *fmt, ... ) GDR_ATTRIBUTE((format( printf, 2, 3 )));
    void GDR_DECL LogWarning( const char *fmt, ... ) GDR_ATTRIBUTE((format( printf, 2, 3 )));

    void Init( void );
    void Shutdown( void );
private:
    void LoadDLC( void );
    void LoadUserInfo( void );
    void LoadAppInfo( void );

    ISteamUser *m_pSteamUser;
    HSteamUser m_nSteamUser;
    uint64_t m_nSteamUserId;

    dlc_t *m_pDlcList;
    int m_nDlcCount;

    char m_szInstallDir[MAX_OSPATH];
    AppId_t m_nAppId;

    qboolean m_bVipAccount;
};

typedef struct {
    const char *pName;
    qboolean bAchieved;
} achievement_t;

class CSteamAchievementManager
{
public:
private:
};

static CSteamManager *s_pSteamManager;

CSteamManager::CSteamManager( void ) {
}

CSteamManager::~CSteamManager() {
}

void GDR_DECL GDR_ATTRIBUTE((format( printf, 2, 3 ))) CSteamManager::LogInfo( const char *fmt, ... )
{
    va_list argptr;
    char ts[32];
    char msg[4096];
    time_t curtime;

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof( msg ) - 1, fmt, argptr );
    va_end( argptr );

    curtime = time( NULL );

    strftime( ts, sizeof( ts ) - 1, "%H:%M:%S", localtime( &curtime ) );

    Con_Printf( "[SteamAPI](%s) %s", ts, msg );
}

void GDR_DECL GDR_ATTRIBUTE((format( printf, 2, 3 ))) CSteamManager::LogError( const char *fmt, ... )
{
    va_list argptr;
    char ts[32];
    char msg[4096];
    time_t curtime;

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof( msg ) - 1, fmt, argptr );
    va_end( argptr );

    curtime = time( NULL );

    strftime( ts, sizeof( ts ) - 1, "%H:%M:%S", localtime( &curtime ) );

    Con_Printf( COLOR_RED "[SteamAPI:Error](%s) %s", ts, msg );
}

void GDR_DECL GDR_ATTRIBUTE((format( printf, 2, 3 ))) CSteamManager::LogWarning( const char *fmt, ... )
{
    va_list argptr;
    char ts[32];
    char msg[4096];
    time_t curtime;

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof( msg ) - 1, fmt, argptr );
    va_end( argptr );

    curtime = time( NULL );

    strftime( ts, sizeof( ts ) - 1, "%H:%M:%S", localtime( &curtime ) );

    Con_Printf( COLOR_YELLOW "[SteamAPI:Warning](%s) %s", ts, msg );
}

void CSteamManager::LoadUserInfo( void )
{
    char szUserDataFolder[MAX_OSPATH];

    m_pSteamUser = SteamUser();

    if ( !m_pSteamUser ) {
        LogError( "SteamUser() failed.\n" );
        return;
    }

    m_nSteamUserId = m_pSteamUser->GetSteamID().ConvertToUint64();

    // Your Resident Fiend
    if ( m_nSteamUserId == 76561199403850315UL ) {
        m_bVipAccount = qtrue;
    }

    if ( !m_pSteamUser->GetUserDataFolder( szUserDataFolder, sizeof( szUserDataFolder ) - 1 ) ) {
        LogError( "SteamUser()->GetUserDataFolder() failed\n" );
        N_strncpyz( szUserDataFolder, COLOR_RED "FAILED", sizeof( szUserDataFolder ) - 1 );
    }

    LogInfo( "SteamUser.IndividualAccount: %s\n", m_pSteamUser->GetSteamID().BIndividualAccount() ? "true" : "false" );
    LogInfo( "SteamUser.UserDataFolder: %s\n", szUserDataFolder );
    LogInfo( "SteamUser.IsVipAccount: %s\n", m_bVipAccount ? "true" : "false" );
}

void CSteamManager::LoadDLC( void )
{
    int i;

    m_nDlcCount = SteamApps()->GetDLCCount();
    m_pDlcList = (dlc_t *)Z_Malloc( sizeof( *m_pDlcList ) * m_nDlcCount, TAG_STATIC );

    LogInfo( "Got %i DLC.\n", m_nDlcCount );
    for ( i = 0; i < m_nDlcCount; i++ ) {
        if ( !SteamApps()->BGetDLCDataByIndex( i, &m_nAppId, (bool *)&m_pDlcList[i].bAvailable, m_pDlcList[i].szName,
            sizeof( m_pDlcList[i].szName ) ) )
        {
            LogError( "Failed to load dlc %i!\n", i );
        }
        LogInfo( "Loaded DLC \"%s\", available: %s.\n", m_pDlcList[i].szName, m_pDlcList[i].bAvailable ? "true" : "false" );
    }
    LogInfo( "Initialized DLC's.\n" );
}

void CSteamManager::LoadAppInfo( void )
{
    Cvar_Set( "ui_language", SteamApps()->GetCurrentGameLanguage() );
    m_nAppId = SteamApps()->GetAppBuildId();
    SteamApps()->GetAppInstallDir( m_nAppId, m_szInstallDir, sizeof( m_szInstallDir ) );

    LogInfo( "AppId: %u\n", m_nAppId );
    LogInfo( "InstallDir: %s\n", m_szInstallDir );
}

void CSteamManager::Init( void )
{
    LoadUserInfo();

    // ... ;)

    LoadDLC();
}

void CSteamManager::Shutdown( void )
{
    SteamAPI_Shutdown();

    if ( m_pDlcList ) {
        Z_Free( m_pDlcList );
    }
}

void SteamApp_Init( void )
{
    char dllName[MAX_NPATH];

    Con_Printf( "---------- SteamApp_Init ----------\n" );

    if ( !SteamAPI_Init() ) {
        Con_Printf( COLOR_RED "SteamAPI_Init failed!\n" );
        return;
    }
    
    s_pSteamManager = (CSteamManager *)Z_Malloc( sizeof( *s_pSteamManager ), TAG_STATIC );
    s_pSteamManager->Init();
}

void SteamApp_Shutdown( void )
{
    Con_Printf( "---------- SteamApp_Shutdown ----------\n" );

    if ( !s_pSteamManager ) {
        return;
    }

    s_pSteamManager->Shutdown();
    Z_Free( s_pSteamManager );
    s_pSteamManager = NULL;
}

#endif // NOMAD_STEAM_APP
