#include "module_public.h"
#include <glm/gtc/type_ptr.hpp>
#include <limits.h>
#include "aswrappedcall.h"
#include "imgui_stdlib.h"

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
#define REGISTER_METHOD_FUNCTION( obj, decl, funcPtr ) \
    ValidateMethod( __func__, decl, g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( obj, decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
    ValidateFunction( __func__, decl,\
        g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_OBJECT_TYPE( name, obj, traits ) \
    ValidateObjectType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterObjectType( name, sizeof(obj), traits | asGetTypeTraits<obj>() ) )
#define REGISTER_OBJECT_PROPERTY( obj, type, var, offset ) \
    ValidateObjectProperty( __func__, "" type " " #var "", g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( obj, "" type " " #var "", offset ) )
#define REGISTER_OBJECT_BEHAVIOUR( obj, type, decl, funcPtr ) \
    ValidateObjectBehaviour( __func__, #type, g_pModuleLib->GetScriptEngine()->RegisterObjectBehaviour( obj, type, decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_TYPEDEF( type, alias ) \
    ValidateTypedef( __func__, alias, g_pModuleLib->GetScriptEngine()->RegisterTypedef( alias, type ) )

#define SET_NAMESPACE( name ) g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( name )
#define RESET_NAMESPACE() g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( "" )

static bool Add( const char *name ) {
    for ( const auto& it : g_pModuleLib->m_RegisteredProcs ) {
        if ( !N_strcmp( name, it.c_str() ) ) {
            return false;
        }
    }
    g_pModuleLib->m_RegisteredProcs.emplace_back( name );
    return true;
}

static void ValidateEnumType( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register enumeration type %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateEnumValue( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register enumeration value %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateFunction( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register global function %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateMethod( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register method function %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateObjectType( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register object type %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateObjectBehaviour( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register object behaviour %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateObjectProperty( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register object property %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateTypedef( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register typedef %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateGlobalVar( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register global variable %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

DEFINE_CALLBACK( CvarRegisterGeneric ) {
    vmCvar_t vmCvar;

    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *value = (string_t *)pGeneric->GetArgObject( 1 );
    asDWORD flags = pGeneric->GetArgDWord( 2 );
    asINT64 *intValue = (asINT64 *)pGeneric->GetArgAddress( 3 );
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

    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *value = (string_t *)pGeneric->GetArgObject( 1 );
    asINT64 *intValue = (asINT64 *)pGeneric->GetArgAddress( 2 );
    float *floatValue = (float *)pGeneric->GetArgAddress( 3 );
    asINT32 *modificationCount = (asINT32 *)pGeneric->GetArgAddress( 4 );
    const cvarHandle_t cvarHandle = pGeneric->GetArgWord( 5 );

    memset( &vmCvar, 0, sizeof(vmCvar) );
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

DEFINE_CALLBACK( CvarSetValueGeneric ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const string_t *value = (const string_t *)pGeneric->GetArgObject( 1 );
    Cvar_Set( name->c_str(), value->c_str() );
}

DEFINE_CALLBACK( Snd_PlaySfx ) {
    REQUIRE_ARG_COUNT( 1 );
    Snd_PlaySfx( pGeneric->GetArgWord( 0 ) );
}

DEFINE_CALLBACK( Snd_SetLoopingTrack ) {
    REQUIRE_ARG_COUNT( 1 );
    Snd_SetLoopingTrack( pGeneric->GetArgWord( 0 ) );
}

DEFINE_CALLBACK( Snd_ClearLoopingTrack ) {
    Snd_ClearLoopingTrack();
}

DEFINE_CALLBACK( Snd_RegisterSfx ) {
    REQUIRE_ARG_COUNT( 1 );
    pGeneric->SetReturnWord( Snd_RegisterSfx( (const char *)pGeneric->GetArgAddress( 0 ) ) );
}

DEFINE_CALLBACK( Snd_RegisterTrack ) {
    REQUIRE_ARG_COUNT( 1 );
    pGeneric->SetReturnWord( Snd_RegisterTrack( (const char *)pGeneric->GetArgAddress( 0 ) ) );
}

void ModuleLib_Register_FileSystem( void )
{
    SET_NAMESPACE( "TheNomad::Engine::FileSystem" );

    RESET_NAMESPACE();
}

DEFINE_CALLBACK( GetMapData ) {
    REQUIRE_ARG_COUNT( 3 );
    CScriptArray *tileData = (CScriptArray *)pGeneric->GetArgAddress( 0 );
    CScriptArray *soundBits = (CScriptArray *)pGeneric->GetArgAddress( 1 );
}

DEFINE_CALLBACK( BeginSaveSection ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    g_pArchiveHandler->BeginSaveSection( name->c_str() );
}

DEFINE_CALLBACK( EndSaveSection ) {
    g_pArchiveHandler->EndSaveSection();
}

DEFINE_CALLBACK( SaveInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int8_t arg = *(const int8_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveChar( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int16_t arg = *(const int16_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int32_t arg = *(const int32_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int64_t arg = *(const int64_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveLong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint8_t arg = *(const uint8_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveByte( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint16_t arg = *(const uint16_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint32_t arg = *(const uint32_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint64_t arg = *(const uint64_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveULong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveArray ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const CScriptArray *arg = (const CScriptArray *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveArray( name->c_str(), arg );
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

DEFINE_CALLBACK( SaveVec2 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec2 v = *(const glm::vec2 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec2( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( SaveVec3 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec3 v = *(const glm::vec3 *)pGeneric->GetArgObject( 1 );
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
    *(int8_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadChar( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int16_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadShort( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadInt( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int64_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadLong( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint8_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadByte( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint16_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadUShort( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadUInt( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadULong( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadFloat ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(float *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadFloat( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadString ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *arg = (string_t *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadString( name->c_str(), arg, *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadArray ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    CScriptArray *arg = (CScriptArray *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadArray( name->c_str(), arg, *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadVec2 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    glm::vec2 *arg = (glm::vec2 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadVec2( name->c_str(), glm::value_ptr( *arg ), *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadVec3 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    glm::vec3 *arg = (glm::vec3 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadVec3( name->c_str(), glm::value_ptr( *arg ), *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadVec4 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    glm::vec4 *arg = (glm::vec4 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadVec4( name->c_str(), glm::value_ptr( *arg ), *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( ConstructVec3Generic ) {
    ::new ( pGeneric->GetObject() ) glm::vec3();
}

DEFINE_CALLBACK( DestructVec3Generic ) {
    using glm::vec3;
    ( *(vec3 *)pGeneric->GetObject() ).~vec3();
}

DEFINE_CALLBACK( CopyConstructVec3Generic ) {
    ::new ( pGeneric->GetObject() ) glm::vec3( *(const glm::vec3 *)pGeneric->GetArgObject( 0 )) ;
}

DEFINE_CALLBACK( ValueConstructVec3Generic ) {
    const float x = pGeneric->GetArgFloat( 0 );
    const float y = pGeneric->GetArgFloat( 1 );
    const float z = pGeneric->GetArgFloat( 2 );
    ::new ( pGeneric->GetObject() ) glm::vec3( x, y, z );
}

DEFINE_CALLBACK( EqualsVec3Generic ) {
    const glm::vec3& b = *(const glm::vec3 *)pGeneric->GetArgObject( 0 );
    *(bool *)pGeneric->GetAddressOfReturnLocation() = *(const glm::vec3 *)pGeneric->GetObject() == b;
}

DEFINE_CALLBACK( CmpVec3Generic ) {
    const glm::vec3& b = *(const glm::vec3 *)pGeneric->GetArgObject( 0 );
    const glm::vec3& a = *(const glm::vec3 *)pGeneric->GetObject();

    int cmp = 0;
    if ( a.x < b.x && a.y < b.y && a.z < b.z ) {
        cmp = -1;
    } else if ( b.x < a.x && b.y < a.y && b.z < a.z ) {
        cmp = 1;
    }

    *(int32_t *)pGeneric->GetAddressOfReturnLocation() = cmp;
}

DEFINE_CALLBACK( AssignVec3ToVec3Generic ) {
    const glm::vec3 *v = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    glm::vec3 *self = (glm::vec3 *)pGeneric->GetObject();
    *self = *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( AssignVec3ToValueGeneric ) {
    const float *v = (const float *)pGeneric->GetArgAddress( 0 );
    glm::vec3 *self = (glm::vec3 *)pGeneric->GetObject();
    self->x = v[0];
    self->y = v[1];
    self->z = v[2];
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( AddAssignVec3Generic ) {
    const glm::vec3 *v = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    glm::vec3 *self = (glm::vec3 *)pGeneric->GetObject();
    *self += *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( SubAssignVec3Generic ) {
    const glm::vec3 *v = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    glm::vec3 *self = (glm::vec3 *)pGeneric->GetObject();
    *self -= *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( MulAssignVec3Generic ) {
    const glm::vec3 *v = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    glm::vec3 *self = (glm::vec3 *)pGeneric->GetObject();
    *self *= *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( DivAssignVec3Generic ) {
    const glm::vec3 *v = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    glm::vec3 *self = (glm::vec3 *)pGeneric->GetObject();
    *self /= *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( AddVec3Generic ) {
    const glm::vec3 *a = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    const glm::vec3 *b = (const glm::vec3 *)pGeneric->GetArgObject( 1 );
    *(glm::vec3 *)pGeneric->GetAddressOfReturnLocation() = *a + *b;
}

DEFINE_CALLBACK( SubVec3Generic ) {
    const glm::vec3 *a = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    const glm::vec3 *b = (const glm::vec3 *)pGeneric->GetArgObject( 1 );
    *(glm::vec3 *)pGeneric->GetAddressOfReturnLocation() = *a - *b;
}

DEFINE_CALLBACK( MulVec3Generic ) {
    const glm::vec3 *a = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    const glm::vec3 *b = (const glm::vec3 *)pGeneric->GetArgObject( 1 );
    *(glm::vec3 *)pGeneric->GetAddressOfReturnLocation() = *a * *b;
}

DEFINE_CALLBACK( DivVec3Generic ) {
    const glm::vec3 *a = (const glm::vec3 *)pGeneric->GetArgObject( 0 );
    const glm::vec3 *b = (const glm::vec3 *)pGeneric->GetArgObject( 1 );
    *(glm::vec3 *)pGeneric->GetAddressOfReturnLocation() = *a / *b;
}

DEFINE_CALLBACK( IndexVec3Generic ) {
    *(float *)pGeneric->GetAddressOfReturnLocation() = ( *(glm::vec3 *)pGeneric->GetObject() )[ pGeneric->GetArgDWord( 0 ) ];
}

DEFINE_CALLBACK( ConstructVec2Generic ) {
    ::new ( pGeneric->GetObject() ) glm::vec2();
}

DEFINE_CALLBACK( DestructVec2Generic ) {
    using glm::vec2;
    ( *(vec2 *)pGeneric->GetObject() ).~vec2();
}

DEFINE_CALLBACK( CopyConstructVec2Generic ) {
    ::new ( pGeneric->GetObject() ) glm::vec2( *(const glm::vec2 *)pGeneric->GetArgObject( 0 )) ;
}

DEFINE_CALLBACK( ValueConstructVec2Generic ) {
    const float x = pGeneric->GetArgFloat( 0 );
    const float y = pGeneric->GetArgFloat( 1 );
    ::new ( pGeneric->GetObject() ) glm::vec2( x, y );
}

DEFINE_CALLBACK( EqualsVec2Generic ) {
    const glm::vec2& b = *(const glm::vec2 *)pGeneric->GetArgObject( 0 );
    *(bool *)pGeneric->GetAddressOfReturnLocation() = *(const glm::vec2 *)pGeneric->GetObject() == b;
}

DEFINE_CALLBACK( CmpVec2Generic ) {
    const glm::vec2& b = *(const glm::vec2 *)pGeneric->GetArgObject( 0 );
    const glm::vec2& a = *(const glm::vec2 *)pGeneric->GetObject();

    int cmp = 0;
    if ( a.x < b.x && a.y < b.y ) {
        cmp = -1;
    } else if ( b.x < a.x && b.y < a.y ) {
        cmp = 1;
    }

    *(int32_t *)pGeneric->GetAddressOfReturnLocation() = cmp;
}

DEFINE_CALLBACK( AssignVec2ToVec2Generic ) {
    const glm::vec2 *v = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    glm::vec2 *self = (glm::vec2 *)pGeneric->GetObject();
    *self = *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( AssignVec2ToValueGeneric ) {
    const float *v = (const float *)pGeneric->GetArgAddress( 0 );
    glm::vec2 *self = (glm::vec2 *)pGeneric->GetObject();
    self->x = v[0];
    self->y = v[1];
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( AddAssignVec2Generic ) {
    const glm::vec2 *v = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    glm::vec2 *self = (glm::vec2 *)pGeneric->GetObject();
    *self += *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( SubAssignVec2Generic ) {
    const glm::vec2 *v = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    glm::vec2 *self = (glm::vec2 *)pGeneric->GetObject();
    *self -= *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( MulAssignVec2Generic ) {
    const glm::vec2 *v = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    glm::vec2 *self = (glm::vec2 *)pGeneric->GetObject();
    *self *= *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( DivAssignVec2Generic ) {
    const glm::vec2 *v = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    glm::vec2 *self = (glm::vec2 *)pGeneric->GetObject();
    *self /= *v;
    pGeneric->SetReturnAddress( self );
}

DEFINE_CALLBACK( AddVec2Generic ) {
    const glm::vec2 *a = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    const glm::vec2 *b = (const glm::vec2 *)pGeneric->GetArgObject( 1 );
    *(glm::vec2 *)pGeneric->GetAddressOfReturnLocation() = *a + *b;
}

DEFINE_CALLBACK( SubVec2Generic ) {
    const glm::vec2 *a = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    const glm::vec2 *b = (const glm::vec2 *)pGeneric->GetArgObject( 1 );
    *(glm::vec2 *)pGeneric->GetAddressOfReturnLocation() = *a - *b;
}

DEFINE_CALLBACK( MulVec2Generic ) {
    const glm::vec2 *a = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    const glm::vec2 *b = (const glm::vec2 *)pGeneric->GetArgObject( 1 );
    *(glm::vec2 *)pGeneric->GetAddressOfReturnLocation() = *a * *b;
}

DEFINE_CALLBACK( DivVec2Generic ) {
    const glm::vec2 *a = (const glm::vec2 *)pGeneric->GetArgObject( 0 );
    const glm::vec2 *b = (const glm::vec2 *)pGeneric->GetArgObject( 1 );
    *(glm::vec2 *)pGeneric->GetAddressOfReturnLocation() = *a / *b;
}

DEFINE_CALLBACK( IndexVec2Generic ) {
    *(float *)pGeneric->GetAddressOfReturnLocation() = ( *(glm::vec2 *)pGeneric->GetObject() )[ pGeneric->GetArgDWord( 0 ) ];
}

DEFINE_CALLBACK( ConsolePrint ) {
    const string_t *msg = (const string_t *)pGeneric->GetArgAddress( 0 );
    Con_Printf( "%s", msg->c_str() );
}

DEFINE_CALLBACK( GameError ) {
    const string_t *msg = (const string_t *)pGeneric->GetArgAddress( 0 );
    N_Error( ERR_DROP, "%s", msg->c_str() );
}

DEFINE_CALLBACK( RenderScene ) {
    renderSceneRef_t refdef;
    memset( &refdef, 0, sizeof(refdef) );
    refdef.x = pGeneric->GetArgDWord( 0 );
    refdef.y = pGeneric->GetArgDWord( 1 );
    refdef.width = pGeneric->GetArgDWord( 2 );
    refdef.height = pGeneric->GetArgDWord( 3 );
    refdef.flags = pGeneric->GetArgDWord( 4 );
    refdef.time = pGeneric->GetArgDWord( 5 );
    re.RenderScene( &refdef );
}

DEFINE_CALLBACK( AddEntityToScene ) {
    refEntity_t refEntity;
    memset( &refEntity, 0, sizeof(refEntity) );
    refEntity.hShader = pGeneric->GetArgWord( 0 );
    VectorCopy( refEntity.origin, (float *)pGeneric->GetArgAddress( 1 ) );
    refEntity.flags = pGeneric->GetArgDWord( 2 );
}

DEFINE_CALLBACK( AddPolyToScene ) {
    CScriptArray *pPolyList = (CScriptArray *)pGeneric->GetArgAddress( 1 );
    re.AddPolyToScene( pGeneric->GetArgWord( 0 ), (const polyVert_t *)pPolyList->GetBuffer(), pPolyList->GetSize() );
}

DEFINE_CALLBACK( AddSpriteToScene ) {
    re.AddSpriteToScene( (const vec_t *)pGeneric->GetArgAddress( 0 ), pGeneric->GetArgWord( 1 ), pGeneric->GetArgWord( 2 ) );
}

DEFINE_CALLBACK( RegisterShader ) {
    pGeneric->SetReturnWord( re.RegisterShader( ( (const string_t *)pGeneric->GetArgAddress( 0 ) )->c_str() ) );
}

DEFINE_CALLBACK( RegisterSpriteSheet ) {
    const string_t *npath = (const string_t *)pGeneric->GetArgAddress( 0 );

    pGeneric->SetReturnWord( re.RegisterSpriteSheet( npath->c_str(), pGeneric->GetArgDWord( 1 ), pGeneric->GetArgDWord( 2 ),
        pGeneric->GetArgDWord( 3 ), pGeneric->GetArgDWord( 4 ) ) );
}

DEFINE_CALLBACK( ConstructPolyVert ) {
    memset( pGeneric->GetObject(), 0, sizeof(polyVert_t) );
}

DEFINE_CALLBACK( CopyConstructPolyVert ) {
    memcpy( pGeneric->GetObject(), pGeneric->GetArgObject( 0 ), sizeof(polyVert_t) );
}

DEFINE_CALLBACK( OpenFileRead ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenRead( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFileWrite ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenWrite( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFileAppend ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenAppend( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFileRW ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenRW( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFile ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    fileHandle_t *hFile = (fileHandle_t *)pGeneric->GetArgAddress( 1 );
    fileMode_t mode = (fileMode_t )pGeneric->GetArgDWord( 2 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenFile( fileName->c_str(), hFile, mode, H_SGAME );
}

DEFINE_CALLBACK( CloseFile ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    FS_VM_FClose( arg, H_SGAME );
}

DEFINE_CALLBACK( GetFileLength ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FileLength( arg, H_SGAME );
}

DEFINE_CALLBACK( GetFilePosition ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FileTell( arg, H_SGAME );
}

DEFINE_CALLBACK( FileSetPosition ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    uint32_t offset = pGeneric->GetArgDWord( 1 );
    uint32_t whence = pGeneric->GetArgDWord( 2 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FileSeek( arg, offset, whence, H_SGAME );
}

DEFINE_CALLBACK( Write ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    const CScriptHandle *handle = (const CScriptHandle *)pGeneric->GetArgObject( 1 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_Write( handle->GetRef(), handle->GetType()->GetSize(), arg, H_SGAME );
}

DEFINE_CALLBACK( Read ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    CScriptHandle *handle = (CScriptHandle *)pGeneric->GetArgObject( 1 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_Read( handle->GetRef(), handle->GetType()->GetSize(), arg, H_SGAME );
}

DEFINE_CALLBACK( LoadFile ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    CScriptArray *buffer = (CScriptArray *)pGeneric->GetArgObject( 1 );

    void *v;
    uint64_t length = FS_LoadFile( fileName->c_str(), &v );
    if ( !length || !v ) {
        Con_Printf( COLOR_RED "ERROR: failed to load file '%s' at vm request\n", fileName->c_str() );
        *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = 0;
        return;
    }

    buffer->Resize( length );
    memcpy( buffer->GetBuffer(), v, length );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = length;

    FS_FreeFile( v );
}

DEFINE_CALLBACK( ModuleAssertion ) {
}

DEFINE_CALLBACK( CmdArgcGeneric ) {
    *(uint32_t *)pGeneric->GetAddressOfReturnLocation() = Cmd_Argc();
}

DEFINE_CALLBACK( CmdArgvGeneric ) {
    *(string_t *)pGeneric->GetAddressOfReturnLocation() = Cmd_Argv( pGeneric->GetArgDWord( 0 ) );
}

DEFINE_CALLBACK( CmdArgsGeneric ) {
    *(string_t *)pGeneric->GetAddressOfReturnLocation() = Cmd_ArgsFrom( pGeneric->GetArgDWord( 0 ) );
}

DEFINE_CALLBACK( CmdAddCommandGeneric ) {
    Cmd_AddCommand( ( (string_t *)pGeneric->GetArgObject( 0 ) )->c_str(), NULL );
}

DEFINE_CALLBACK( CmdRemoveCommandGeneric ) {
    Cmd_RemoveCommand( ( (string_t *)pGeneric->GetArgObject( 0 ) )->c_str() );
}

static int32_t ImGui_InputText( const string_t *label, string_t *str, ImGuiInputTextFlags flags = 0 ) {
    return ImGui::InputText( label->c_str(), str, flags );
}

static bool ImGui_BeginTable( const string_t *title, int32_t numColumns, ImGuiTableFlags flags )
{
    return ImGui::BeginTable( title->c_str(), numColumns, flags );

}

//
// script globals
//
static const int8_t script_S_COLOR_BLACK = S_COLOR_BLACK;
static const int8_t script_S_COLOR_RED = S_COLOR_RED;
static const int8_t script_S_COLOR_GREEN = S_COLOR_GREEN;
static const int8_t script_S_COLOR_YELLOW = S_COLOR_YELLOW;
static const int8_t script_S_COLOR_BLUE = S_COLOR_BLUE;
static const int8_t script_S_COLOR_CYAN = S_COLOR_CYAN;
static const int8_t script_S_COLOR_MAGENTA = S_COLOR_MAGENTA;
static const int8_t script_S_COLOR_WHITE = S_COLOR_WHITE;
static const int8_t script_S_COLOR_RESET = S_COLOR_RESET;

static const string_t script_COLOR_BLACK = COLOR_BLACK;
static const string_t script_COLOR_RED = COLOR_RED;
static const string_t script_COLOR_GREEN = COLOR_GREEN;
static const string_t script_COLOR_YELLOW = COLOR_YELLOW;
static const string_t script_COLOR_BLUE = COLOR_BLUE;
static const string_t script_COLOR_CYAN = COLOR_CYAN;
static const string_t script_COLOR_MAGENTA = COLOR_MAGENTA;
static const string_t script_COLOR_WHITE = COLOR_WHITE;
static const string_t script_COLOR_RESET = COLOR_RESET;

static const asQWORD script_MAX_TOKEN_CHARS = MAX_TOKEN_CHARS;
static const asQWORD script_MAX_STRING_CHARS = MAX_STRING_CHARS;
static const asQWORD script_MAX_STRING_TOKENS = MAX_STRING_TOKENS;
static const asQWORD script_MAX_INFO_KEY = MAX_INFO_KEY;
static const asQWORD script_MAX_INFO_STRING = MAX_INFO_STRING;
static const asQWORD script_MAX_INFO_VALUE = MAX_INFO_VALUE;
static const asQWORD script_MAX_CVAR_NAME = MAX_CVAR_NAME;
static const asQWORD script_MAX_CVAR_VALUE = MAX_CVAR_VALUE;
static const asQWORD script_MAX_EDIT_LINE = MAX_EDIT_LINE;
static const asQWORD script_MAX_NPATH = MAX_NPATH;
static const asQWORD script_MAX_OSPATH = MAX_OSPATH;
static const asQWORD script_MAX_VERTS_ON_POLY = MAX_VERTS_ON_POLY;
static const asQWORD script_MAX_UI_FONTS = MAX_UI_FONTS;
static const int32_t script_FS_INVALID_HANDLE = FS_INVALID_HANDLE;

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

static const asDWORD script_NOMAD_VERSION = NOMAD_VERSION;
static const asDWORD script_NOMAD_VERSION_UPDATE = NOMAD_VERSION_UPDATE;
static const asDWORD script_NOMAD_VERSION_PATCH = NOMAD_VERSION_PATCH;

void ModuleLib_Register_Engine( void )
{
//    SET_NAMESPACE( "TheNomad::Constants" );
    { // Constants
        REGISTER_GLOBAL_VAR( "const int32 FS_INVALID_HANDLE", &script_FS_INVALID_HANDLE );

        REGISTER_GLOBAL_VAR( "const uint32 CVAR_CHEAT", &script_CVAR_CHEAT );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_ROM", &script_CVAR_ROM );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_INIT", &script_CVAR_INIT );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_LATCH", &script_CVAR_LATCH );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_NODEFAULT", &script_CVAR_NODEFAULT );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_NORESTART", &script_CVAR_NORESTART );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_NOTABCOMPLETE", &script_CVAR_NOTABCOMPLETE );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_PROTECTED", &script_CVAR_PROTECTED );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_TEMP", &script_CVAR_TEMP );
        REGISTER_GLOBAL_VAR( "const uint32 CVAR_SAVE", &script_CVAR_SAVE );

        REGISTER_GLOBAL_VAR( "const uint32 NOMAD_VERSION", &script_NOMAD_VERSION );
        REGISTER_GLOBAL_VAR( "const uint32 NOMAD_VERSION_UPDATE", &script_NOMAD_VERSION_UPDATE );
        REGISTER_GLOBAL_VAR( "const uint32 NOMAD_VERSION_PATCH", &script_NOMAD_VERSION_PATCH );
    }

    { // ModuleInfo
        REGISTER_GLOBAL_VAR( "const int32 MODULE_VERSION_MAJOR", g_pModuleLib->GetCurrentHandle()->VersionMajor() );
        REGISTER_GLOBAL_VAR( "const int32 MODULE_VERSION_UPDATE", g_pModuleLib->GetCurrentHandle()->VersionUpdate() );
        REGISTER_GLOBAL_VAR( "const int32 MODULE_VERSION_PATCH", g_pModuleLib->GetCurrentHandle()->VersionPatch() );
        REGISTER_GLOBAL_VAR( "const string MODULE_NAME", eastl::addressof( g_pModuleLib->GetCurrentHandle()->GetName() ) );
    }

    { // Math
        RESET_NAMESPACE(); // should this be defined at a global level?

        {
            REGISTER_OBJECT_TYPE( "vec2", glm::vec2, asOBJ_VALUE );
            REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f()", ConstructVec2Generic );
            REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f( const vec2& in )", CopyConstructVec2Generic );
            REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_CONSTRUCT, "void f( float, float )", ValueConstructVec2Generic );
            REGISTER_OBJECT_BEHAVIOUR( "vec2", asBEHAVE_DESTRUCT, "void f()", DestructVec2Generic );
            REGISTER_OBJECT_PROPERTY( "vec2", "float", x, offsetof( glm::vec2, x ) );
            REGISTER_OBJECT_PROPERTY( "vec2", "float", y, offsetof( glm::vec2, y ) );

            REGISTER_METHOD_FUNCTION( "vec2", "vec2& opAssign( const vec2& in )", AssignVec2ToVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2& opAddAssign( const vec2& in )", AddAssignVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2& opSubAssign( const vec2& in )", SubAssignVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2& opMulAssign( const vec2& in )", MulAssignVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2& opDivAssign( const vec2& in )", DivAssignVec2Generic );

            REGISTER_METHOD_FUNCTION( "vec2", "bool opEquals( const vec2& in ) const", EqualsVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "int opCmp( const vec2& in ) const", CmpVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "float& opIndex( uint )", IndexVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "const float& opSub( uint ) const", IndexVec2Generic );

            REGISTER_METHOD_FUNCTION( "vec2", "vec2 opAdd( const vec2& in ) const", AddVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2 opSub( const vec2& in ) const", SubVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2 opMul( const vec2& in ) const", MulVec2Generic );
            REGISTER_METHOD_FUNCTION( "vec2", "vec2 opDiv( const vec2& in ) const", DivVec2Generic );
        }
        {
            REGISTER_OBJECT_TYPE( "vec3", glm::vec3, asOBJ_VALUE );
            REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f()", ConstructVec3Generic );
            REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f( const vec3& in )", CopyConstructVec3Generic );
            REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_CONSTRUCT, "void f( float, float, float )", ValueConstructVec3Generic );
            REGISTER_OBJECT_BEHAVIOUR( "vec3", asBEHAVE_DESTRUCT, "void f()", DestructVec3Generic );
            REGISTER_OBJECT_PROPERTY( "vec3", "float", x, offsetof( glm::vec3, x ) );
            REGISTER_OBJECT_PROPERTY( "vec3", "float", y, offsetof( glm::vec3, y ) );
            REGISTER_OBJECT_PROPERTY( "vec3", "float", z, offsetof( glm::vec3, z ) );

            REGISTER_METHOD_FUNCTION( "vec3", "vec3& opAssign( const vec3& in )", AssignVec3ToVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3& opAddAssign( const vec3& in )", AddAssignVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3& opSubAssign( const vec3& in )", SubAssignVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3& opMulAssign( const vec3& in )", MulAssignVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3& opDivAssign( const vec3& in )", DivAssignVec3Generic );

            REGISTER_METHOD_FUNCTION( "vec3", "bool opEquals( const vec3& in ) const", EqualsVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "int opCmp( const vec3& in ) const", CmpVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "float& opIndex( uint )", IndexVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "const float& opSub( uint ) const", IndexVec3Generic );

            REGISTER_METHOD_FUNCTION( "vec3", "vec3 opAdd( const vec3& in ) const", AddVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3 opSub( const vec3& in ) const", SubVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3 opMul( const vec3& in ) const", MulVec3Generic );
            REGISTER_METHOD_FUNCTION( "vec3", "vec3 opDiv( const vec3& in ) const", DivVec3Generic );
        }
    }

    SET_NAMESPACE( "ImGui" );
    { // ImGui
        #undef REGISTER_GLOBAL_FUNCTION
        #define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
            ValidateFunction( __func__, decl,\
                g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, WRAP_FN( funcPtr ), asCALL_GENERIC ) )
        
        RESET_NAMESPACE();
        REGISTER_ENUM_TYPE( "ImGuiTableFlags" );
        REGISTER_ENUM_VALUE( "ImGuiTableFlags", "ImGuiTableFlags_None", ImGuiTableFlags_None );
        REGISTER_ENUM_VALUE( "ImGuiTableFlags", "ImGuiTableFlags_Resizable", ImGuiTableFlags_Resizable );
        REGISTER_ENUM_VALUE( "ImGuiTableFlags", "ImGuiTableFlags_Reorderable", ImGuiTableFlags_Reorderable );

        REGISTER_ENUM_TYPE( "ImGuiInputTextFlags" );
        REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "ImGuiInputTextFlags_EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue );
        REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "ImGuiInputTextFlags_AllowTabInput", ImGuiInputTextFlags_AllowTabInput );
        REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "ImGuiInputTextFlags_CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine );

        // this isn't currently supported
        CheckASCall( g_pModuleLib->GetScriptEngine()->RegisterFuncdef( "int ImGuiInputTextCalback( ref@ )" ) );

        SET_NAMESPACE( "ImGui" );

        REGISTER_GLOBAL_FUNCTION( "bool ImGui::Begin( const string& in, bool& in = null, const int = 0 )", ImGui::Begin );
        REGISTER_GLOBAL_FUNCTION( "void ImGui::End()", ImGui::End );
        REGISTER_GLOBAL_FUNCTION( "bool ImGui::BeginTable( const string& in, int, ImGuiTableFlags = 0, const vec2& in = ( 0.0f, 0.0f ), float = 0.0f )", ImGui_BeginTable );
        REGISTER_GLOBAL_FUNCTION( "void ImGui::EndTable()", ImGui::EndTable );
        REGISTER_GLOBAL_FUNCTION( "int ImGui::InputText( const string& in, string& out, ImGuiInputTextFlags = 0 )", ImGui_InputText );

        #undef REGISTER_GLOBAL_FUNCTION
        #define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
            ValidateFunction( __func__, decl,\
                g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
    }
    RESET_NAMESPACE();

    SET_NAMESPACE( "TheNomad" );
	
	SET_NAMESPACE( "TheNomad::Engine" );
	{ // Engine
        
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CvarRegister( const string& in, const string& in, uint, int64& out, float& out, int& out, int& out )", CvarRegisterGeneric );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CvarSet( const string& in, const string& in )", CvarSetValueGeneric );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CvarUpdate( const string& in, string& out, int64& out, float& out, int& out, const int )", CvarUpdateGeneric );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CmdArgc()", CmdArgcGeneric );
        REGISTER_GLOBAL_FUNCTION( "const string& TheNomad::Engine::CmdArgv( uint )", CmdArgvGeneric );
        REGISTER_GLOBAL_FUNCTION( "const string& TheNomad::Engine::CmdArgs( uint )", CmdArgsGeneric );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CmdAddCommand( const string& in )", CmdAddCommandGeneric );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::CmdRemoveCommand( const string& in )", CmdRemoveCommandGeneric );

		{ // SoundSystem
			SET_NAMESPACE( "TheNomad::Engine::SoundSystem" );
			
			// could this be a class, YES, but I won't force it on the modder
			
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::PlaySfx( int )", Snd_PlaySfx );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::SetLoopingTrack( int )", Snd_SetLoopingTrack );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::ClearLoopingTrack()", Snd_ClearLoopingTrack );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterSfx( const string& in )", Snd_RegisterSfx );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterTrack( const string& in )", Snd_RegisterTrack );

            RESET_NAMESPACE();
		}
		{ // FileSystem
			SET_NAMESPACE( "TheNomad::Engine::FileSystem" );
			
			// could this be a class, YES, but I won't force it on the modder
			
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRead( const string& in )", OpenFileRead );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileWrite( const string& in )", OpenFileWrite );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileAppend( const string& in )", OpenFileAppend );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::OpenFile( const string& in, int, int& out )", OpenFile );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::FileSystem::CloseFile( int )", CloseFile );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetLength( int )", GetFileLength );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetPosition( int )", GetFilePosition );
            REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::SetPosition( int, uint, uint )", FileSetPosition );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::Write( const ref@, uint64, int )", Write );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::Read( ref@, uint64, int )", Read );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::LoadFile( const string& in, array<int8>& out )", LoadFile );

            RESET_NAMESPACE();
		}
        { // Renderer
            SET_NAMESPACE( "TheNomad::Engine::Renderer" );

//            REGISTER_OBJECT_TYPE( "PolyVert", polyVert_t, asOBJ_GC | asOBJ_REF );
//            REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyVert", asBEHAVE_CONSTRUCT, "void f()", ConstructPolyVert );

            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_RenderScene( uint, uint, uint, uint, uint, uint )", RenderScene );
//            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddEntityToScene( int, vec3, uint,  )", AddEntityToScene );
//            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddPolyToScene( int, const array<PolyVert>@ )", AddPolyToScene );
            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddSpriteToScene( float, float, float, int, int )", AddSpriteToScene );
            REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RE_RegisterShader( const string& in )", RegisterShader );
            REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RE_RegisterSprite( const string& in, uint, uint, uint, uint )", RegisterSpriteSheet );

            RESET_NAMESPACE();
        }
	}
    
	SET_NAMESPACE( "TheNomad::GameSystem" );
	{
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::BeginSaveSection( const string& in )", BeginSaveSection );
		REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::EndSaveSection()", EndSaveSection );
        REGISTER_GLOBAL_FUNCTION( "int TheNomad::GameSystem::FindSaveSection( const string& in )", FindSaveSection );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt8( const string& in, int8 )", SaveInt8 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt16( const string& in, int16 )", SaveInt16 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt32( const string& in, int32 )", SaveInt32 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt64( const string& in, int64 )", SaveInt64 );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt8( const string& in, uint8 )", SaveUInt8 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt16( const string& in, uint16 )", SaveUInt16 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt32( const string& in, uint32 )", SaveUInt32 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt64( const string& in, uint64 )", SaveUInt64 );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveChar( const string& in, int8 )", SaveInt8 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveShort( const string& in, int16 )", SaveInt16 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt( const string& in, int32 )", SaveInt32 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveLong( const string& in, int64 )", SaveInt64 );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveByte( const string& in, uint8 )", SaveUInt8 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUShort( const string& in, uint16 )", SaveUInt16 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt( const string& in, uint32 )", SaveUInt32 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveULong( const string& in, uint64 )", SaveUInt64 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<T>& in )", SaveArray );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveString( const string& in, const string& in )", SaveString );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveVec2( const string& in, const vec2& in )", SaveVec2 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveVec3( const string& in, const vec3& in )", SaveVec3 );
//        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveVec4( const string& in, const vec4& )", SaveVec4 );

        REGISTER_GLOBAL_FUNCTION( "int8 TheNomad::GameSystem::LoadInt8( const string& in, int )", LoadInt8 );
        REGISTER_GLOBAL_FUNCTION( "int16 TheNomad::GameSystem::LoadInt16( const string& in, int )", LoadInt16 );
        REGISTER_GLOBAL_FUNCTION( "int32 TheNomad::GameSystem::LoadInt32( const string& in, int )", LoadInt32 );
        REGISTER_GLOBAL_FUNCTION( "int64 TheNomad::GameSystem::LoadInt64( const string& in, int )", LoadInt64 );

        REGISTER_GLOBAL_FUNCTION( "uint8 TheNomad::GameSystem::LoadUInt8( const string& in, int )", LoadUInt8 );
        REGISTER_GLOBAL_FUNCTION( "uint16 TheNomad::GameSystem::LoadUInt16( const string& in, int )", LoadUInt16 );
        REGISTER_GLOBAL_FUNCTION( "uint32 TheNomad::GameSystem::LoadUInt32( const string& in, int )", LoadUInt32 );
        REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::GameSystem::LoadUInt64( const string& in, int )", LoadUInt64 );

        REGISTER_GLOBAL_FUNCTION( "int8 TheNomad::GameSystem::LoadChar( const string& in, int )", LoadInt8 );
        REGISTER_GLOBAL_FUNCTION( "int16 TheNomad::GameSystem::LoadShort( const string& in, int )", LoadInt16 );
        REGISTER_GLOBAL_FUNCTION( "int32 TheNomad::GameSystem::LoadInt( const string& in, int )", LoadInt32 );
        REGISTER_GLOBAL_FUNCTION( "int64 TheNomad::GameSystem::LoadLong( const string& in, int )", LoadInt64 );

        REGISTER_GLOBAL_FUNCTION( "uint8 TheNomad::GameSystem::LoadByte( const string& in, uint )", LoadUInt8 );
        REGISTER_GLOBAL_FUNCTION( "uint16 TheNomad::GameSystem::LoadUShort( const string& in, uint )", LoadUInt16 );
        REGISTER_GLOBAL_FUNCTION( "uint32 TheNomad::GameSystem::LoadUInt( const string& in, uint )", LoadUInt32 );
        REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::GameSystem::LoadULong( const string& in, uint )", LoadUInt64 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadArray( const string& in, array<T>& out )", LoadArray );

        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadString( const string& in, string& out, int )", LoadString );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadVec2( const string& in, vec2& out, int )", LoadVec2 );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadVec3( const string& in, vec3& out, int )", LoadVec3 );
//        REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadVec4( const string& in, vec4& out, int )", LoadVec4 );

        
		// these are here because if they change in the map editor or the engine, it'll break sgame
		REGISTER_ENUM_TYPE( "EntityType" );
		REGISTER_ENUM_VALUE( "EntityType", "Playr", ET_PLAYR );
		REGISTER_ENUM_VALUE( "EntityType", "Mob", ET_MOB );
		REGISTER_ENUM_VALUE( "EntityType", "Bot", ET_BOT );
		REGISTER_ENUM_VALUE( "EntityType", "Item", ET_ITEM );
		REGISTER_ENUM_VALUE( "EntityType", "Wall", ET_WALL );
		REGISTER_ENUM_VALUE( "EntityType", "Weapon", ET_WEAPON );

        REGISTER_ENUM_TYPE( "GameDifficulty" );
        REGISTER_ENUM_VALUE( "GameDifficulty", "VeryEasy", DIF_NOOB );
        REGISTER_ENUM_VALUE( "GameDifficulty", "Easy", DIF_RECRUIT );
        REGISTER_ENUM_VALUE( "GameDifficulty", "Normal", DIF_MERC );
        REGISTER_ENUM_VALUE( "GameDifficulty", "Hard", DIF_NOMAD );
        REGISTER_ENUM_VALUE( "GameDifficulty", "VeryHard", DIF_BLACKDEATH );
        REGISTER_ENUM_VALUE( "GameDifficulty", "TryYourBest", DIF_MINORINCONVENIECE );
		
		REGISTER_ENUM_TYPE( "DirType" );
		REGISTER_ENUM_VALUE( "DirType", "North", DIR_NORTH );
		REGISTER_ENUM_VALUE( "DirType", "NorthEast", DIR_NORTH_EAST );
		REGISTER_ENUM_VALUE( "DirType", "East", DIR_EAST );
		REGISTER_ENUM_VALUE( "DirType", "SouthEast", DIR_SOUTH_EAST );
		REGISTER_ENUM_VALUE( "DirType", "South", DIR_SOUTH );
		REGISTER_ENUM_VALUE( "DirType", "SouthWest", DIR_SOUTH_WEST );
		REGISTER_ENUM_VALUE( "DirType", "West", DIR_WEST );
		REGISTER_ENUM_VALUE( "DirType", "NorthWest", DIR_NORTH_WEST );
		REGISTER_ENUM_VALUE( "DirType", "Inside", DIR_NULL );
		
	    REGISTER_GLOBAL_FUNCTION( "int TheNomad::GameSystem::GetMapData( array<uint32>& out, array<int32>& out, array<vec3>& out, int& out, int& out )", GetMapData );
    }

    RESET_NAMESPACE();
	
	//
	// misc & global funcdefs
	//
	
	REGISTER_GLOBAL_FUNCTION( "void ConsolePrint( const string& in )", ConsolePrint );
	REGISTER_GLOBAL_FUNCTION( "void GameError( const string& in )", GameError );
    REGISTER_GLOBAL_FUNCTION( "void Assert( bool bValue )", ModuleAssertion );
}
