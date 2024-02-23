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

typedef struct {
	char *name;
	int32_t nameLength;
	int32_t type;
	uint32_t dataSize;
	union {
		int8_t s8;
		int16_t s16;
		int32_t s32;
		int64_t s64;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		float f;
		char *str;
		vec2_t v2;
		vec3_t v3;
		vec4_t v4;
	} data;
} ngdfield_t;


#pragma pack( push, 1 )
// version, 64 bits
typedef union {
	struct {
		uint16_t m_nVersionMajor;
		uint16_t m_nVersionUpdate;
		uint32_t m_nVersionPatch;
	};
	uint64_t m_nVersion;
} version_t;

typedef struct
{
	int32_t ident;
	version_t version;
} ngdvalidation_t;
#pragma pack( pop )

typedef struct {
	ngdvalidation_t validation;
	int64_t numSections;

    gamedata_t gamedata;
} ngdheader_t;

typedef struct {
    char name[MAX_STRING_CHARS];
    int64_t size;
    int64_t numFields;
} ngdsection_write_t;

typedef struct ngdsection_read_s
{
	char name[MAX_STRING_CHARS];
	int64_t nameLength;
	int64_t size;
	int64_t numFields;
	
	ngdfield_t *m_pFieldList;
	
	struct ngdsection_read_s *next;
} ngdsection_read_t;

typedef struct {
	char name[MAX_OSPATH];
	
	ngdheader_t header;
	
	ngdsection_read_t *m_pSectionList;
} ngd_file_t;

class CGameArchive
{
public:
    CGameArchive( void ) = default;
    ~CGameArchive() = default;

    void BeginSaveSection( const char *name );
    void EndSaveSection( void );

    const char **GetSaveFiles( uint64_t *nFiles ) const;

    void SaveFloat( const char *name, float data );

    void SaveByte( const char *name, uint8_t data );
    void SaveUShort( const char *name, uint16_t data );
    void SaveUInt( const char *name, uint32_t data );
    void SaveULong( const char *name, uint64_t data );

    void SaveChar( const char *name, int8_t data );
    void SaveShort( const char *name, int16_t data );
    void SaveInt( const char *name, int32_t data );
    void SaveLong( const char *name, int64_t data );

    void SaveVec2( const char *name, const vec2_t data );
    void SaveVec3( const char *name, const vec3_t data );
    void SaveVec4( const char *name, const vec4_t data );

    void SaveString( const char *name, const char *data );

    float LoadFloat( const char *name, nhandle_t hSection );

    uint8_t LoadByte( const char *name, nhandle_t hSection );
    uint16_t LoadUShort( const char *name, nhandle_t hSection );
    uint32_t LoadUInt( const char *name, nhandle_t hSection );
    uint64_t LoadULong( const char *name, nhandle_t hSection );

    int8_t LoadChar( const char *name, nhandle_t hSection );
    int16_t LoadShort( const char *name, nhandle_t hSection );
    int32_t LoadInt( const char *name, nhandle_t hSection );
    int64_t LoadLong( const char *name, nhandle_t hSection );

    void LoadVec2( const char *name, vec2_t data, nhandle_t hSection );
    void LoadVec3( const char *name, vec3_t data, nhandle_t hSection );
    void LoadVec4( const char *name, vec4_t data, nhandle_t hSection );

    void LoadString( const char *name, char *pBuffer, int32_t maxLength, nhandle_t hSection );

    bool Load( const char *filename );
    bool Save( void );
    bool LoadPartial( const char *filename, gamedata_t *gd );

    nhandle_t GetSection( const char *name );

    friend void G_InitArchiveHandler( void );
    friend void G_ShutdownArchiveHandler( void );
private:
    void AddField( const char *name, int32_t type, const void *data, uint32_t dataSize );
    bool ValidateHeader( const void *header ) const;
    qboolean LoadArchiveFile( const char *filename, uint64_t index );
    const ngdfield_t *FindField( const char *name, int32_t type, nhandle_t hSection ) const;

    fileHandle_t m_hFile;
    ngdsection_read_t *m_pSectionList;
    
    int64_t m_nSections;
    int64_t m_nSectionDepth;
    ngdsection_write_t m_Section;
    uint64_t m_nArchiveFiles;
    
    char **m_pArchiveFileList;
    ngd_file_t **m_pArchiveCache;
    ngd_file_t *m_pCurrentArchive;
};

void G_InitArchiveHandler( void );
void G_ShutdownArchiveHandler( void );

extern CGameArchive *g_pArchiveHandler;

#endif