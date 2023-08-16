#include "n_shared.h"
#include <steam/steam_api.h>
#include "n_steam.h"

typedef enum
{
    STAT_INT = 0,
    STAT_FLOAT,
    STAT_AVGRATE
} NomadStatType;

#define STAT_ID(id, type, name) { id, type, name, { 0, 0 } }
typedef struct
{
    int32 m_id;
    NomadStatType m_statType;
    const char *m_statName;
    
    union {
        int32 i;
        float f;
    } m_value;
} stat_t;

class SteamStats
{
public:
    SteamStats(stat_t *stats, int32 nStats);
    ~SteamStats();

    bool RequestStats(void);
    bool StoreStats(void);

    STEAM_CALLBACK(SteamStats, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
    STEAM_CALLBACK(SteamStats, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
private:
    int64 m_appId;
    stat_t *m_pStats;
    int32 m_nStats;
    bool m_initialized;
};

SteamStats::SteamStats(stat_t *stats, int32 nStats)
{
    m_appId = SteamManager::GetAppId();
    m_pStats = stats;
    m_nStats = nStats;
    m_initialized = RequestStats();
}

SteamStats::~SteamStats()
{
}

bool SteamStats::RequestStats(void)
{
    if (!SteamManager::Initialized())
        return false;
    if (!SteamManager::UserLoggedOn())
        return false;
    
    return SteamManager::FetchStats();
}

bool SteamStats::StoreStats(void)
{
    if (m_initialized) {
        for (uint32_t i = 0; i < m_nStats; ++i) {
            stat_t *stat = &m_pStats[i];
            
            switch (stat->m_statType) {
            case STAT_INT:
                break;
            case STAT_FLOAT:
                break;
            };
        }
    }
}


// all the achievements, if you want to play the game without being spoiled, then skip this enum
typedef enum
{
    ACH_BOOTCAMP,
} NomadAchievements;

#define ACH_ID( id, name ) { id, #id, name, "", false, 0 }
typedef struct
{
	int32 m_AchievementID;
	const char *m_pAchievementID;
	char m_name[128];
	char m_description[256];
	bool m_achieved;
	int32 m_iconImage;
} achievement_t;

achievement_t achievmentInfo[] = {
    ACH_ID(ACH_BOOTCAMP, "Basic Bootcamp")
} achievement_t;

#define STEAM_LOG(...) Con_Printf("[Steam Log] " __VA_ARGS__)
#define STEAM_ERROR(...) Con_Printf("[Steam Error] " __VA_ARGS__)

class SteamAchievements
{
public:
	SteamAchievements(achievement_t *achievements, int32 nAchievements);
	~SteamAchievements();

	bool RequestStats(void);
	bool SetAchievement(const char *id);

	STEAM_CALLBACK(SteamAchievements, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
	STEAM_CALLBACK(SteamAchievements, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
	STEAM_CALLBACK(SteamAchievements, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);
private:
	int64 m_appId;
	achievement_t *m_achievements; // Achievements data
	int32 m_nAchievements; // The number of Achievements
	bool m_initialized; // Have we called Request stats and received the callback?
};

SteamAchievements::SteamAchievements(achievement_t *achievements, int32 nAchievements)
    : m_callbackUserStatsReceived{ this, &SteamAchievements::OnUserStatsReceived },
    m_callbackUserStatsStored{ this, &SteamAchievements::OnUserStatsStored },
    m_callbackAchievementsStored{ this, &SteamAchievements::OnAchievementStored }
{
    m_appId = SteamUtils()->GetAppID();
    m_achievements = achievements;
    m_nAchievements = nAchievements;
    RequestStats();

    m_initialized = false;
}

SteamAchievements::~SteamAchievements()
{

}

bool SteamAchievements::RequestStats(void)
{
    if (!SteamManager::Initialized())
        return false;
    if (!SteamManager::UserLoggedOn())
        return false;

    return SteamManager::GetUserStats();
}

bool SteamAchievements::SetAchievement(const char *id)
{
    if (SteamManager::Initialized()) {
        SteamManager::UpdateAchievement(id);
        return SteamManager::UpdateStats();
    }
    return false;
}

void SteamAchievements::OnUserStatsReceived(UserStatsReceieved_t *pCallback)
{
    if (pCallback->m_nGameID == m_appId) {
        if (pCallback->m_eResult == k_EResultOK) {
            STEAM_LOG("received stats and achievements from steam");
            m_initialized = true;

            // load the stuff
            for (uint32_t i = 0; i < m_nAchievements; ++i) {
                achievement_t *ach = &m_achievements;

                SteamManager::FetchAchievementStatus(ach->m_pAchievementID, &ach->m_achieved);
                N_strncpy(ach->m_name, SteamManager::FetchAchievemntName(ach->m_pAchievementID), sizeof(ach->m_name));
                N_strncpy(ach->m_description, SteamManager::FetchAchievmentDescription(ach->m_pAchievementID), sizeof(ach->m_description));
            }
        }
        else { // oh no
            STEAM_ERROR("callback failed: RequestStats - failed, error code: %i", pCallback->m_eResult);
        }
    }
}

void SteamAchievements::OnUserStatsStored(UserStatsStored_t *pCallback)
{
    if (pCallback->m_nGameID == m_appId) {
        if (pCallback->m_eResult == k_EResultOK)
            STEAM_LOG("successfully stored steam user stats");
        else
            STEAM_ERROR("callback failed: StatsStored - failed, error code: %i", pCallback->m_eResult);
    }
}

void SteamAchievements::OnAchievementStored(UserAchievementStored_t *pCallback)
{
    if (pCallback->m_nGameID == m_appId)
        STEAM_LOG("successfully stored steam achievement data");
}


typedef struct SteamDLC_s
{
    AppId_t appId;
    char name[128];
    bool available;
    bool installed;

    inline SteamDLC_s(AppId_t _appId, const char *_name, bool _available)
    {
        appId = _appId;
        N_strncpy(name, _name, sizeof(name));
        available = _available;
    }
} SteamDLC_t;

// manages everything to do with the steam api
class SteamManager
{
public:
    static void Init(void);

    static const char *FetchAchievementName(const char *id);
    static const char *FetchAchievementDescription(const char *id);

    void GetNumberOfPlayers(void);
    void ListDLC(void);
    void SteamFrame(void);
private:
    void InitInput(void);
    void InitCloud(void);

    static SteamManager* steamManager;

    ~SteamManager();

    STEAM_CALLBACK();
    STEAM_CALLBACK(SteamManager, OnGameOverlayActivated, GameOverlayActivated_t);

    void OnGetNumberOfPlayers(NumberOfCurrentPlayers_t *pCallback, bool bIOFailure);
    
    CCallResult<SteamManager, NumberOfCurrentPlayers_t> m_numberOfPlayers;
    
    CSteamAchievements* m_steamAchievements;
    SteamDLC_t *m_dlcList;
    InputHandle_t *m_inputHandles;
    
    char m_installDir[MAX_GDR_PATH];
    
    uint32_t m_numInputHandles;
    AppId_t m_gameId;
};

void SteamManager::Init(void)
{
    steamManager = (SteamManager *)Hunk_Alloc(sizeof(SteamManager), "steamAPI", h_low);
    construct(steamManager);

    SteamAPI_Init();
    steamManager->m_gameId = SteamUtils()->GetAppID();
    SteamApps()->GetAppInstallDir(steamManager->m_gameId, steamManager->m_installDir, MAX_GDR_PATH);
    steamManager->m_inputHandles = (InputHandle_t *)Hunk_Alloc(sizeof(InputHandle_t) * STEAM_INPUT_MAX_COUNT, "steamInput", h_low);

    steamManager->m_steamAchievements = (CSteamAchievements *)Hunk_Alloc(sizeof(CSteamAchievements), "steamACH", h_low);
    construct(steamManager->m_steamAchievements, achievementInfo, arraylen(achievementInfo));

    steamManager->InitInput();
}

void SteamManager::InitCloud(void)
{
    SteamRemoteStorage()->Init();
}

void SteamManager::InitInput(void)
{
    // init the steam api thing
    SteamInput()->Init(false);

    m_numInputHandles = SteamInput()->GetConnectedControllers(m_inputHandles);

    for (uint32_t i = 0; i < 3; i++) {
        InputHandle_t handle = SteamInput()->GetControllerForGamepadIndex(i);
        if (!handle) { // regular xbox (modern one) controller that can be used with xinput
            STEAM_LOG("input handle %i is a regular xbox controller", i);
        }
        else { // something else
            ESteamInputType type = SteamInput()->GetInputTypeForHandle(handle);
            switch (type) {
            case k_ESteamInputType_Unknown:
                STEAM_ERROR("unknown controller type"); // wtf...
                break;
            case k_ESteamInputType_SteamController:
                STEAM_LOG("input handle %i is a steam controller", i);
                break;
            case k_ESteamInputType_XBox360Controller:
                STEAM_LOG("input handle %i is an xbox 360 controller", i);
                break;
            case k_ESteamInputType_XBoxOneController:
                STEAM_LOG("input handle %i is an xbox one controller", i);
                break;
            case k_ESteamInputType_GenericGamepad:
                STEAM_LOG("input handle %i is a DirectInput controller", i);
                break;
            case k_ESteamInputType_PS4Controller:
                STEAM_LOG("input handle %i is a ps4 controller", i);
                break;
            case k_ESteamInputType_PS3Controller:
                STEAM_LOG("input handle %i is a ps3 controller", i);
                break;
            };
        }
    }
}


void SteamManager::SteamFrame(void)
{

}

void SteamManager::ListDLC(void)
{
    uint32_t DLCcount = SteamApps()->GetDLCCount();
    m_dlcList = (SteamDLC_t *)Hunk_Alloc(sizeof(SteamDLC_t), "steamDLC", h_low);

    for (uint32_t i = 0; i < DLCcount; ++i) {
        AppId_t appId;
        bool available;
        char name[128];
        bool success = SteamApps()->BGetDLCDataByIndex(i, &appId, &available, name, 128);
        if (success)
            ::new ((void *)&m_dlcList[i]) SteamDLC_t( appId, name, available );
        else
            STEAM_ERROR("BGetDLCDataByIndex failed on %i", i);
    }
}

void SteamManager::GetNumberOfPlayers(void)
{
    STEAM_LOG("getting current number of players");
    SteamAPICall_t steamAPICall = SteamUserStats()->GetNumberOfCurrentPlayers();
    m_numberOfPlayers.Set(steamAPICall, this, &SteamManager::OnGetNumberOfPlayers);
}

void SteamManager::OnGetNumberOfPlayers(NumberOfCurrentPlayers_t *pCallback, bool bIOFailure)
{
    if (bIOFailure || !pCallback->m_bSuccess) {
        STEAM_ERROR("NumberOfCurrentPlayers_t failed");
        return;
    }
    STEAM_LOG("number of players in game: %i", pCallback->m_cPlayers);
}

void SteamManager::OnGameOverlayActivated(GameOverlayActivated_t *pCallback)
{
    if (pCallback->m_bActive)
        STEAM_LOG("overlay now active");
    else
        STEAM_LOG("overlay now inactive");
}


SteamManager::~SteamManager()
{
    SteamInput()->Shutdown();
    SteamAPI_Shutdown();
}