#ifndef __MODULE_PROC_DEF__
#define __MODULE_PROC_DEF__

#pragma once

#include "module_public.h"

// glm has a lot of very fuzzy template types
using vec2 = glm::vec<2, float, glm::packed_highp>;
using vec3 = glm::vec<3, float, glm::packed_highp>;
using vec4 = glm::vec<4, float, glm::packed_highp>;
using ivec2 = glm::vec<2, int, glm::packed_highp>;
using ivec3 = glm::vec<3, int, glm::packed_highp>;
using ivec4 = glm::vec<4, int, glm::packed_highp>;
using uvec2 = glm::vec<2, unsigned, glm::packed_highp>;
using uvec3 = glm::vec<3, unsigned, glm::packed_highp>;
using uvec4 = glm::vec<4, unsigned, glm::packed_highp>;

#define SCRIPT_CALLBACK( name ) void Script_##name( asIScriptGeneric *pGeneric );
#define SCRIPT_CALLBACK_DEF( name ) void Script_##name( asIScriptGeneric *pGeneric )

#define CALLBACK_BEGIN( nArgs ) Assert( pGeneric->GetArgCount() == nArgs ); int __script_arg_index = 0;

template<typename T>
inline T *ScriptCallbackParameterPtr( int& index, asIScriptGeneric *pGeneric ) {
    T *value = (T *)pGeneric->GetArgAddress( index );
    index++;
    return value;
}

template<typename T>
inline T *ScriptCallbackParameterObject( int& index, asIScriptGeneric *pGeneric ) {
    T *value = (T *)pGeneric->GetArgObject( index );
    index++;
    return value;
}

template<typename T, typename Fn>
inline const T ScriptCallbackParameter( int& index, Fn&& fn ) {
    T value = (T)Fn( index );
    index++;
    return value;
}

#define CALLBACK_PARAM_BOOL( name ) \
    bool name = ScriptCallbackParameter<bool>( __script_arg_index, \
        [pGeneric]( int index ) -> bool { return (bool)pGeneric->GetArgDWord( index ); } );
#define CALLBACK_PARAM_I8( name ) \
    int8_t name = ScriptCallbackParameter<int8_t>( __script_arg_index, \
        [pGeneric]( int index ) -> int8_t { return (int8_t)pGeneric->GetArgByte( index ); } );
#define CALLBACK_PARAM_I16( name ) \
    int16_t name = ScriptCallbackParameter<int16_t>( __script_arg_index, \
        [pGeneric]( int index ) -> int16_t { return (int16_t)pGeneric->GetArgWord( index ); } );
#define CALLBACK_PARAM_I32( name ) \
    int32_t name = ScriptCallbackParameter<int32_t>( __script_arg_index, \
        [pGeneric]( int index ) -> int32_t { return (int32_t)pGeneric->GetArgDWord( index ); } );
#define CALLBACK_PARAM_I64( name ) \
    int64_t name = ScriptCallbackParameter<int64_t>( __script_arg_index, \
        [pGeneric]( int index ) -> int64_t { return (int64_t)pGeneric->GetArgQWord( index ); } );
#define CALLBACK_PARAM_U8( name ) \
    uint8_t name = ScriptCallbackParameter<uint8_t>( __script_arg_index, \
        [pGeneric]( int index ) -> uint8_t { return (uint8_t)pGeneric->GetArgByte( index ); } );
#define CALLBACK_PARAM_U16( name ) \
    uint16_t name = ScriptCallbackParameter<uint16_t>( __script_arg_index, \
        [pGeneric]( int index ) -> uint16_t { return (uint16_t)pGeneric->GetArgWord( index ); } );
#define CALLBACK_PARAM_U32( name ) \
    uint32_t name = ScriptCallbackParameter<uint32_t>( __script_arg_index, \
        [pGeneric]( int index ) -> uint32_t { return (uint32_t)pGeneric->GetArgDWord( index ); } );
#define CALLBACK_PARAM_U64( name ) \
    uint64_t name = ScriptCallbackParameter<uint64_t>( __script_arg_index, \
        [pGeneric]( int index ) -> uint64_t { return (uint64_t)pGeneric->GetArgQWord( index ); } );
#define CALLBACK_PARAM_FLOAT( name ) \
    float name = ScriptCallbackParameter<float>( __script_arg_index, \
        [pGeneric]( int index ) -> float { return pGeneric->GetArgDouble( index ); } );
#define CALLBACK_PARAM_DOUBLE( name ) \
    double name = ScriptCallbackParameter<float>( __script_arg_index, \
        [pGeneric]( int index ) -> float { return pGeneric->GetArgFloat( index ); } );
#define CALLBACK_PARAM_PTR( type, name ) \
    type *name = ScriptCallbackParameterPtr<type>( __script_arg_index, pGeneric );
#define CALLBACK_PARAM_STRING( name ) \
    string_t *name = ScriptCallbackParameterObject<string_t>( __script_arg_index, pGeneric );
#define CALLBACK_PARAM_OBJ( type, name ) \
    type *name = ScriptCallbackParameterObject<type>( __script_arg_index, pGeneric );
#define RETURN_I8( value ) pGeneric->SetReturnByte( value );
#define RETURN_I16( value ) pGeneric->SetReturnWord( value );
#define RETURN_I32( value ) pGeneric->SetReturnDWord( value );
#define RETURN_I64( value ) pGeneric->SetReturnQWord( value );
#define RETURN_U8( value ) pGeneric->SetReturnByte( value );
#define RETURN_U16( value ) pGeneric->SetReturnWord( value );
#define RETURN_U32( value ) pGeneric->SetReturnDWord( value );
#define RETURN_U64( value ) pGeneric->SetReturnQWord( value );
#define RETURN_PTR( value ) pGeneric->SetReturnAddress( value );
#define RETURN_BOOL( value ) pGeneric->SetReturnDWord( value );
#define GET_RETURN_DATA( type ) ( *(type *)pGeneric->GetAddressOfReturnLocation() )
#define CONSTRUCT_OBJECT( type, ... ) ::new ( pGeneric->GetAddressOfReturnLocation() ) type( __VA_ARGS__ );

//==============================================================
// ImGui
//

SCRIPT_CALLBACK( ImGuiBegin )
SCRIPT_CALLBACK( ImGuiEnd )
SCRIPT_CALLBACK( ImGuiSliderInt )
SCRIPT_CALLBACK( ImGuiSliderFloat )
SCRIPT_CALLBACK( ImGuiSliderVec2 )
SCRIPT_CALLBACK( ImGuiSliderVec3 )
SCRIPT_CALLBACK( ImGuiSliderVec4 )
SCRIPT_CALLBACK( ImGuiSliderAngle )
SCRIPT_CALLBACK( ImGuiBeginTable )
SCRIPT_CALLBACK( ImGuiEndTable )
SCRIPT_CALLBACK( ImGuiBeginChild )
SCRIPT_CALLBACK( ImGuiEndChild )
SCRIPT_CALLBACK( ImGuiDragInt )
SCRIPT_CALLBACK( ImGuiDragFloat )
SCRIPT_CALLBACK( ImGuiColorEdit3 )
SCRIPT_CALLBACK( ImGuiColorEdit4 )
SCRIPT_CALLBACK( ImGuiPushStyleColor )
SCRIPT_CALLBACK( ImGuiPopStyleColor )
SCRIPT_CALLBACK( ImGuiPushStyleVar )
SCRIPT_CALLBACK( ImGuiPopStyleVar )

//==============================================================
// TheNomad::Engine
//

SCRIPT_CALLBACK( CvarRegister )
SCRIPT_CALLBACK( CvarUpdate )
SCRIPT_CALLBACK( CvarVariableInteger )
SCRIPT_CALLBACK( CvarVariableFloat )
SCRIPT_CALLBACK( CvarVariableString )
SCRIPT_CALLBACK( CvarSet )
SCRIPT_CALLBACK( CvarSetIntegerValue )
SCRIPT_CALLBACK( CvarSetFloatValue )
SCRIPT_CALLBACK( CvarSetStringValue )

//==============================================================
// TheNomad::Engine::SoundSystem
//

SCRIPT_CALLBACK( RegisterSfx )
SCRIPT_CALLBACK( RegisterTrack )
SCRIPT_CALLBACK( PlaySfx )
SCRIPT_CALLBACK( SetLoopingTrack )
SCRIPT_CALLBACK( ClearLoopingTrack )

//==============================================================
// TheNomad::Engine::FileSystem
//

SCRIPT_CALLBACK( OpenFileRead )
SCRIPT_CALLBACK( OpenFileWrite )
SCRIPT_CALLBACK( OpenFileRW )
SCRIPT_CALLBACK( OpenFileAppend )
SCRIPT_CALLBACK( OpenFileMode )
SCRIPT_CALLBACK( CloseFile )
SCRIPT_CALLBACK( LoadFile )
SCRIPT_CALLBACK( GetFileLength )
SCRIPT_CALLBACK( GetFilePosition )
SCRIPT_CALLBACK( SetFilePosition )
SCRIPT_CALLBACK( ListFiles )
SCRIPT_CALLBACK( WriteInt8 )
SCRIPT_CALLBACK( WriteInt16 )
SCRIPT_CALLBACK( WriteInt32 )
SCRIPT_CALLBACK( WriteInt64 )
SCRIPT_CALLBACK( WriteUInt8 )
SCRIPT_CALLBACK( WriteUInt16 )
SCRIPT_CALLBACK( WriteUInt32 )
SCRIPT_CALLBACK( WriteUInt64 )
SCRIPT_CALLBACK( WriteString )
SCRIPT_CALLBACK( WriteArray )
SCRIPT_CALLBACK( ReadInt8 )
SCRIPT_CALLBACK( ReadInt16 )
SCRIPT_CALLBACK( ReadInt32 )
SCRIPT_CALLBACK( ReadInt64 )
SCRIPT_CALLBACK( ReadUInt8 )
SCRIPT_CALLBACK( ReadUInt16 )
SCRIPT_CALLBACK( ReadUInt32 )
SCRIPT_CALLBACK( ReadUInt64 )
SCRIPT_CALLBACK( ReadString )
SCRIPT_CALLBACK( ReadArray )

//==============================================================
// TheNomad::Engine::Renderer
//

SCRIPT_CALLBACK( DrawImage )
SCRIPT_CALLBACK( AddSpriteToScene )
SCRIPT_CALLBACK( AddPolyToScene )
SCRIPT_CALLBACK( AddDLightToScene )
SCRIPT_CALLBACK( AddEntityToScene )
SCRIPT_CALLBACK( ClearScene )
SCRIPT_CALLBACK( RenderScene )
SCRIPT_CALLBACK( RegisterShader )
SCRIPT_CALLBACK( RegisterSpriteSheet )
SCRIPT_CALLBACK( RegisterSprite )

//==============================================================
// TheNomad::GameSystem
//

SCRIPT_CALLBACK( BBoxAssign )

SCRIPT_CALLBACK( SetCameraPos )
SCRIPT_CALLBACK( GetString )
SCRIPT_CALLBACK( GetGPUConfig )

SCRIPT_CALLBACK( CastRay )
SCRIPT_CALLBACK( CheckWallHit )

SCRIPT_CALLBACK( LoadMap )
SCRIPT_CALLBACK( SetActiveMap )
SCRIPT_CALLBACK( GetSpawnData )
SCRIPT_CALLBACK( GetCheckpointData )
SCRIPT_CALLBACK( GetTileData )

//==============================================================
// TheNomad::Util
//

SCRIPT_CALLBACK( GetModuleList )
SCRIPT_CALLBACK( IsModuleActive )
SCRIPT_CALLBACK( StrICmp )
SCRIPT_CALLBACK( BoundsIntersect )

SCRIPT_CALLBACK( ConsolePrint )
SCRIPT_CALLBACK( GameError )

#endif