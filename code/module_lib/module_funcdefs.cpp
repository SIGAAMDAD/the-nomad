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

#include "../engine/n_shared.h"
#include "module_public.h"
#include <glm/gtc/type_ptr.hpp>
#include <limits.h>
#include "aswrappedcall.h"
#include "imgui_stdlib.h"
#include "../ui/ui_lib.h"
#include "module_stringfactory.hpp"
#include "../game/g_world.h"
#include "module_funcdefs.hpp"
#include "scriptlib/scriptarray.h"

#include "module_engine/module_polyvert.h"
#include "module_engine/module_polyquad.h"
#include "module_engine/module_raycast.h"
#include "../system/sys_timer.h"
#include "scriptlib/scriptjson.h"
#include "module_engine/module_gpuconfig.h"
#include "scriptlib/scriptconvert.h"
#include "module_engine/module_bbox.h"
#ifdef USE_QUAKE3_SOUND
#include "../game/snd_public.h"
#endif
#include "../sound/snd_local.h"

void ScriptLib_Register_Sound( void );
void ScriptLib_Register_Game( void );
void ScriptLib_Register_Engine( void );
void ScriptLib_Register_Renderer( void );

//
// c++ compatible wrappers around angelscript engine function calls
//

#define REQUIRE_ARG_COUNT( amount ) \
	Assert( pGeneric->GetArgCount() == amount )
#define DEFINE_CALLBACK( name ) \
	static void ModuleLib_##name ( asIScriptGeneric *pGeneric )
#define GETVAR( type, index ) \
	*(const type *)pGeneric->GetArgAddress( index )

#define REGISTER_GLOBAL_VAR( name, addr ) \
	ValidateGlobalVar( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterGlobalProperty( name, (void *)addr ) )
#define REGISTER_ENUM_TYPE( name ) \
	ValidateEnumType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterEnum( name ) )
#define REGISTER_ENUM_VALUE( type, name, value ) \
	ValidateEnumValue( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterEnumValue( type, name, value ) )
#define REGISTER_METHOD_FUNCTION( obj, decl, classType, name, parameters, returnType ) \
	ValidateMethod( __func__, decl, g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( obj, decl, WRAP_MFN_PR( classType, name, parameters, returnType ), asCALL_GENERIC ) )
#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
	ValidateFunction( __func__, decl,\
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, funcPtr, asCALL_GENERIC ) )
#define REGISTER_OBJECT_TYPE( name, obj, traits ) \
	ValidateObjectType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterObjectType( name, sizeof(obj), traits | asGetTypeTraits<obj>() ) )
#define REGISTER_OBJECT_PROPERTY( obj, var, offset ) \
	ValidateObjectProperty( __func__, var, g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( obj, var, offset ) )
#define REGISTER_OBJECT_BEHAVIOUR( obj, type, decl, funcPtr ) \
	ValidateObjectBehaviour( __func__, #type, g_pModuleLib->GetScriptEngine()->RegisterObjectBehaviour( obj, type, decl, funcPtr, asCALL_GENERIC ) )
#define REGISTER_TYPEDEF( type, alias ) \
	ValidateTypedef( __func__, alias, g_pModuleLib->GetScriptEngine()->RegisterTypedef( alias, type ) )

#define SET_NAMESPACE( name ) g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( name )
#define RESET_NAMESPACE() g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( "" )

static void ValidateEnumType( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register enumeration type %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateEnumValue( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register enumeration value %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateFunction( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register global function %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateMethod( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register method function %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateObjectType( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register object type %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateObjectBehaviour( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register object behaviour %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateObjectProperty( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register object property %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateTypedef( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register typedef %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

static void ValidateGlobalVar( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register global variable %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

DEFINE_CALLBACK( CvarRegisterGeneric ) {
	vmCvar_t vmCvar;

	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	string_t *value = (string_t *)pGeneric->GetArgObject( 1 );
	asDWORD flags = pGeneric->GetArgDWord( 2 );
	asINT32 *intValue = (asINT32 *)pGeneric->GetArgAddress( 3 );
	float *floatValue = (float *)pGeneric->GetArgAddress( 4 );
	asINT32 *modificationCount = (asINT32 *)pGeneric->GetArgAddress( 5 );
	cvarHandle_t *cvarHandle = (cvarHandle_t *)pGeneric->GetArgAddress( 6 );

	Cvar_Register( &vmCvar, name->c_str(), value->c_str(), flags, CVAR_PRIVATE );

	*value = (const char *)vmCvar.s;
	*intValue = vmCvar.i;
	*floatValue = vmCvar.f;
	*modificationCount = vmCvar.modificationCount;
	*cvarHandle = vmCvar.handle;
}

DEFINE_CALLBACK( CvarUpdateGeneric ) {
	vmCvar_t vmCvar;

	string_t *value = (string_t *)pGeneric->GetArgObject( 0 );
	asINT32 *intValue = (asINT32 *)pGeneric->GetArgAddress( 1 );
	float *floatValue = (float *)pGeneric->GetArgAddress( 2 );
	asINT32 *modificationCount = (asINT32 *)pGeneric->GetArgAddress( 3 );
	const cvarHandle_t cvarHandle = pGeneric->GetArgWord( 4 );

	memset( &vmCvar, 0, sizeof( vmCvar ) );
	N_strncpyz( vmCvar.s, value->c_str(), sizeof(vmCvar.s) );
	vmCvar.i = *intValue;
	vmCvar.f = *floatValue;
	vmCvar.modificationCount = *modificationCount;
	vmCvar.handle = cvarHandle;
	Cvar_Update( &vmCvar, CVAR_PRIVATE );

	*value = (const char *)vmCvar.s;
	*intValue = vmCvar.i;
	*floatValue = vmCvar.f;
	*modificationCount = vmCvar.modificationCount;
}

static void CvarSetValue( asIScriptGeneric *pGeneric ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const string_t *value = (const string_t *)pGeneric->GetArgObject( 1 );
	Cvar_Set( name->c_str(), value->c_str() );
}

static void CvarVariableString( asIScriptGeneric *pGeneric ) {
	::new ( pGeneric->GetAddressOfReturnLocation() ) string_t( Cvar_VariableString(
		( (const string_t *)pGeneric->GetArgObject( 0 ) )->c_str() ) );
}

static void CvarVariableInt( asIScriptGeneric *pGeneric ) {
	pGeneric->SetReturnDWord( Cvar_VariableInteger( ( (const string_t *)pGeneric->GetArgObject( 0 ) )->c_str() ) );
}

static void CvarVariableFloat( asIScriptGeneric *pGeneric ) {
	pGeneric->SetReturnFloat( Cvar_VariableFloat( ( (const string_t *)pGeneric->GetArgObject( 0 ) )->c_str() ) );
}

static nhandle_t SndRegisterSfx( const string_t *npath ) {
	return Snd_RegisterSfx( npath->c_str() );
}

static nhandle_t SndRegisterTrack( const string_t *npath ) {
	return Snd_RegisterTrack( npath->c_str() );
}

DEFINE_CALLBACK( BeginSaveSection ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	g_pArchiveHandler->BeginSaveSection( g_pModuleLib->GetCurrentHandle()->GetName().c_str(), name->c_str() );
}

DEFINE_CALLBACK( EndSaveSection ) {
	g_pArchiveHandler->EndSaveSection();
}

DEFINE_CALLBACK( SaveInt8 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const int8_t arg = *(const int8_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveChar( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt16 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const int16_t arg = *(const int16_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt32 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const int32_t arg = *(const int32_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt64 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const int64_t arg = *(const int64_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveLong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt8 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const uint8_t arg = *(const uint8_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveByte( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt16 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const uint16_t arg = *(const uint16_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveUShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt32 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const uint32_t arg = *(const uint32_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveUInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt64 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const uint64_t arg = *(const uint64_t *)pGeneric->GetAddressOfArg( 1 );
	g_pArchiveHandler->SaveULong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveFloat ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const float arg = pGeneric->GetArgFloat( 1 );
	g_pArchiveHandler->SaveFloat( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveString ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const string_t *arg = (const string_t *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->SaveString( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveArray ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const CScriptArray *arg = (const CScriptArray *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->SaveArray( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveVec2 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const vec2 v = *(const vec2 *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->SaveVec2( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( SaveVec3 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const vec3 v = *(const vec3 *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->SaveVec3( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( SaveVec4 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	const glm::vec4 v = *(const glm::vec4 *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->SaveVec4( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( FindSaveSection ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(int32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->GetSection( name->c_str() );
}

DEFINE_CALLBACK( LoadInt8 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(int8_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadChar( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadInt16 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(int16_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadShort( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadInt32 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(int32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadInt( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadInt64 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(int64_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadLong( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadUInt8 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(uint8_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadByte( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadUInt16 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(uint16_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadUShort( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadUInt32 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(uint32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadUInt( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadUInt64 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(uint64_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadULong( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadFloat ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	*(float *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadFloat( name->c_str(), (int32_t)pGeneric->GetArgDWord( 1 ) );
}

DEFINE_CALLBACK( LoadString ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	string_t *arg = (string_t *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->LoadString( name->c_str(), arg, (int32_t)pGeneric->GetArgDWord( 2 ) );
}

DEFINE_CALLBACK( LoadVec2 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	vec2 *arg = (vec2 *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->LoadVec2( name->c_str(), glm::value_ptr( *arg ), (int32_t)pGeneric->GetArgDWord( 2 ) );
}

DEFINE_CALLBACK( LoadVec3 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	vec3 *arg = (vec3 *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->LoadVec3( name->c_str(), glm::value_ptr( *arg ), (int32_t)pGeneric->GetArgDWord( 2 ) );
}

DEFINE_CALLBACK( LoadVec4 ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	glm::vec4 *arg = (glm::vec4 *)pGeneric->GetArgObject( 1 );
	g_pArchiveHandler->LoadVec4( name->c_str(), glm::value_ptr( *arg ), (int32_t)pGeneric->GetArgDWord( 2 ) );
}


DEFINE_CALLBACK( AddVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec4( *(const vec4 *)pGeneric->GetObjectData() + *(const vec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec4( *(const vec4 *)pGeneric->GetObjectData() - *(const vec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec4( *(const vec4 *)pGeneric->GetObjectData() * *(const vec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec4( *(const vec4 *)pGeneric->GetObjectData() / *(const vec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexVec4Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(vec4 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsVec4Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const vec4 *)pGeneric->GetObjectData() == *(const vec4 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpVec4Generic ) {
	const vec4& b = *(const vec4 *)pGeneric->GetArgObject( 0 );
	const vec4& a = *(const vec4 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z && b.w < a.w ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}


DEFINE_CALLBACK( AddVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec3( *(const vec3 *)pGeneric->GetObjectData() + *(const vec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec3( *(const vec3 *)pGeneric->GetObjectData() - *(const vec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec3( *(const vec3 *)pGeneric->GetObjectData() * *(const vec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec3( *(const vec3 *)pGeneric->GetObjectData() / *(const vec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexVec3Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(vec3 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsVec3Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const vec3 *)pGeneric->GetObjectData() == *(const vec3 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpVec3Generic ) {
	const vec3& b = *(const vec3 *)pGeneric->GetArgObject( 0 );
	const vec3& a = *(const vec3 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}

DEFINE_CALLBACK( AddVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec2( *(const vec2 *)pGeneric->GetObjectData() + *(const vec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec2( *(const vec2 *)pGeneric->GetObjectData() - *(const vec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec2( *(const vec2 *)pGeneric->GetObjectData() * *(const vec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) vec2( *(const vec2 *)pGeneric->GetObjectData() / *(const vec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexVec2Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(vec2 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsVec2Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const vec2 *)pGeneric->GetObjectData() == *(const vec2 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpVec2Generic ) {
	const vec2& b = *(const vec2 *)pGeneric->GetArgObject( 0 );
	const vec2& a = *(const vec2 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}


DEFINE_CALLBACK( AddIVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec4( *(const ivec4 *)pGeneric->GetObjectData() + *(const ivec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubIVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec4( *(const ivec4 *)pGeneric->GetObjectData() - *(const ivec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulIVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec4( *(const ivec4 *)pGeneric->GetObjectData() * *(const ivec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivIVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec4( *(const ivec4 *)pGeneric->GetObjectData() / *(const ivec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexIVec4Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(vec4 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsIVec4Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const ivec4 *)pGeneric->GetObjectData() == *(const ivec4 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpIVec4Generic ) {
	const ivec4& b = *(const ivec4 *)pGeneric->GetArgObject( 0 );
	const ivec4& a = *(const ivec4 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z && b.w < a.w ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}


DEFINE_CALLBACK( AddIVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec3( *(const ivec3 *)pGeneric->GetObjectData() + *(const ivec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubIVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec3( *(const ivec3 *)pGeneric->GetObjectData() - *(const ivec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulIVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec3( *(const ivec3 *)pGeneric->GetObjectData() * *(const ivec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivIVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec3( *(const ivec3 *)pGeneric->GetObjectData() / *(const ivec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexIVec3Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(ivec3 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsIVec3Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const ivec3 *)pGeneric->GetObjectData() == *(const ivec3 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpIVec3Generic ) {
	const ivec3& b = *(const ivec3 *)pGeneric->GetArgObject( 0 );
	const ivec3& a = *(const ivec3 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}

DEFINE_CALLBACK( AddIVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec2( *(const ivec2 *)pGeneric->GetObjectData() + *(const ivec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubIVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec2( *(const ivec2 *)pGeneric->GetObjectData() - *(const ivec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulIVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec2 ( *(const ivec2 *)pGeneric->GetObjectData() * *(const ivec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivIVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) ivec2( *(const ivec2 *)pGeneric->GetObjectData() / *(const ivec2 *)pGeneric->GetArgObject( 0 ) ) ;
}

DEFINE_CALLBACK( IndexIVec2Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(ivec2 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsIVec2Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const ivec2 *)pGeneric->GetObjectData() == *(const ivec2 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpIVec2Generic ) {
	const ivec2& b = *(const ivec2 *)pGeneric->GetArgObject( 0 );
	const ivec2& a = *(const ivec2 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}

DEFINE_CALLBACK( AddUVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec4( *(const uvec4 *)pGeneric->GetObjectData() + *(const uvec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubUVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec4( *(const uvec4 *)pGeneric->GetObjectData() - *(const uvec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulUVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec4( *(const uvec4 *)pGeneric->GetObjectData() * *(const uvec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivUVec4Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec4( *(const uvec4 *)pGeneric->GetObjectData() / *(const uvec4 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexUVec4Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(vec4 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsUVec4Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const uvec4 *)pGeneric->GetObjectData() == *(const uvec4 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpUVec4Generic ) {
	const uvec4& b = *(const uvec4 *)pGeneric->GetArgObject( 0 );
	const uvec4& a = *(const uvec4 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z && b.w < a.w ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}


DEFINE_CALLBACK( AddUVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec3( *(const uvec3 *)pGeneric->GetObjectData() + *(const uvec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubUVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec3( *(const uvec3 *)pGeneric->GetObjectData() - *(const uvec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulUVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec3( *(const uvec3 *)pGeneric->GetObjectData() * *(const uvec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivUVec3Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec3( *(const uvec3 *)pGeneric->GetObjectData() / *(const uvec3 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexUVec3Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(uvec3 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsUVec3Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const uvec3 *)pGeneric->GetObjectData() == *(const uvec3 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpUVec3Generic ) {
	const uvec3& b = *(const uvec3 *)pGeneric->GetArgObject( 0 );
	const uvec3& a = *(const uvec3 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}

DEFINE_CALLBACK( AddUVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec2( *(const uvec2 *)pGeneric->GetObjectData() + *(const uvec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( SubUVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec2( *(const uvec2 *)pGeneric->GetObjectData() - *(const uvec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( MulUVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec2( *(const uvec2 *)pGeneric->GetObjectData() * *(const uvec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( DivUVec2Generic ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) uvec2( *(const uvec2 *)pGeneric->GetObjectData() / *(const uvec2 *)pGeneric->GetArgObject( 0 ) );
}

DEFINE_CALLBACK( IndexUVec2Generic ) {
	pGeneric->SetReturnAddress( eastl::addressof( ( *(uvec2 *)pGeneric->GetObjectData() )[ pGeneric->GetArgDWord( 0 ) ] ) );
}

DEFINE_CALLBACK( EqualsUVec2Generic ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = *(const uvec2 *)pGeneric->GetObjectData() == *(const uvec2 *)pGeneric->GetArgObject( 0 );
}

DEFINE_CALLBACK( CmpUVec2Generic ) {
	const uvec2& b = *(const uvec2 *)pGeneric->GetArgObject( 0 );
	const uvec2& a = *(const uvec2 *)pGeneric->GetObjectData();

	int cmp = 0;
	if ( a.x < b.x && a.y < b.y ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y ) {
		cmp = 1;
	}

	pGeneric->SetReturnDWord( (asDWORD)cmp );
}

DEFINE_CALLBACK( OpAddTimer ) {
	const CTimer& a = *(const CTimer *)pGeneric->GetObjectData();
	const CTimer& b = *(const CTimer *)pGeneric->GetArgObject( 0 );

	new ( pGeneric->GetAddressOfReturnLocation() ) CTimer( a + b );
}

DEFINE_CALLBACK( OpSubTimer ) {
	const CTimer& a = *(const CTimer *)pGeneric->GetObjectData();
	const CTimer& b = *(const CTimer *)pGeneric->GetArgObject( 0 );

	new ( pGeneric->GetAddressOfReturnLocation() ) CTimer( a - b );
}

static void ConsolePrint( const string_t *msg ) {
	Con_Printf( "%s", msg->c_str() );
}

static void ConsoleWarning( const string_t *msg ) {
	Con_Printf( COLOR_YELLOW "WARNING: %s", msg->c_str() );
}

static void GameError( const string_t *msg ) {
	throw ModuleException( msg );
}

static int32_t StringBufferToInt( const CScriptArray *pBuffer ) {
	return atoi( (const char *)pBuffer->GetBuffer() );
}

static uint32_t StringBufferToUInt( const CScriptArray *pBuffer ) {
	return atoll( (const char *)pBuffer->GetBuffer() );
}

static float StringBufferToFloat( const CScriptArray *pBuffer ) {
	return N_atof( (const char *)pBuffer->GetBuffer() );
}

static int32_t StringToInt( const string_t *str ) {
	return atoi( str->c_str() );
}

static uint32_t StringToUInt( const string_t *str ) {
	return atoll( str->c_str() );
}

static float StringToFloat( const string_t *str ) {
	return N_atof( str->c_str() );
}

static bool BoundsIntersect( const CModuleBoundBox *a, const CModuleBoundBox *b ) {
	const bbox_t a2 = a->ToPOD();
	const bbox_t b2 = b->ToPOD();
	return BoundsIntersect( &a2, &b2 );
}

static bool BoundsIntersectPoint( const CModuleBoundBox *bbox, const vec3 *point ) {
	const bbox_t a = bbox->ToPOD();
	return BoundsIntersectPoint( &a, (const float *)point );
}

static bool BoundsIntersectSphere( const CModuleBoundBox *bbox, const vec3 *point, float radius ) {
	const bbox_t a = bbox->ToPOD();
	return BoundsIntersectSphere( &a, (const float *)point,radius  );
}

static int StrICmp( const string_t *str1, const string_t *str2 ) {
	return N_stricmp( str1->c_str(), str2->c_str() );
}

static int StrCmp( const string_t *str1, const string_t *str2 ) {
	return strcmp( str1->c_str(), str2->c_str() );
}

static int StrICmpn( const string_t *str1, const string_t *str2, asDWORD count ) {
	return N_stricmpn( str1->c_str(), str2->c_str(), count );
}

static int StrCmpn( const string_t *str1, const string_t *str2, asDWORD count ) {
	return strncmp( str1->c_str(), str2->c_str(), count );
}

static void KeyGetBinding( asIScriptGeneric *pGeneric ) {
	::new ( pGeneric->GetAddressOfReturnLocation() ) string_t( Key_GetBinding( pGeneric->GetArgDWord( 0 ) ) );
}

static void KeyIsDown( asIScriptGeneric *pGeneric ) {
	*(bool *)pGeneric->GetAddressOfReturnLocation() = Key_IsDown( pGeneric->GetArgDWord( 0 ) );
}

static void KeyAnyDown( asIScriptGeneric *pGeneric ) {
	pGeneric->SetReturnDWord( Key_AnyDown() );
}

static void KeyGetKey( asIScriptGeneric *pGeneric ) {
	pGeneric->SetReturnDWord( Key_GetKey( ( (const string_t *)pGeneric->GetArgObject( 0 ) )->c_str() ) );
}

static void SysMilliseconds( asIScriptGeneric *pGeneric ) {
	pGeneric->SetReturnQWord( Sys_Milliseconds() );
}

static void ScriptPolyQuad_Construct( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) CModulePolyQuad();
}

static void ScriptPolyQuad_CopyConstruct( asIScriptGeneric *pGeneric ) {
	CModulePolyQuad *obj = (CModulePolyQuad *)pGeneric->GetObjectData();
	const CModulePolyQuad *other = (CModulePolyQuad *)pGeneric->GetArgObject( 0 );

	new ( obj ) CModulePolyQuad( *other );
	pGeneric->SetReturnAddress( obj );
}

static void ScriptPolyQuad_Destruct( asIScriptGeneric *pGeneric ) {
	( (CModulePolyQuad *)pGeneric->GetObjectData() )->~CModulePolyQuad();
}

static void ScriptPolyQuad_OpAssign( asIScriptGeneric *pGeneric ) {
	CModulePolyQuad *obj = (CModulePolyQuad *)pGeneric->GetObjectData();
	const CModulePolyQuad *other = (CModulePolyQuad *)pGeneric->GetArgObject( 0 );

	new ( obj ) CModulePolyQuad( *other );
	pGeneric->SetReturnAddress( obj );
}

static void ScriptPolyQuad_OpIndex( asIScriptGeneric *pGeneric ) {
	CModulePolyQuad *obj = (CModulePolyQuad *)pGeneric->GetObjectData();
	const int index = pGeneric->GetArgDWord( 0 );

	pGeneric->SetReturnAddress( &obj->Get( index ) );
}

static void LoadWorld( asIScriptGeneric *pGeneric ) {
	const string_t *npath = (const string_t *)pGeneric->GetArgObject( 0 );
	re.LoadWorld( npath->c_str() );
}

static void ClearScene( asIScriptGeneric *pGeneric ) {
	re.ClearScene();
}

DEFINE_CALLBACK( RenderScene ) {
	renderSceneRef_t refdef;
	
	memset( &refdef, 0, sizeof( refdef ) );
	
	refdef.x = pGeneric->GetArgDWord( 0 );
	refdef.y = pGeneric->GetArgDWord( 1 );
	refdef.width = pGeneric->GetArgDWord( 2 );
	refdef.height = pGeneric->GetArgDWord( 3 );
	refdef.flags = pGeneric->GetArgDWord( 4 );
	refdef.time = pGeneric->GetArgDWord( 5 );

	re.RenderScene( &refdef );
}

static void AddEntityToScene( asIScriptGeneric *pGeneric ) {
	refEntity_t refEntity;

	nhandle_t sheetNum = (nhandle_t)pGeneric->GetArgDWord( 0 );
	nhandle_t spriteId = (nhandle_t)pGeneric->GetArgDWord( 1 );
	int renderfx = (int)pGeneric->GetArgDWord( 2 );
	const vec3& lightingOrigin = *(const vec3 *)pGeneric->GetArgObject( 3 );
	const vec3& origin = *(const vec3 *)pGeneric->GetArgObject( 4 );
	uint64_t frame = pGeneric->GetArgQWord( 5 );
	uint32_t flags = pGeneric->GetArgDWord( 6 );
	uint32_t color = pGeneric->GetArgDWord( 7 );
	const vec2& shaderTexCoord = *(const vec2 *)pGeneric->GetArgObject( 8 );
	float shaderTime = pGeneric->GetArgFloat( 9 );
	float radius = pGeneric->GetArgFloat( 10 );
	float rotation = pGeneric->GetArgFloat( 11 );
	const vec2& scale = *(const vec2 *)pGeneric->GetArgObject( 12 );

	memset( &refEntity, 0, sizeof( refEntity ) );
	VectorCopy( refEntity.origin, origin );
	VectorCopy( refEntity.lightingOrigin, lightingOrigin );
	VectorCopy2( refEntity.shaderTexCoord, shaderTexCoord );
	refEntity.sheetNum = sheetNum;
	refEntity.spriteId = spriteId;
	refEntity.renderfx = renderfx;
	refEntity.frame = frame;
	refEntity.flags = flags;
	refEntity.radius = radius;
	refEntity.rotation = rotation;
	VectorCopy2( refEntity.scale, scale );
	refEntity.shaderTime.f = shaderTime;

	re.AddEntityToScene( &refEntity );
}

static void AddDLightToScene( asIScriptGeneric *pGeneric ) {
	const vec3& origin = *(const vec3 *)pGeneric->GetArgObject( 0 );
	float range = pGeneric->GetArgFloat( 1 );
	float constant = pGeneric->GetArgFloat( 2 );
	float linear = pGeneric->GetArgFloat( 3 );
	float quadratic = pGeneric->GetArgFloat( 4 );
	float brightness = pGeneric->GetArgFloat( 5 );
	const vec3& color = *(const vec3 *)pGeneric->GetArgObject( 6 );

	re.AddDynamicLightToScene( (const vec_t *)glm::value_ptr( origin ), range, constant, linear, quadratic,
		brightness, (const vec_t *)glm::value_ptr( color ) );
}

static void AddLineToScene( asIScriptGeneric *pGeneric ) {
	nhandle_t hShader = (nhandle_t)pGeneric->GetArgDWord( 0 );
	const CModulePolyQuad *quad = (const CModulePolyQuad *)pGeneric->GetArgObject( 1 );

	polyVert_t *verts;
	const CModulePolyVert *vtx;
	int numVerts, i;

	numVerts = 2;
	vtx = &quad->Get( 0 );
	verts = (polyVert_t *)alloca( sizeof( *verts ) * numVerts );
	for ( i = 0; i < numVerts; i++ ) {
		VectorCopy( verts[i].xyz, vtx[i].m_Origin );
		VectorCopy( verts[i].worldPos, vtx[i].m_WorldPos );
		VectorCopy2( verts[i].uv, vtx[i].m_TexCoords );
		verts[i].modulate = vtx[i].m_Color;
	}

	re.AddPolyToScene( hShader, verts, numVerts );
}

static void AddQuadToScene( asIScriptGeneric *pGeneric ) {
	nhandle_t hShader = (nhandle_t)pGeneric->GetArgDWord( 0 );
	const CModulePolyQuad *quad = (const CModulePolyQuad *)pGeneric->GetArgObject( 1 );

	polyVert_t *verts;
	const CModulePolyVert *vtx;
	int numVerts, i;

	numVerts = quad->Count();
	vtx = &quad->Get( 0 );
	verts = (polyVert_t *)alloca( sizeof( *verts ) * numVerts );
	for ( i = 0; i < numVerts; i++ ) {
		VectorCopy( verts[i].xyz, vtx[i].m_Origin );
		VectorCopy( verts[i].worldPos, vtx[i].m_WorldPos );
		VectorCopy2( verts[i].uv, vtx[i].m_TexCoords );
		verts[i].modulate = vtx[i].m_Color;
	}

	re.AddPolyToScene( hShader, verts, numVerts );
}

static void AddPolyToScene( nhandle_t hShader, const CScriptArray *pPolyList ) {
	polyVert_t *verts;
	const CModulePolyVert *vtx;
	int numVerts, i;

	numVerts = pPolyList->GetSize();
	vtx = (const CModulePolyVert *)pPolyList->GetBuffer();
	verts = (polyVert_t *)alloca( sizeof( *verts ) * numVerts );
	for ( i = 0; i < numVerts; i++ ) {
		VectorCopy( verts[i].xyz, vtx[i].m_Origin );
		VectorCopy( verts[i].worldPos, vtx[i].m_WorldPos );
		VectorCopy2( verts[i].uv, vtx[i].m_TexCoords );
		verts[i].modulate = vtx[i].m_Color;
	}

	re.AddPolyToScene( hShader, verts, numVerts );
}

static void AddSpriteToScene( asIScriptGeneric *pGeneric ) {
	const vec3& origin = *(const vec3 *)pGeneric->GetArgObject( 0 );
	const nhandle_t hShader = (nhandle_t)pGeneric->GetArgDWord( 1 );

	re.AddSpriteToScene( (vec_t *)glm::value_ptr( origin ), hShader );
}

static void RegisterShader( asIScriptGeneric *pGeneric ) {
	const string_t *npath = (const string_t *)pGeneric->GetArgObject( 0 );
	*(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = re.RegisterShader( npath->c_str() );
}

DEFINE_CALLBACK( RegisterSpriteSheet ) {
	const string_t *npath = (const string_t *)pGeneric->GetArgObject( 0 );
	pGeneric->SetReturnDWord( re.RegisterSpriteSheet( npath->c_str(), pGeneric->GetArgDWord( 1 ), pGeneric->GetArgDWord( 2 ),
		pGeneric->GetArgDWord( 3 ), pGeneric->GetArgDWord( 4 ) ) );
}

DEFINE_CALLBACK( DrawImage ) {
	re.DrawImage( pGeneric->GetArgFloat( 0 ), pGeneric->GetArgFloat( 1 ), pGeneric->GetArgFloat( 2 ), pGeneric->GetArgFloat( 3 ),
		pGeneric->GetArgFloat( 4 ), pGeneric->GetArgFloat( 5 ), pGeneric->GetArgFloat( 6 ), pGeneric->GetArgFloat( 7 ),
		(nhandle_t)pGeneric->GetArgDWord( 8 ) );
}

DEFINE_CALLBACK( SetColor ) {
	// glm::vec4 has the same exact data layout as a vec4_t (i checked), we'll be fine
	re.SetColor( (const float *)pGeneric->GetArgObject( 0 ) );
}

static nhandle_t RegisterSprite( nhandle_t hSpriteSheet, uint32_t nIndex ) {
	return re.RegisterSprite( hSpriteSheet, nIndex );
}

static void PushListener( asIScriptGeneric *pGeneric )
{
	pGeneric->SetReturnDWord( (asDWORD)s_SoundWorld->PushListener( pGeneric->GetArgDWord( 0 ) ) );
}

static void SetEmitterPosition( asIScriptGeneric *pGeneric )
{
	nhandle_t hEmitter = (nhandle_t)pGeneric->GetArgDWord( 0 );
	const vec3& position = *(const vec3 *)pGeneric->GetArgObject( 1 );
	const float forward = pGeneric->GetArgFloat( 2 );
	const float up = pGeneric->GetArgFloat( 3 );
	const float speed = pGeneric->GetArgFloat( 4 );

	s_SoundWorld->SetEmitterPosition( hEmitter, glm::value_ptr( position ), forward, up, speed );
}

static void PlayEmitterSound( asIScriptGeneric *pGeneric )
{
	nhandle_t hEmitter = (nhandle_t )pGeneric->GetArgDWord( 0 );
	float nVolume = pGeneric->GetArgFloat( 1 );
	uint32_t nListenerMask = pGeneric->GetArgDWord( 2 );
	sfxHandle_t hSfx = (sfxHandle_t)pGeneric->GetArgDWord( 3 );

	s_SoundWorld->PlayEmitterSound( hEmitter, nVolume, nListenerMask, hSfx );
}

static void RegisterEmitter( asIScriptGeneric *pGeneric )
{
	pGeneric->SetReturnDWord( (asDWORD)s_SoundWorld->RegisterEmitter( pGeneric->GetArgDWord( 0 ) ) );
}

static void RemoveEmitter( asIScriptGeneric *pGeneric )
{
	s_SoundWorld->RemoveEmitter( (nhandle_t)pGeneric->GetArgDWord( 0 ) );
}

static void CastRay( asIScriptGeneric *pGeneric ) {
	ray_t ray;

	const vec3& start = *(const vec3 *)pGeneric->GetArgObject( 0 );
	vec3& end = *(vec3 *)pGeneric->GetArgObject( 1 );
	vec3& origin = *(vec3 *)pGeneric->GetArgObject( 2 );
	uint32_t *entityNumber = (uint32_t *)pGeneric->GetArgAddress( 3 );
	const float length = pGeneric->GetArgFloat( 4 );
	const float angle = pGeneric->GetArgFloat( 5 );
	const uint32_t flags = pGeneric->GetArgDWord( 6 );

	memset( &ray, 0, sizeof( ray ) );
	VectorCopy( ray.start, start );
	ray.angle = angle;
	ray.flags = flags;
	ray.entityNumber = ENTITYNUM_INVALID;
	ray.length = length;

	g_world->CastRay( &ray );

	*entityNumber = ray.entityNumber;
	VectorCopy( origin, ray.origin );
	VectorCopy( end, ray.end );
}

static void TransformCameraFromWorld( asIScriptGeneric *pGeneric ) {
	const vec3& origin = *(const vec3 *)pGeneric->GetArgObject( 0 );

	gi.cameraPos = gi.viewProjectionMatrix * glm::vec4( origin.x, gi.mapCache.info.height - origin.y, gi.cameraZoom, 1.0f );
}

static void CheckWallHit( asIScriptGeneric *pGeneric ) {
	const vec3& origin = *(const vec3 *)pGeneric->GetArgObject( 0 );
	const dirtype_t dir = (const dirtype_t)pGeneric->GetArgDWord( 1 );

	*(bool *)pGeneric->GetAddressOfReturnLocation() = g_world->CheckWallHit( (const float *)glm::value_ptr( origin ), dir );
}

static void LoadMap( asIScriptGeneric *pGeneric ) {
	const string_t *npath = (const string_t *)pGeneric->GetArgObject( 0 );
	*(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = G_LoadMap( npath->c_str() );
}

DEFINE_CALLBACK( SetActiveMap ) {
	nhandle_t hMap = (nhandle_t)pGeneric->GetArgDWord( 0 );
	uint32_t *nCheckpoints = (uint32_t *)pGeneric->GetArgAddress( 1 );
	uint32_t *nSpawns = (uint32_t *)pGeneric->GetArgAddress( 2 );
	uint32_t *nTiles = (uint32_t *)pGeneric->GetArgAddress( 3 );
	int32_t *pWidth = (int32_t *)pGeneric->GetArgAddress( 4 );
	int32_t *pHeight = (int32_t *)pGeneric->GetArgAddress( 5 );

	G_SetActiveMap( hMap, nCheckpoints, nSpawns, nTiles, pWidth, pHeight );
}

static void GetSkinData( asIScriptGeneric *pGeneric ) {
	const string_t *skinName = (const string_t *)pGeneric->GetArgObject( 0 );

	string_t *description = (string_t *)pGeneric->GetArgObject( 1 );
	string_t *displayText = (string_t *)pGeneric->GetArgObject( 2 );

	description->resize( MAX_DESCRIPTION_LENGTH );
	displayText->resize( MAX_DISPLAY_NAME_LENGTH );

	uvec2 *torsoSheetSize = (uvec2 *)pGeneric->GetArgObject( 3 );
	uvec2 *torsoSpriteSize = (uvec2 *)pGeneric->GetArgObject( 4 );

	uvec2 *armsSheetSize = (uvec2 *)pGeneric->GetArgObject( 5 );
	uvec2 *armsSpriteSize = (uvec2 *)pGeneric->GetArgObject( 6 );

	uvec2 *legsSheetSize = (uvec2 *)pGeneric->GetArgObject( 7 );
	uvec2 *legsSpriteSize = (uvec2 *)pGeneric->GetArgObject( 8 );

	G_GetSkinData( skinName->c_str(), description->data(), displayText->data(), (uvec_t *)torsoSheetSize, (uvec_t *)torsoSpriteSize,
		(uvec_t *)armsSheetSize, (uvec_t *)armsSpriteSize, (uvec_t *)legsSheetSize, (uvec_t *)legsSpriteSize );
}

static void GetCheckpointData( asIScriptGeneric *pGeneric ) {
	uvec3 *xyz = (uvec3 *)pGeneric->GetArgObject( 0 );
	uvec2 *areaLock = (uvec2 *)pGeneric->GetArgObject( 1 );
	uint32_t index = pGeneric->GetArgDWord( 1 );

	G_GetCheckpointData( (uvec_t *)xyz, (uvec_t *)areaLock, index );
}

static void GetSpawnData( asIScriptGeneric *pGeneric ) {
	uvec3 *xyz = (uvec3 *)pGeneric->GetArgObject( 0 );
	uint32_t *type = (uint32_t *)pGeneric->GetArgAddress( 1 );
	uint32_t *id = (uint32_t *)pGeneric->GetArgAddress( 2 );
	uint32_t spawnIndex = pGeneric->GetArgDWord( 3 );
	uint32_t *pCheckpointIndex = (uint32_t *)pGeneric->GetArgAddress( 4 );

	G_GetSpawnData( (uvec_t *)xyz, type, id, spawnIndex, pCheckpointIndex );
}

static void GetTileData( asIScriptGeneric *pGeneric ) {
	CScriptArray *tiles = (CScriptArray *)pGeneric->GetArgObject( 0 );

	tiles->Resize( gi.mapCache.info.numLevels );
	for ( uint32_t i = 0; i < gi.mapCache.info.numLevels; i++ ) {
		( (CScriptArray *)tiles->At( i ) )->Resize( gi.mapCache.info.numTiles );
		G_GetTileData( (uint64_t *)( (CScriptArray *)tiles->At( i ) )->GetBuffer(), i );
	}
}

static void GetGPUConfig( asIScriptGeneric *pGeneric ) {
	CModuleGPUConfig *config = (CModuleGPUConfig *)pGeneric->GetArgObject( 0 );

	config->gpuConfig = gi.gpuConfig;
	config->shaderVersionString = config->gpuConfig.shader_version_str;
	config->versionString = config->gpuConfig.version_string;
	config->rendererString = config->gpuConfig.renderer_string;
	config->extensionsString = config->gpuConfig.extensions_string;
}

static void SetCameraPos( asIScriptGeneric *pGeneric ) {
	const vec2 *pos = (const vec2 *)pGeneric->GetArgObject( 0 );
	const float zoom = pGeneric->GetArgFloat( 1 );
	const float rotation = pGeneric->GetArgFloat( 2 );

	G_SetCameraData( (const vec_t *)pos, zoom, rotation );
}

static nhandle_t ModuleOpenFileRead( const string_t *fileName ) {
	return FS_VM_FOpenRead( fileName->c_str(), H_SGAME );
}

static nhandle_t ModuleOpenFileWrite( const string_t *fileName ) {
	return FS_VM_FOpenWrite( fileName->c_str(), H_SGAME );
}

static nhandle_t ModuleOpenFileAppend( const string_t *fileName ) {
	return FS_VM_FOpenAppend( fileName->c_str(), H_SGAME );
}

static nhandle_t ModuleOpenFileRW( const string_t *fileName ) {
	return FS_VM_FOpenRW( fileName->c_str(), H_SGAME );
}

static uint64_t ModuleOpenFile( const string_t *fileName, fileHandle_t *hFile, fileMode_t mode ) {
	return FS_VM_FOpenFile( fileName->c_str(), hFile, mode, H_SGAME );
}

static void CloseFile( fileHandle_t arg ) {
	FS_VM_FClose( arg, H_SGAME );
}

static uint64_t GetFileLength( fileHandle_t arg ) {
	return FS_VM_FileLength( arg, H_SGAME );
}

static fileOffset_t GetFilePosition( fileHandle_t arg ) {
	return FS_VM_FileTell( arg, H_SGAME );
}

static uint64_t FileSetPosition( fileHandle_t arg, fileOffset_t offset, uint32_t whence ) {
	return FS_VM_FileSeek( arg, offset, whence, H_SGAME );
}

static uint64_t WriteInt8( fileHandle_t arg, int8_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteInt16( fileHandle_t arg, int16_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteInt32( fileHandle_t arg, int32_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteInt64( fileHandle_t arg, int64_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteUInt8( fileHandle_t arg, uint8_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteUInt16( fileHandle_t arg, uint16_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteUInt32( fileHandle_t arg, uint32_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteUInt64( fileHandle_t arg, uint64_t data ) {
	return FS_VM_Write( &data, sizeof( data ), arg, H_SGAME );
}

static uint64_t WriteString( fileHandle_t arg, string_t *data ) {
	uint64_t length;

	length = data->size();
	if ( !FS_VM_Write( &length, sizeof( length ), arg, H_SGAME ) ) {
		return 0;
	}

	return FS_VM_Write( data->c_str(), length, arg, H_SGAME );
}

static uint64_t ReadInt8( fileHandle_t arg, int8_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadInt16( fileHandle_t arg, int16_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadInt32( fileHandle_t arg, int32_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadInt64( fileHandle_t arg, int64_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadUInt8( fileHandle_t arg, uint8_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadUInt16( fileHandle_t arg, uint16_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadUInt32( fileHandle_t arg, uint32_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadUInt64( fileHandle_t arg, uint64_t *data ) {
	return FS_VM_Read( data, sizeof( *data ), arg, H_SGAME );
}

static uint64_t ReadString( fileHandle_t arg, string_t *data ) {
	uint64_t length;

	if ( !FS_VM_Read( &length, sizeof( length ), arg, H_SGAME ) ) {
		return 0;
	}

	data->resize( length );
	return FS_VM_Read( data->data(), length, arg, H_SGAME );
}

static void LoadFileString( asIScriptGeneric *pGeneric ) {
	void *v;
	uint64_t length;
	const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
	string_t *buffer = (string_t *)pGeneric->GetArgObject( 1 );
	
	length = FS_LoadFile( fileName->c_str(), &v );
	if ( !length || !v ) {
		Con_Printf( COLOR_RED "ERROR: failed to load file '%s' at vm request\n", fileName->c_str() );
		pGeneric->SetReturnQWord( 0 );
		return;
	}

	buffer->resize( length );
	memcpy( buffer->data(), v, length );

	FS_FreeFile( v );

	pGeneric->SetReturnQWord( length );
}

static void LoadFile( asIScriptGeneric *pGeneric ) {
	void *v;
	uint64_t length;
	const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
	CScriptArray *buffer = (CScriptArray *)pGeneric->GetArgObject( 1 );

	length = FS_LoadFile( fileName->c_str(), &v );
	if ( !length || !v ) {
		Con_Printf( COLOR_RED "ERROR: failed to load file '%s' at vm request\n", fileName->c_str() );
		pGeneric->SetReturnQWord( 0 );
		return;
	}

	buffer->Resize( length );
	memcpy( buffer->GetBuffer(), v, length );

	FS_FreeFile( v );

	pGeneric->SetReturnQWord( length );
}

static void ListFiles( asIScriptGeneric *pGeneric )
{
	uint64_t numFiles, i;

	const string_t *path = (const string_t *)pGeneric->GetArgObject( 0 );
	const string_t *ext = (const string_t *)pGeneric->GetArgObject( 1 );

	CScriptArray *returnList = CScriptArray::Create( g_pModuleLib->GetScriptEngine()->GetTypeInfoByName( "array" ) );
	char **fileList = FS_ListFiles( path->c_str(), ext->c_str(), &numFiles );

	returnList->Resize( numFiles );
	for ( i = 0; i < numFiles; i++ ) {
		new ( returnList->At( i ) ) string_t( fileList[i] );
	}

	*(CScriptArray **)pGeneric->GetAddressOfReturnLocation() = returnList;
}

static void MakeDir( asIScriptGeneric *pGeneric ) {
}

static void RemoveDir( asIScriptGeneric *pGeneric ) {
}

static void RemoveFile( asIScriptGeneric *pGeneric ) {
	const string_t *npath = (const string_t *)pGeneric->GetArgObject( 0 );
	FS_Remove( npath->c_str() );
	FS_HomeRemove( npath->c_str() );
}

static void GetHomePath( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) string_t( FS_GetHomePath() );
}

static void GetBaseGameDir( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) string_t( FS_GetBaseGameDir() );
}

static void GetBasePath( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) string_t( FS_GetBasePath() );
}

static void GetGamePath( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) string_t( FS_GetGamePath() );
}

static bool FileExists( const string_t *npath ) {
	return FS_FileExists( npath->c_str() );
}

static void GetModuleDependencies( CScriptArray *depList, const string_t *modName ) {
	const CModuleInfo *info = g_pModuleLib->GetModule( modName->c_str() );

	if ( !info ) {
		Con_Printf( COLOR_YELLOW "GetModuleDependencies: no such module '%s'!\n", modName->c_str() );
		return;
	}

	depList->Resize( info->m_nDependencies );
	for ( uint64_t i = 0; info->m_nDependencies; i++ ) {
		*(string_t *)depList->At( i ) = info->m_pDependencies[i].c_str();
	}
}

static void AddScriptSectionToModule( asIScriptGeneric *pGeneric )
{
	const string_t& moduleName = *(const string_t *)pGeneric->GetArgObject( 0 );
	const string_t& fileName = *(const string_t *)pGeneric->GetArgObject( 1 );
	CModuleInfo *pHandle;

	pHandle = g_pModuleLib->GetModule( moduleName.c_str() );
	if ( !pHandle ) {
		N_Error( ERR_DROP, "Util::AddScriptSectionToModule: invalid module '%s'", moduleName.c_str() );
	}

	Con_Printf( "Loading dynamic module section '%s'...\n", fileName.c_str() );

	if ( !pHandle->m_pHandle->LoadSourceFile( fileName ) ) {
		pGeneric->SetReturnDWord( false );
		return;
	}

	CheckASCall( g_pModuleLib->GetScriptModule()->Build() );

	pGeneric->SetReturnDWord( true );
}

static void LoadFunctionFromSection( asIScriptGeneric *pGeneric )
{
	char szName[MAX_STRING_CHARS];
	const string_t& moduleName = *(const string_t *)pGeneric->GetArgObject( 0 );
	const string_t& funcName = *(const string_t *)pGeneric->GetArgObject( 1 );
	const string_t& section = *(const string_t *)pGeneric->GetArgObject( 2 );
	asIScriptFunction **pFunction = (asIScriptFunction **)pGeneric->GetArgObject( 3 );
	CModuleInfo *pHandle;

	pHandle = g_pModuleLib->GetModule( moduleName.c_str() );
	if ( !pHandle ) {
		N_Error( ERR_DROP, "Util::LoadFunctionFromSection: invalid module '%s'", moduleName.c_str() );
	}

	Com_snprintf( szName, sizeof( szName ) - 1, "TheNomad::SGame::%s::%s", section.c_str(), funcName.c_str() );

	Con_Printf( "Loading dynamic script function proc '%s' from '%s'...\n", szName, section.c_str() );

	*pFunction = g_pModuleLib->GetScriptModule()->GetFunctionByName( szName );
	if ( !*pFunction ) {
		Con_Printf( COLOR_YELLOW "Failed to load dynamic function proc '%s'.\n", szName );
	}
}

static void GetModuleList( CScriptArray *modList ) {
	const CModuleInfo *loadList = g_pModuleLib->GetLoadList();

	modList->Resize( g_pModuleLib->GetModCount() );
	for ( uint64_t i = 0; i < g_pModuleLib->GetModCount(); i++ ) {
		string_t *str = (string_t *)modList->At( i );
		*str = loadList[i].m_szName;
	}
}

static void LoadModuleFunction( asIScriptGeneric *pGeneric )
{
	const string_t& moduleName = *(const string_t *)pGeneric->GetArgObject( 0 );
	const string_t& funcName = *(const string_t *)pGeneric->GetArgObject( 1 );
	asIScriptFunction *func = (asIScriptFunction *)pGeneric->GetAddressOfArg( 2 );

	g_pModuleLib->GetCurrentHandle()->LoadFunction( moduleName, funcName, &func );
}

static bool IsModuleActive( const string_t *modName ) {
	return ModsMenu_IsModuleActive( modName->c_str() );
}

static void ModuleAssertion( bool bCheck ) {
	asIScriptContext *pContext = g_pModuleLib->GetScriptContext();
	asIScriptFunction *pFunc = pContext->GetSystemFunction();

	if ( bCheck ) {
		return;
	}

	N_Error( ERR_FATAL,
		"Module Assertion Exception thrown ->\n"
		" Line: %i\n"
		" Section: %s\n"
		" Module: %s\n"
	, pContext->GetLineNumber(), pFunc->GetScriptSectionName(), pFunc->GetModuleName() );
}

static void CmdArgvFixed( asIScriptGeneric *pGeneric ) {
	CScriptArray *pBuffer = (CScriptArray *)pGeneric->GetArgObject( 0 );
	uint32_t nIndex = pGeneric->GetArgDWord( 1 );

	Cmd_ArgvBuffer( nIndex, (char *)pBuffer->GetBuffer(), pBuffer->GetSize() );
}

static void CmdArgc( asIScriptGeneric *pGeneric ) {
	pGeneric->SetReturnDWord( Cmd_Argc() );
}

static void CmdArgv( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) string_t( Cmd_Argv( pGeneric->GetArgDWord( 0 ) ) );
}

static void CmdArgs( asIScriptGeneric *pGeneric ) {
	new ( pGeneric->GetAddressOfReturnLocation() ) string_t( Cmd_ArgsFrom( pGeneric->GetArgDWord( 0 ) ) );
}

static void CmdAddCommand( asIScriptGeneric *pGeneric ) {
	const string_t *cmd = (const string_t *)pGeneric->GetArgObject( 0 );
	Cmd_AddCommand( cmd->c_str(), NULL );
}

static void CmdRemoveCommand( asIScriptGeneric *pGeneric ) {
	const string_t *cmd = (const string_t *)pGeneric->GetArgObject( 0 );
	Cmd_RemoveCommand( cmd->c_str() );
}

static void CmdExecuteCommand( asIScriptGeneric *pGeneric ) {
	const string_t *cmd = (const string_t *)pGeneric->GetArgObject( 0 );
	Cbuf_ExecuteText( EXEC_APPEND, cmd->c_str() );
}

static void GetJoystickAngle( asIScriptGeneric *pGeneric ) {
	int joystickIndex = pGeneric->GetArgDWord( 0 );
	float *angle = (float *)pGeneric->GetArgAddress( 1 );
	ivec2 *joystickPosition = (ivec2 *)pGeneric->GetArgObject( 2 );

	Com_JoystickGetAngle( joystickIndex, angle, (ivec_t *)glm::value_ptr( *joystickPosition ) );
}

static void GetMousePosition( asIScriptGeneric *pGeneric ) {
	int x, y;
	
	SDL_GetMouseState( &x, &y );

	new ( pGeneric->GetAddressOfReturnLocation() ) ivec2( x, y );
}

static void AllocateExternalScriptObject( asIScriptGeneric *pGeneric )
{
	asIScriptObject *pObject;
	asIScriptFunction *pFactory;
	asITypeInfo *pTypeInfo;
	char szFactoryName[ 1024 ];
	
	const string_t& nameSpace = *(const string_t *)pGeneric->GetArgObject( 0 );
	const string_t& name = *(const string_t *)pGeneric->GetArgObject( 1 );

	memset( szFactoryName, 0, sizeof( szFactoryName ) );
	snprintf( szFactoryName, sizeof( szFactoryName ) - 1, "%s::Script::%s@ %s()", nameSpace.c_str(), name.c_str(), name.c_str() );

	pTypeInfo = g_pModuleLib->GetScriptModule()->GetTypeInfoByDecl( va( "moblib::Script::%s", /*nameSpace.c_str(),*/ name.c_str() ) );
	if ( !pTypeInfo ) {
		Con_Printf( COLOR_RED "ERROR: invalid script class type \"moblib::Script::%s\"!\n", /*nameSpace.c_str(),*/ name.c_str() );
		return;
	}

	pFactory = pTypeInfo->GetFactoryByDecl( szFactoryName );
	if ( !pFactory ) {
		Con_Printf( COLOR_RED "ERROR: script class \"%s::%s\" has no default factory!\n", nameSpace.c_str(), name.c_str() );
		return;
	}

	g_pModuleLib->GetScriptContext()->PushState();
	g_pModuleLib->GetScriptContext()->Prepare( pFactory );
	g_pModuleLib->GetScriptContext()->Execute();

	pObject = *(asIScriptObject **)g_pModuleLib->GetScriptContext()->GetAddressOfReturnValue();
	pObject->AddRef();

	pGeneric->SetReturnObject( pObject );

	g_pModuleLib->GetScriptContext()->PopState();
}

static void GetString( asIScriptGeneric *pGeneric ) {
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	string_t *value = (string_t *)pGeneric->GetArgObject( 1 );

	const stringHash_t *hash = strManager->ValueForKey( name->c_str() );
	*value = hash->value;
}

static bool ImGui_BeginCombo( const string_t *label, const string_t *preview ) {
	return ImGui::BeginCombo( label->c_str(), preview->c_str() );
}

static bool ImGui_Begin( const string_t *label, bool *open, ImGuiWindowFlags flags ) {
	return ImGui::Begin( label->c_str(), open, flags );
}

static void ImGui_End( void ) {
	ImGui::End();
}

static void ImGui_SetWindowPos( const vec2 *pos ) {
	ImGui::SetWindowPos( ImVec2( pos->x, pos->y ) );
}

static void ImGui_SetWindowSize( const vec2 *size ) {
	ImGui::SetWindowSize( ImVec2( size->x, size->y ) );
}

static bool ImGui_ArrowButton( const string_t *label, ImGuiDir dir ) {
	return ImGui::ArrowButton( label->c_str(), dir );
}

static bool ImGui_Button( const string_t *label, const vec2 *size ) {
	return ImGui::Button( label->c_str(), ImVec2( size->x, size->y ) );
}

static void ImGui_PushStyleColor_U32( ImGuiCol idx, ImU32 col ) {
	ImGui::PushStyleColor( idx, col );
}

static void ImGui_PushStyleColor_V4( asIScriptGeneric *pGeneric )
{
	const ImGuiCol idx = (ImGuiCol)pGeneric->GetArgDWord( 0 );
	const vec4 *col = (const vec4 *)pGeneric->GetArgObject( 1 );

	ImGui::PushStyleColor( idx, ImVec4( col->r, col->g, col->b, col->a ) );
}

static bool ImGui_BeginTable( const string_t *label, int nColumns, ImGuiTableFlags flags ) {
	return ImGui::BeginTable( label->c_str(), nColumns, flags );
}

static bool ImGui_Selectable( const string_t *label, bool selected, int, const vec2 * ) {
	return ImGui::Selectable( label->c_str(), selected );
}

static bool ImGui_InputText( const string_t *label, string_t *str, ImGuiInputTextFlags flags ) {
	return ImGui::InputText( label->c_str(), str, flags );
}

static void ImGui_Image( asIScriptGeneric *pGeneric ) {
	nhandle_t hShader = pGeneric->GetArgDWord( 0 );
	const vec2& pos = *(const vec2 *)pGeneric->GetArgObject( 1 );
	const vec2& size = *(const vec2 *)pGeneric->GetArgObject( 2 );
	const vec2& uv0 = *(const vec2 *)pGeneric->GetArgObject( 3 );
	const vec2& uv1 = *(const vec2 *)pGeneric->GetArgObject( 4 );

	ImGui::SetCursorScreenPos( ImVec2( pos.x, pos.y ) );
	ImGui::Image( (ImTextureID)(uintptr_t)hShader, ImVec2( size.x, size.y ), ImVec2( uv0.x, uv0.y ), ImVec2( uv1.x, uv1.y ) );
}

static void ImGui_InputInt( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	int32_t v = pGeneric->GetArgDWord( 1 );
	ImGuiInputTextFlags flags = pGeneric->GetArgDWord( 2 );

	if ( ImGui::InputInt( label->c_str(), &v, 1, 100, flags ) ) {
		*(int32_t *)pGeneric->GetAddressOfReturnLocation() = v;
	} else {
		*(int32_t *)pGeneric->GetAddressOfReturnLocation() = pGeneric->GetArgDWord( 1 );
	}
}

static void ImGui_InputFloat( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	float v = pGeneric->GetArgFloat( 1 );
	ImGuiInputTextFlags flags = pGeneric->GetArgDWord( 2 );

	if ( ImGui::InputFloat( label->c_str(), &v, 0.0f, 0.0f, "%.3f", flags ) ) {
		*(float *)pGeneric->GetAddressOfReturnLocation() = v;
	} else {
		*(float *)pGeneric->GetAddressOfReturnLocation() = pGeneric->GetArgFloat( 1 );
	}
}

static void ImGui_Text( const string_t *text ) {
	ImGui::TextUnformatted( text->c_str() );
}

static bool ImGui_ColorEdit3( const string_t *label, vec3 *col, ImGuiColorEditFlags flags ) {
	return ImGui::ColorEdit3( label->c_str(), (float *)col, flags );
}

static bool ImGui_ColorEdit4( const string_t *label, vec4 *col, ImGuiColorEditFlags flags ) {
	return ImGui::ColorEdit4( label->c_str(), (float *)col, flags );
}

static float ImGui_GetFontScale( void ) {
	const ImFont *pFont = ImGui::GetFont();
	return pFont ? pFont->Scale : 1.0f;
}

static void FontCache_RegisterFont( asIScriptGeneric *pGeneric ) {
	const string_t *pName = (const string_t *)pGeneric->GetArgObject( 0 );
	const string_t *pVariant = (const string_t *)pGeneric->GetArgObject( 1 );
	const float fScale = pGeneric->GetArgFloat( 2 );
	pGeneric->SetReturnDWord( (asUINT)FontCache()->RegisterFont( pName->c_str(), pVariant->c_str(), fScale ) );
}

static void FontCache_SetActiveFont( asIScriptGeneric *pGeneric ) {
	FontCache()->SetActiveFont( (nhandle_t)pGeneric->GetArgDWord( 0 ) );
}

// stfu gcc
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
static void ImGui_TextColored( const vec4 *color, const string_t *text ) {
	ImGui::TextColored( ImVec4( color->r, color->g, color->b, color->a ), text->c_str() );
}
#pragma GCC diagnostic pop

static void ImGui_DragInt( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	int32_t v = (int32_t)pGeneric->GetArgDWord( 1 );
	float speed = pGeneric->GetArgFloat( 2 );
	int32_t min = (int32_t)pGeneric->GetArgDWord( 3 );
	int32_t max = (int32_t)pGeneric->GetArgDWord( 4 );
	const string_t *format = (const string_t *)pGeneric->GetArgObject( 5 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 6 );

	if ( ImGui::DragInt( label->c_str(), &v, speed, min, max, format->c_str(), flags ) ) {
		*(int32_t *)pGeneric->GetAddressOfReturnLocation() = v;
	} else {
		*(int32_t *)pGeneric->GetAddressOfReturnLocation() = pGeneric->GetArgDWord( 1 );
	}
}

static void ImGui_DragFloat( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	float v = (int32_t)pGeneric->GetArgFloat( 1 );
	float speed = pGeneric->GetArgFloat( 2 );
	float min = (int32_t)pGeneric->GetArgFloat( 3 );
	float max = (int32_t)pGeneric->GetArgFloat( 4 );
	const string_t *format = (const string_t *)pGeneric->GetArgObject( 5 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 6 );

	if ( ImGui::DragFloat( label->c_str(), &v, speed, min, max, format->c_str(), flags ) ) {
		*(float *)pGeneric->GetAddressOfReturnLocation() = v;
	} else {
		*(float *)pGeneric->GetAddressOfReturnLocation() = pGeneric->GetArgFloat( 1 );
	}
}

static void ImGui_SliderInt( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	int32_t v = (int32_t)pGeneric->GetArgDWord( 1 );
	int32_t min = (int32_t)pGeneric->GetArgDWord( 2 );
	int32_t max = (int32_t)pGeneric->GetArgDWord( 3 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 4 );

	if ( ImGui::SliderInt( label->c_str(), &v, min, max, "%i", flags ) ) {
		*(int32_t *)pGeneric->GetAddressOfReturnLocation() = v;
	} else {
		*(int32_t *)pGeneric->GetAddressOfReturnLocation() = pGeneric->GetArgDWord( 1 );
	}
}

static void ImGui_SliderFloat( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	float v = pGeneric->GetArgFloat( 1 );
	float min = pGeneric->GetArgFloat( 2 );
	float max = pGeneric->GetArgFloat( 3 );
	ImGuiSliderFlags flags = *(int *)pGeneric->GetArgAddress( 4 );

	if ( ImGui::SliderFloat( label->c_str(), &v, min, max, "%.3f", flags ) ) {
		*(float *)pGeneric->GetAddressOfReturnLocation() = v;
	} else {
		*(float *)pGeneric->GetAddressOfReturnLocation() = pGeneric->GetArgDWord( 1 );
	}
}

static void ImGui_SliderFloat2( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	vec2 *v = (vec2 *)pGeneric->GetArgAddress( 1 );
	float min = pGeneric->GetArgFloat( 2 );
	float max = pGeneric->GetArgFloat( 3 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 4 );

	*(bool *)pGeneric->GetAddressOfReturnLocation() = ImGui::SliderFloat2( label->c_str(), (float *)v, min, max, "%.3f", flags );
}

static void ImGui_SliderFloat3( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	vec3 *v = (vec3 *)pGeneric->GetArgAddress( 1 );
	float min = pGeneric->GetArgFloat( 2 );
	float max = pGeneric->GetArgFloat( 3 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 4 );

	*(bool *)pGeneric->GetAddressOfReturnLocation() = ImGui::SliderFloat3( label->c_str(), (float *)v, min, max, "%.3f", flags );
}

static void ImGui_SliderFloat4( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	vec4 *v = (vec4 *)pGeneric->GetArgAddress( 1 );
	float min = pGeneric->GetArgFloat( 2 );
	float max = pGeneric->GetArgFloat( 3 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 4 );

	*(bool *)pGeneric->GetAddressOfReturnLocation() = ImGui::SliderFloat4( label->c_str(), (float *)v, min, max, "%.3f", flags );
}

static void ImGui_SliderAngle( asIScriptGeneric *pGeneric ) {
	const string_t *label = (const string_t *)pGeneric->GetArgObject( 0 );
	float *v = (float *)pGeneric->GetArgAddress( 1 );
	float min = pGeneric->GetArgFloat( 2 );
	float max = pGeneric->GetArgFloat( 3 );
	ImGuiSliderFlags flags = pGeneric->GetArgDWord( 4 );

	*(bool *)pGeneric->GetAddressOfReturnLocation() = ImGui::SliderAngle( label->c_str(), v, min, max, "%.0f deg", flags );
}

static bool ImGui_RadioButton( const string_t *label, bool selected ) {
	return ImGui::RadioButton( label->c_str(), selected );
}

static void ImGui_ProgressBar( asIScriptGeneric *pGeneric ) {
	float fraction = pGeneric->GetArgFloat( 0 );
	const vec2& size = *(const vec2 *)pGeneric->GetArgObject( 1 );
	const string_t *overlay = (const string_t *)pGeneric->GetArgObject( 2 );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
	ImGui::ProgressBar( fraction, ImVec2( size.x, size.y ), overlay->c_str() );
	ImGui::PopStyleColor();
}

/*
static void RegisterGameObject( asIScriptGeneric *pGeneric ) {
	asITypeInfo *pTypeInfo;
	asIScriptFunction *pFunction;
	void *pObject;

	pTypeInfo = g_pModuleLib->GetScriptEngine()->GetTypeInfoById( pGeneric->GetObjectTypeId() );
	pFunction = pTypeInfo->GetFactoryByDecl( va( "%s::%s@ %s()", pTypeInfo->GetNamespace(), pTypeInfo->GetName(),
		pTypeInfo->GetName() ) );
	
	Con_Printf( "Added GameObject System \"%s::%s\"\n", pTypeInfo->GetNamespace(), pTypeInfo->GetName() );

	// allocate the new GameObject
	CheckASCall( g_pModuleLib->GetCurrentHandle()->GetContext()->Prepare( pFunction ) );
	CheckASCall( g_pModuleLib->GetCurrentHandle()->GetContext()->Execute() );

	pObject = g_pModuleLib->GetCurrentHandle()->GetContext()->GetReturnAddress();

	// initialize it
	pFunction = pTypeInfo->GetMethodByDecl( va( "void %s::%s OnInit()", pTypeInfo->GetNamespace(), pTypeInfo->GetName() ) );
	CheckASCall( g_pModuleLib->GetCurrentHandle()->GetContext()->Prepare( pFunction ) );
	CheckASCall( g_pModuleLib->GetCurrentHandle()->GetContext()->SetObject( pObject ) );
	CheckASCall( g_pModuleLib->GetCurrentHandle()->GetContext()->Execute() );

	CheckASCall( pGeneric->SetReturnAddress( pObject ) );

	g_pModuleLib->GetGameObjects()->InsertLast( pObject );
}
*/

static void DebugPrint( asIScriptGeneric *pGeneric ) {
	if ( !Cvar_VariableInteger( "sgame_DebugMode" ) ) {
		return;
	}

	const string_t *msg = (const string_t *)pGeneric->GetArgObject( 0 );
	Con_Printf( COLOR_GREEN "DEBUG: %s", msg->c_str() );
}

static void ProfileBlockBegin( asIScriptGeneric *pGeneric )
{
	const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
	PROFILE_BLOCK_BEGIN( name->c_str() );
}

static void ProfileBlockEnd( asIScriptGeneric *pGeneric )
{
	PROFILE_BLOCK_END;
}

//
// script globals
//
static const int32_t script_S_COLOR_BLACK = S_COLOR_BLACK;
static const int32_t script_S_COLOR_RED = S_COLOR_RED;
static const int32_t script_S_COLOR_GREEN = S_COLOR_GREEN;
static const int32_t script_S_COLOR_YELLOW = S_COLOR_YELLOW;
static const int32_t script_S_COLOR_BLUE = S_COLOR_BLUE;
static const int32_t script_S_COLOR_CYAN = S_COLOR_CYAN;
static const int32_t script_S_COLOR_MAGENTA = S_COLOR_MAGENTA;
static const int32_t script_S_COLOR_WHITE = S_COLOR_WHITE;
static const int32_t script_S_COLOR_RESET = S_COLOR_RESET;

static const string_t script_COLOR_BLACK = COLOR_BLACK;
static const string_t script_COLOR_RED = COLOR_RED;
static const string_t script_COLOR_GREEN = COLOR_GREEN;
static const string_t script_COLOR_YELLOW = COLOR_YELLOW;
static const string_t script_COLOR_BLUE = COLOR_BLUE;
static const string_t script_COLOR_CYAN = COLOR_CYAN;
static const string_t script_COLOR_MAGENTA = COLOR_MAGENTA;
static const string_t script_COLOR_WHITE = COLOR_WHITE;
static const string_t script_COLOR_RESET = COLOR_RESET;

static const asDWORD script_MAX_MAP_WIDTH = MAX_MAP_WIDTH;
static const asDWORD script_MAX_MAP_HEIGHT = MAX_MAP_HEIGHT;

static const asDWORD script_MAXPRINTMSG = MAXPRINTMSG;
static const asDWORD script_MAX_TOKEN_CHARS = MAX_TOKEN_CHARS;
static const asDWORD script_MAX_STRING_CHARS = MAX_STRING_CHARS;
static const asDWORD script_MAX_STRING_TOKENS = MAX_STRING_TOKENS;
static const asDWORD script_MAX_INFO_KEY = MAX_INFO_KEY;
static const asDWORD script_MAX_INFO_STRING = MAX_INFO_STRING;
static const asDWORD script_MAX_INFO_VALUE = MAX_INFO_VALUE;
static const asDWORD script_MAX_CVAR_NAME = MAX_CVAR_NAME;
static const asDWORD script_MAX_CVAR_VALUE = MAX_CVAR_VALUE;
static const asDWORD script_MAX_EDIT_LINE = MAX_EDIT_LINE;
static const asDWORD script_MAX_NPATH = MAX_NPATH;
static const asDWORD script_MAX_OSPATH = MAX_OSPATH;
static const asDWORD script_MAX_VERTS_ON_POLY = MAX_VERTS_ON_POLY;
static const asDWORD script_MAX_UI_FONTS = MAX_UI_FONTS;

static const asDWORD script_RSF_NORWORLDMODEL = RSF_NOWORLDMODEL;
static const asDWORD script_RSF_ORTHO_TYPE_CORDESIAN = RSF_ORTHO_TYPE_CORDESIAN;
static const asDWORD script_RSF_ORTHO_TYPE_WORLD = RSF_ORTHO_TYPE_WORLD;
static const asDWORD script_RSF_ORTHO_TYPE_SCREENSPACE = RSF_ORTHO_TYPE_SCREENSPACE;

static const asDWORD script_RT_SPRITE = RT_SPRITE;
static const asDWORD script_RT_LIGHTNING = RT_LIGHTNING;
static const asDWORD script_RT_POLY = RT_POLY;

static const int32_t script_FS_INVALID_HANDLE = FS_INVALID_HANDLE;
static const asDWORD script_FS_OPEN_READ = (asDWORD)FS_OPEN_READ;
static const asDWORD script_FS_OPEN_WRITE = (asDWORD)FS_OPEN_WRITE;
static const asDWORD script_FS_OPEN_APPEND = (asDWORD)FS_OPEN_APPEND;

static const asDWORD script_ENTITYNUM_INVALID = ENTITYNUM_INVALID;
static const asDWORD script_ENTITYNUM_WALL = ENTITYNUM_INVALID - 1;

static const asQWORD script_MAX_INT8 = CHAR_MAX;
static const asQWORD script_MAX_INT16 = SHRT_MAX;
static const asQWORD script_MAX_INT32 = INT_MAX;
static const asQWORD script_MAX_INT64 = LONG_MAX;
static const asQWORD script_MAX_UINT8 = UCHAR_MAX;
static const asQWORD script_MAX_UINT16 = USHRT_MAX;
static const asQWORD script_MAX_UINT32 = UINT_MAX;
static const asQWORD script_MAX_UINT64 = ULONG_MAX;

static const asQWORD script_MIN_INT8 = CHAR_MIN;
static const asQWORD script_MIN_INT16 = SHRT_MIN;
static const asQWORD script_MIN_INT32 = INT_MIN;

static const asDWORD script_CVAR_ROM = CVAR_ROM;
static const asDWORD script_CVAR_CHEAT = CVAR_CHEAT;
static const asDWORD script_CVAR_INIT = CVAR_INIT;
static const asDWORD script_CVAR_LATCH = CVAR_LATCH;
static const asDWORD script_CVAR_NODEFAULT = CVAR_NODEFAULT;
static const asDWORD script_CVAR_NORESTART = CVAR_NORESTART;
static const asDWORD script_CVAR_NOTABCOMPLETE = CVAR_NOTABCOMPLETE;
static const asDWORD script_CVAR_PROTECTED = CVAR_PROTECTED;
static const asDWORD script_CVAR_SAVE = CVAR_SAVE;
static const asDWORD script_CVAR_TEMP = CVAR_TEMP;
static const asDWORD script_CVAR_INVALID_HANDLE = CVAR_INVALID_HANDLE;

static const asDWORD script_SURFACEPARM_FLESH = SURFACEPARM_FLESH;
static const asDWORD script_SURFACEPARM_LAVA = SURFACEPARM_LAVA;
static const asDWORD script_SURFACEPARM_METAL = SURFACEPARM_METAL;
static const asDWORD script_SURFACEPARM_WOOD = SURFACEPARM_WOOD;
static const asDWORD script_SURFACEPARM_NODAMAGE = SURFACEPARM_NODAMAGE;
static const asDWORD script_SURFACEPARM_NODLIGHT = SURFACEPARM_NODLIGHT;
static const asDWORD script_SURFACEPARM_NOMARKS = SURFACEPARM_NOMARKS;
static const asDWORD script_SURFACEPARM_NOMISSILE = SURFACEPARM_NOMISSILE;
static const asDWORD script_SURFACEPARM_NOSTEPS = SURFACEPARM_NOSTEPS;
static const asDWORD script_SURFACEPARM_WATER = SURFACEPARM_WATER;

static const asDWORD script_NOMAD_VERSION = NOMAD_VERSION;
static const asDWORD script_NOMAD_VERSION_UPDATE = NOMAD_VERSION_UPDATE;
static const asDWORD script_NOMAD_VERSION_PATCH = NOMAD_VERSION_PATCH;
static const string_t script_NOMAD_VERSION_STRING = GLN_VERSION;
static const string_t script_NOMAD_VERSION_STR = va( "%u", NOMAD_VERSION_FULL );

static const float script_M_PI = M_PI;
static const float script_FLT_MAX = FLT_MAX;
static const float script_FLT_MIN = FLT_MIN;

template<typename T>
static int CompareVec2( const T& a, const T& b ) {
	int cmp;

	cmp = 0;
	if ( a.x < b.x && a.y < b.y ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y ) {
		cmp = 1;
	}
	return cmp;
}

template<typename T>
static int CompareVec3( const T& a, const T& b ) {
	int cmp;

	cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z ) {
		cmp = 1;
	}
	return cmp;
}

template<typename T>
static int CompareVec4( const T& a, const T& b ) {
	int cmp;

	cmp = 0;
	if ( a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w ) {
		cmp = -1;
	} else if ( b.x < a.x && b.y < a.y && b.z < a.z && b.w < a.w ) {
		cmp = 1;
	}
	return cmp;
}
/*
template<typename T, typename P, int numElements>
static void Register_VecType( const char *name, const char *p_name )
{
	REGISTER_OBJECT_TYPE( name, T, asOBJ_VALUE );

	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( T, ( void ) ) );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, va( "void f( const %s& in )", name ), WRAP_CON( T, ( const T& ) ) );
	
	switch ( numElements ) {
	case 4:
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, va( "void f( %s, %s, %s, %s )", p_name, p_name, p_name, p_name ), WRAP_CON( T, ( P, P, P, P ) ) );
		break;
	case 3:
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, va( "void f( %s, %s, %s )", p_name, p_name, p_name ), WRAP_CON( T, ( P, P, P ) ) );
		break;
	case 2:
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, va( "void f( %s, %s )", p_name, p_name ), WRAP_CON( T, ( P, P ) ) );
		break;
	};
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, va( "void f( %s )", p_name ), WRAP_CON( T, ( P ) ) );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_DESTRUCT, "void f()", WRAP_DES( T ) );

	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const %s& in )", name, name ), T, operator=, ( const T& ), T& );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const %s& in )", name, name ), T, operator+=, ( const T& ), T& );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const %s& in )", name, name ), T, operator-=, ( const T& ), T& );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const %s& in )", name, name ), T, operator*=, ( const T& ), T& );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const %s& in )", name, name ), T, operator/=, ( const T& ), T& );

	REGISTER_METHOD_FUNCTION( name, va( "bool opEquals( const %s& in ) const", name ), T, operator==, ( const T& ), bool );
	switch ( numElements ) {
	case 4:
		REGISTER_OBJECT_PROPERTY( name, va( "%s r", p_name ), offsetof( T, r ) );
		REGISTER_OBJECT_PROPERTY( name, va( "%s g", p_name ), offsetof( T, g ) );
		REGISTER_OBJECT_PROPERTY( name, va( "%s b", p_name ), offsetof( T, b ) );
		REGISTER_OBJECT_PROPERTY( name, va( "%s a", p_name ), offsetof( T, a ) );
		g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( name, va( "int opCmp( const %s& in ) const", name ), WRAP_FN_PR( CompareVec4, ( const T&, const T& ), int ), asCALL_GENERIC );
		break;
	case 3:
		REGISTER_OBJECT_PROPERTY( name, va( "%s x", p_name ), offsetof( T, x ) );
		REGISTER_OBJECT_PROPERTY( name, va( "%s y", p_name ), offsetof( T, y ) );
		REGISTER_OBJECT_PROPERTY( name, va( "%s z", p_name ), offsetof( T, z ) );
		g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( name, va( "int opCmp( const %s& in ) const", name ), WRAP_FN_PR( CompareVec3, ( const T&, const T& ), int ), asCALL_GENERIC );
		break;
	case 2:
		REGISTER_OBJECT_PROPERTY( name, va( "%s x", p_name ), offsetof( T, x ) );
		REGISTER_OBJECT_PROPERTY( name, va( "%s y", p_name ), offsetof( T, y ) );
		g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( name, va( "int opCmp( const %s& in ) const", name ), WRAP_FN_PR( CompareVec2, ( const T&, const T& ), int ), asCALL_GENERIC );
		break;
	};
	REGISTER_METHOD_FUNCTION( name, va( "%s& opIndex( uint )", p_name ), T, operator[], ( uint32_t ), P& );
	REGISTER_METHOD_FUNCTION( name, va( "const %s& opIndex( uint ) const", p_name ), T, operator[], ( uint32_t ), const P& );

	REGISTER_METHOD_FUNCTION( name, va( "%s opAdd( const %s& in ) const" ), T, operator+, ( const T& ), T );
	REGISTER_METHOD_FUNCTION( name, va( "%s opSub( const %s& in ) const" ), T, operator-, ( const T& ), T );
	REGISTER_METHOD_FUNCTION( name, va( "%s opDiv( const %s& in ) const" ), T, operator/, ( const T& ), T );
	REGISTER_METHOD_FUNCTION( name, va( "%s opMul( const %s& in ) const" ), T, operator*, ( const T& ), T );
}
*/

extern void ScriptLib_Register_GLM( void );

void ModuleLib_Register_Engine( void )
{
	PROFILE_FUNCTION();

	CScriptConvert::Register( g_pModuleLib->GetScriptEngine() );

	REGISTER_TYPEDEF( "int8", "char" );

	{ // Constants
		REGISTER_GLOBAL_VAR( "const float M_PI", &script_M_PI );
		REGISTER_GLOBAL_VAR( "const float FLT_MIN", &script_FLT_MIN );
		REGISTER_GLOBAL_VAR( "const float FLT_MAX", &script_FLT_MAX );
	}

	{ // Math
		RESET_NAMESPACE(); // should this be defined at a global level?
		/*
		Register_VecType<vec2, float, 2>( "vec2", "float" );
		Register_VecType<vec3, float, 2>( "vec3", "float" );
		Register_VecType<vec4, float, 2>( "vec4", "float" );

		Register_VecType<ivec2, int, 2>( "ivec2", "int" );
		Register_VecType<ivec3, int, 2>( "ivec3", "int" );
		Register_VecType<ivec4, int, 2>( "ivec4", "int" );

		Register_VecType<uvec2, unsigned, 2>( "uvec2", "uint" );
		Register_VecType<uvec3, unsigned, 2>( "uvec3", "uint" );
		Register_VecType<uvec4, unsigned, 2>( "uvec4", "uint" );
		*/

/*
		{
			REGISTER_OBJECT_TYPE( "vec2", vec2, asOBJ_VALUE );
			
			REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( vec2, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f( float )", WRAP_CON( vec2, ( float) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f( const vec2& in )", WRAP_CON( vec2, ( const vec2& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f( float, float )", WRAP_CON( vec2, ( float, float ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( vec2 ) );

			REGISTER_OBJECT_PROPERTY( "vec2", "float x", offsetof( vec2, x ) );
			REGISTER_OBJECT_PROPERTY( "vec2", "float y", offsetof( vec2, y ) );

			REGISTER_METHOD_FUNCTION( "vec2", "vec2& opAssign( const vec2& in )", vec2, operator=, ( const vec2& ), vec2& );
			REGISTER_METHOD_FUNCTION( "vec2", "vec2& opAddAssign( const vec2& in )", vec2, operator+=, ( const vec2& ), vec2& );
			REGISTER_METHOD_FUNCTION( "vec2", "vec2& opSubAssign( const vec2& in )", vec2, operator-=, ( const vec2& ), vec2& );
			REGISTER_METHOD_FUNCTION( "vec2", "vec2& opMulAssign( const vec2& in )", vec2, operator*=, ( const vec2& ), vec2& );
			REGISTER_METHOD_FUNCTION( "vec2", "vec2& opDivAssign( const vec2& in )", vec2, operator/=, ( const vec2& ), vec2& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "bool opEquals( const vec2& in ) const", asFUNCTION( ModuleLib_EqualsVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "int opCmp( const vec2& in ) const", asFUNCTION( ModuleLib_CmpVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "float& opIndex( uint )", asFUNCTION( ModuleLib_IndexVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "const float& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexVec2Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "vec2 opAdd( const vec2& in ) const", asFUNCTION( ModuleLib_AddVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "vec2 opSub( const vec2& in ) const", asFUNCTION( ModuleLib_SubVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "vec2 opDiv( const vec2& in ) const", asFUNCTION( ModuleLib_DivVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec2", "vec2 opMul( const vec2& in ) const", asFUNCTION( ModuleLib_MulVec2Generic ), asCALL_GENERIC );
		}
		{
			REGISTER_OBJECT_TYPE( "vec3", vec3, asOBJ_VALUE );
			REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( vec3, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f( float )", WRAP_CON( vec3, ( float ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f( const vec3& in )", WRAP_CON( vec3, ( const vec3& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f( float, float, float )", WRAP_CON( vec3, ( float, float, float ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( vec3 ) );
			REGISTER_OBJECT_PROPERTY( "vec3", "float x", offsetof( vec3, x ) );
			REGISTER_OBJECT_PROPERTY( "vec3", "float y", offsetof( vec3, y ) );
			REGISTER_OBJECT_PROPERTY( "vec3", "float z", offsetof( vec3, z ) );

			REGISTER_METHOD_FUNCTION( "vec3", "vec3& opAssign( const vec3& in )", vec3, operator=, ( const vec3& ), vec3& );
			REGISTER_METHOD_FUNCTION( "vec3", "vec3& opAddAssign( const vec3& in )", vec3, operator+=, ( const vec3& ), vec3& );
			REGISTER_METHOD_FUNCTION( "vec3", "vec3& opSubAssign( const vec3& in )", vec3, operator-=, ( const vec3& ), vec3& );
			REGISTER_METHOD_FUNCTION( "vec3", "vec3& opMulAssign( const vec3& in )", vec3, operator*=, ( const vec3& ), vec3& );
			REGISTER_METHOD_FUNCTION( "vec3", "vec3& opDivAssign( const vec3& in )", vec3, operator/=, ( const vec3& ), vec3& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "bool opEquals( const vec3& in ) const", asFUNCTION( ModuleLib_EqualsVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "int opCmp( const vec3& in ) const", asFUNCTION( ModuleLib_CmpVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "float& opIndex( uint )", asFUNCTION( ModuleLib_IndexVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "const float& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexVec3Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "vec3 opAdd( const vec3& in ) const", asFUNCTION( ModuleLib_AddVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "vec3 opSub( const vec3& in ) const", asFUNCTION( ModuleLib_SubVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "vec3 opDiv( const vec3& in ) const", asFUNCTION( ModuleLib_DivVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec3", "vec3 opMul( const vec3& in ) const", asFUNCTION( ModuleLib_MulVec3Generic ), asCALL_GENERIC );
		}
		{
			REGISTER_OBJECT_TYPE( "vec4", vec4, asOBJ_VALUE );
			REGISTER_OBJECT_BEHAVIOUR( "vec4", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( vec4, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec4", asBEHAVE_CONSTRUCT, "void f( float )", WRAP_CON( vec4, ( float ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec4", asBEHAVE_CONSTRUCT, "void f( const vec4& in )", WRAP_CON( vec4, ( const vec4& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec4", asBEHAVE_CONSTRUCT, "void f( float, float, float, float )", WRAP_CON( vec4, ( float, float, float, float ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "vec4", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( vec4 ) );
			REGISTER_OBJECT_PROPERTY( "vec4", "float r", offsetof( vec4, r ) );
			REGISTER_OBJECT_PROPERTY( "vec4", "float g", offsetof( vec4, g ) );
			REGISTER_OBJECT_PROPERTY( "vec4", "float b", offsetof( vec4, b ) );
			REGISTER_OBJECT_PROPERTY( "vec4", "float a", offsetof( vec4, a ) );

			REGISTER_METHOD_FUNCTION( "vec4", "vec4& opAssign( const vec4& in )", vec4, operator=, ( const vec4& ), vec4& );
			REGISTER_METHOD_FUNCTION( "vec4", "vec4& opAddAssign( const vec4& in )", vec4, operator+=, ( const vec4& ), vec4& );
			REGISTER_METHOD_FUNCTION( "vec4", "vec4& opSubAssign( const vec4& in )", vec4, operator-=, ( const vec4& ), vec4& );
			REGISTER_METHOD_FUNCTION( "vec4", "vec4& opMulAssign( const vec4& in )", vec4, operator*=, ( const vec4& ), vec4& );
			REGISTER_METHOD_FUNCTION( "vec4", "vec4& opDivAssign( const vec4& in )", vec4, operator/=, ( const vec4& ), vec4& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "bool opEquals( const vec4& in ) const", asFUNCTION( ModuleLib_EqualsVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "int opCmp( const vec4& in ) const", asFUNCTION( ModuleLib_CmpVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "float& opIndex( uint )", asFUNCTION( ModuleLib_IndexVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "const float& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexVec4Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "vec4 opAdd( const vec4& in ) const", asFUNCTION( ModuleLib_AddVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "vec4 opSub( const vec4& in ) const", asFUNCTION( ModuleLib_SubVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "vec4 opDiv( const vec4& in ) const", asFUNCTION( ModuleLib_DivVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "vec4", "vec4 opMul( const vec4& in ) const", asFUNCTION( ModuleLib_MulVec4Generic ), asCALL_GENERIC );
		}

		{
			REGISTER_OBJECT_TYPE( "ivec2", ivec2, asOBJ_VALUE );
			
			REGISTER_OBJECT_BEHAVIOUR( "ivec2", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( ivec2, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec2", asBEHAVE_CONSTRUCT, "void f( int )", WRAP_CON( ivec2, ( int ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec2", asBEHAVE_CONSTRUCT, "void f( const ivec2& in )", WRAP_CON( ivec2, ( const ivec2& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec2", asBEHAVE_CONSTRUCT, "void f( int, int )", WRAP_CON( ivec2, ( int, int ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec2", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( ivec2 ) );

			REGISTER_OBJECT_PROPERTY( "ivec2", "int x", offsetof( ivec2, x ) );
			REGISTER_OBJECT_PROPERTY( "ivec2", "int y", offsetof( ivec2, y ) );

			REGISTER_METHOD_FUNCTION( "ivec2", "ivec2& opAssign( const ivec2& in )", ivec2, operator=, ( const ivec2& ), ivec2& );
			REGISTER_METHOD_FUNCTION( "ivec2", "ivec2& opAddAssign( const ivec2& in )", ivec2, operator+=, ( const ivec2& ), ivec2& );
			REGISTER_METHOD_FUNCTION( "ivec2", "ivec2& opSubAssign( const ivec2& in )", ivec2, operator-=, ( const ivec2& ), ivec2& );
			REGISTER_METHOD_FUNCTION( "ivec2", "ivec2& opMulAssign( const ivec2& in )", ivec2, operator*=, ( const ivec2& ), ivec2& );
			REGISTER_METHOD_FUNCTION( "ivec2", "ivec2& opDivAssign( const ivec2& in )", ivec2, operator/=, ( const ivec2& ), ivec2& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "bool opEquals( const ivec2& in ) const", asFUNCTION( ModuleLib_EqualsIVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "int opCmp( const ivec2& in ) const", asFUNCTION( ModuleLib_CmpIVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "int& opIndex( uint )", asFUNCTION( ModuleLib_IndexIVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "const int& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexIVec2Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "ivec2 opAdd( const ivec2& in ) const", asFUNCTION( ModuleLib_AddIVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "ivec2 opSub( const ivec2& in ) const", asFUNCTION( ModuleLib_SubIVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "ivec2 opDiv( const ivec2& in ) const", asFUNCTION( ModuleLib_DivIVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec2", "ivec2 opMul( const ivec2& in ) const", asFUNCTION( ModuleLib_MulIVec2Generic ), asCALL_GENERIC );
		}
		{
			REGISTER_OBJECT_TYPE( "ivec3", ivec3, asOBJ_VALUE );
			REGISTER_OBJECT_BEHAVIOUR( "ivec3", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( ivec3, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec3", asBEHAVE_CONSTRUCT, "void f( int )", WRAP_CON( ivec3, ( int ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec3", asBEHAVE_CONSTRUCT, "void f( const ivec3& in )", WRAP_CON( ivec3, ( const ivec3& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec3", asBEHAVE_CONSTRUCT, "void f( int, int, int )", WRAP_CON( ivec3, ( int, int, int ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec3", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( ivec3 ) );
			REGISTER_OBJECT_PROPERTY( "ivec3", "int x", offsetof( ivec3, x ) );
			REGISTER_OBJECT_PROPERTY( "ivec3", "int y", offsetof( ivec3, y ) );
			REGISTER_OBJECT_PROPERTY( "ivec3", "int z", offsetof( ivec3, z ) );

			REGISTER_METHOD_FUNCTION( "ivec3", "ivec3& opAssign( const ivec3& in )", ivec3, operator=, ( const ivec3& ), ivec3& );
			REGISTER_METHOD_FUNCTION( "ivec3", "ivec3& opAddAssign( const ivec3& in )", ivec3, operator+=, ( const ivec3& ), ivec3& );
			REGISTER_METHOD_FUNCTION( "ivec3", "ivec3& opSubAssign( const ivec3& in )", ivec3, operator-=, ( const ivec3& ), ivec3& );
			REGISTER_METHOD_FUNCTION( "ivec3", "ivec3& opMulAssign( const ivec3& in )", ivec3, operator*=, ( const ivec3& ), ivec3& );
			REGISTER_METHOD_FUNCTION( "ivec3", "ivec3& opDivAssign( const ivec3& in )", ivec3, operator/=, ( const ivec3& ), ivec3& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "bool opEquals( const ivec3& in ) const", asFUNCTION( ModuleLib_EqualsIVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "int opCmp( const ivec3& in ) const", asFUNCTION( ModuleLib_CmpIVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "int& opIndex( uint )", asFUNCTION( ModuleLib_IndexIVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "const int& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexIVec3Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "ivec3 opAdd( const ivec3& in ) const", asFUNCTION( ModuleLib_AddIVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "ivec3 opSub( const ivec3& in ) const", asFUNCTION( ModuleLib_SubIVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "ivec3 opDiv( const ivec3& in ) const", asFUNCTION( ModuleLib_DivIVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec3", "ivec3 opMul( const ivec3& in ) const", asFUNCTION( ModuleLib_MulIVec3Generic ), asCALL_GENERIC );
		}
		{
			REGISTER_OBJECT_TYPE( "ivec4", vec4, asOBJ_VALUE );
			REGISTER_OBJECT_BEHAVIOUR( "ivec4", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( ivec4, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec4", asBEHAVE_CONSTRUCT, "void f( int )", WRAP_CON( ivec4, ( int ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec4", asBEHAVE_CONSTRUCT, "void f( const ivec4& in )", WRAP_CON( ivec4, ( const ivec4& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec4", asBEHAVE_CONSTRUCT, "void f( int, int, int, int )", WRAP_CON( ivec4, ( int, int, int, int ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "ivec4", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( ivec4 ) );
			REGISTER_OBJECT_PROPERTY( "ivec4", "int r", offsetof( ivec4, r ) );
			REGISTER_OBJECT_PROPERTY( "ivec4", "int g", offsetof( ivec4, g ) );
			REGISTER_OBJECT_PROPERTY( "ivec4", "int b", offsetof( ivec4, b ) );
			REGISTER_OBJECT_PROPERTY( "ivec4", "int a", offsetof( ivec4, a ) );

			REGISTER_METHOD_FUNCTION( "ivec4", "ivec4& opAssign( const ivec4& in )", ivec4, operator=, ( const ivec4& ), ivec4& );
			REGISTER_METHOD_FUNCTION( "ivec4", "ivec4& opAddAssign( const ivec4& in )", ivec4, operator+=, ( const ivec4& ), ivec4& );
			REGISTER_METHOD_FUNCTION( "ivec4", "ivec4& opSubAssign( const ivec4& in )", ivec4, operator-=, ( const ivec4& ), ivec4& );
			REGISTER_METHOD_FUNCTION( "ivec4", "ivec4& opMulAssign( const ivec4& in )", ivec4, operator*=, ( const ivec4& ), ivec4& );
			REGISTER_METHOD_FUNCTION( "ivec4", "ivec4& opDivAssign( const ivec4& in )", ivec4, operator/=, ( const ivec4& ), ivec4& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "bool opEquals( const ivec4& in ) const", asFUNCTION( ModuleLib_EqualsIVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "int opCmp( const ivec4& in ) const", asFUNCTION( ModuleLib_CmpIVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "int& opIndex( uint )", asFUNCTION( ModuleLib_IndexIVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "const int& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexIVec4Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "ivec4 opAdd( const ivec4& in ) const", asFUNCTION( ModuleLib_AddIVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "ivec4 opSub( const ivec4& in ) const", asFUNCTION( ModuleLib_SubIVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "ivec4 opDiv( const ivec4& in ) const", asFUNCTION( ModuleLib_DivIVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "ivec4", "ivec4 opMul( const ivec4& in ) const", asFUNCTION( ModuleLib_MulIVec4Generic ), asCALL_GENERIC );
		}

		{
			REGISTER_OBJECT_TYPE( "uvec2", uvec2, asOBJ_VALUE );
			
			REGISTER_OBJECT_BEHAVIOUR( "uvec2", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( uvec2, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec2", asBEHAVE_CONSTRUCT, "void f( uint )", WRAP_CON( uvec2, ( unsigned ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec2", asBEHAVE_CONSTRUCT, "void f( const uvec2& in )", WRAP_CON( uvec2, ( const uvec2& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec2", asBEHAVE_CONSTRUCT, "void f( uint, uint )", WRAP_CON( uvec2, ( unsigned, unsigned ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec2", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( uvec2 ) );

			REGISTER_OBJECT_PROPERTY( "uvec2", "uint x", offsetof( uvec2, x ) );
			REGISTER_OBJECT_PROPERTY( "uvec2", "uint y", offsetof( uvec2, y ) );

			REGISTER_METHOD_FUNCTION( "uvec2", "uvec2& opAssign( const uvec2& in )", uvec2, operator=, ( const uvec2& ), uvec2& );
			REGISTER_METHOD_FUNCTION( "uvec2", "uvec2& opAddAssign( const uvec2& in )", uvec2, operator+=, ( const uvec2& ), uvec2& );
			REGISTER_METHOD_FUNCTION( "uvec2", "uvec2& opSubAssign( const uvec2& in )", uvec2, operator-=, ( const uvec2& ), uvec2& );
			REGISTER_METHOD_FUNCTION( "uvec2", "uvec2& opMulAssign( const uvec2& in )", uvec2, operator*=, ( const uvec2& ), uvec2& );
			REGISTER_METHOD_FUNCTION( "uvec2", "uvec2& opDivAssign( const uvec2& in )", uvec2, operator/=, ( const uvec2& ), uvec2& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "bool opEquals( const uvec2& in ) const", asFUNCTION( ModuleLib_EqualsUVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "int opCmp( const uvec2& in ) const", asFUNCTION( ModuleLib_CmpUVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "uint& opIndex( uint )", asFUNCTION( ModuleLib_IndexUVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "const uint& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexUVec2Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "uvec2 opAdd( const uvec2& in ) const", asFUNCTION( ModuleLib_AddUVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "uvec2 opSub( const uvec2& in ) const", asFUNCTION( ModuleLib_SubUVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "uvec2 opDiv( const uvec2& in ) const", asFUNCTION( ModuleLib_DivUVec2Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec2", "uvec2 opMul( const uvec2& in ) const", asFUNCTION( ModuleLib_MulUVec2Generic ), asCALL_GENERIC );
		}
		{
			REGISTER_OBJECT_TYPE( "uvec3", uvec3, asOBJ_VALUE );
			REGISTER_OBJECT_BEHAVIOUR( "uvec3", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( uvec3, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec3", asBEHAVE_CONSTRUCT, "void f( uint )", WRAP_CON( uvec3, ( unsigned ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec3", asBEHAVE_CONSTRUCT, "void f( const uvec3& in )", WRAP_CON( uvec3, ( const uvec3& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec3", asBEHAVE_CONSTRUCT, "void f( uint, uint, uint )", WRAP_CON( uvec3, ( unsigned, unsigned, unsigned ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec3", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( uvec3 ) );
			REGISTER_OBJECT_PROPERTY( "uvec3", "uint x", offsetof( uvec3, x ) );
			REGISTER_OBJECT_PROPERTY( "uvec3", "uint y", offsetof( uvec3, y ) );
			REGISTER_OBJECT_PROPERTY( "uvec3", "uint z", offsetof( uvec3, z ) );

			REGISTER_METHOD_FUNCTION( "uvec3", "uvec3& opAssign( const uvec3& in )", uvec3, operator=, ( const uvec3& ), uvec3& );
			REGISTER_METHOD_FUNCTION( "uvec3", "uvec3& opAddAssign( const uvec3& in )", uvec3, operator+=, ( const uvec3& ), uvec3& );
			REGISTER_METHOD_FUNCTION( "uvec3", "uvec3& opSubAssign( const uvec3& in )", uvec3, operator-=, ( const uvec3& ), uvec3& );
			REGISTER_METHOD_FUNCTION( "uvec3", "uvec3& opMulAssign( const uvec3& in )", uvec3, operator*=, ( const uvec3& ), uvec3& );
			REGISTER_METHOD_FUNCTION( "uvec3", "uvec3& opDivAssign( const uvec3& in )", uvec3, operator/=, ( const uvec3& ), uvec3& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "bool opEquals( const uvec3& in ) const", asFUNCTION( ModuleLib_EqualsUVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "int opCmp( const uvec3& in ) const", asFUNCTION( ModuleLib_CmpUVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "uint& opIndex( uint )", asFUNCTION( ModuleLib_IndexUVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "const uint& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexUVec3Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "uvec3 opAdd( const uvec3& in ) const", asFUNCTION( ModuleLib_AddUVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "uvec3 opSub( const uvec3& in ) const", asFUNCTION( ModuleLib_SubUVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "uvec3 opDiv( const uvec3& in ) const", asFUNCTION( ModuleLib_DivUVec3Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec3", "uvec3 opMul( const uvec3& in ) const", asFUNCTION( ModuleLib_MulUVec3Generic ), asCALL_GENERIC );
		}
		{
			REGISTER_OBJECT_TYPE( "uvec4", uvec4, asOBJ_VALUE );
			REGISTER_OBJECT_BEHAVIOUR( "uvec4", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( uvec4, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec4", asBEHAVE_CONSTRUCT, "void f( uint )", WRAP_CON( uvec4, ( unsigned ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec4", asBEHAVE_CONSTRUCT, "void f( const uvec4& in )", WRAP_CON( uvec4, ( const uvec4& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec4", asBEHAVE_CONSTRUCT, "void f( uint, uint, uint, uint )", WRAP_CON( uvec4, ( unsigned, unsigned, unsigned, unsigned ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "uvec4", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( uvec4 ) );
			REGISTER_OBJECT_PROPERTY( "uvec4", "uint r", offsetof( uvec4, r ) );
			REGISTER_OBJECT_PROPERTY( "uvec4", "uint g", offsetof( uvec4, g ) );
			REGISTER_OBJECT_PROPERTY( "uvec4", "uint b", offsetof( uvec4, b ) );
			REGISTER_OBJECT_PROPERTY( "uvec4", "uint a", offsetof( uvec4, a ) );

			REGISTER_METHOD_FUNCTION( "uvec4", "uvec4& opAssign( const uvec4& in )", uvec4, operator=, ( const uvec4& ), uvec4& );
			REGISTER_METHOD_FUNCTION( "uvec4", "uvec4& opAddAssign( const uvec4& in )", uvec4, operator+=, ( const uvec4& ), uvec4& );
			REGISTER_METHOD_FUNCTION( "uvec4", "uvec4& opSubAssign( const uvec4& in )", uvec4, operator-=, ( const uvec4& ), uvec4& );
			REGISTER_METHOD_FUNCTION( "uvec4", "uvec4& opMulAssign( const uvec4& in )", uvec4, operator*=, ( const uvec4& ), uvec4& );
			REGISTER_METHOD_FUNCTION( "uvec4", "uvec4& opDivAssign( const uvec4& in )", uvec4, operator/=, ( const uvec4& ), uvec4& );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "bool opEquals( const uvec4& in ) const", asFUNCTION( ModuleLib_EqualsUVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "int opCmp( const uvec4& in ) const", asFUNCTION( ModuleLib_CmpUVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "uint& opIndex( uint )", asFUNCTION( ModuleLib_IndexUVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "const uint& opIndex( uint ) const", asFUNCTION( ModuleLib_IndexUVec4Generic ), asCALL_GENERIC );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "uvec4 opAdd( const uvec4& in ) const", asFUNCTION( ModuleLib_AddUVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "uvec4 opSub( const uvec4& in ) const", asFUNCTION( ModuleLib_SubUVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "uvec4 opDiv( const uvec4& in ) const", asFUNCTION( ModuleLib_DivUVec4Generic ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "uvec4", "uvec4 opMul( const uvec4& in ) const", asFUNCTION( ModuleLib_MulUVec4Generic ), asCALL_GENERIC );
		}
		*/
		ScriptLib_Register_GLM();
	}

	SET_NAMESPACE( "ImGui" );
	{ // ImGui
		#undef REGISTER_GLOBAL_FUNCTION
		#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr, params, returnType ) \
			ValidateFunction( __func__, decl,\
				g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, WRAP_FN_PR( funcPtr, params, returnType ), asCALL_GENERIC ) )
		
		RESET_NAMESPACE();

		REGISTER_ENUM_TYPE( "ImGuiWindowFlags" );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "None", ImGuiWindowFlags_None );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoTitleBar", ImGuiWindowFlags_NoTitleBar );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoResize", ImGuiWindowFlags_NoResize );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoMouseInputs", ImGuiWindowFlags_NoMouseInputs );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoMove", ImGuiWindowFlags_NoMove );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoCollapse", ImGuiWindowFlags_NoCollapse );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoDecoration", ImGuiWindowFlags_NoDecoration );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoBackground", ImGuiWindowFlags_NoBackground );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoScrollbar", ImGuiWindowFlags_NoScrollbar );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar );
		REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize );

		REGISTER_ENUM_TYPE( "ImGuiTableFlags" );
		REGISTER_ENUM_VALUE( "ImGuiTableFlags", "None", ImGuiTableFlags_None );
		REGISTER_ENUM_VALUE( "ImGuiTableFlags", "Resizable", ImGuiTableFlags_Resizable );
		REGISTER_ENUM_VALUE( "ImGuiTableFlags", "Reorderable", ImGuiTableFlags_Reorderable );

		REGISTER_ENUM_TYPE( "ImGuiInputTextFlags" );
		REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "None", ImGuiInputTextFlags_None );
		REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue );
		REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "AllowTabInput", ImGuiInputTextFlags_AllowTabInput );
		REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine );
		REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "ReadOnly", ImGuiInputTextFlags_ReadOnly );

		REGISTER_ENUM_TYPE( "ImGuiDir" );
		REGISTER_ENUM_VALUE( "ImGuiDir", "Left", ImGuiDir_Left );
		REGISTER_ENUM_VALUE( "ImGuiDir", "Right", ImGuiDir_Right );
		REGISTER_ENUM_VALUE( "ImGuiDir", "Down", ImGuiDir_Down );
		REGISTER_ENUM_VALUE( "ImGuiDir", "Up", ImGuiDir_Up );
		REGISTER_ENUM_VALUE( "ImGuiDir", "None", ImGuiDir_None );

		REGISTER_ENUM_TYPE( "ImGuiCol" );
		REGISTER_ENUM_VALUE( "ImGuiCol", "Border", ImGuiCol_Border );
		REGISTER_ENUM_VALUE( "ImGuiCol", "Button", ImGuiCol_Button );
		REGISTER_ENUM_VALUE( "ImGuiCol", "ButtonActive", ImGuiCol_ButtonActive );
		REGISTER_ENUM_VALUE( "ImGuiCol", "ButtonHovered", ImGuiCol_ButtonHovered );
		REGISTER_ENUM_VALUE( "ImGuiCol", "WindowBg", ImGuiCol_WindowBg );
		REGISTER_ENUM_VALUE( "ImGuiCol", "MenuBarBg", ImGuiCol_MenuBarBg );
		REGISTER_ENUM_VALUE( "ImGuiCol", "Text", ImGuiCol_Text );
		REGISTER_ENUM_VALUE( "ImGuiCol", "TextDisabled", ImGuiCol_TextDisabled );
		REGISTER_ENUM_VALUE( "ImGuiCol", "TextSelectedBg", ImGuiCol_TextSelectedBg );
		REGISTER_ENUM_VALUE( "ImGuiCol", "FrameBg", ImGuiCol_FrameBg );
		REGISTER_ENUM_VALUE( "ImGuiCol", "FrameBgActive", ImGuiCol_FrameBgActive );
		REGISTER_ENUM_VALUE( "ImGuiCol", "FrameBgHovered", ImGuiCol_FrameBgHovered );
		REGISTER_ENUM_VALUE( "ImGuiCol", "CheckMark", ImGuiCol_CheckMark );
		REGISTER_ENUM_VALUE( "ImGuiCol", "PopupBg", ImGuiCol_PopupBg );
		REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarBg", ImGuiCol_ScrollbarBg );
		REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarGrab", ImGuiCol_ScrollbarGrab );
		REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive );
		REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered );

		SET_NAMESPACE( "ImGui" );

		REGISTER_GLOBAL_FUNCTION( "bool ImGui::Begin( const string& in, ref@ = null, ImGuiWindowFlags = ImGuiWindowFlags::None )", ImGui_Begin, ( const string_t *, bool *, ImGuiWindowFlags ), bool );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::End()", ImGui_End, ( void ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::SetWindowSize( const vec2& in )", ImGui_SetWindowSize, ( const vec2 * ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::SetWindowPos( const vec2& in )", ImGui_SetWindowPos, ( const vec2 * ), void );
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::BeginTable( const string& in, int, ImGuiTableFlags = ImGuiTableFlags::None )", ImGui_BeginTable, ( const string_t *, int, ImGuiTableFlags ), bool );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::SetWindowFontScale( float )", ImGui::SetWindowFontScale, ( float ), void );
		REGISTER_GLOBAL_FUNCTION( "float ImGui::GetFontScale()", ImGui_GetFontScale, ( void ), float );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::EndTable()", ImGui::EndTable, ( void ), void );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"void ImGui::Image( int, const vec2& in, const vec2& in, const vec2& in = vec2( 0, 0 ), const vec2& in = vec2( 1, 1 ) )",
			asFUNCTION( ImGui_Image ), asCALL_GENERIC
		);
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::InputText( const string& in, string& out, ImGuiInputTextFlags = ImGuiInputTextFlags::None )", ImGui_InputText, ( const string_t *, string_t *, ImGuiInputTextFlags ), bool );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"int ImGui::InputInt( const string& in, int, ImGuiInputTextFlags = ImGuiInputTextFlags::None )", asFUNCTION( ImGui_InputInt ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float ImGui::InputFloat( const string& in, float, ImGuiInputTextFlags = ImGuiInputTextFlags::None )", asFUNCTION( ImGui_InputFloat ),
			asCALL_GENERIC
		);
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::TableNextColumn()", ImGui::TableNextColumn, ( void ), bool );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::TableNextRow( int = 0, float = 0 )", ImGui::TableNextRow, ( ImGuiTableRowFlags, float ), void );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"void ImGui::PushStyleColor( ImGuiCol, const vec4& in )", asFUNCTION( ImGui_PushStyleColor_V4 ), asCALL_GENERIC );
//        REGISTER_GLOBAL_FUNCTION( "void ImGui::PushStyleColor( ImGuiCol, const uint32 )", ImGui_PushStyleColor_U32, ( ImGuiCol, const ImU32 ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::PopStyleColor( int = 1 )", ImGui::PopStyleColor, ( int ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::Text( const string& in )", ImGui_Text, ( const string_t * ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::TextColored( const vec4& in, const string& in )", ImGui_TextColored, ( const vec4 *, const string_t * ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::SameLine( float = 0.0f, float = -1.0f )", ImGui::SameLine, ( float, float ), void );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::NewLine()", ImGui::NewLine, ( void ), void );
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::ArrowButton( const string& in, ImGuiDir )", ImGui_ArrowButton, ( const string_t *, ImGuiDir ), bool );
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::RadioButton( const string& in, bool )", ImGui_RadioButton, ( const string_t *, bool ), bool );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::SetCursorScreenPos( const vec2& in )", ImGui::SetCursorScreenPos, ( const ImVec2& ), void );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void ImGui::ProgressBar( float, const vec2& in = vec2( -FLT_MIN, 0.0f ), const string& in = \"\" )",
			asFUNCTION( ImGui_ProgressBar ), asCALL_GENERIC );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::Separator()", ImGui::Separator, ( void ), void );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"int ImGui::SliderInt( const string& in, int, int, int, int = 0 )", asFUNCTION( ImGui_SliderInt ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float ImGui::SliderFloat( const string& in, float, float, float, int = 0 )", asFUNCTION( ImGui_SliderFloat ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"bool ImGui::SliderVec2( const string& in, vec2& out, float, float, int = 0 )", asFUNCTION( ImGui_SliderFloat2 ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"bool ImGui::SliderVec3( const string& in, vec3& out, float, float, int = 0 )", asFUNCTION( ImGui_SliderFloat3 ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"bool ImGui::SliderVec4( const string& in, vec4& out, float, float, int = 0 )", asFUNCTION( ImGui_SliderFloat4 ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"bool ImGui::SliderAngle( const string& in, float& out, float = -360.0f, float = 360.0f, int = 0 )", asFUNCTION( ImGui_SliderAngle ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"int ImGui::DragInt( const string& in, int, float = 1.0f, int = 0, int = 0, const string& in = \"%d\", int = 0 )", asFUNCTION( ImGui_DragInt ),
			asCALL_GENERIC
		);
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float ImGui::DragFloat( const string& in, float, float = 1.0f, float = 0.0f, float = 0.0f, const string& in = \"%f\", int = 0 )", asFUNCTION( ImGui_DragFloat ),
			asCALL_GENERIC
		);
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::ColorEdit3( const string& in, vec3& in, int = 0 )", ImGui_ColorEdit3, ( const string_t *, vec3 *, int ), bool );
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::ColorEdit4( const string& in, vec4& in, int = 0 )", ImGui_ColorEdit4, ( const string_t *, vec4 *, int ), bool );
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::Button( const string& in, const vec2& in = vec2( 0.0f ) )", ImGui_Button, ( const string_t *, const vec2 * ), bool );

		REGISTER_GLOBAL_FUNCTION( "bool ImGui::BeginCombo( const string& in, const string& in )", ImGui_BeginCombo, ( const string_t *, const string_t * ), bool );
		REGISTER_GLOBAL_FUNCTION( "bool ImGui::Selectable( const string& in, bool = false, int = 0, const vec2& in = vec2( 0.0f ) )", ImGui_Selectable, ( const string_t *, bool, int, const vec2 * ), bool );
		REGISTER_GLOBAL_FUNCTION( "void ImGui::EndCombo()", ImGui::EndCombo, ( void ), void );

		#undef REGISTER_GLOBAL_FUNCTION
		#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
			ValidateFunction( __func__, decl,\
				g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, funcPtr, asCALL_GENERIC ) )
	}
	RESET_NAMESPACE();

	SET_NAMESPACE( "TheNomad" );
	
	SET_NAMESPACE( "TheNomad::Engine" );
	{ // Engine
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::GetJoystickAngle( int, float& out, ivec2& out )", asFUNCTION( GetJoystickAngle ) );
		REGISTER_GLOBAL_FUNCTION( "const ivec2 TheNomad::Engine::GetMousePosition()", asFUNCTION( GetMousePosition ) );

		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::ProfileBlockBegin( const string& in )", asFUNCTION( ProfileBlockBegin ) );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::ProfileBlockEnd()", asFUNCTION( ProfileBlockEnd ) );

		ScriptLib_Register_Engine();

		SET_NAMESPACE( "TheNomad::Engine::System" );
		REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::System::Milliseconds()", asFUNCTION( SysMilliseconds ) );
		SET_NAMESPACE( "TheNomad::Engine" );

		REGISTER_OBJECT_TYPE( "Timer", CTimer, asOBJ_VALUE );
		REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Timer", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( CTimer, ( void ) ) );
		REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Timer", asBEHAVE_CONSTRUCT, "void f( uint32 )", WRAP_CON( CTimer, ( uint32_t ) ) );
		REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Timer", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( CTimer ) );

		g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "TheNomad::Engine::Timer",
			"const TheNomad::Engine::Timer opAdd( const TheNomad::Engine::Timer& in )", asFUNCTION( ModuleLib_OpAddTimer ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "TheNomad::Engine::Timer",
			"const TheNomad::Engine::Timer opSub( const TheNomad::Engine::Timer& in )", asFUNCTION( ModuleLib_OpAddTimer ), asCALL_GENERIC );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "const TheNomad::Engine::Timer& opAddAssign( const TheNomad::Engine::Timer& in )", CTimer, operator+=, ( const CTimer& ), CTimer& );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "const TheNomad::Engine::Timer& opSubAssign( const TheNomad::Engine::Timer& in )", CTimer, operator-=, ( const CTimer& ), CTimer& );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "const TheNomad::Engine::Timer& opAssign( const TheNomad::Engine::Timer& in )", CTimer, operator=, ( const CTimer& ), const CTimer& );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "void Start()", CTimer, Start, ( void ), void );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "void Stop()", CTimer, Stop, ( void ), void );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "void Clear()", CTimer, Stop, ( void ), void );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "uint64 Milliseconds() const", CTimer, Milliseconds, ( void ) const, uint64_t );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "uint64 Seconds() const", CTimer, Seconds, ( void ) const, uint64_t );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "uint32 Minutes() const", CTimer, Minutes, ( void ) const, uint32_t );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "uint64 ElapsedMilliseconds() const", CTimer, ElapsedMilliseconds, ( void ) const, uint64_t );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "uint64 ElapsedSeconds() const", CTimer, ElapsedSeconds, ( void ) const, uint64_t );
		REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Timer", "uint32 ElapsedMinutes() const", CTimer, ElapsedMinutes, ( void ) const, uint32_t );

		{ // SoundSystem
			ScriptLib_Register_Sound();
		}
		{ // FileSystem
			SET_NAMESPACE( "TheNomad::Engine::FileSystem" );
			
			// could this be a class, YES, but I won't force it on the modder
			
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRead( const string& in )", WRAP_FN( ModuleOpenFileRead ) );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileWrite( const string& in )", WRAP_FN( ModuleOpenFileWrite ) );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileAppend( const string& in )", WRAP_FN( ModuleOpenFileAppend ) );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRW( const string& in )", WRAP_FN( ModuleOpenFileRW ) );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::OpenFile( const string& in, int, int& out )", WRAP_FN( ModuleOpenFile ) );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::FileSystem::CloseFile( int )", WRAP_FN( CloseFile ) );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetLength( int )", WRAP_FN( GetFileLength ) );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetPosition( int )", WRAP_FN( GetFilePosition ) );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::SetPosition( int, uint64, uint )", WRAP_FN( FileSetPosition ) );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::LoadFile( const string& in, array<int8>& out )",asFUNCTION( LoadFile ) );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::LoadFile( const string& in, string& out )", asFUNCTION( LoadFileString ) );
			REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::FileExists( const string& in )", WRAP_FN( FileExists ) );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"array<string>@ TheNomad::Engine::FileSystem::ListFiles()", asFUNCTION( ListFiles ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"bool TheNomad::Engine::FileSystem::MakeDir( const string& in )", asFUNCTION( MakeDir ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"bool TheNomad::Engine::FileSystem::RemoveDir( const string& in )", asFUNCTION( RemoveDir ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"bool TheNomad::Engine::FileSystem::RemoveFile( const string& in )", asFUNCTION( RemoveFile ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"string TheNomad::Engine::FileSystem::GetHomePath()", asFUNCTION( GetHomePath ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"string TheNomad::Engine::FileSystem::GetBaseGameDir()", asFUNCTION( GetBaseGameDir ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"string TheNomad::Engine::FileSystem::GetBasePath()", asFUNCTION( GetBasePath ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"string TheNomad::Engine::FileSystem::GetGamePath()", asFUNCTION( GetGamePath ), asCALL_GENERIC );

			{
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt8( char, int )", WRAP_FN( WriteInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt16( int16, int )", WRAP_FN( WriteInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt32( int32, int )", WRAP_FN( WriteInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt64( int64, int )", WRAP_FN( WriteInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt8( char, int )", WRAP_FN( WriteUInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt16( uint16, int )", WRAP_FN( WriteUInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt32( uint32, int )", WRAP_FN( WriteUInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt64( uint64, int )", WRAP_FN( WriteUInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteChar( char, int )", WRAP_FN( WriteInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteShort( int16, int )", WRAP_FN( WriteInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt( int32, int )", WRAP_FN( WriteInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteLong( int64, int )", WRAP_FN( WriteInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteByte( char, int )", WRAP_FN( WriteUInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUShort( uint16, int )", WRAP_FN( WriteUInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt( uint32, int )", WRAP_FN( WriteUInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteULong( uint64, int )", WRAP_FN( WriteUInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteString( const string& in, int )", WRAP_FN( WriteString ) );
			}
			{
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt8( char, int )", WRAP_FN( ReadInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt16( int16, int )", WRAP_FN( ReadInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt32( int32, int )", WRAP_FN( ReadInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt64( int64, int )", WRAP_FN( ReadInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt8( char, int )", WRAP_FN( ReadUInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt16( uint16, int  )", WRAP_FN( ReadUInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt32( uint32, int  )", WRAP_FN( ReadUInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt64( uint64, int  )", WRAP_FN( ReadUInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadChar( char, int )", WRAP_FN( ReadInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadShort( int16, int )", WRAP_FN( ReadInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt( int32, int )", WRAP_FN( ReadInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadLong( int64, int )", WRAP_FN( ReadInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadByte( char, int )", WRAP_FN( ReadUInt8 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUShort( uint16, int )", WRAP_FN( ReadUInt16 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt( uint32, int )", WRAP_FN( ReadUInt32 ) );
				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadULong( uint64, int )", WRAP_FN( ReadUInt64 ) );

				REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadString( string& out, int )", WRAP_FN( ReadString ) );
			}

			RESET_NAMESPACE();
		}
		{ // UserInterface
			SET_NAMESPACE( "TheNomad::Engine::UserInterface" );

			CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"int TheNomad::Engine::UserInterface::RegisterFont( const string& in, const string& in = \"Regular\", float = 1.0f )",
				asFUNCTION( FontCache_RegisterFont ),
				asCALL_GENERIC ) );
			
			CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
				"void TheNomad::Engine::UserInterface::SetActiveFont( int )", asFUNCTION( FontCache_SetActiveFont ),
				asCALL_GENERIC ) );
		}
		{ // Renderer
			SET_NAMESPACE( "TheNomad::Engine::Renderer" );

			REGISTER_OBJECT_TYPE( "GPUConfig", CModuleGPUConfig, asOBJ_VALUE );

			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::GPUConfig", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( CModuleGPUConfig, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::GPUConfig", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( CModuleGPUConfig ) );

			REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Renderer::GPUConfig", "TheNomad::Engine::Renderer::GPUConfig& opAssign( const TheNomad::Engine::Renderer::GPUConfig& in )", CModuleGPUConfig, operator=, ( const CModuleGPUConfig& ), CModuleGPUConfig& );

			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::GPUConfig", "string vendor", offsetof( CModuleGPUConfig, vendorString ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::GPUConfig", "string renderer", offsetof( CModuleGPUConfig, rendererString ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::GPUConfig", "string extensions", offsetof( CModuleGPUConfig, extensionsString ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::GPUConfig", "string version", offsetof( CModuleGPUConfig, versionString ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::GPUConfig", "string shaderVersion", offsetof( CModuleGPUConfig, shaderVersionString ) );
			CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterObjectProperty(
				"TheNomad::Engine::Renderer::GPUConfig", "uint32 screenWidth", offsetof( CModuleGPUConfig, gpuConfig ), offsetof( gpuConfig_t, vidWidth ) ) );
			CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterObjectProperty(
				"TheNomad::Engine::Renderer::GPUConfig", "uint32 screenHeight", offsetof( CModuleGPUConfig, gpuConfig ), offsetof( gpuConfig_t, vidHeight ) ) );
			CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterObjectProperty(
				"TheNomad::Engine::Renderer::GPUConfig", "bool isFullscreen", offsetof( CModuleGPUConfig, gpuConfig ), offsetof( gpuConfig_t, isFullscreen ) ) );
			
			/*
			REGISTER_OBJECT_TYPE( "PolyVert", CModulePolyVert, asOBJ_VALUE );

			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyVert", asBEHAVE_CONSTRUCT, "void f()", WRAP_CON( CModulePolyVert, ( void ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyVert", asBEHAVE_CONSTRUCT,
				"void f( const TheNomad::Engine::Renderer::PolyVert& in )", WRAP_CON( CModulePolyVert, ( const CModulePolyVert& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyVert", asBEHAVE_CONSTRUCT,
				"void f( const vec3& in, const vec3& in, const vec2& in, uint32 )", WRAP_CON( CModulePolyVert, ( const vec3&, const vec3&, const vec2&, const color4ub_t& ) ) );
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyVert", asBEHAVE_DESTRUCT, "void f()", WRAP_DES( CModulePolyVert ) );
			
			REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Renderer::PolyVert", "TheNomad::Engine::Renderer::PolyVert& opAssign( const TheNomad::Engine::Renderer::PolyVert& in )",
				CModulePolyVert, operator=, ( const CModulePolyVert& ), CModulePolyVert& );
			REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Renderer::PolyVert", "void Set( const TheNomad::Engine::Renderer::PolyVert& in )",
				CModulePolyVert, Set, ( const CModulePolyVert& ), void );
			REGISTER_METHOD_FUNCTION( "TheNomad::Engine::Renderer::PolyVert", "void Set( const vec3& in, const vec3& in, const vec2& in, uint32 )",
				CModulePolyVert, Set, ( const vec3&, const vec3&, const vec2&, const color4ub_t& ), void );
			
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::PolyVert", "vec3 xyz", offsetof( CModulePolyVert, m_Origin ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::PolyVert", "vec3 worldPos", offsetof( CModulePolyVert, m_WorldPos ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::PolyVert", "vec2 uv", offsetof( CModulePolyVert, m_TexCoords ) );
			REGISTER_OBJECT_PROPERTY( "TheNomad::Engine::Renderer::PolyVert", "uint32 color", offsetof( CModulePolyVert, m_Color ) );

			REGISTER_OBJECT_TYPE( "PolyQuad", CModulePolyQuad, asOBJ_VALUE );
			
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyQuad", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( ScriptPolyQuad_Construct ) );
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyQuad", asBEHAVE_CONSTRUCT, "void f( const TheNomad::Engine::Renderer::PolyQuad& in )", asFUNCTION( ScriptPolyQuad_CopyConstruct ) );
			REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyQuad", asBEHAVE_DESTRUCT, "void f()", asFUNCTION( ScriptPolyQuad_Destruct ) );

			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod(
				"TheNomad::Engine::Renderer::PolyQuad", "TheNomad::Engine::Renderer::PolyQuad& opAssign()",
				asFUNCTION( ScriptPolyQuad_OpAssign ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod(
				"TheNomad::Engine::Renderer::PolyQuad", "TheNomad::Engine::Renderer::PolyVert& opIndex( int )",
				asFUNCTION( ScriptPolyQuad_OpIndex ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterObjectMethod(
				"TheNomad::Engine::Renderer::PolyQuad", "const TheNomad::Engine::Renderer::PolyVert& opIndex( int ) const",
				asFUNCTION( ScriptPolyQuad_OpIndex ), asCALL_GENERIC );

			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::ClearScene()", asFUNCTION( ClearScene ) );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void TheNomad::Engine::Renderer::RenderScene( uint, uint, uint, uint, uint, uint )", asFUNCTION( ModuleLib_RenderScene ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void TheNomad::Engine::Renderer::AddEntityToScene( int, int, int, const vec3& in, const vec3& in, uint64,"
				" uint32, uint32, const vec2& in, float, float, float, const vec2& in )",
				asFUNCTION( AddEntityToScene ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void TheNomad::Engine::Renderer::AddDLightToScene( const vec3& in, float, float, float, float, float, const vec3& in )",
				asFUNCTION( AddDLightToScene ), asCALL_GENERIC );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::AddQuadToScene( int, const TheNomad::Engine::Renderer::PolyQuad& in )", asFUNCTION( AddQuadToScene ) );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::AddLineToScene( int, const TheNomad::Engine::Renderer::PolyQuad& in )", asFUNCTION( AddLineToScene ) );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::AddPolyToScene( int, const TheNomad::Engine::Renderer::PolyVert[]& in )", WRAP_FN_PR( AddPolyToScene, ( nhandle_t, const CScriptArray * ), void ) );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::AddSpriteToScene( const vec3& in, int )", asFUNCTION( AddSpriteToScene ) );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RegisterShader( const string& in )", asFUNCTION( RegisterShader ) );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "int TheNomad::Engine::Renderer::RegisterSpriteSheet( const string& in, uint, uint, uint, uint )", asFUNCTION( ModuleLib_RegisterSpriteSheet ), asCALL_GENERIC );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RegisterSprite( int, uint )", WRAP_FN( RegisterSprite ) );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::LoadWorld( const string& in )", asFUNCTION( LoadWorld ) );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void TheNomad::Engine::Renderer::SetColor( const vec4& in color )", asFUNCTION( ModuleLib_SetColor ), asCALL_GENERIC );
			g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void TheNomad::Engine::Renderer::DrawImage( float, float, float, float, float, float, float, float, int )", asFUNCTION( ModuleLib_DrawImage ), asCALL_GENERIC );
			*/

			ScriptLib_Register_Renderer();

			RESET_NAMESPACE();
		}
	}

	ScriptLib_Register_Game();

	SET_NAMESPACE( "TheNomad" );
	{ // Util
		SET_NAMESPACE( "TheNomad::Util" );

		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Util::LoadFunction( const string& in moduleName, const string& in funcName, ?&in )",
			asFUNCTION( LoadModuleFunction ) );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::Util::GetModuleList( array<string>& out )", WRAP_FN( GetModuleList ) );
		REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::IsModuleActive( const string& in )", WRAP_FN( IsModuleActive ) );

		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StrICmp( const string& in, const string& in )", WRAP_FN( StrICmp ) );
		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StrCmp( const string& in, const string& in )", WRAP_FN( StrCmp ) );
		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StrICmpn( const string& in, const string& in, uint )", WRAP_FN( StrICmpn ) );
		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StrCmpn( const string& in, const string& in, uint )", WRAP_FN( StrCmpn ) );
		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StringToInt( const string& in )", WRAP_FN( StringToInt ) );
		REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Util::StringToUInt( const string& in )", WRAP_FN( StringToUInt ) );
		REGISTER_GLOBAL_FUNCTION( "float TheNomad::Util::StringToFloat( const string& in )", WRAP_FN( StringToFloat ) );
		REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StringToInt( const int8[]& in )", WRAP_FN( StringBufferToInt ) );
		REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Util::StringToUInt( const int8[]& in )", WRAP_FN( StringBufferToUInt ) );
		REGISTER_GLOBAL_FUNCTION( "float TheNomad::Util::StringToFloat( const int8[]& in )", WRAP_FN( StringBufferToFloat ) );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float TheNomad::Util::Distance( const vec2& in, const vec2& in )", WRAP_FN_PR( disBetweenOBJ, ( const vec2&, const vec2& ), float ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float TheNomad::Util::Distance( const ivec2& in, const ivec2& in )", WRAP_FN_PR( disBetweenOBJ, ( const glm::ivec2&, const glm::ivec2& ), int ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float TheNomad::Util::Distance( const vec3& in, const vec3& in )", WRAP_FN_PR( disBetweenOBJ, ( const vec3&, const vec3& ), float ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float TheNomad::Util::Distance( const uvec3& in, const uvec3& in )", WRAP_FN_PR( disBetweenOBJ, ( const uvec3_t, const uvec3_t ), unsigned ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"float TheNomad::Util::Distance( const ivec3& in, const ivec3& in )", WRAP_FN_PR( disBetweenOBJ, ( const ivec3_t, const ivec3_t ), int ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"bool TheNomad::Util::AddScriptSectionToModule( const string& in, const string& in )", asFUNCTION( AddScriptSectionToModule ), asCALL_GENERIC );
		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction(
			"bool TheNomad::Util::LoadFunctionFromSection( const string& in, const string& in, const string& in, ?& out )", asFUNCTION( LoadFunctionFromSection ), asCALL_GENERIC );
		REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::BoundsIntersect( const TheNomad::GameSystem::BBox& in, const TheNomad::GameSystem::BBox& in )", WRAP_FN_PR( BoundsIntersect, ( const CModuleBoundBox *, const CModuleBoundBox * ), bool ) );
		REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::BoundsIntersectPoint( const TheNomad::GameSystem::BBox& in, const vec3& in )", WRAP_FN_PR( BoundsIntersectPoint, ( const CModuleBoundBox *, const vec3 * ), bool ) );
		REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::BoundsIntersectSphere( const TheNomad::GameSystem::BBox& in, const vec3& in, float )", WRAP_FN_PR( BoundsIntersectSphere, ( const CModuleBoundBox *, const vec3 *, float ), bool ) );

		CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterInterface( "ScriptClass" ) );
		REGISTER_GLOBAL_FUNCTION( "TheNomad::Util::ScriptClass@ TheNomad::Util::AllocateExternalScriptClass( const string& in nameSpace, const string& in name )",
			asFUNCTION( AllocateExternalScriptObject ) );
	}
	RESET_NAMESPACE();
	
	//
	// misc & global funcdefs
	//
	
	REGISTER_GLOBAL_FUNCTION( "void ConsolePrint( const string& in )", WRAP_FN( ConsolePrint ) );
	REGISTER_GLOBAL_FUNCTION( "void ConsoleWarning( const string& in )", WRAP_FN( ConsoleWarning ) );
	REGISTER_GLOBAL_FUNCTION( "void GameError( const string& in )", WRAP_FN( GameError ) );

	//    SET_NAMESPACE( "TheNomad::Constants" );
	{ // Constants
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_FLESH", &script_SURFACEPARM_FLESH );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_LAVA", &script_SURFACEPARM_LAVA );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_METAL", &script_SURFACEPARM_METAL );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_WOOD", &script_SURFACEPARM_WOOD );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_NODAMAGE", &script_SURFACEPARM_NODAMAGE );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_NODLIGHT", &script_SURFACEPARM_NODLIGHT );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_NOMARKS", &script_SURFACEPARM_NOMARKS );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_NOMISSILE", &script_SURFACEPARM_NOMISSILE );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_NOSTEPS", &script_SURFACEPARM_NOSTEPS );
		REGISTER_GLOBAL_VAR( "const uint32 SURFACEPARM_WATER", &script_SURFACEPARM_WATER );

		REGISTER_GLOBAL_VAR( "const int32 FS_INVALID_HANDLE", &script_FS_INVALID_HANDLE );
		REGISTER_GLOBAL_VAR( "const uint32 FS_OPEN_READ", &script_FS_OPEN_READ );
		REGISTER_GLOBAL_VAR( "const uint32 FS_OPEN_WRITE", &script_FS_OPEN_WRITE );
		REGISTER_GLOBAL_VAR( "const uint32 FS_OPEN_APPEND", &script_FS_OPEN_APPEND );

		REGISTER_GLOBAL_VAR( "const uint32 MAXPRINTMSG", &script_MAXPRINTMSG );
		REGISTER_GLOBAL_VAR( "const uint32 MAX_TOKEN_CHARS", &script_MAX_TOKEN_CHARS );
		REGISTER_GLOBAL_VAR( "const uint32 MAX_STRING_CHARS", &script_MAX_STRING_CHARS );
		REGISTER_GLOBAL_VAR( "const uint32 MAX_STRING_TOKENS", &script_MAX_STRING_TOKENS );
		REGISTER_GLOBAL_VAR( "const uint32 MAX_NPATH", &script_MAX_NPATH );
		REGISTER_GLOBAL_VAR( "const uint32 MAX_MAP_WIDTH", &script_MAX_MAP_WIDTH );
		REGISTER_GLOBAL_VAR( "const uint32 MAX_MAP_HEIGHT", &script_MAX_MAP_HEIGHT );

		REGISTER_GLOBAL_VAR( "const string COLOR_BLACK", &script_COLOR_BLACK );
		REGISTER_GLOBAL_VAR( "const string COLOR_RED", &script_COLOR_RED );
		REGISTER_GLOBAL_VAR( "const string COLOR_GREEN", &script_COLOR_GREEN );
		REGISTER_GLOBAL_VAR( "const string COLOR_YELLOW", &script_COLOR_YELLOW );
		REGISTER_GLOBAL_VAR( "const string COLOR_BLUE", &script_COLOR_BLUE );
		REGISTER_GLOBAL_VAR( "const string COLOR_CYAN", &script_COLOR_CYAN );
		REGISTER_GLOBAL_VAR( "const string COLOR_MAGENTA", &script_COLOR_MAGENTA );
		REGISTER_GLOBAL_VAR( "const string COLOR_WHITE", &script_COLOR_WHITE );
		REGISTER_GLOBAL_VAR( "const string COLOR_RESET", &script_COLOR_RESET );

		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_BLACK", &script_S_COLOR_BLACK );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_RED", &script_S_COLOR_RED );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_GREEN", &script_S_COLOR_GREEN );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_YELLOW", &script_S_COLOR_YELLOW );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_BLUE", &script_S_COLOR_BLUE );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_CYAN", &script_S_COLOR_CYAN );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_MAGENTA", &script_S_COLOR_MAGENTA );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_WHITE", &script_S_COLOR_WHITE );
		REGISTER_GLOBAL_VAR( "const int32 S_COLOR_RESET", &script_S_COLOR_RESET );

		REGISTER_GLOBAL_VAR( "const uint32 RSF_NOWORLDMODEL", &script_RSF_NORWORLDMODEL );
		REGISTER_GLOBAL_VAR( "const uint32 RSF_ORTHO_TYPE_CORDESIAN", &script_RSF_ORTHO_TYPE_CORDESIAN );
		REGISTER_GLOBAL_VAR( "const uint32 RSF_ORTHO_TYPE_WORLD", &script_RSF_ORTHO_TYPE_WORLD );
		REGISTER_GLOBAL_VAR( "const uint32 RSF_ORTHO_TYPE_SCREENSPACE", &script_RSF_ORTHO_TYPE_SCREENSPACE );

		REGISTER_GLOBAL_VAR( "const uint32 CVAR_CHEAT", &script_CVAR_CHEAT );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_ROM", &script_CVAR_ROM );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_INIT", &script_CVAR_INIT );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_LATCH", &script_CVAR_LATCH );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_NODEFAULT", &script_CVAR_NODEFAULT );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_NORESTART", &script_CVAR_NORESTART );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_NOTABCOMPLETE", &script_CVAR_NOTABCOMPLETE );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_TEMP", &script_CVAR_TEMP );
		REGISTER_GLOBAL_VAR( "const uint32 CVAR_SAVE", &script_CVAR_SAVE );

		REGISTER_GLOBAL_VAR( "const uint32 NOMAD_VERSION", &script_NOMAD_VERSION );
		REGISTER_GLOBAL_VAR( "const uint32 NOMAD_VERSION_UPDATE", &script_NOMAD_VERSION_UPDATE );
		REGISTER_GLOBAL_VAR( "const uint32 NOMAD_VERSION_PATCH", &script_NOMAD_VERSION_PATCH );

		REGISTER_GLOBAL_VAR( "const uint32 GAME_VERSION", &script_NOMAD_VERSION );
		REGISTER_GLOBAL_VAR( "const uint32 GAME_VERSION_UPDATE", &script_NOMAD_VERSION_UPDATE );
		REGISTER_GLOBAL_VAR( "const uint32 GAME_VERSION_PATCH", &script_NOMAD_VERSION_PATCH );

		REGISTER_GLOBAL_VAR( "const uint32 ENTITYNUM_INVALID", &script_ENTITYNUM_INVALID );
		REGISTER_GLOBAL_VAR( "const uint32 ENTITYNUM_WALL", &script_ENTITYNUM_WALL );
		
		REGISTER_GLOBAL_VAR( "const string GAME_NAME", &script_NOMAD_VERSION_STRING );
		REGISTER_GLOBAL_VAR( "const string NOMAD_VERSION_STRING", &script_NOMAD_VERSION_STR );

		REGISTER_GLOBAL_VAR( "const vec3 Vec3Origin", vec3_origin );
		REGISTER_GLOBAL_VAR( "const vec2 Vec2Origin", vec2_origin );

		REGISTER_GLOBAL_VAR( "const vec4 colorGreen", colorGreen );
		REGISTER_GLOBAL_VAR( "const vec4 colorBlack", colorBlack );
		REGISTER_GLOBAL_VAR( "const vec4 colorYellow", colorYellow );
		REGISTER_GLOBAL_VAR( "const vec4 colorBlue", colorBlue );
		REGISTER_GLOBAL_VAR( "const vec4 colorRed", colorRed );
		REGISTER_GLOBAL_VAR( "const vec4 colorMagenta", colorMagenta );
		REGISTER_GLOBAL_VAR( "const vec4 colorCyan", colorCyan );
		REGISTER_GLOBAL_VAR( "const vec4 colorWhite", colorWhite );
		REGISTER_GLOBAL_VAR( "const vec4 colorGold", colorGold );

		REGISTER_GLOBAL_VAR( "float Game_CameraZoom", &gi.cameraZoom );
		REGISTER_GLOBAL_VAR( "vec3 Game_CameraWorldPos", &gi.cameraWorldPos );
		REGISTER_GLOBAL_VAR( "vec3 Game_CameraPos", &gi.cameraPos );
		REGISTER_GLOBAL_VAR( "vec3 Game_PlayerPos", &gi.playerPos );
		REGISTER_GLOBAL_VAR( "const int Game_FrameTime", &com_frameTime );

		REGISTER_GLOBAL_VAR( "const TheNomad::GameSystem::DirType[] InverseDirs", inversedirs );

	#ifdef _NOMAD_DEBUG
		g_pModuleLib->GetScriptBuilder()->DefineWord( "NOMAD_DEBUG" );
	#endif
	}

	CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void DebugPrint( const string& in )",
		asFUNCTION( DebugPrint ), asCALL_GENERIC ) );
}