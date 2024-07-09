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

#ifndef __MODULE_HANDLE__
#define __MODULE_HANDLE__

#pragma once

#include "module_public.h"
#include "angelscript/angelscript.h"
#include "angelscript/as_compiler.h"
#include "angelscript/as_context.h"
#include "module_jit.h"
#include "scriptlib/scriptarray.h"
#include "scriptpreprocessor.h"

typedef uint64_t EModuleFuncId;

enum : uint64_t {
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
	ModuleOnJoystickEvent,

    NumFuncs
};

typedef struct {
	char name[MAX_NPATH];
	UtlVector<asIScriptFunction *> funcs;
} DynamicModule_t;

using ModuleIncludePath = eastl::string;

class CModuleHandle
{
public:
    CModuleHandle( const char *pName, const char *pDescription, const nlohmann::json& sourceFiles,
		int32_t moduleVersionMajor, int32_t moduleVersionUpdate, int32_t moduleVersionPatch,
		const nlohmann::json& includePaths, bool bIsDynamicModule );
    ~CModuleHandle();

    void SaveToCache( void ) const;
	int LoadFromCache( void );

    void ClearMemory( void );
    const string_t& GetName( void ) const;
	const string_t& GetDescription( void ) const;

	void LoadFunction( const string_t& moduleName, const string_t& funcName, asIScriptFunction **pFunction );

	const char *GetModulePath( void ) const;

    int CallFunc( EModuleFuncId nCallId, uint32_t nArgs, int *pArgList );

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

	const nlohmann::json& GetIncludePaths( void ) const {
		return m_IncludePaths;
	}
	const nlohmann::json& GetSourceFiles( void ) const {
		return m_SourceFiles;
	}

	bool LoadSourceFile( const string_t& filename );
	bool InitCalls( void );
private:
	void AddDefines( Preprocessor& preprocessor ) const;
	void RegisterGameObject( void );
	void PrepareContext( asIScriptFunction *pFunction );
	void Build( const nlohmann::json& sourceFiles );

	string_t m_szName;
	string_t m_szDescription;

	asIScriptFunction *m_pFuncTable[NumFuncs];

	qboolean m_bLoaded;

	uint32_t m_nStateStack;
	EModuleFuncId m_nLastCallId;

	int32_t m_nVersionMajor;
	int32_t m_nVersionUpdate;
	int32_t m_nVersionPatch;
	
	nlohmann::json m_SourceFiles;
	nlohmann::json m_IncludePaths;

	UtlHashMap<string_t, DynamicModule_t> m_DynamicModules;
};

class CModuleCacheHandle : public asIBinaryStream
{
public:
	CModuleCacheHandle( const char *path, fileMode_t mode );

	virtual int Read( void *pBuffer, asUINT nBytes ) override;
	virtual int Write( const void *pBuffer, asUINT nBytes ) override;

	virtual ~CModuleCacheHandle() override;

	fileHandle_t m_hFile;
};

class CModuleCacheManager
{
public:
	CModuleCacheManager( void );
private:
	nlohmann::json m_Data;
};

typedef struct {
    const char *name;
    uint64_t callId;
    uint32_t expectedArgs;
    qboolean required;
	qboolean mainOnly; // only nomadmain can have this
} moduleFunc_t;

extern const moduleFunc_t funcDefs[NumFuncs];

#endif