#ifndef __MODULE_PUBLIC__
#define __MODULE_PUBLIC__

#pragma once

#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "../game/g_archive.h"

#include <glm/glm.hpp>

#include <EASTL/allocator.h>
#include <EASTL/allocator_malloc.h>

class CModuleAllocator
{
public:
    EASTL_ALLOCATOR_EXPLICIT CModuleAllocator( const char* pName = EASTL_NAME_VAL( EASTL_ALLOCATOR_DEFAULT_NAME ) ) { }
	CModuleAllocator( const CModuleAllocator& x ) { }
	CModuleAllocator( const CModuleAllocator& x, const char* pName ) { }

	CModuleAllocator& operator=( const CModuleAllocator& x ) = default;

	void* allocate( size_t n, int flags = 0 );
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 );
	void  deallocate( void* p, size_t n );

	const char* get_name( void ) const { return NULL; }
	void        set_name( const char* pName ) { }
private:
	#if EASTL_NAME_ENABLED
		const char* mpName; // Debug name, used to track memory.
	#endif
};

#include "module_memory.h"
#include <EASTL/fixed_string.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_map.h>
#include <EASTL/fixed_hash_map.h>
#include <EASTL/fixed_set.h>
#include <EASTL/sort.h>
#include <EASTL/string_hash_map.h>
#include <EASTL/unordered_map.h>

#include "../engine/n_allocator.h"
#include "../rendercommon/r_public.h"

template<typename T>
using UtlStringHashTable = eastl::string_hash_map<T, eastl::hash<const char *>, eastl::str_equal_to<const char *>, CModuleAllocator>;
template<typename T>
using UtlVector = eastl::vector<T, CModuleAllocator>;
template<typename Key, typename Value>
using UtlHashMap = eastl::hash_map<Key, Value, eastl::hash<Key>, eastl::equal_to<Key>, CModuleAllocator, true>;
template<typename Key, typename Value>
using UtlMap = eastl::unordered_map<Key, Value, eastl::hash<Key>, eastl::equal_to<Key>, CModuleAllocator, true>;
template<typename Key, typename Compare = eastl::less<Key>>
using UtlSet = eastl::set<Key, Compare, CModuleAllocator>;
using UtlString = eastl::basic_string<char, eastl::allocator_malloc<char>>;

#include "nlohmann/json.hpp"
#include "module_handle.h"
#include "module_buffer.hpp"
#include "scriptarray.h"
#include "scriptdictionary.h"
#include "scriptbuilder.h"
#include "scriptstdstring.h"
#include "scriptmath.h"
#include "contextmgr.h"

typedef struct
{
    void *(*Malloc)(uint64_t size);
    void *(*Realloc)(void *ptr, uint64_t nsize); // really just for stb_image.h
    void (*FreeAll)(void);
	void (*Free)(void *ptr);
    char *(*CopyString)(const char *str);
#ifdef _NOMAD_DEBUG
    void *(*Hunk_AllocDebug)(uint64_t size, ha_pref where, const char *label, const char *file, uint64_t line);
#else
	void *(*Hunk_Alloc)(uint64_t size, ha_pref where);
#endif
    void *(*Hunk_AllocateTempMemory)( uint64_t size );
    void (*Hunk_FreeTempMemory)( void *buf );

    void (*Sys_FreeFileList)( char **list );

    void (GDR_DECL *Printf)( int level, const char *fmt, ... ) GDR_ATTRIBUTE((format(printf, 2, 3)));
    void GDR_NORETURN (GDR_DECL *Error)(errorCode_t code, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));

    void (*Cvar_VariableStringBuffer)(const char *name, char *buffer, uint64_t bufferSize);
    void (*Cvar_VariableStringBufferSafe)(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag);
    int64_t (*Cvar_VariableInteger)(const char *name);
    cvar_t *(*Cvar_Get)(const char *name, const char *value, uint32_t flags);
    void (*Cvar_SetGroup)(cvar_t *cv, cvarGroup_t group);
    void (*Cvar_SetDescription)(cvar_t *cv, const char *description);
    void (*Cvar_Set)(const char *name, const char *value);
    void (*Cvar_CheckRange)(cvar_t *var, const char *mins, const char *maxs, cvartype_t type);
    int (*Cvar_CheckGroup)(cvarGroup_t group);
    void (*Cvar_ResetGroup)( cvarGroup_t group, qboolean resetModifiedFlags );
    void (*Cvar_Reset)(const char *name);
    const char *(*Cvar_VariableString)(const char *name);

    void (*Cmd_AddCommand)(const char* name, cmdfunc_t function);
    void (*Cmd_RemoveCommand)(const char* name);
    void (*Cmd_ExecuteCommand)(const char* name);
    void (*Cmd_ExecuteString)(const char *str);
    uint32_t (*Cmd_Argc)(void);
    char *(*Cmd_ArgsFrom)(uint32_t index);
    const char *(*Cmd_Argv)(uint32_t index);

    uint64_t (*Milliseconds)(void);

    qboolean (*Key_IsDown)(uint32_t keynum);

    void (*FS_FreeFileList)(char **list);
    uint64_t (*FS_Write)(const void *buffer, uint64_t size, fileHandle_t f);
    uint64_t (*FS_Read)(void *buffer, uint64_t size, fileHandle_t);
    fileOffset_t (*FS_FileSeek)(fileHandle_t f, fileOffset_t offset, uint32_t whence);
    fileOffset_t (*FS_FileTell)(fileHandle_t f);
    uint64_t (*FS_FileLength)(fileHandle_t f);
    qboolean (*FS_FileExists)(const char *filename);
    fileHandle_t (*FS_FOpenRead)(const char *path);
    fileHandle_t (*FS_FOpenWrite)(const char *path);
    void (*FS_FClose)(fileHandle_t f);
    void (*FS_FreeFile)(void *buffer);
    uint64_t (*FS_LoadFile)(const char *path, void **buffer);
    char **(*FS_ListFiles)(const char *path, const char *extension, uint64_t *numfiles);
    void (*FS_WriteFile)(const char *npath, const void *buffer, uint64_t size);

    void *(*Sys_LoadDLL)(const char *name);
    void *(*Sys_GetProcAddress)(void *handle, const char *name);
    void (*Sys_CloseDLL)(void *handle);
} moduleImport_t;

class CModuleHandle;

struct CModuleInfo
{
	CModuleInfo( nlohmann::json& parse, CModuleHandle *pHandle ) {
		N_strncpyz( m_szName, parse["module_name"].get<std::string>().c_str(), MAX_NPATH );
        
        m_Dependencies.reserve( parse["dependencies"].size() );
        for ( const auto& it : parse["dependencies"] ) {
            m_Dependencies.emplace_back( eastl::move( it.get<std::string>().c_str() ) );
        }

		m_GameVersion.m_nVersionMajor = parse["version"]["game_version_major"];
		m_GameVersion.m_nVersionUpdate = parse["version"]["game_version_update"];
		m_GameVersion.m_nVersionPatch = parse["version"]["game_version_patch"];

		m_nModVersionMajor = parse["version"]["version_major"];
		m_nModVersionUpdate = parse["version"]["version_update"];
		m_nModVersionPatch = parse["version"]["version_patch"];

		m_pHandle = pHandle;

        Con_Printf( " - Added module \"%s\", v%i.%i.%i\n", m_szName, m_nModVersionMajor, m_nModVersionUpdate, m_nModVersionPatch );
	}
	~CModuleInfo() {
		delete m_pHandle;
	}

	char m_szName[MAX_NPATH];
	UtlVector<eastl::string> m_Dependencies;

	CModuleHandle *m_pHandle;
	
	int32_t m_nModVersionMajor;
	int32_t m_nModVersionUpdate;
	int32_t m_nModVersionPatch;

	version_t m_GameVersion;
};

class CContextMgr;
class CScriptBuilder;

class CModuleLib
{
public:
    CModuleLib( void );
    ~CModuleLib();

    void Init( void );
    void Shutdown( void );
	CModuleInfo *GetModule( const char *pName );
	int ModuleCall( CModuleInfo *pModule, EModuleFuncId nCallId, uint32_t nArgs, ... );

	// only for module_lib
	CScriptBuilder *GetScriptBuilder( void );
	asIScriptEngine *GetScriptEngine( void );
	CContextMgr *GetContextManager( void );
private:
	void LoadModule( const char *pModuleName );

	UtlVector<CModuleInfo> m_LoadList;

	CScriptBuilder *m_pScriptBuilder;
	CContextMgr *m_pContextManager;
	asIScriptEngine *m_pEngine;
};

extern moduleImport_t moduleImport;

typedef CModuleLib *(*GetModuleAPI_t)( const moduleImport_t *, const renderExport_t *, version_t nGameVersion );
CModuleLib *InitModuleLib( const moduleImport_t *pImport, const renderExport_t *pExport, version_t nGameVersion );

extern CModuleLib *g_pModuleLib;

extern cvar_t *ml_debugMode;

#endif