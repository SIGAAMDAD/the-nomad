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

void SteamApp_Init( void );
void SteamApp_CloudSave( void );
void SteamApp_Frame( void );
void SteamApp_Shutdown( void );

// don't judge me...
typedef enum {
    R_U_CHEATING = 0,
    COMPLETE_DOMINATION,
    GENEVA_SUGGESTION,
    PYROMANIAC,
    MASSACRE,
    JUST_A_MINOR_INCONVENIENCE,
    JACK_THE_RIPPER,
    EXPLOSION_CONNOISSEUIR,
    GOD_OF_WAR,
    BOOM_HEADSHOT,
    RESPECTFUL,
    A_LEGEND_IS_BORN,
    BUILDING_THE_LEGEND,
    SAME_SHIT_DIFFERENT_DAY,
    SHUT_THE_FUCK_UP,
    UNEARTHED_ARCANA,
    AWAKEN_THE_ANCIENTS,
    ITS_HIGH_NOON,
    DEATH_FROM_ABOVE,
    KOMBATANT,
    LAUGHING_IN_DEATHS_FACE,
    RIGHT_BACK_AT_YOU,
    BITCH_SLAP,
    CHEFS_SPECIAL,
    KNUCKLE_SANDWICH,
    BROTHER_IN_ARMS,
    BRU,
    GIT_PWNED,
    SILENT_DEATH,
    WELL_DONE_WEEB,
    ITS_TREASON_THEN,
    HEARTLESS,
    BUSHIDO,
    MAXIMUS_THE_MERCIFUL,
    CHEER_UP_LOVE_THE_CALVARYS_HERE,
    WORSE_THAN_DEATH,
    LOOKS_LIKE_MEATS_BACK_ON_OUR_MENU_BOYS,
    GYAT,
    TO_THE_SLAUGHTER,
    ONE_MAN_ARMY,
    YOU_CALL_THAT_A_KNIFE,
    AMERICA_FUCK_YEAH,
    SUSSY,
    LIVE_TO_FIGHT_ANOTHER_DAY,
    REMEMBER_US,
    ZANDATSU_THAT_SHIT,
    MORE,
    EDGELORD,
    THAT_ACTUALLY_WORKED,
    NANOMACHINES_SON,
    COOL_GUYS_DONT_LOOK_AT_EXPLOSIONS,
    DOUBLE_TAKE,
    TRIPLE_THREAT,
    DAYUUM_I_AINT_GONNA_SUGARCOAT_IT,
    BACK_FROM_THE_BRINK,
    DANCE_DANCE_DANCE,
    BANG_BANG_I_SHOT_EM_DOWN,
    BOP_IT,
    JUST_A_LEAP_OF_FAITH,
    SEND_THEM_TO_JESUS,
    RIZZLORD,
    AHHH_GAHHH_HAAAAAAA,
    ABSOLUTELY_NECESSARY_PRECAUTIONS,
    STOP_HITTING_YOURSELF,

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