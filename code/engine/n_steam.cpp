/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "n_shared.h"
#include "n_steam.h"
#include "n_common.h"
#include "n_cvar.h"
#include "../game/g_game.h"
#include <EASTL/vector.h>
#include <EASTL/map.h>

#define TESTING_APP_ID 480

#define STAT_ID( id, type, name ) { id, type, name, { 0 } }

typedef enum {
    STAT_INT = 0,
    STAT_FLOAT = 1
} StatType_t;

typedef struct {
    const char *m_pName;
    int m_nID;
    StatType_t m_nType;
    union {
        int m_iValue;
        float m_fValue;
    };
} Stat_t;

CSteamManager *g_pSteamManager;

class CSteamStatsManager
{
public:
    CSteamStatsManager( void );
    ~CSteamStatsManager();

    void AddAchievement( const char *pName, const char *pDescription );

    bool StoreStats( void );
    bool RequestStats( void );
    bool SetAchievement( const char *pszID );

    STEAM_CALLBACK( CSteamStatsManager, OnUserStatsReceived, UserStatsReceived_t,
        m_CallbackUserStatsReceived );
    STEAM_CALLBACK( CSteamStatsManager, OnUserStatsStored, UserStatsStored_t,
        m_CallbackUserStatsStored );
    STEAM_CALLBACK( CSteamStatsManager, OnAchievementStored, UserAchievementStored_t,
        m_CallbackAchievementStored );
private:
    void InitAchievements( void );

    eastl::vector<achievement_t> m_Achievements;
    eastl::vector<Stat_t> m_Stats;
    qboolean m_bStatsInitialized;
};

static CSteamStatsManager *s_pSteamStats;

CSteamStatsManager::CSteamStatsManager( void )
    : m_CallbackUserStatsReceived( this, &CSteamStatsManager::OnUserStatsReceived ),
    m_CallbackUserStatsStored( this, &CSteamStatsManager::OnUserStatsStored ),
    m_CallbackAchievementStored( this, &CSteamStatsManager::OnAchievementStored )
{
    m_bStatsInitialized = qfalse;
    InitAchievements();
}

CSteamStatsManager::~CSteamStatsManager( void ) {
}

void CSteamStatsManager::InitAchievements( void )
{
    int i;

    for ( i = 0; i < NUM_ACHIEVEMENTS; i++ ) {
        if ( !SteamUserStats()->GetAchievement( g_szAchivements[i].m_pchAchievementID, (bool *)&g_szAchivements[i].m_bAchieved ) ) {
            g_pSteamManager->LogError( "GetAchievement() - failed on '%s'\n", g_szAchivements[i].m_szName );
        }
    }
}

bool CSteamStatsManager::RequestStats( void )
{
    // is steam loaded?
    if ( SteamUserStats() == NULL || SteamUser() == NULL ) {
        return false;
    }
    // is the user logged on?
    if ( !SteamUser()->BLoggedOn() ) {
        return false;
    }
    m_bStatsInitialized = SteamUserStats()->RequestCurrentStats();
    return m_bStatsInitialized;
}

bool CSteamStatsManager::StoreStats( void )
{
    if ( g_pSteamManager ) {
        for ( int iStat = 0; iStat < m_Stats.size(); iStat++ ) {
            Stat_t *stat = &m_Stats[iStat];

            switch ( stat->m_nType ) {
            case STAT_INT:
                SteamUserStats()->SetStat( stat->m_pName, stat->m_iValue );
                break;
            case STAT_FLOAT:
                SteamUserStats()->SetStat( stat->m_pName, stat->m_fValue );
                break;
            default:
                break;
            };
        }
        return SteamUserStats()->StoreStats();
    }
    return false;
}

bool CSteamStatsManager::SetAchievement( const char *pszID )
{
	// have we received a callback from Steam yet?
	if ( g_pSteamManager ) {
		SteamUserStats()->SetAchievement( pszID );
		return SteamUserStats()->StoreStats();
	}
	return false;
}

void CSteamStatsManager::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	if ( g_pSteamManager->GetAppId() == pCallback->m_nGameID ) {
		if ( pCallback->m_eResult == k_EResultOK ) {
			g_pSteamManager->LogInfo( "Received stats and achievements from Steam\n" );

			// load achievements
			for ( int iAch = 0; iAch < NUM_ACHIEVEMENTS; ++iAch ) {
				achievement_t *ach = &g_szAchivements[iAch];

				SteamUserStats()->GetAchievement( ach->m_pchAchievementID, (bool *)&ach->m_bAchieved );
				N_strncpyz( ach->m_szName, SteamUserStats()->GetAchievementDisplayAttribute( ach->m_pchAchievementID,
					"name" ), sizeof( ach->m_szName ) - 1 );
                N_strncpyz( ach->m_szDescription, SteamUserStats()->GetAchievementDisplayAttribute( ach->m_pchAchievementID,
					"desc" ), sizeof( ach->m_szDescription ) - 1 );
			}

            // load stats
            for ( int iStat = 0; iStat < m_Stats.size(); iStat++ ) {
                Stat_t *stat = &m_Stats[iStat];

                switch ( stat->m_nType ) {
                case STAT_INT:
                    SteamUserStats()->GetStat( stat->m_pName, &stat->m_iValue );
                    break;
                case STAT_FLOAT:
                    SteamUserStats()->GetStat( stat->m_pName, &stat->m_fValue );
                    break;
                default:
                    break;
                };
            }
		}
		else {
			g_pSteamManager->LogInfo( "RequestStats - failed, %i\n", pCallback->m_eResult );
		}
	}
}

void CSteamStatsManager::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	if ( g_pSteamManager->GetAppId() == pCallback->m_nGameID ) {
		if ( pCallback->m_eResult == k_EResultOK ) {
			g_pSteamManager->LogInfo( "Stored stats for Steam\n" );
		}
        else if ( pCallback->m_eResult == k_EResultInvalidParam ) {
            g_pSteamManager->LogInfo( "OnUserStatsStored - some failed to validate\n" );

            // fake callback to get a reload
            UserStatsReceived_t callback;
            callback.m_eResult = k_EResultOK;
            callback.m_nGameID = g_pSteamManager->GetAppId();
            OnUserStatsReceived( &callback );
        }
		else {
			g_pSteamManager->LogInfo( "OnUserStatsStored - failed, %i\n", pCallback->m_eResult );
		}
	}
}

void CSteamStatsManager::OnAchievementStored( UserAchievementStored_t *pCallback )
{
    if ( g_pSteamManager->GetAppId() != pCallback->m_nGameID ) {
        return;
    }

    g_pSteamManager->LogInfo( "Stored Achievement for Steam\n" );
}

void CSteamStatsManager::AddAchievement( const char *pName, const char *pDescription ) {
}

CSteamManager::CSteamManager( void )
    : m_CallbackRemoteStorageFileReadAsyncComplete( this, &CSteamManager::OnRemoteStorageFileReadAsyncComplete ),
    m_CallbackRemoteStorageFileWriteAsyncComplete( this, &CSteamManager::OnRemoteStorageFileWriteAsyncComplete ),
    m_CallbackLowBatteryPower( this, &CSteamManager::OnLowBatteryPower ),
    m_CallbackTimedTrialStatus( this, &CSteamManager::OnTimedTrialStatus )
{
}

CSteamManager::~CSteamManager() {
}

//==========================================================
// Steam API Callbacks
//

void CSteamManager::OnLowBatteryPower( LowBatteryPower_t *pCallback )
{
    if ( gi.state != GS_LEVEL ) {
        return;
    }
    switch ( pCallback->m_nMinutesBatteryLeft ) {
    case 9:
        LogWarning( "Saving game with 9 minutes of battery left...\n" );
        Cbuf_ExecuteText( EXEC_APPEND, "sgame.save_game\n" );
        break;
    case 5:
        LogWarning( "Saving game with 5 minutes of battery left...\n" );
        Cbuf_ExecuteText( EXEC_APPEND, "sgame.save_game\n" );
        break;
    case 1:
        LogWarning( "Saving game with 1 minute of battery left...\n" );
        Cbuf_ExecuteText( EXEC_APPEND, "sgame.save_game\n" );
        break;
    default:
        break;
    };
}

void CSteamManager::OnTimedTrialStatus( TimedTrialStatus_t *pCallback )
{

}

void CSteamManager::OnRemoteStorageFileReadAsyncComplete( RemoteStorageFileReadAsyncComplete_t *pCallback )
{
    if ( pCallback->m_eResult != k_EResultOK ) {
        LogError( "" );
        return;
    }

    m_ReadAsyncLock.Lock();
    SteamRemoteStorage()->FileReadAsyncComplete( pCallback->m_hFileReadAsync, m_pReadAsyncBuffer, pCallback->m_cubRead );
    m_ReadAsyncLock.Unlock();
}

void CSteamManager::OnRemoteStorageFileWriteAsyncComplete( RemoteStorageFileWriteAsyncComplete_t *pCallback )
{
    if ( pCallback->m_eResult != k_EResultOK ) {
        LogError( "Error writing file '%s' to steam cloud (bytes:%lu, address:%p)\n", m_pWriteAsyncFileName, m_nWriteAsyncLength,
            m_pWriteAsyncBuffer );
    }
}

void GDR_DECL GDR_ATTRIBUTE((format( printf, 2, 3 ))) CSteamManager::LogInfo( const char *fmt, ... )
{
    va_list argptr;
    char ts[32];
    char msg[MAXPRINTMSG];
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
    char msg[MAXPRINTMSG];
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
    char msg[MAXPRINTMSG];
    time_t curtime;

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof( msg ) - 1, fmt, argptr );
    va_end( argptr );

    curtime = time( NULL );

    strftime( ts, sizeof( ts ) - 1, "%H:%M:%S", localtime( &curtime ) );

    Con_Printf( COLOR_YELLOW "[SteamAPI:Warning](%s) %s", ts, msg );
}

void CSteamManager::Save( const char *pszFileName )
{
    void *pBuffer;
    uint64_t nLength;

    if ( !g_pSteamManager ) {
        return;
    }

    LogInfo( "Saving file '%s' to steam cloud...\n", pszFileName );

    nLength = FS_LoadFile( pszFileName, &pBuffer );
    if ( !nLength || !pBuffer ) {
        LogWarning( "Error loading file '%s' from engine\n", pszFileName );
        return;
    }
    if ( !SteamRemoteStorage()->FileWrite( pszFileName, pBuffer, nLength ) ) {
        LogError( "Error writing file '%s' to steam cloud (bytes:%lu, address:%p)\n", pszFileName, nLength, pBuffer );
    }
    FS_FreeFile( pBuffer );

#ifdef SAVEFILE_MOD_SAFETY
    if ( !SteamRemoteStorage()->BeginFileWriteBatch() ) {
        
    }
    if ( !SteamRemoteStorage()->EndFileWriteBatch() ) {

    }
#endif
}

void CSteamManager::LoadUserInfo( void )
{
    m_pSteamUser = SteamUser();

    LogInfo( "------------------------------------\n" );
    LogInfo( "Loading User Info...\n" );

    if ( !m_pSteamUser ) {
        LogError( "SteamUser() failed.\n" );
        return;
    }

    m_nSteamUserId = m_pSteamUser->GetSteamID().ConvertToUint64();

    // Your Resident Fiend
    if ( m_nSteamUserId == 76561199403850315UL ) {
        m_bVipAccount = qtrue;
    }

    if ( !m_pSteamUser->GetUserDataFolder( m_szUserDataFolder, sizeof( m_szUserDataFolder ) - 1 ) ) {
        LogError( "SteamUser()->GetUserDataFolder() failed\n" );
        N_strncpyz( m_szUserDataFolder, COLOR_RED "FAILED", sizeof( m_szUserDataFolder ) - 1 );
    }

    LogInfo( "SteamUser.Id: %lu\n", m_nSteamUserId );
    LogInfo( "SteamUser.IndividualAccount: %s\n", m_pSteamUser->GetSteamID().BIndividualAccount() ? "true" : "false" );
    LogInfo( "SteamUser.UserDataFolder: %s\n", m_szUserDataFolder );
    LogInfo( "SteamUser.IsVipAccount: %s\n", m_bVipAccount ? "true" : "false" );
    LogInfo( "SteamUser.UserName: %s\n", SteamFriends()->GetPersonaName() );
}

void CSteamManager::LoadInput( void )
{
    int XinputSlotIndex, numControllers;
    InputHandle_t szInputHandle[STEAM_INPUT_MAX_COUNT];
    InputHandle_t hInputHandle;

    LogInfo( "------------------------------------\n" );
    LogInfo( "Loading Input Info...\n" );

    SteamInput()->Init( true );
    numControllers = SteamInput()->GetConnectedControllers( szInputHandle );

    LogInfo( "Got %i controllers\n", numControllers );

    XinputSlotIndex = 0;
    hInputHandle = SteamInput()->GetControllerForGamepadIndex( XinputSlotIndex );
    if ( hInputHandle == 0 ) {
        LogInfo( "Standard Xbox Controller\n" );
    } else {
        ESteamInputType inputType = SteamInput()->GetInputTypeForHandle( hInputHandle );
        switch ( inputType ) {
        case k_ESteamInputType_Unknown:
            LogWarning( "Unknown input device type!\n" );
            break;
        case k_ESteamInputType_PS3Controller:
            LogInfo( "PS3 Controller Connected\n" );
            break;
        case k_ESteamInputType_PS4Controller:
            LogInfo( "PS4 Controller Connected\n" );
            break;
        case k_ESteamInputType_PS5Controller:
            LogInfo( "PS5 Controller Connected\n" );
            break;
        case k_ESteamInputType_XBox360Controller:
            LogInfo( "Xbox 360 Controller Connected\n" );
            break;
        case k_ESteamInputType_XBoxOneController:
            LogInfo( "Xbox One Controller Connected\n" );
            break;
        case k_ESteamInputType_GenericGamepad:
            LogInfo( "Generic Gamepad Controller Connected\n" );
            break;
        default:
            break;
        };
    }
}

void CSteamManager::LoadDLC( void )
{
    int i;

    LogInfo( "------------------------------------\n" );
    LogInfo( "Loading DLC Info...\n" );

    m_nDlcCount = SteamApps()->GetDLCCount();
    if ( !m_nDlcCount ) {
        return;
    }
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
    char szCommandLine[MAX_CMD_BUFFER];
    uint32 nTimedTrialSecondsAllowed, nTimedTrialSecondsPlayed;
    bool bIsTimedTrial;

    LogInfo( "------------------------------------\n" );
    LogInfo( "Loading App Info...\n" );

    Cvar_Set( "ui_language", SteamApps()->GetCurrentGameLanguage() );
    m_nAppId = SteamUtils()->GetAppID();
    m_nAppBuildId = SteamApps()->GetAppBuildId();

    memset( m_szInstallDir, 0, sizeof( m_szInstallDir ) );
    SteamApps()->GetAppInstallDir( m_nAppId, m_szInstallDir, sizeof( m_szInstallDir ) );

    memset( szCommandLine, 0, sizeof( szCommandLine ) );
    SteamApps()->GetLaunchCommandLine( szCommandLine, sizeof( szCommandLine ) );
    if ( *szCommandLine ) {
        Com_ParseCommandLine( szCommandLine );
    }

    if ( ( bIsTimedTrial = SteamApps()->BIsTimedTrial( &nTimedTrialSecondsAllowed, &nTimedTrialSecondsPlayed ) ) ) {
        if ( nTimedTrialSecondsPlayed > nTimedTrialSecondsAllowed ) {
            N_Error( ERR_FATAL, "*** STEAM *** Time Trial has expired" );
        }
    }

    LogInfo( "Language: %s\n", SteamApps()->GetCurrentGameLanguage() );
    LogInfo( "AppId: %u\n", m_nAppId );
    LogInfo( "AppBuildId: %u\n", m_nAppBuildId );
    LogInfo( "InstallDir: %s\n", m_szInstallDir );
    LogInfo( "IsTimedTrial: %s\n", bIsTimedTrial ? "true" : "false" );
    if ( m_nAppId == TESTING_APP_ID ) {
        LogInfo( "IsTestBuild: true\n" );
    } else {
        LogInfo( "IsTestBuild: false\n" );
    }
}

void CSteamManager::WriteFile( const char *npath )
{
    void *pBuffer;
    uint64_t nLength;

    nLength = FS_LoadFile( npath, &pBuffer );
    if ( !nLength || !pBuffer ) {
        return;
    }
    if ( !SteamRemoteStorage()->FileWrite( npath, pBuffer, nLength ) ) {
        LogError( "Error writing file '%s' to steam cloud (bytes:%lu, address:%p)\n", npath, nLength, pBuffer );
    }
    FS_FreeFile( pBuffer );
}

void CSteamManager::SaveEngineFiles( void )
{
    if ( !SteamRemoteStorage()->IsCloudEnabledForApp() || !SteamRemoteStorage()->IsCloudEnabledForAccount() ) {
        return;
    }

    LogInfo( "Saving Engine Configurations from Steam Cloud\n" );

    SteamRemoteStorage()->BeginFileWriteBatch();

    // save config
    WriteFile( LOG_DIR "/" NOMAD_CONFIG );

    // save skins config
    WriteFile( LOG_DIR "/skins.cfg" );

    // save load list
    WriteFile( CACHE_DIR "/loadlist.json" );
    
    SteamRemoteStorage()->EndFileWriteBatch();
}

void CSteamManager::LoadEngineFiles( void )
{
    char *pBuffer;
    int32 nLength;
    const char *path;

    if ( !SteamRemoteStorage()->IsCloudEnabledForApp() || !SteamRemoteStorage()->IsCloudEnabledForAccount() ) {
        return;
    }

    LogInfo( "Loading Engine Configurations to Steam Cloud\n" );

    path = LOG_DIR "/" NOMAD_CONFIG;
    nLength = SteamRemoteStorage()->GetFileSize( path );
    if ( nLength ) {
        pBuffer = (char *)Hunk_AllocateTempMemory( nLength );
        if ( SteamRemoteStorage()->FileRead( path, pBuffer, nLength ) ) {
            FS_WriteFile( path, pBuffer, nLength );
        }
        Hunk_FreeTempMemory( pBuffer );
    }

    path = LOG_DIR "/skins.cfg";
    nLength = SteamRemoteStorage()->GetFileSize( path );
    if ( nLength ) {
        pBuffer = (char *)Hunk_AllocateTempMemory( nLength );
        if ( SteamRemoteStorage()->FileRead( path, pBuffer, nLength ) ) {
            FS_WriteFile( path, pBuffer, nLength );
        }
        Hunk_FreeTempMemory( pBuffer );
    }

    path = CACHE_DIR "/loadlist.json";
    nLength = SteamRemoteStorage()->GetFileSize( path );
    if ( nLength ) {
        pBuffer = (char *)Hunk_AllocateTempMemory( nLength );
        if ( SteamRemoteStorage()->FileRead( path, pBuffer, nLength ) ) {
            FS_WriteFile( path, pBuffer, nLength );
        }
        Hunk_FreeTempMemory( pBuffer );
    }
}

extern "C" void GDR_DECL SteamAPIDebugMessageHook( int nSeverity, const char *pchDebugText )
{
    Con_DPrintf( "%s", pchDebugText );
    if ( nSeverity >= 1 ) {
        DebuggerBreak();
    }
}

void CSteamManager::Init( void )
{
    LoadAppInfo();

    LoadUserInfo();

    LoadInput();

    // ... ;)

    LoadDLC();

    if ( SteamUtils()->GetCurrentBatteryPower() < 20 ) {
        LogWarning( "Charge Ur Goddamn Device!\n" );
    }

//    SteamFriends()->ActivateGameOverlay( "stats" );

    Cvar_Set( "name", SteamFriends()->GetPersonaName() );

    LoadEngineFiles();

    SteamUtils()->SetWarningMessageHook( &SteamAPIDebugMessageHook );
}

void CSteamManager::Shutdown( void )
{
    SteamInput()->Shutdown();

    SteamAPI_Shutdown();

    if ( m_pDlcList ) {
        Z_Free( m_pDlcList );
    }
}

void CSteamManager::AppInfo_f( void )
{
    char szCommandLine[MAX_CMD_BUFFER];
    uint32 nTimedTrialSecondsAllowed, nTimedTrialSecondsPlayed;
    bool bIsTimedTrial;
    int i;

    memset( szCommandLine, 0, sizeof( szCommandLine ) );
    SteamApps()->GetLaunchCommandLine( szCommandLine, sizeof( szCommandLine ) );

    if ( ( bIsTimedTrial = SteamApps()->BIsTimedTrial( &nTimedTrialSecondsAllowed, &nTimedTrialSecondsPlayed ) ) ) {
        if ( nTimedTrialSecondsPlayed > nTimedTrialSecondsAllowed ) {
            N_Error( ERR_FATAL, "*** STEAM *** Time Trial has expired" );
        }
    }

    Con_Printf( "\n" );
    Con_Printf( "--------------------------\n" );
    Con_Printf( "[Steam AppInfo]\n" );
    Con_Printf( "Language: %s\n", SteamApps()->GetCurrentGameLanguage() );
    Con_Printf( "AppId: %u\n", g_pSteamManager->m_nAppId );
    Con_Printf( "AppBuildId: %u\n", g_pSteamManager->m_nAppBuildId );
    Con_Printf( "InstallDir: %s\n", g_pSteamManager->m_szInstallDir );
    Con_Printf( "IsTimedTrial: %s\n", bIsTimedTrial ? "true" : "false" );
    Con_Printf( "CommandLine: %s\n", szCommandLine );
    Con_Printf( "SteamUser.Id: %lu\n", g_pSteamManager->m_nSteamUserId );
    Con_Printf( "SteamUser.IndividualAccount: %s\n", g_pSteamManager->m_pSteamUser->GetSteamID().BIndividualAccount() ? "true" : "false" );
    Con_Printf( "SteamUser.UserDataFolder: %s\n", g_pSteamManager->m_szUserDataFolder );
    Con_Printf( "SteamUser.IsVipAccount: %s\n", g_pSteamManager->m_bVipAccount ? "true" : "false" );
    Con_Printf( "DlcCount: %i\n", g_pSteamManager->m_nDlcCount );
    if ( g_pSteamManager->m_nAppId == TESTING_APP_ID ) {
        Con_Printf( "IsTestBuild: true\n" );
    } else {
        Con_Printf( "IsTestBuild: false\n" );
    }
    for ( i = 0; i < g_pSteamManager->m_nDlcCount; i++ ) {
        Con_Printf( "DlcInfo[%i]: %s - %s\n", i, g_pSteamManager->m_pDlcList[i].szName,
            g_pSteamManager->m_pDlcList[i].bAvailable ? "active" : "inactive" );
    }
    Con_Printf( "--------------------------\n" );
}

void CSteamManager::Message_f( void )
{
    const char *msg;

    if ( Cmd_Argc() != 2 ) {
        Con_Printf( "usage: say <message>\n" );
        return;
    }

    msg = Cmd_Argv( 0 );

    
}

static void Steam_ListCloudFiles_f( void )
{
    int32 i, count;
    int32 size;
    const char *name;

    Con_Printf( "\n" );
    Con_Printf( "Steam Cloud Files:\n" );

    if ( !SteamRemoteStorage()->IsCloudEnabledForApp() || !SteamRemoteStorage()->IsCloudEnabledForAccount() ) {
        Con_Printf( "Disabled\n" );
    }

    count = SteamRemoteStorage()->GetFileCount();
    for ( i = 0; i < count; i++ ) {
        name = SteamRemoteStorage()->GetFileNameAndSize( i, &size );
        Con_Printf( "  %4i: %s ( size: %i )\n", i, name, size );
    }
}

static void Steam_UnlockAchievement_f( void )
{
    const char *achievementID;

    if ( Cmd_Argc() < 2 ) {
        Con_Printf( "usage: steam.unlock_achievement <achievementID>\n" );
        return;
    }

    achievementID = Cmd_Argv( 1 );
    if ( !s_pSteamStats->SetAchievement( achievementID ) ) {
        g_pSteamManager->LogError( "SetAchievement() - failed on '%s'\n", achievementID );
    }
    g_pSteamManager->LogInfo( "Unlocked Achievement '%s'\n", achievementID );
}

static void Steam_ClearAchievement_f( void )
{
    const char *achievementID;

    if ( Cmd_Argc() < 2 ) {
        Con_Printf( "usage: steam.clear_achievement <achievementID>\n" );
        return;
    }

    achievementID = Cmd_Argv( 1 );
    if ( !SteamUserStats()->ClearAchievement( achievementID ) ) {
        g_pSteamManager->LogError( "ClearAchievement() - failed on '%s'\n", achievementID );
    }
    g_pSteamManager->LogInfo( "Reset Achievement '%s'\n", achievementID );
}

void SteamApp_Init( void )
{
    char dllName[MAX_NPATH];

    Con_Printf( "---------- SteamApp_Init ----------\n" );

    if ( !SteamAPI_Init() ) {
        Con_Printf( COLOR_RED "\nSteamAPI_Init failed!\n" );
        return;
    }
    
    g_pSteamManager = new ( Z_Malloc( sizeof( *g_pSteamManager ), TAG_STATIC ) ) CSteamManager();
    g_pSteamManager->Init();

    s_pSteamStats = new ( Z_Malloc( sizeof( *s_pSteamStats ), TAG_STATIC ) ) CSteamStatsManager();

    Cmd_AddCommand( "steam.appinfo", CSteamManager::AppInfo_f );
    Cmd_AddCommand( "steam.listcloudfiles", Steam_ListCloudFiles_f );
    Cmd_AddCommand( "steam.unlock_achievement", Steam_UnlockAchievement_f );
    Cmd_AddCommand( "steam.clear_achievement", Steam_ClearAchievement_f );
}

void SteamApp_Frame( void )
{
    if ( !g_pSteamManager ) {
        return;
    }

    SteamFriends()->ActivateGameOverlay( "achievements" );
    SteamAPI_RunCallbacks();
}

void SteamApp_CloudSave( void )
{
    if ( !g_pSteamManager ) {
        return;
    }
    g_pSteamManager->SaveEngineFiles();
}

void SteamApp_Shutdown( void )
{
    Con_Printf( "---------- SteamApp_Shutdown ----------\n" );

    if ( !g_pSteamManager ) {
        return;
    }

    g_pSteamManager->Shutdown();
    Z_Free( g_pSteamManager );
    g_pSteamManager = NULL;

    Z_Free( s_pSteamStats );
    s_pSteamStats = NULL;

    Cmd_RemoveCommand( "steam.appinfo" );
    Cmd_RemoveCommand( "steam.listcloudfiles" );
    Cmd_RemoveCommand( "steam.unlock_achievement" );
    Cmd_RemoveCommand( "steam.clear_achievement" );
}

#define ACH_ID( id, name, desc ) { id, #id, name, desc, 0, 0 }

achievement_t g_szAchivements[NUM_ACHIEVEMENTS] = {
    ACH_ID( ACH_R_U_CHEATING,                           "R U Cheating?",
        "" ),
    ACH_ID( ACH_COMPLETE_DOMINATION,                    "Complete Domination",
        "" ),
    ACH_ID( ACH_GENEVA_SUGGESTION,                      "Geneva Suggestion",
        "" ),
    ACH_ID( ACH_PYROMANIAC,                             "Pyromaniac",
        "" ),
    ACH_ID( ACH_MASSACRE,                               "Massacre",
        "" ),
    ACH_ID( ACH_JUST_A_MINOR_INCONVENIENCE,             "Just A Minor Inconvenience",
        "" ),
    ACH_ID( ACH_JACK_THE_RIPPER,                        "Jack The Ripper",
        "" ),
    ACH_ID( ACH_EXPLOSION_CONNOISSEUIR,                 "Explosion Connoisseuir",
        "" ),
    ACH_ID( ACH_GOD_OF_WAR,                             "God Of War",
        "" ),
    ACH_ID( ACH_BOOM_HEADSHOT,                          "BOOM! Headshot",
        "" ),
    ACH_ID( ACH_RESPECTFUL,                             "Respectful",
        "" ),
    ACH_ID( ACH_A_LEGEND_IS_BORN,                       "A Legend Is Born",
        "" ),
    ACH_ID( ACH_BUILDING_THE_LEGEND,                    "Building The Legend",
        "" ),
    ACH_ID( ACH_SAME_SHIT_DIFFERENT_DAY,                "Same Shit Different Day",
        "" ),
    ACH_ID( ACH_SHUT_THE_FUCK_UP,                       "Shut The Fuck Up",
        "" ),
    ACH_ID( ACH_UNEARTHED_ARCANA,                       "Unearthed Arcana",
        "" ),
    ACH_ID( ACH_AWAKEN_THE_ANCIENTS,                    "Awaken The Ancients",
        "" ),
    ACH_ID( ACH_ITS_HIGH_NOON,                          "It's High Noon",
        "" ),
    ACH_ID( ACH_DEATH_FROM_ABOVE,                       "Death From Above",
        "" ),
    ACH_ID( ACH_KOMBATANT,                              "Kombatant",
        "" ),
    ACH_ID( ACH_LAUGHING_IN_DEATHS_FACE,                "Laughing in Death's Face",
        "" ),
    ACH_ID( ACH_RIGHT_BACK_AT_YOU,                      "Right Back At You",
        "" ),
    ACH_ID( ACH_BITCH_SLAP,                             "Bitch Slap",
        "" ),
    ACH_ID( ACH_CHEFS_SPECIAL,                          "Chef's Special",
        "" ),
    ACH_ID( ACH_KNUCKLE_SANDWICH,                       "Knuckle Sandwich",
        "" ),
    ACH_ID( ACH_BROTHER_IN_ARMS,                        "Brother in Arms",
        "" ),
    ACH_ID( ACH_BRU,                                    "BRU",
        "" ),
    ACH_ID( ACH_GIT_PWNED,                              "GIT PWNED",
        "" ),
    ACH_ID( ACH_SILENT_DEATH,                           "Silent Death",
        "" ),
    ACH_ID( ACH_WELL_DONE_WEEB,                         "Well Done Weeb!",
        "" ),
    ACH_ID( ACH_ITS_TREASON_THEN,                       "It's Treason Then!",
        "" ),
    ACH_ID( ACH_HEARTLESS,                              "Heartless",
        "" ),
    ACH_ID( ACH_BUSHIDO,                                "Bushido",
        "" ),
    ACH_ID( ACH_MAXIMUS_THE_MERCIFUL,                   "Maximus The Merciful",
        "" ),
    ACH_ID( ACH_CHEER_UP_LOVE_THE_CALVARYS_HERE,        "Cheer Up Love, The Calvary's Here!",
        "" ),
    ACH_ID( ACH_WORSE_THAN_DEATH,                       "Worse Than Death",
        "" ),
    ACH_ID( ACH_LOOKS_LIKE_MEATS_BACK_ON_OUR_MENU_BOYS, "Looks Like Meat's Back On Our Menu Boys!",
        "" ),
    ACH_ID( ACH_GYAT,                                   "Gyat",
        "" ),
    ACH_ID( ACH_TO_THE_SLAUGHTER,                       "To The Slaughter",
        "" ),
    ACH_ID( ACH_ONE_MAN_ARMY,                           "One Man Army",
        "" ),
    ACH_ID( ACH_YOU_CALL_THAT_A_KNIFE,                  "You Call THAT A Knife?",
        "" ),
    ACH_ID( ACH_AMERICA_FUCK_YEAH,                      "America, FUCK YEAH!",
        "" ),
    ACH_ID( ACH_SUSSY,                                  "Sussy",
        "" ),
    ACH_ID( ACH_LIVE_TO_FIGHT_ANOTHER_DAY,              "Live To Fight Another Day",
        "" ),
    ACH_ID( ACH_REMEMBER_US,                            "Remember Us",
        "" ),
    ACH_ID( ACH_ZANDATSU_THAT_SHIT,                     "Zandatsu That Shit",
        "" ),
    ACH_ID( ACH_MORE,                                   "MORE!",
        "" ),
    ACH_ID( ACH_EDGELORD,                               "Edgelord",
        "" ),
    ACH_ID( ACH_THAT_ACTUALLY_WORKED,                   "That Actually WORKED?",
        "" ),
    ACH_ID( ACH_NANOMACHINES_SON,                       "Nanomachines, Son!",
        "" ),
    ACH_ID( ACH_COOL_GUYS_DONT_LOOK_AT_EXPLOSIONS,      "Cool Guys Don't Look At Explosions",
        "" ),
    ACH_ID( ACH_DOUBLE_TAKE,                            "Double Take",
        "" ),
    ACH_ID( ACH_TRIPLE_THREAT,                          "Triple Threat",
        "" ),
    ACH_ID( ACH_DAYUUM_I_AINT_GONNA_SUGARCOAT_IT,       "DAYUUM! I Ain't Gonna Sugarcoat It",
        "" ),
    ACH_ID( ACH_BACK_FROM_THE_BRINK,                    "Back From The Brink",
        "" ),
    ACH_ID( ACH_DANCE_DANCE_DANCE,                      "Dance! Dance! DANCE!",
        "" ),
    ACH_ID( ACH_BANG_BANG_I_SHOT_EM_DOWN,               "Bang, Bang, I Shot 'Em Down",
        "" ),
    ACH_ID( ACH_BOP_IT,                                 "Bop It",
        "" ),
    ACH_ID( ACH_JUST_A_LEAP_OF_FAITH,                   "Just A Leap Of Faith",
        "" ),
    ACH_ID( ACH_SEND_THEM_TO_JESUS,                     "Send Them To Jesus",
        "" ),
    ACH_ID( ACH_RIZZLORD,                               "Rizzlord",
        "" ),
    ACH_ID( ACH_AHHH_GAHHH_HAAAAAAA,                    "Ahhh! Gahhh! HAAAAAAA!",
        "" ),
    ACH_ID( ACH_ABSOLUTELY_NECESSARY_PRECAUTIONS,       "Absolutely Necessary Precautions",
        "" ),
    ACH_ID( ACH_STOP_HITTING_YOURSELF,                  "Stop Hitting Yourself",
        "" )
};
