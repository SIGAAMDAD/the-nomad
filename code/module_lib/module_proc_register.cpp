#include "module_proc_def.h"

#define REGISTER_ENUM_TYPE( name ) \
    CheckASCall( pEngine->RegisterEnum( name ) )
#define REGISTER_ENUM_VALUE( type, decl, value ) \
    CheckASCall( pEngine->RegisterEnumValue( type, decl, value ) )
#define REGISTER_GLOBAL_VAR( decl, ptr ) \
    CheckASCall( pEngine->RegisterGlobalProperty( decl, (void *)ptr ) )
#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
    CheckASCall( pEngine->RegisterGlobalFunction( decl, asFUNCTION( ScriptLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_CLASS_METHOD( className, decl, funcPtr, params, retn) \
    CheckASCall( pEngine->RegisterObjectMethod( #className, decl, asFUNCTION( ScriptLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_CLASS_TYPE( className, size, flags ) \
    CheckASCall( pEngine->RegisterObjectType( #className, size, asGetTypeTraits<className>() | flags ) )
#define REGISTER_CLASS_PROPERTY( className, decl, offset ) \
    CheckASCall( pEngine->RegisterObjectProperty( className, decl, offset ) )
#define REGISTER_CLASS_BEHAVE( className, behave, decl, funcPtr, params, retn ) \
    CheckASCall( pEngine->RegisterObjectBehaviour( #className, behave, decl, asFUNCTION( ScriptLib_##funcPtr ), asCALL_GENERIC ) )

#define SET_NAMESPACE( name ) \
    CheckASCall( pEngine->SetDefaultNamespace( name ) )

static const nhandle_t script_FS_INVALID_HANDLE = FS_INVALID_HANDLE;

static void ModuleLib_Register_Globals( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
    g_pModuleLib->GetScriptBuilder()->DefineWord( "FS_INVALID_HANDLE -1" );
}

static void ModuleLib_Register_FileSystem( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();

    SET_NAMESPACE( "TheNomad::Engine::FileSystem" );

    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRead( const string& in )", ModuleOpenFileRead );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileWrite( const string& in )", ModuleOpenFileWrite );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileAppend( const string& in )", ModuleOpenFileAppend );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRW( const string& in )", ModuleOpenFileRW );
    REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::OpenFile( const string& in, int, int& out )", ModuleOpenFile );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::FileSystem::CloseFile( int )", CloseFile );
    REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetLength( int )", GetFileLength );
    REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetPosition( int )", GetFilePosition );
    REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::SetPosition( int, uint64, uint )", SetFilePosition );
    REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::LoadFile( const string& in, array<int8>& out )", LoadFileBuffer );
    REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::LoadFile( const string& in, string& out )", LoadFileString );
    REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::FileExists( const string& in )", FileExists );
    REGISTER_GLOBAL_FUNCTION( "array<string>@ TheNomad::Engine::FileSystem::ListFiles()", ListFiles );
    REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::MakeDir( const string& in )", MakeDir );
    REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::RemoveDir( const string& in )", RemoveDir );
    REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::RemoveFile( const string& in )", RemoveFile );
    REGISTER_GLOBAL_FUNCTION( "string TheNomad::Engine::FileSystem::GetHomePath()", GetHomePath );
    REGISTER_GLOBAL_FUNCTION( "string TheNomad::Engine::FileSystem::GetBaseGameDir()", GetBaseGameDir );
    REGISTER_GLOBAL_FUNCTION( "string TheNomad::Engine::FileSystem::GetBasePath()", GetBasePath );
    REGISTER_GLOBAL_FUNCTION( "string TheNomad::Engine::FileSystem::GetGamePath()", GetGamePath );

    {
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt8( char, int )", WriteInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt16( int16, int )", WriteInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt32( int32, int )", WriteInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt64( int64, int )", WriteInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt8( char, int )", WriteUInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt16( uint16, int )", WriteUInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt32( uint32, int )", WriteUInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt64( uint64, int )", WriteUInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteChar( char, int )", WriteInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteShort( int16, int )", WriteInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteInt( int32, int )", WriteInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteLong( int64, int )", WriteInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteByte( char, int )", WriteUInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUShort( uint16, int )", WriteUInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteUInt( uint32, int )", WriteUInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteULong( uint64, int )", WriteUInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::WriteString( const string& in, int )", WriteString );
    }
    {
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt8( char, int )", ReadInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt16( int16, int )", ReadInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt32( int32, int )", ReadInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt64( int64, int )", ReadInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt8( char, int )", ReadUInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt16( uint16, int  )", ReadUInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt32( uint32, int  )", ReadUInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt64( uint64, int  )", ReadUInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadChar( char, int )", ReadInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadShort( int16, int )", ReadInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadInt( int32, int )", ReadInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadLong( int64, int )", ReadInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadByte( char, int )", ReadUInt8 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUShort( uint16, int )", ReadUInt16 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadUInt( uint32, int )", ReadUInt32 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadULong( uint64, int )", ReadUInt64 );
        REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Engine::FileSystem::ReadString( string& out, int )", ReadString );
    }
}

static void ModuleLib_Register_Cvar( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

static void ModuleLib_Register_Cmd( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

static void ModuleLib_Register_Game( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

static void ModuleLib_Register_ImGui( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

void ModuleLib_Register_Engine( void )
{
}
