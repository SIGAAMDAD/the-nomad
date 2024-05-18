#include "module_proc_def.h"

//==============================================================
// ImGui
//

SCRIPT_CALLBACK_DEF( ImGuiBegin ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_PTR( bool, open )
    CALLBACK_PARAM_U32( flags )
    RETURN_BOOL( ImGui::Begin( label->c_str(), open, flags ) )
}
SCRIPT_CALLBACK_DEF( ImGuiEnd ) {
    CALLBACK_BEGIN( 0 )
    ImGui::End();
}
SCRIPT_CALLBACK_DEF( ImGuiSliderInt ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_I32( curvalue )
    CALLBACK_PARAM_I32( min )
    CALLBACK_PARAM_I32( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )

    const int value = curvalue;
    if ( ImGui::SliderInt( label->c_str(), &curvalue, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( int ) = curvalue;
    } else {
        GET_RETURN_DATA( int ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiSliderFloat ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_FLOAT( curvalue )
    CALLBACK_PARAM_FLOAT( min )
    CALLBACK_PARAM_FLOAT( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )

    const float value = curvalue;
    if ( ImGui::SliderFloat( label->c_str(), &curvalue, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( float ) = curvalue;
    } else {
        GET_RETURN_DATA( float ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiSliderVec2 ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_OBJ( vec2, curvalue )
    CALLBACK_PARAM_FLOAT( min )
    CALLBACK_PARAM_FLOAT( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )

    const vec2 value = *curvalue;
    if ( ImGui::SliderFloat2( label->c_str(), (float *)curvalue, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( vec2 ) = *curvalue;
    } else {
        GET_RETURN_DATA( vec2 ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiSliderVec3 ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_OBJ( vec3, curvalue )
    CALLBACK_PARAM_FLOAT( min )
    CALLBACK_PARAM_FLOAT( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )

    const vec3 value = *curvalue;
    if ( ImGui::SliderFloat3( label->c_str(), (float *)curvalue, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( vec3 ) = *curvalue;
    } else {
        GET_RETURN_DATA( vec3 ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiSliderVec4 ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_OBJ( vec4, curvalue )
    CALLBACK_PARAM_FLOAT( min )
    CALLBACK_PARAM_FLOAT( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )
    
    const vec4 value = *curvalue;
    if ( ImGui::SliderFloat4( label->c_str(), (float *)curvalue, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( vec4 ) = *curvalue;
    } else {
        GET_RETURN_DATA( vec4 ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiSliderAngle ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_FLOAT( curvalue )
    CALLBACK_PARAM_FLOAT( min )
    CALLBACK_PARAM_FLOAT( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )
    
    const float value = curvalue;
    if ( ImGui::SliderAngle( label->c_str(), &curvalue, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( float ) = curvalue;
    } else {
        GET_RETURN_DATA( float ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiBeginTable ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_I32( numColumns )
    CALLBACK_PARAM_U32( flags )
    RETURN_BOOL( ImGui::BeginTable( label->c_str(), numColumns, flags ) )
}
SCRIPT_CALLBACK_DEF( ImGuiEndTable ) {
    CALLBACK_BEGIN( 0 )
    ImGui::EndTable();
}
SCRIPT_CALLBACK_DEF( ImGuiBeginChild ) {
}
SCRIPT_CALLBACK_DEF( ImGuiEndChild ) {
}
SCRIPT_CALLBACK_DEF( ImGuiDragInt ) {
}
SCRIPT_CALLBACK_DEF( ImGuiDragFloat ) {
}
SCRIPT_CALLBACK_DEF( ImGuiColorEdit3 ) {
}
SCRIPT_CALLBACK_DEF( ImGuiColorEdit4 ) {
}

//==============================================================
// TheNomad::Engine
//

SCRIPT_CALLBACK_DEF( CvarRegister ) {
    
}
SCRIPT_CALLBACK_DEF( CvarUpdate ) {
    
}
SCRIPT_CALLBACK_DEF( CvarVariableInteger ) {
    
}
SCRIPT_CALLBACK_DEF( CvarVariableFloat ) {
    
}
SCRIPT_CALLBACK_DEF( CvarVariableString ) {
    
}
SCRIPT_CALLBACK_DEF( CvarSet ) {
    
}
SCRIPT_CALLBACK_DEF( CvarSetIntegerValue ) {
    
}
SCRIPT_CALLBACK_DEF( CvarSetFloatValue ) {
    
}
SCRIPT_CALLBACK_DEF( CvarSetStringValue ) {
    
}

//==============================================================
// TheNomad::Engine::SoundSystem
//

SCRIPT_CALLBACK_DEF( RegisterSfx )
SCRIPT_CALLBACK_DEF( RegisterTrack )
SCRIPT_CALLBACK_DEF( PlaySfx )
SCRIPT_CALLBACK_DEF( SetLoopingTrack )
SCRIPT_CALLBACK_DEF( ClearLoopingTrack )

//==============================================================
// TheNomad::Engine::FileSystem
//

SCRIPT_CALLBACK_DEF( OpenFileRead )
SCRIPT_CALLBACK_DEF( OpenFileWrite )
SCRIPT_CALLBACK_DEF( OpenFileRW )
SCRIPT_CALLBACK_DEF( OpenFileAppend )
SCRIPT_CALLBACK_DEF( OpenFileMode )
SCRIPT_CALLBACK_DEF( CloseFile )
SCRIPT_CALLBACK_DEF( LoadFile )
SCRIPT_CALLBACK_DEF( GetFileLength )
SCRIPT_CALLBACK_DEF( GetFilePosition )
SCRIPT_CALLBACK_DEF( SetFilePosition )
SCRIPT_CALLBACK_DEF( ListFiles )
SCRIPT_CALLBACK_DEF( WriteInt8 )
SCRIPT_CALLBACK_DEF( WriteInt16 )
SCRIPT_CALLBACK_DEF( WriteInt32 )
SCRIPT_CALLBACK_DEF( WriteInt64 )
SCRIPT_CALLBACK_DEF( WriteUInt8 )
SCRIPT_CALLBACK_DEF( WriteUInt16 )
SCRIPT_CALLBACK_DEF( WriteUInt32 )
SCRIPT_CALLBACK_DEF( WriteUInt64 )
SCRIPT_CALLBACK_DEF( WriteString )
SCRIPT_CALLBACK_DEF( WriteArray )
SCRIPT_CALLBACK_DEF( ReadInt8 )
SCRIPT_CALLBACK_DEF( ReadInt16 )
SCRIPT_CALLBACK_DEF( ReadInt32 )
SCRIPT_CALLBACK_DEF( ReadInt64 )
SCRIPT_CALLBACK_DEF( ReadUInt8 )
SCRIPT_CALLBACK_DEF( ReadUInt16 )
SCRIPT_CALLBACK_DEF( ReadUInt32 )
SCRIPT_CALLBACK_DEF( ReadUInt64 )
SCRIPT_CALLBACK_DEF( ReadString )
SCRIPT_CALLBACK_DEF( ReadArray )

//==============================================================
// TheNomad::Engine::Renderer
//

SCRIPT_CALLBACK_DEF( DrawImage )
SCRIPT_CALLBACK_DEF( AddSpriteToScene )
SCRIPT_CALLBACK_DEF( AddPolyToScene )
SCRIPT_CALLBACK_DEF( AddDLightToScene )
SCRIPT_CALLBACK_DEF( AddEntityToScene )
SCRIPT_CALLBACK_DEF( ClearScene )
SCRIPT_CALLBACK_DEF( RenderScene )
SCRIPT_CALLBACK_DEF( RegisterShader )
SCRIPT_CALLBACK_DEF( RegisterSpriteSheet )
SCRIPT_CALLBACK_DEF( RegisterSprite )

//==============================================================
// TheNomad::GameSystem
//

SCRIPT_CALLBACK_DEF( BBoxAssign )

SCRIPT_CALLBACK_DEF( SetCameraPos )
SCRIPT_CALLBACK_DEF( GetString )
SCRIPT_CALLBACK_DEF( GetGPUConfig )

SCRIPT_CALLBACK_DEF( CastRay )
SCRIPT_CALLBACK_DEF( CheckWallHit )

SCRIPT_CALLBACK_DEF( LoadMap )
SCRIPT_CALLBACK_DEF( SetActiveMap )
SCRIPT_CALLBACK_DEF( GetSpawnData )
SCRIPT_CALLBACK_DEF( GetCheckpointData )
SCRIPT_CALLBACK_DEF( GetTileData )

//==============================================================
// TheNomad::Util
//

SCRIPT_CALLBACK_DEF( GetModuleList )
SCRIPT_CALLBACK_DEF( IsModuleActive )
SCRIPT_CALLBACK_DEF( StrICmp )
SCRIPT_CALLBACK_DEF( BoundsIntersect )

SCRIPT_CALLBACK_DEF( ConsolePrint )
SCRIPT_CALLBACK_DEF( GameError )