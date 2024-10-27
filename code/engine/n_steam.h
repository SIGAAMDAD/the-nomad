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

#ifndef __N_STEAM__
#define __N_STEAM__

#pragma once

#include "n_shared.h"
#include "gln_files.h"
#include "n_threads.h"

#ifdef NOMAD_STEAMAPP

#include <steam/steam_api.h>

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
    void Save( const char *pszFileName );
    void Load( void );

    void SaveEngineFiles( void );

    GDR_INLINE AppId_t GetAppId( void ) const {
        return m_nAppId;
    }

    STEAM_CALLBACK( CSteamManager, OnRemoteStorageFileReadAsyncComplete, RemoteStorageFileReadAsyncComplete_t,
        m_CallbackRemoteStorageFileReadAsyncComplete );
    STEAM_CALLBACK( CSteamManager, OnRemoteStorageFileWriteAsyncComplete, RemoteStorageFileWriteAsyncComplete_t,
        m_CallbackRemoteStorageFileWriteAsyncComplete );
    
    STEAM_CALLBACK( CSteamManager, OnLowBatteryPower, LowBatteryPower_t, m_CallbackLowBatteryPower );
    STEAM_CALLBACK( CSteamManager, OnTimedTrialStatus, TimedTrialStatus_t,  m_CallbackTimedTrialStatus );

    static void Message_f( void );
    static void AppInfo_f( void );
private:
    void LoadDLC( void );
    void LoadUserInfo( void );
    void LoadAppInfo( void );
    void LoadInput( void );

    void WriteFile( const char *npath );
    void LoadEngineFiles( void );

    ISteamUser *m_pSteamUser;
    HSteamUser m_nSteamUser;
    uint64_t m_nSteamUserId;

    dlc_t *m_pDlcList;
    int m_nDlcCount;

    char m_szUserDataFolder[MAX_OSPATH];

    char m_szInstallDir[MAX_OSPATH];
    AppId_t m_nAppId;
    AppId_t m_nAppBuildId;

    qboolean m_bVipAccount;

    CThreadMutex m_ReadAsyncLock;
    void *m_pReadAsyncBuffer;

    CThreadMutex m_WriteAsyncLock;
    const char *m_pWriteAsyncFileName;
    void *m_pWriteAsyncBuffer;
    uint64_t m_nWriteAsyncLength;
};

extern CSteamManager *g_pSteamManager;

#endif

void SteamApp_Init( void );
void SteamApp_CloudSave( void );
void SteamApp_Frame( void );
void SteamApp_Shutdown( void );

// don't judge me...
typedef enum {
    ACH_R_U_CHEATING = 0,
    ACH_COMPLETE_DOMINATION,
    ACH_GENEVA_SUGGESTION,
    ACH_PYROMANIAC,
    ACH_MASSACRE,
    ACH_JUST_A_MINOR_INCONVENIENCE,
    ACH_JACK_THE_RIPPER,
    ACH_EXPLOSION_CONNOISSEUIR,
    ACH_GOD_OF_WAR,
    ACH_BOOM_HEADSHOT,
    ACH_RESPECTFUL,
    ACH_A_LEGEND_IS_BORN,
    ACH_BUILDING_THE_LEGEND,
    ACH_SAME_SHIT_DIFFERENT_DAY,
    ACH_SHUT_THE_FUCK_UP,
    ACH_UNEARTHED_ARCANA,
    ACH_AWAKEN_THE_ANCIENTS,
    ACH_ITS_HIGH_NOON,
    ACH_DEATH_FROM_ABOVE,
    ACH_KOMBATANT,
    ACH_LAUGHING_IN_DEATHS_FACE,
    ACH_RIGHT_BACK_AT_YOU,
    ACH_BITCH_SLAP,
    ACH_CHEFS_SPECIAL,
    ACH_KNUCKLE_SANDWICH,
    ACH_BROTHER_IN_ARMS,
    ACH_BRU,
    ACH_GIT_PWNED,
    ACH_SILENT_DEATH,
    ACH_WELL_DONE_WEEB,
    ACH_ITS_TREASON_THEN,
    ACH_HEARTLESS,
    ACH_BUSHIDO,
    ACH_MAXIMUS_THE_MERCIFUL,
    ACH_CHEER_UP_LOVE_THE_CALVARYS_HERE,
    ACH_WORSE_THAN_DEATH,
    ACH_LOOKS_LIKE_MEATS_BACK_ON_OUR_MENU_BOYS,
    ACH_GYAT,
    ACH_TO_THE_SLAUGHTER,
    ACH_ONE_MAN_ARMY,
    ACH_YOU_CALL_THAT_A_KNIFE,
    ACH_AMERICA_FUCK_YEAH,
    ACH_SUSSY,
    ACH_LIVE_TO_FIGHT_ANOTHER_DAY,
    ACH_REMEMBER_US,
    ACH_ZANDATSU_THAT_SHIT,
    ACH_MORE,
    ACH_EDGELORD,
    ACH_THAT_ACTUALLY_WORKED,
    ACH_NANOMACHINES_SON,
    ACH_COOL_GUYS_DONT_LOOK_AT_EXPLOSIONS,
    ACH_DOUBLE_TAKE,
    ACH_TRIPLE_THREAT,
    ACH_DAYUUM_I_AINT_GONNA_SUGARCOAT_IT,
    ACH_BACK_FROM_THE_BRINK,
    ACH_DANCE_DANCE_DANCE,
    ACH_BANG_BANG_I_SHOT_EM_DOWN,
    ACH_BOP_IT,
    ACH_JUST_A_LEAP_OF_FAITH,
    ACH_SEND_THEM_TO_JESUS,
    ACH_RIZZLORD,
    ACH_AHHH_GAHHH_HAAAAAAA,
    ACH_ABSOLUTELY_NECESSARY_PRECAUTIONS,
    ACH_STOP_HITTING_YOURSELF,

    NUM_ACHIEVEMENTS
} achievementNum_t;

typedef struct {
    int m_eAchievementID;
    const char *m_pchAchievementID;
    char m_szName[128];
    char m_szDescription[256];
    qboolean m_bAchieved;
    nhandle_t m_hIconShader;
} achievement_t;

extern achievement_t g_szAchivements[NUM_ACHIEVEMENTS];

#endif