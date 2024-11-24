#ifndef __MODULE_FUNCDEFS_H__
#define __MODULE_FUNCDEFS_H__

#pragma once

#include "../module_public.h"
#include <EASTL/functional.h>

#define REGISTER_ENUM_TYPE( name ) \
	ValidateEnumType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterEnum( name ) )
#define REGISTER_ENUM_VALUE( type, name, value ) \
	ValidateEnumValue( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterEnumValue( type, name, value ) )
#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr, call ) \
	ValidateFunction( __func__, decl, g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, funcPtr, call ) )
#define REGISTER_OBJECT_TYPE( name, obj, traits ) \
	ValidateObjectType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterObjectType( name, sizeof( obj ), traits | asGetTypeTraits<obj>() ) )
#define REGISTER_OBJECT_PROPERTY( obj, var, offset ) \
	ValidateObjectProperty( __func__, var, g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( obj, var, offset ) )
#define REGISTER_OBJECT_BEHAVIOUR( obj, type, decl, funcPtr, call ) \
	ValidateObjectBehaviour( __func__, #type, g_pModuleLib->GetScriptEngine()->RegisterObjectBehaviour( obj, type, decl, funcPtr, call ) )
#define REGISTER_TYPEDEF( type, alias ) \
	ValidateTypedef( __func__, alias, g_pModuleLib->GetScriptEngine()->RegisterTypedef( alias, type ) )
#define REGISTER_METHOD_FUNCTION( obj, decl, funcPtr, call ) \
	ValidateMethod( __func__, decl, g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( obj, decl, funcPtr, call ) )

#define SET_NAMESPACE( name ) g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( name )
#define RESET_NAMESPACE() g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( "" )

GDR_INLINE void ValidateEnumType( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register enumeration type %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateEnumValue( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register enumeration value %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateFunction( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register global function %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateMethod( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register method function %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateObjectType( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register object type %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateObjectBehaviour( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register object behaviour %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateObjectProperty( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register object property %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateTypedef( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register typedef %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

GDR_INLINE void ValidateGlobalVar( const char *func, const char *name, int result ) {
	if ( result < 0 ) {
		N_Error( ERR_DROP, "%s: failed to register global variable %s -- %s", func, name, AS_PrintErrorString( result ) );
	}
}

void ScriptLib_Register_Sound( void );

#endif