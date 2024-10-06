#ifndef __MODULE_PUBLIC__
#define __MODULE_PUBLIC__

#pragma once

#include "../engine/n_shared.h"
#include "../game/g_game.h"

#include <string>
#include <glm/glm.hpp>

#include <EASTL/allocator.h>
#include <EASTL/allocator_malloc.h>
#include "angelscript/angelscript.h"

const char *AS_PrintErrorString( int code );
GDR_INLINE void CheckValue( const char *caller, const char *func, int value ) {
	if ( value < asSUCCESS ) {
		N_Error( ERR_FATAL, "%s: %s failed -- %s", caller, func, AS_PrintErrorString( value ) );
	}
}

#define CheckASCall( call ) \
	CheckValue( __func__, #call, call )

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

constexpr GDR_INLINE bool operator==( const eastl::allocator& a, const CModuleAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const eastl::allocator& a, const CModuleAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator==( const CModuleAllocator& a, const eastl::allocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const CModuleAllocator& a, const eastl::allocator& b ) {
	return true;
}

#include "module_memory.h"
#include <EASTL/fixed_string.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_map.h>
#include <EASTL/fixed_hash_map.h>
#include <EASTL/fixed_set.h>
#include <EASTL/sort.h>
#include <EASTL/string_hash_map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/array.h>
#include <EASTL/functional.h>
#include <EASTL/string_view.h>
#include <EASTL/list.h>

#include "scriptlib/script_cache_ids.h"

#include "../engine/n_allocator.h"
#include "../rendercommon/r_public.h"

// literally just meant for json
template<typename T>
class CModuleAllocatorTemplated
{
public:

	EASTL_ALLOCATOR_EXPLICIT CModuleAllocatorTemplated( const char* pName = EASTL_NAME_VAL( EASTL_ALLOCATOR_DEFAULT_NAME ) ) { }
	CModuleAllocatorTemplated( const CModuleAllocatorTemplated& x ) { }
	CModuleAllocatorTemplated( const CModuleAllocatorTemplated& x, const char* pName ) { }

	CModuleAllocatorTemplated& operator=( const CModuleAllocatorTemplated& x ) = default;

	T* allocate( size_t n, int flags = 0 ) {
		return (T *)Mem_ClearedAlloc( sizeof( T ) );
	}
	T* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 ) {
		return (T *)Mem_ClearedAlloc( sizeof( T ) );
	}
	void  deallocate( T* p, size_t n ) {
		Mem_Free( p );
	}
};

class CModuleStringAllocator
{
public:
	EASTL_ALLOCATOR_EXPLICIT CModuleStringAllocator( const char* pName = EASTL_NAME_VAL( EASTL_ALLOCATOR_DEFAULT_NAME ) ) { }
	CModuleStringAllocator( const CModuleStringAllocator& x ) { }
	CModuleStringAllocator( const CModuleStringAllocator& x, const char* pName ) { }

	CModuleStringAllocator& operator=( const CModuleStringAllocator& x ) = default;

	void* allocate( size_t n, int flags = 0 ) {
		return Mem_Alloc( n );
	}
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 ) {
		return Mem_Alloc( n );
	}
	void  deallocate( void *p, size_t n ) {
		Mem_Free( p );
	}
};

constexpr GDR_INLINE bool operator==( const CModuleAllocator& a, const CModuleAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const CModuleAllocator& a, const CModuleAllocator& b ) {
	return false;
}

constexpr GDR_INLINE bool operator==( const CModuleStringAllocator& a, const CModuleStringAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const CModuleStringAllocator& a, const CModuleStringAllocator& b ) {
	return false;
}

constexpr GDR_INLINE bool operator==( const eastl::allocator& a, const CModuleStringAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const eastl::allocator& a, const CModuleStringAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator==( const CModuleStringAllocator& a, const eastl::allocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const CModuleStringAllocator& a, const eastl::allocator& b ) {
	return true;
}

constexpr GDR_INLINE bool operator==( const CModuleAllocator& a, const CModuleStringAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const CModuleAllocator& a, const CModuleStringAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator==( const CModuleStringAllocator& a, const CModuleAllocator& b ) {
	return true;
}
constexpr GDR_INLINE bool operator!=( const CModuleStringAllocator& a, const CModuleAllocator& b ) {
	return true;
}

using string_view_t = eastl::basic_string_view<char>;
using string_t = eastl::fixed_string<char, 256, true, eastl::allocator_malloc<char>>;
//using string_t = eastl::basic_string<char, memory::std_allocator<char, memory::virtual_memory_allocator>>;
using UtlString = eastl::fixed_string<char, 528, true, eastl::allocator_malloc<char>>;
namespace eastl {
	// for some reason, the eastl doesn't support eastl::hash<eastl::fixed_string>
	template<> struct hash<UtlString> {
		size_t operator()( const UtlString& str ) const {
			const unsigned char *p = (const unsigned char *)str.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};
	template<> struct hash<string_t> {
		size_t operator()( const string_t& str ) const {
			const unsigned char *p = (const unsigned char *)str.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};
};

template<typename T>
using UtlStringHashTable = eastl::string_hash_map<T, eastl::hash<const char *>, eastl::str_equal_to<const char *>, CModuleAllocator>;
template<typename T, typename Allocator = CModuleAllocator>
using UtlVector = eastl::vector<T, Allocator>;
template<typename Key, typename Value, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>>
using UtlHashMap = eastl::hash_map<Key, Value, Hash, Predicate, CModuleAllocator, true>;
template<typename Key, typename Value, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>>
using UtlMap = eastl::unordered_map<Key, Value, Hash, Predicate, CModuleAllocator, true>;
template<typename Key, typename Compare = eastl::less<Key>>
using UtlSet = eastl::set<Key, Compare, CModuleAllocator>;
template<typename T, typename Allocator = CModuleAllocator>
using UtlList = eastl::list<T, Allocator>;

#include "nlohmann/json.hpp"
#include "module_handle.h"
#include "module_buffer.hpp"
#include "scriptbuilder.h"
#include "scriptlib/scriptdictionary.h"
#include "scriptlib/scriptarray.h"
#include "scriptlib/scriptstdstring.h"
#include "scriptlib/scriptmath.h"
#include "scriptlib/scripthandle.h"
#include "scriptlib/scriptparser.h"
#include "contextmgr.h"
#include "module_jit.h"

#include "../game/g_archive.h"

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

template<typename T, typename... Args>
inline T *CreateObject( Args&&... args ) {
	return (T *)::new ( Mem_Alloc( sizeof(T) ) ) T( eastl::forward<Args>( args )... );
}
template<typename T>
inline void DeleteObject( T *pObject ) {
	pObject->~T();
	Mem_Free( (void *)pObject );
}

struct CModuleInfo
{
	CModuleInfo( nlohmann::json& parse, CModuleHandle *pHandle ) {
		string_t *pString;

		N_strncpyz( m_szName, parse[ "Name" ].get<string_t>().c_str(), MAX_NPATH );
		N_strncpyz( m_szId, parse[ "Id" ].get<string_t>().c_str(), MAX_NPATH );
		
		m_nDependencies = parse.at( "DependedModules" ).size();
		m_pDependencies = (string_t *)Hunk_Alloc( sizeof( *m_pDependencies ) * m_nDependencies, h_high );
		pString = m_pDependencies;
		for ( const auto& it : parse.at( "DependedModules" ) ) {
			*pString++ = eastl::move( it.get<string_t>() );
		}

		m_GameVersion.m_nVersionMajor = parse[ "Version" ][ "GameVersionMajor" ];
		m_GameVersion.m_nVersionUpdate = parse[ "Version" ][ "GameVersionUpdate" ];
		m_GameVersion.m_nVersionPatch = parse[ "Version" ][ "GameVersionPatch" ];

		m_nModVersionMajor = parse[ "Version" ][ "VersionMajor" ];
		m_nModVersionUpdate = parse[ "Version" ][ "VersionUpdate" ];
		m_nModVersionPatch = parse[ "Version" ][ "VersionPatch" ];

		m_pHandle = pHandle;

		Con_Printf( "...loaded module \"%s\", v%i.%i.%i\n", m_szName, m_nModVersionMajor, m_nModVersionUpdate, m_nModVersionPatch );
	}
	~CModuleInfo() {
	}

	inline bool operator==( const UtlString& other ) const {
		return N_stricmp( m_szName, other.c_str() ) == 0;
	}

	char m_szName[MAX_NPATH];
	char m_szId[MAX_NPATH];
	uint32_t m_nDependencies;
	string_t *m_pDependencies;

	CModuleHandle *m_pHandle;
	
	int32_t m_nModVersionMajor;
	int32_t m_nModVersionUpdate;
	int32_t m_nModVersionPatch;

	version_t m_GameVersion;
};

inline bool operator==( CModuleInfo *const m1, const UtlString& m2 ) {
	return *m1 == m2;
}

class CContextMgr;
class CScriptBuilder;

#include "module_debugger.h"

class CModuleCrashData
{
public:
	CModuleCrashData( const char *pType, CModuleHandle *pModule )
		: m_pExceptionType( pType ), m_pMainModule( pModule )
	{ }
	CModuleCrashData( void ) = default;
	CModuleCrashData( const CModuleCrashData & ) = default;
	~CModuleCrashData() = default;

	const char *m_pExceptionType;
	CModuleHandle *m_pMainModule;
	eastl::vector<CModuleHandle *> m_InvolvedModules;
};

typedef struct module_s {
	CModuleInfo *info;
	uint64_t numDependencies;
	uint64_t bootIndex;
	qboolean active;        // is it active?
	qboolean valid;         // did it fail to load?
	qboolean isRequired;
	qboolean allDepsActive;

	module_s& operator=( module_s& other );
	module_s& operator=( const module_s& other );
	bool operator==( const string_t& other ) const;
	bool operator!=( const string_t& other ) const;
	bool operator<( const module_s& other ) const;
	bool operator>( const module_s& other ) const;
	bool operator==( const module_s& other ) const;
	bool operator!=( const module_s& other ) const;
} module_t;

//
// required modules, those made with love, by Your Resident Fiend
//
inline const char *requiredModules[] = {
	"nomadmain"
};

inline qboolean IsRequiredModule( const char *name ) {
	for ( const auto& it : requiredModules ) {
		if ( N_streq( name, it ) ) {
			return qtrue;
		}
	}
	return qfalse;
}

inline qboolean IsLoadedAfter( const module_t *mod, const module_t *dep ) {
	return ( dep > mod );
}

inline qboolean IsLoadedBefore( const module_t *mod, const module_t *dep ) {
	return ( dep > mod );
}

inline qboolean IsDependentOn( const module_t *mod, const module_t *dep ) {
	if ( !mod->info ) {
		Con_Printf( COLOR_YELLOW "WARNING: bad mod info\n" );
		return qfalse;
	}
	for ( uint32_t i = 0; i < mod->info->m_nDependencies; i++ ) {
		if ( N_streq( mod->info->m_pDependencies[i].c_str(), dep->info->m_szName ) ) {
			return qtrue;
		}
	}
	return qfalse;
}


inline module_s& module_s::operator=( module_s& other ) {
	info = other.info;
	active = other.active;
	valid = other.valid;
	numDependencies = other.numDependencies;
	bootIndex = other.bootIndex;
	isRequired = other.isRequired;
	allDepsActive = other.allDepsActive;
	return *this;
}

inline module_s& module_s::operator=( const module_s& other ) {
	info = const_cast<CModuleInfo *>( other.info );
	active = other.active;
	valid = other.valid;
	numDependencies = other.numDependencies;
	bootIndex = other.bootIndex;
	isRequired = other.isRequired;
	allDepsActive = other.allDepsActive;
	return *this;
}

inline bool module_s::operator<( const module_s& other ) const {
	if ( IsDependentOn( eastl::addressof( other ), this ) ) {
		return true;
	}
	return N_strcmp( info->m_szName, other.info->m_szName ) == 1 ? true : false;
}
inline bool module_s::operator>( const module_s& other ) const {
	if ( IsDependentOn( this, eastl::addressof( other ) ) ) {
		return false;
	}
	return N_strcmp( other.info->m_szName, info->m_szName ) == -1 ? true : false;
}
inline bool module_s::operator==( const module_s& other ) const {
	return N_strcmp( info->m_szName, other.info->m_szName ) == 0;
}
inline bool module_s::operator!=( const module_s& other ) const {
	return N_strcmp( info->m_szName, other.info->m_szName ) != 0;
}

inline bool module_s::operator==( const string_t& other ) const {
	return N_strcmp( info->m_szName, other.c_str() ) == 0;
}

inline bool module_s::operator!=( const string_t& other ) const {
	return N_strcmp( info->m_szName, other.c_str() ) != 0;
}

typedef struct {
	int64_t ident;
	version_t gameVersion;
	int32_t moduleVersionMajor;
	int32_t moduleVersionUpdate;
	int32_t moduleVersionPatch;
	uint32_t checksum;
	qboolean hasDebugSymbols;
	uint64_t modCount;
	char modList[0];
} asCodeCacheHeader_t;

class CModuleLib
{
public:
	CModuleLib( void );
	~CModuleLib();

	void Shutdown( qboolean quit );
	CModuleInfo *GetModule( const char *pName );
	int ModuleCall( CModuleInfo *pModule, EModuleFuncId nCallId, uint32_t nArgs, ... );
	CModuleInfo *GetLoadList( void );
	uint64_t GetModCount( void ) const;

	// runs all modules besides for sgame
	void RunModules( EModuleFuncId nCallId, uint32_t nArgs, ... );

	// only for module_lib
	CScriptBuilder *GetScriptBuilder( void );
	asIScriptEngine *GetScriptEngine( void );
	CContextMgr *GetContextManager( void );
	void RegisterCvar( const UtlString& name, const UtlString& value, uint32_t flags, bool trackChanges, uint32_t privateFlag );
	bool AddDefaultProcs( void ) const;

	void SetHandle( CModuleHandle *pHandle ) {
		m_pCurrentHandle = pHandle;
	}
	const CModuleHandle *GetCurrentHandle( void ) const {
		return m_pCurrentHandle;
	}
	CModuleHandle *GetCurrentHandle( void ) {
		return m_pCurrentHandle;
	}
	asIScriptModule *GetScriptModule( void ) {
		return m_pModule;
	}
	asIScriptContext *GetScriptContext( void ) {
		return m_pContext;
	}

	CDebugger *GetDebugger( void );

	qboolean IsModuleInCache( const char *name ) const;

	module_t *m_pModList;
private:
	void SaveByteCodeCache( void );
	bool LoadByteCodeCache( void );
	void LoadModList( void );
	void LoadModule( const char *pModuleName );

	CModuleInfo *m_pLoadList;
	uint64_t m_nModuleCount;

	CScriptBuilder *m_pScriptBuilder;
	CContextMgr *m_pContextManager;
	asIScriptEngine *m_pEngine;

	qboolean m_bRegistered;
	qboolean m_bRecursiveShutdown;

	asCJITCompiler *m_pCompiler;
	CModuleHandle *m_pCurrentHandle;

	asIScriptModule *m_pModule;
	asIScriptContext *m_pContext;

	asCodeCacheHeader_t *m_pCacheData;
};

extern moduleImport_t moduleImport;

typedef CModuleLib *(*GetModuleAPI_t)( const moduleImport_t *, const renderExport_t *, version_t nGameVersion );
CModuleLib *InitModuleLib( const moduleImport_t *pImport, const renderExport_t *pExport, version_t nGameVersion );

extern CModuleLib *g_pModuleLib;

#ifdef _NOMAD_DEBUG
extern void *AS_Alloc( size_t nSize, const char *fileName, const uint32_t lineNumber );
extern void AS_Free( void *pBuffer, const char *fileName, const uint32_t lineNumber );
#else
extern void *AS_Alloc( size_t nSize );
extern void AS_Free( void *pBuffer );
#endif

extern cvar_t *ml_debugMode;
extern cvar_t *ml_angelScript_DebugPrint;
extern cvar_t *ml_alwaysCompile;
extern cvar_t *ml_allowJIT;
extern cvar_t *ml_garbageCollectionIterations;

#endif