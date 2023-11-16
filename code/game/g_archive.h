#ifndef __G_ARCHIVE__
#define __G_ARCHIVE__

#pragma once

typedef struct {
    char mapName[MAX_GDR_PATH];
    gamedif_t diff;

    // bff-specific information
    char bffName[MAX_BFF_PATH];
    uint64_t bffId;
    uint32_t levelIndex;
} gamedata_t;

class CGameArchive
{
public:
    CGameArchive( void );
    ~CGameArchive();

    bool Load( const char *filename );
    bool Save( const char *filename );
    bool LoadPartial( const char *filename, gamedata_t *gd );
private:
    void GDR_ATTRIBUTE((format(printf, 2, 3))) Error( const char *fmt, ... ) const;
    bool ValidateHeader( const void *header ) const;

    const char *curfile;
};

#endif