#include "../engine/n_shared.h"
#include <glm/gtc/type_ptr.hpp>
#include "module_proc_def.h"
#include "module_funcdefs.hpp"
#include "../ui/ui_public.hpp"
#include "../ui/ui_string_manager.h"

#include "module_engine/module_bbox.h"

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
    CALLBACK_BEGIN( 4 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_OBJ( const vec2, size )
    CALLBACK_PARAM_U32( childFlags )
    CALLBACK_PARAM_U32( windowFlags )
    RETURN_BOOL( ImGui::BeginChild( ImGui::GetID( label->c_str() ), ImVec2( size->x, size->y ), childFlags, windowFlags ) )
}
SCRIPT_CALLBACK_DEF( ImGuiEndChild ) {
    CALLBACK_BEGIN( 0 )
    ImGui::EndChild();
}
SCRIPT_CALLBACK_DEF( ImGuiDragInt ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_I32( curvalue )
    CALLBACK_PARAM_I32( min )
    CALLBACK_PARAM_I32( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )

    const int value = curvalue;
    if ( ImGui::DragInt( label->c_str(), &curvalue, 1.0f, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( int ) = curvalue;
    } else {
        GET_RETURN_DATA( int ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiDragFloat ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_FLOAT( curvalue )
    CALLBACK_PARAM_FLOAT( min )
    CALLBACK_PARAM_FLOAT( max )
    CALLBACK_PARAM_STRING( format )
    CALLBACK_PARAM_U32( flags )

    const float value = curvalue;
    if ( ImGui::DragFloat( label->c_str(), &curvalue, 1.0f, min, max, format->c_str(), flags ) ) {
        GET_RETURN_DATA( float ) = curvalue;
    } else {
        GET_RETURN_DATA( float ) = value;
    }
}
SCRIPT_CALLBACK_DEF( ImGuiColorEdit3 ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_OBJ( vec3, curvalue )
    CALLBACK_PARAM_U32( flags )

    const vec3& value = *curvalue;
    if ( ImGui::ColorEdit3( label->c_str(), (float *)glm::value_ptr( *curvalue ), flags ) ) {
        CONSTRUCT_OBJECT( vec3, *curvalue );
    } else {
        CONSTRUCT_OBJECT( vec3, value );
    }
}
SCRIPT_CALLBACK_DEF( ImGuiColorEdit4 ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_STRING( label )
    CALLBACK_PARAM_OBJ( vec4, curvalue )
    CALLBACK_PARAM_U32( flags )

    const vec4& value = *curvalue;
    if ( ImGui::ColorEdit4( label->c_str(), (float *)glm::value_ptr( *curvalue ), flags ) ) {
        CONSTRUCT_OBJECT( vec4, *curvalue );
    } else {
        CONSTRUCT_OBJECT( vec4, value );
    }
}
SCRIPT_CALLBACK_DEF( ImGuiPushStyleColor ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_U32( index )
    CALLBACK_PARAM_OBJ( const vec4, color )
    ImGui::PushStyleColor( index, (const float *)glm::value_ptr( *color ) );
}
SCRIPT_CALLBACK_DEF( ImGuiPopStyleColor ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( count )
    ImGui::PopStyleColor( count );
}
SCRIPT_CALLBACK_DEF( ImGuiPushStyleVar ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_U32( index )
    CALLBACK_PARAM_OBJ( const vec2, style )
    ImGui::PushStyleVar( index, ImVec2( style->x, style->y ) );
}
SCRIPT_CALLBACK_DEF( ImGuiPopStyleVar ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( count )
    ImGui::PopStyleVar( count );
}

//==============================================================
// TheNomad::Engine
//

SCRIPT_CALLBACK_DEF( CvarRegister ) {
    vmCvar_t vmCvar;

    CALLBACK_BEGIN( 7 )
    CALLBACK_PARAM_STRING( name )
    CALLBACK_PARAM_OBJ( string_t, value )
    CALLBACK_PARAM_U32( flags )
    CALLBACK_PARAM_PTR( asINT32, intValue )
    CALLBACK_PARAM_PTR( float, floatValue )
    CALLBACK_PARAM_PTR( asINT32, modificationCount )
    CALLBACK_PARAM_PTR( cvarHandle_t, cvarHandle )

    Cvar_Register( &vmCvar, name->c_str(), value->c_str(), flags, CVAR_PRIVATE );

    *value = (const char *)vmCvar.s;
    *intValue = vmCvar.i;
    *floatValue = vmCvar.f;
    *modificationCount = vmCvar.modificationCount;
    *cvarHandle = vmCvar.handle;
}
SCRIPT_CALLBACK_DEF( CvarUpdate ) {
    vmCvar_t vmCvar;

    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_OBJ( string_t, value )
    CALLBACK_PARAM_U32( flags )
    CALLBACK_PARAM_PTR( asINT32, intValue )
    CALLBACK_PARAM_PTR( float, floatValue )
    CALLBACK_PARAM_PTR( asINT32, modificationCount )
    CALLBACK_PARAM_I32( cvarHandle )

    memset( &vmCvar, 0, sizeof( vmCvar ) );
    N_strncpyz( vmCvar.s, value->c_str(), sizeof( vmCvar.s ) - 1 );
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
SCRIPT_CALLBACK_DEF( CvarVariableInteger ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( name )
    RETURN_I32( Cvar_VariableInteger( name->c_str() ) )
}
SCRIPT_CALLBACK_DEF( CvarVariableFloat ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( name )
    RETURN_FLOAT( Cvar_VariableFloat( name->c_str() ) )
}
SCRIPT_CALLBACK_DEF( CvarVariableString ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( name )
    CONSTRUCT_OBJECT( string_t, Cvar_VariableString( name->c_str() ) )
}
SCRIPT_CALLBACK_DEF( CvarSet ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( name )
    CALLBACK_PARAM_STRING( value )
    Cvar_Set( name->c_str(), value->c_str() );
}
SCRIPT_CALLBACK_DEF( CvarSetIntegerValue ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( name )
    CALLBACK_PARAM_I32( value )
    Cvar_SetIntegerValue( name->c_str(), value );
}
SCRIPT_CALLBACK_DEF( CvarSetFloatValue ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( name )
    CALLBACK_PARAM_FLOAT( value )
    Cvar_SetFloatValue( name->c_str(), value );
}
SCRIPT_CALLBACK_DEF( CvarSetStringValue ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( name )
    CALLBACK_PARAM_STRING( value )
    Cvar_SetStringValue( name->c_str(), value->c_str() );
}
SCRIPT_CALLBACK_DEF( CmdArgc ) {
    RETURN_U32( Cmd_Argc() )
}
SCRIPT_CALLBACK_DEF( CmdArgv ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_U32( nIndex )
    CONSTRUCT_OBJECT( string_t, Cmd_Argv( nIndex ) )
}
SCRIPT_CALLBACK_DEF( CmdArgs ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_U32( nIndex )
    CONSTRUCT_OBJECT( string_t, Cmd_ArgsFrom( nIndex ) )
}
SCRIPT_CALLBACK_DEF( CmdAddCommand ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( name )
    Cmd_AddCommand( name->c_str(), NULL );
}
SCRIPT_CALLBACK_DEF( CmdRemoveCommand ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( name )
    Cmd_RemoveCommand( name->c_str() );
}
SCRIPT_CALLBACK_DEF( CmdExecuteCommand ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( name )
    Cbuf_ExecuteText( EXEC_APPEND, name->c_str() );
}
SCRIPT_CALLBACK_DEF( KeyGetBinding ) {

}

//==============================================================
// TheNomad::Engine::SoundSystem
//

SCRIPT_CALLBACK_DEF( RegisterSfx ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( Snd_RegisterSfx( npath->c_str() ) )
}
SCRIPT_CALLBACK_DEF( RegisterTrack ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( Snd_RegisterTrack( npath->c_str() ) )
}
SCRIPT_CALLBACK_DEF( PlaySfx ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( hSfx )
    Snd_PlaySfx( hSfx );
}
SCRIPT_CALLBACK_DEF( SetLoopingTrack ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( hTrack )
    Snd_SetLoopingTrack( hTrack );
}
SCRIPT_CALLBACK_DEF( ClearLoopingTrack ) {
    CALLBACK_BEGIN( 0 )
    Snd_ClearLoopingTrack();
}
SCRIPT_CALLBACK_DEF( AddLoopingTrack ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( hTrack )
    Snd_AddLoopingTrack( hTrack );
}
SCRIPT_CALLBACK_DEF( ClearLoopingTracks ) {
    CALLBACK_BEGIN( 0 )
    Snd_ClearLoopingTracks();
}

//==============================================================
// TheNomad::Engine::FileSystem
//

SCRIPT_CALLBACK_DEF( ModuleOpenFileRead ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( FS_VM_FOpenRead( npath->c_str(), H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ModuleOpenFileWrite ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( FS_VM_FOpenWrite( npath->c_str(), H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ModuleOpenFileRW ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( FS_VM_FOpenRW( npath->c_str(), H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ModuleOpenFileAppend ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( FS_VM_FOpenAppend( npath->c_str(), H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ModuleOpenFileMode ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_STRING( npath )
    CALLBACK_PARAM_PTR( asINT32, file )
    CALLBACK_PARAM_I32( mode )
    RETURN_U64( FS_VM_FOpenFile( npath->c_str(), (fileHandle_t *)file, (fileMode_t)mode, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( CloseFile ) {
    CALLBACK_BEGIN( 0 )
    CALLBACK_PARAM_I32( hFile )
    FS_VM_FClose( hFile, H_SGAME );
}
SCRIPT_CALLBACK_DEF( LoadFileBuffer ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( npath )
    CALLBACK_PARAM_PTR( CScriptArray, buffer )

    void *v;
    uint64_t length;

    length = FS_LoadFile( npath->c_str(), &v );
    if ( !length || !v ) {
        Con_Printf( COLOR_RED "Error loading file '%s' at vm request\n", npath->c_str() );
        RETURN_U64( 0 )
        return;
    }

    buffer->Resize( length );
    memcpy( buffer->GetBuffer(), v, length );

    FS_FreeFile( v );

    RETURN_U64( length )
}
SCRIPT_CALLBACK_DEF( LoadFileString ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( npath )
    CALLBACK_PARAM_PTR( string_t, buffer )

    void *v;
    uint64_t length;
    
    length = FS_LoadFile( npath->c_str(), &v );
    if ( !length || !v ) {
        Con_Printf( COLOR_RED "Error loading file '%s' at vm request\n", npath->c_str() );
        RETURN_U64( 0 )
        return;
    }

    buffer->resize( length );
    memcpy( buffer->data(), v, length );

    FS_FreeFile( v );

    RETURN_U64( length )
}
SCRIPT_CALLBACK_DEF( GetFileLength ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( hFile )
    RETURN_U64( FS_VM_FileLength( hFile, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( GetFilePosition ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( hFile )
    RETURN_U64( FS_VM_FileTell( hFile, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( SetFilePosition ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_U64( offset )
    CALLBACK_PARAM_U32( whence )
    RETURN_U64( FS_VM_FileSeek( hFile, offset, whence, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ListFiles ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( path )
    CALLBACK_PARAM_STRING( ext )

    uint64_t numFiles, i;
    CScriptArray *returnList = CScriptArray::Create( g_pModuleLib->GetScriptEngine()->GetTypeInfoByName( "array" ) );
    char **fileList = FS_ListFiles( path->c_str(), ext->c_str(), &numFiles );

    returnList->Resize( numFiles );
    for ( i = 0; i < numFiles; i++ ) {
        new ( returnList->At( i ) ) string_t( fileList[i] );
    }

    GET_RETURN_PTR( CScriptArray ) = returnList;
}
SCRIPT_CALLBACK_DEF( WriteInt8 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_I8( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteInt16 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_I16( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteInt32 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_I32( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteInt64 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_I64( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteUInt8 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_U8( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteUInt16 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_U16( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteUInt32 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_U32( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteUInt64 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_U64( data )
    RETURN_U64( FS_VM_Write( &data, sizeof( data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( WriteString ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_STRING( data )

    uint64_t length = data->size();
    if ( !FS_VM_Write( &length, sizeof( length ), hFile, H_SGAME ) ) {
        RETURN_U64( 0 )
        return;
    }

    RETURN_U64( FS_VM_Write( data->c_str(), length, hFile, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( WriteArray ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( CScriptArray, data )

    const asUINT length = data->GetSize();
    if ( !FS_VM_Write( &length, sizeof( length ), hFile, H_SGAME ) ) {
        RETURN_U64( 0 )
        return;
    }

    const asUINT dataSize = g_pModuleLib->GetScriptEngine()->GetTypeInfoById( data->GetElementTypeId() )->GetSize();
    RETURN_U64( FS_VM_Write( data->GetBuffer(), length * dataSize, hFile, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ReadInt8 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( int8_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadInt16 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( int16_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadInt32 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( int32_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadInt64 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( int64_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadUInt8 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( uint8_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadUInt16 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( uint16_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadUInt32 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( uint32_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadUInt64 ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( uint64_t, data )
    RETURN_U64( FS_VM_Read( data, sizeof( *data ), hFile, H_SGAME ) );
}
SCRIPT_CALLBACK_DEF( ReadString ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_PTR( string_t, data )

    uint64_t length;
    if ( !FS_VM_Read( &length, sizeof( length ), hFile, H_SGAME ) ) {
        RETURN_U64( 0 )
        return;
    }

    data->resize( length );
    RETURN_U64( FS_VM_Read( data->data(), length, hFile, H_SGAME ) )
}
SCRIPT_CALLBACK_DEF( ReadArray ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hFile )
    CALLBACK_PARAM_OBJ( CScriptArray, data )

    asUINT length, dataSize;

    if ( !FS_VM_Read( &length, sizeof( length ), hFile, H_SGAME ) ) {
        RETURN_U64( 0 )
        return;
    }

    data->Resize( length );
    RETURN_U64( FS_VM_Read( data->GetBuffer(), length * dataSize, hFile, H_SGAME ) )
}

//==============================================================
// TheNomad::Engine::Renderer
//

SCRIPT_CALLBACK_DEF( DrawImage ) {
    CALLBACK_BEGIN( 4 )
    CALLBACK_PARAM_FLOAT( x )
    CALLBACK_PARAM_FLOAT( y )
    CALLBACK_PARAM_FLOAT( w )
    CALLBACK_PARAM_FLOAT( h )
    CALLBACK_PARAM_FLOAT( u1 )
    CALLBACK_PARAM_FLOAT( v1 )
    CALLBACK_PARAM_FLOAT( u2 )
    CALLBACK_PARAM_FLOAT( v2 )
    CALLBACK_PARAM_I32( hShader )
    re.DrawImage( x, y, w, h, u1, v1, u2, v2, hShader );
}
SCRIPT_CALLBACK_DEF( AddSpriteToScene ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_OBJ( const vec3, origin )
    CALLBACK_PARAM_I32( hSpriteSheet )
    CALLBACK_PARAM_I32( hSprite )
    CALLBACK_PARAM_BOOL( bNoSpriteSheet )
    re.AddSpriteToScene( (const vec_t *)glm::value_ptr( *origin ), hSpriteSheet, hSprite, bNoSpriteSheet );
}
SCRIPT_CALLBACK_DEF( AddPolyToScene ) {
}
SCRIPT_CALLBACK_DEF( AddDLightToScene ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_OBJ( const vec3, origin )
    CALLBACK_PARAM_FLOAT( brightness )
    CALLBACK_PARAM_OBJ( const vec3, color )

    re.AddDynamicLightToScene( (const vec_t *)glm::value_ptr( *origin ), brightness, (const vec_t *)glm::value_ptr( *color ) );
}
SCRIPT_CALLBACK_DEF( AddEntityToScene ) {
    refEntity_t refEntity;

    CALLBACK_BEGIN( 12 )
    CALLBACK_PARAM_I32( sheetNum )
    CALLBACK_PARAM_I32( spriteId )
    CALLBACK_PARAM_I32( renderfx )
    CALLBACK_PARAM_OBJ( const vec3, lightingOrigin )
    CALLBACK_PARAM_OBJ( const vec3, origin )
    CALLBACK_PARAM_U32( frame )
    CALLBACK_PARAM_U32( flags )
    CALLBACK_PARAM_U32( color )
    CALLBACK_PARAM_OBJ( const vec2, shaderTexCoord )
    CALLBACK_PARAM_FLOAT( shaderTime )
    CALLBACK_PARAM_FLOAT( rotation )
    CALLBACK_PARAM_FLOAT( scale )

    memset( &refEntity, 0, sizeof( refEntity ) );
    VectorCopy( refEntity.origin, *origin );
    VectorCopy( refEntity.lightingOrigin, *lightingOrigin );
    VectorCopy2( refEntity.shaderTexCoord, *shaderTexCoord );
    refEntity.sheetNum = sheetNum;
    refEntity.spriteId = spriteId;
    refEntity.renderfx = renderfx;
    refEntity.frame = frame;
    refEntity.flags = flags;
    refEntity.rotation = rotation;
    refEntity.scale = scale;
    refEntity.shaderTime.f = shaderTime;

    re.AddEntityToScene( &refEntity );
}
SCRIPT_CALLBACK_DEF( ClearScene ) {
    re.ClearScene();
}
SCRIPT_CALLBACK_DEF( RenderScene ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_U32( x )
    CALLBACK_PARAM_U32( y )
    CALLBACK_PARAM_U32( width )
    CALLBACK_PARAM_U32( height )
    CALLBACK_PARAM_U32( flags )
    CALLBACK_PARAM_U32( time )

    renderSceneRef_t refdef;
    
    memset( &refdef, 0, sizeof( refdef ) );
    refdef.x = x;
    refdef.y = y;
    refdef.width = width;
    refdef.height = height;
    refdef.flags = flags;
    refdef.time = time;

    re.RenderScene( &refdef );
}
SCRIPT_CALLBACK_DEF( RegisterShader ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( npath )
    RETURN_I32( re.RegisterShader( npath->c_str() ) )
}
SCRIPT_CALLBACK_DEF( RegisterSpriteSheet ) {
    CALLBACK_BEGIN( 5 )
    CALLBACK_PARAM_STRING( npath )
    CALLBACK_PARAM_U32( sheetWidth )
    CALLBACK_PARAM_U32( sheetHeight )
    CALLBACK_PARAM_U32( spriteWidth )
    CALLBACK_PARAM_U32( spriteHeight )
    RETURN_I32( re.RegisterSpriteSheet( npath->c_str(), sheetWidth, sheetHeight, spriteWidth, spriteHeight ) )
}
SCRIPT_CALLBACK_DEF( RegisterSprite ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_I32( hSpriteSheet )
    CALLBACK_PARAM_U32( hSprite )
    RETURN_I32( re.RegisterSprite( hSpriteSheet, hSprite ) )
}

//==============================================================
// TheNomad::GameSystem
//

SCRIPT_CALLBACK_DEF( CastRay ) {

}
SCRIPT_CALLBACK_DEF( CheckWallHit ) {

}

SCRIPT_CALLBACK_DEF( LoadMap ) {

}

SCRIPT_CALLBACK_DEF( SetActiveMap ) {
    CALLBACK_BEGIN( 6 )
    CALLBACK_PARAM_I32( hMap )
    CALLBACK_PARAM_PTR( uint32_t, nCheckpoints )
    CALLBACK_PARAM_PTR( uint32_t, nSpawns )
    CALLBACK_PARAM_PTR( uint32_t, nTiles )
    CALLBACK_PARAM_PTR( int32_t, nWidth )
    CALLBACK_PARAM_PTR( int32_t, nHeight )

    G_SetActiveMap( hMap, nCheckpoints, nSpawns, nTiles, nWidth, nHeight );
}

SCRIPT_CALLBACK_DEF( GetSpawnData ) {
}
SCRIPT_CALLBACK_DEF( GetCheckpointData ) {
}
SCRIPT_CALLBACK_DEF( GetTileData ) {
}

// bounding box
SCRIPT_CALLBACK_DEF( BBoxAssign ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )
    CONSTRUCT_OBJECT( CModuleBoundBox, *GET_OBJ( CModuleBoundBox ) = ( *bounds ) )
}
SCRIPT_CALLBACK_DEF( BBoxOpAddAssign ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )

    CModuleBoundBox *self = GET_OBJ( CModuleBoundBox );
    *self += *bounds;

    RETURN_PTR( self )
}
SCRIPT_CALLBACK_DEF( BBoxOpAdd ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )
    CONSTRUCT_OBJECT( CModuleBoundBox, *GET_OBJ( CModuleBoundBox ) + *bounds );
}
SCRIPT_CALLBACK_DEF( BBoxOpSubAssign ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )

    CModuleBoundBox *self = GET_OBJ( CModuleBoundBox );
    *self -= *bounds;

    RETURN_PTR( self )
}
SCRIPT_CALLBACK_DEF( BBoxOpSub ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )
    CONSTRUCT_OBJECT( CModuleBoundBox, *GET_OBJ( CModuleBoundBox ) - *bounds );
}
SCRIPT_CALLBACK_DEF( BBoxOpIndex ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_I32( index )
    CONSTRUCT_OBJECT( vec3, ( *GET_OBJ( CModuleBoundBox ) )[ index ] );
}
SCRIPT_CALLBACK_DEF( BBoxCompare ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )
    RETURN_BOOL( *GET_OBJ( CModuleBoundBox ) == ( *bounds ) )
}
SCRIPT_CALLBACK_DEF( BBoxIntersect ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bounds )
    RETURN_BOOL( GET_OBJ( CModuleBoundBox )->IntersectsBounds( *bounds ) )
}
SCRIPT_CALLBACK_DEF( BBoxIntersectPoint ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_OBJ( const vec3, point )
    RETURN_BOOL( GET_OBJ( CModuleBoundBox )->ContainsPoint( *point ) )
}
SCRIPT_CALLBACK_DEF( BBoxIntersectRay ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_OBJ( const vec3, point )
    CALLBACK_PARAM_OBJ( const vec3, dir )
    CALLBACK_PARAM_FLOAT( scale )
    RETURN_BOOL( GET_OBJ( CModuleBoundBox )->RayIntersection( *point, *dir, scale ) )
}
SCRIPT_CALLBACK_DEF( BBoxIntersectLine ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_OBJ( const vec3, start )
    CALLBACK_PARAM_OBJ( const vec3, end )
    RETURN_BOOL( GET_OBJ( CModuleBoundBox )->LineIntersection( *start, *end ) )
}

//==============================================================
// TheNomad::Util
//


SCRIPT_CALLBACK_DEF( GetString ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_STRING( name )
    CALLBACK_PARAM_OBJ( string_t, value )
    
    const stringHash_t *hash = strManager->ValueForKey( name->c_str() );
    *value = hash->value;
}

SCRIPT_CALLBACK_DEF( GetGPUConfig ) {

}

SCRIPT_CALLBACK_DEF( GetModuleList ) {

}
SCRIPT_CALLBACK_DEF( IsModuleActive ) {

}
SCRIPT_CALLBACK_DEF( BoundsIntersect ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, a )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, b )

    const bbox_t a2 = a->ToPOD();
    const bbox_t b2 = b->ToPOD();
    
    RETURN_BOOL( BoundsIntersect( &a2, &b2 ) );
}
SCRIPT_CALLBACK_DEF( BoundsIntersectSphere ) {
    CALLBACK_BEGIN( 3 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bbox )
    CALLBACK_PARAM_OBJ( const vec3, point )
    CALLBACK_PARAM_FLOAT( radius )

    const bbox_t a = bbox->ToPOD();

    RETURN_BOOL( BoundsIntersectSphere( &a, (const vec_t *)glm::value_ptr( *point ), radius ) )
}
SCRIPT_CALLBACK_DEF( BoundsIntersectPoint ) {
    CALLBACK_BEGIN( 2 )
    CALLBACK_PARAM_OBJ( const CModuleBoundBox, bbox )
    CALLBACK_PARAM_OBJ( const vec3, point )

    const bbox_t a = bbox->ToPOD();

    RETURN_BOOL( BoundsIntersectPoint( &a, (const vec_t *)glm::value_ptr( *point ) ) )
}

SCRIPT_CALLBACK_DEF( ConsoleWarning ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( message )
    Con_Printf( COLOR_YELLOW "WARNING: %s", message->c_str() );
}
SCRIPT_CALLBACK_DEF( ConsolePrint ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( message )
    Con_Printf( "%s", message->c_str() );
}
SCRIPT_CALLBACK_DEF( GameError ) {
    CALLBACK_BEGIN( 1 )
    CALLBACK_PARAM_STRING( message )
    throw ModuleException( message->c_str() );
}

SCRIPT_CALLBACK_DEF( StrICmp ) {

}
SCRIPT_CALLBACK_DEF( StrICmpn ) {

}
SCRIPT_CALLBACK_DEF( StrCmp ) {

}
SCRIPT_CALLBACK_DEF( StrCmpn ) {

}
