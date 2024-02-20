#ifndef _N_STEAM_
#define _N_STEAM_

#pragma once

typedef struct {
    char szName[1024];
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

    dlc_t *m_pDlcList;
    uint64_t m_nDlcCount;

    AppId_t m_nAppId;

    void *m_pSteamDLL;
};

#endif