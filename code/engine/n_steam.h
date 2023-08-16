#ifndef _N_STEAM_
#define _N_STEAM_

#pragma once

class SteamStats
{
public:
    SteamStats(stat_t *stats, uint64_t nStats);
    ~SteamStats() = default;

    bool RequestStats(void);
    bool StoreStats(void);

    STEAM_CALLBACK(SteamStats, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
    STEAM_CALLBACK(SteamStats, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
private:
    stat_t *m_pStats;
    uint64_t m_appId;
    uint64_t m_nStats;
    qboolean m_initialized;
};

class SteamAchievements
{
public:
    SteamAchievements(achievement_t *achievements, uint64_t nAchievements);
    ~SteamAchievements() = default;

    bool RequestStats(void);
	bool SetAchievement(const char *id);

	STEAM_CALLBACK(SteamAchievements, OnUserStatsReceived, UserStatsReceived_t, m_callbackUserStatsReceived);
	STEAM_CALLBACK(SteamAchievements, OnUserStatsStored, UserStatsStored_t, m_callbackUserStatsStored);
	STEAM_CALLBACK(SteamAchievements, OnAchievementStored, UserAchievementStored_t, m_callbackAchievementStored);
private:
    achievement_t *m_pAchievements[NUM_ACHIEVEMENTS];
    uint64_t m_nAchievements;
    uint64_t m_appId;
    qboolean m_initialized;
};

class SteamManager
{
public:
    SteamManager(void);
    ~SteamManager();

    static void Init(void);

    static achievement_t *FetchAchievement(const char *id);
    static const char *FetchAchievementDescription(const char *id);
    static const char *FetchAchievementName(const char *id);
    static bool Initialized(void);

    static void RegisterAchievement(const char *id);

    static void GetNumberOfPlayers(void);
    static void ListDLC(void);
    static void SteamFrame(void);
private:
    STEAM_CALLBACK(SteamManager, OnGameOverlayActivated, GameOverlayActivated_t);

    void OnGetNumberOfPlayers(NumberOfCurrentPlayers_t *pCallback, bool IOFailure);

    SteamStats *m_stats;
    SteamAchievements *m_achievements;
    InputHandle_t *m_inputHandles;

    char m_installDir[MAX_GDR_PATH];

    eastl::vector<SteamDLC_t> m_dlcList;

    CCallResult<SteamManager, NumberOfCurrentPlayers_t> m_numberOfPlayers;
    
    uint32_t m_numInputHandles;
    AppId_t m_gameId;
    qboolean m_initialized;
};

inline bool SteamManager::Initialized(void)
{
    return m_initialized == qtrue ? true : false;
}

#endif