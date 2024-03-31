#ifndef __MODULE_HANDLE__
#define __MODULE_HANDLE__

#pragma once

#include "module_public.h"
#include <mono/metadata/assembly.h>

typedef uint64_t EModuleFuncId;

enum : uint64_t
{
    ModuleInit = 0,
	ModuleShutdown,
    ModuleCommandLine,

    ModuleDrawConfiguration,
    ModuleSaveConfiguration,

    ModuleOnKeyEvent,
    ModuleOnMouseEvent,
    ModuleOnLevelStart,
    ModuleOnLevelEnd,
    ModuleOnRunTic,
    ModuleOnSaveGame,
    ModuleOnLoadGame,

    NumFuncs
};

using ModuleIncludePath = eastl::string;

class CModuleHandle
{
public:
    CModuleHandle( const char *pName, const std::vector<std::string>& sourceFiles,
		int32_t moduleVersionMajor, int32_t moduleVersionUpdate, int32_t moduleVersionPatch );
    ~CModuleHandle();

    void SaveToCache( void ) const;
	int LoadFromCache( void );

    void ClearMemory( void );
    const string_t& GetName( void ) const;

	const char *GetModulePath( void ) const;

    int CallFunc( EModuleFuncId nCallId, uint32_t nArgs, uint32_t *pArgList );

	inline void GetVersion( int32_t *major, int32_t *update, int32_t *patch ) const {
		*major = m_nVersionMajor;
		*update = m_nVersionUpdate;
		*patch = m_nVersionPatch;
	}
	inline bool IsValid( void ) const {
		return m_bLoaded;
	}
    inline asIScriptFunction *GetFunction( EModuleFuncId nCallId ) {
        return m_pFuncTable[nCallId];
    }
	const int32_t *VersionMajor( void ) const {
		return &m_nVersionMajor;
	}
	const int32_t *VersionUpdate( void ) const {
		return &m_nVersionUpdate;
	}
	const int32_t *VersionPatch( void ) const {
		return &m_nVersionPatch;
	}

	const std::vector<std::string>& GetIncludePaths( void ) const {
		return m_IncludePaths;
	}
private:
	void Build( const std::vector<std::string>& sourceFiles );
    bool InitCalls( void );
    void LoadSourceFile( const std::string& filename );

    asIScriptFunction *m_pFuncTable[NumFuncs];
	std::vector<std::string> m_IncludePaths;

    string_t m_szName;
    asIScriptContext *m_pScriptContext;
    asIScriptModule *m_pScriptModule;
    MonoAssembly *m_pAssembly;
    MonoImage *m_pImage;
	qboolean m_bLoaded;

	int32_t m_nVersionMajor;
	int32_t m_nVersionUpdate;
	int32_t m_nVersionPatch;
};


typedef struct {
    const char *name;
    uint64_t callId;
    uint32_t expectedArgs;
    qboolean required;
} moduleFunc_t;

extern const moduleFunc_t funcDefs[NumFuncs];

#endif