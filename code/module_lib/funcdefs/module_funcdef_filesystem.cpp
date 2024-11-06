#include "../module_public.h"
#include "module_funcdefs.h"

template<typename Type>
static uint32_t ScriptLib_FS_Write( Type data, fileHandle_t hFile )
{
	return FS_Write( &data, sizeof( data ), hFile );
}

template<typename Type>
static uint32_t ScriptLib_FS_Read( Type *data, fileHandle_t hFile )
{
	return FS_Read( data, sizeof( *data ), hFile );
}

static uint32_t ScriptLib_FS_WriteString( const string_t *data, fileHandle_t hFile )
{
	uint32_t nSize;

	nSize = data->size();
	if ( !FS_Write( &nSize, sizeof( nSize ), hFile ) ) {
		return 0;
	}

	return FS_Write( data->data(), nSize, hFile );
}

static uint32_t ScriptLib_FS_ReadString( string_t *data, fileHandle_t hFile )
{
	uint32_t nSize;

	if ( !FS_Read( &nSize, sizeof( nSize ), hFile ) ) {
		return 0;
	}

	data->resize( nSize );
	return FS_Read( data->data(), nSize, hFile );
}

static fileHandle_t ScriptLib_FS_FOpenRead( const string_t& fileName )
{
	return FS_FOpenRead( fileName.c_str() );
}

static fileHandle_t ScriptLib_FS_FOpenWrite( const string_t& fileName )
{
	return FS_FOpenWrite( fileName.c_str() );
}

static fileHandle_t ScriptLib_FS_FOpenAppend( const string_t& fileName )
{
	return FS_FOpenAppend( fileName.c_str() );
}

static void ScriptLib_FS_WriteFile( const string_t& fileName, const CScriptArray *pBuffer )
{
	FS_WriteFile( fileName.c_str(), pBuffer->GetBuffer(), pBuffer->GetSize() );
}

void ScriptLib_Register_FileSystem( void )
{
	SET_NAMESPACE( "TheNomad::Engine::FileSystem" );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRead( const string& in )", asFUNCTION( ScriptLib_FS_FOpenRead ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileWrite( const string& in )", asFUNCTION( FS_FOpenWrite ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::FileSystem::CloseFile( int )", asFUNCTION( FS_FClose ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteInt8( int8, int )", asFUNCTION( ScriptLib_FS_Write<int8_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteInt16( int16, int )", asFUNCTION( ScriptLib_FS_Write<int16_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteInt32( int32, int )", asFUNCTION( ScriptLib_FS_Write<int32_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteInt64( int64, int )", asFUNCTION( ScriptLib_FS_Write<int64_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteUInt8( uint8, int )", asFUNCTION( ScriptLib_FS_Write<uint8_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteUInt16( uint16, int )", asFUNCTION( ScriptLib_FS_Write<uint16_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteUInt32( uint32, int )", asFUNCTION( ScriptLib_FS_Write<uint32_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteUInt64( uint64, int )", asFUNCTION( ScriptLib_FS_Write<uint64_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteFloat( float, int )", asFUNCTION( ScriptLib_FS_Write<float> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteDouble( double, int )", asFUNCTION( ScriptLib_FS_Write<double> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::WriteString( const string& in, int )", asFUNCTION( ScriptLib_FS_WriteString ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadInt8( int8& out, int )", asFUNCTION( ScriptLib_FS_Read<int8_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadInt16( int16& out, int )", asFUNCTION( ScriptLib_FS_Read<int16_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadInt32( int32& out, int )", asFUNCTION( ScriptLib_FS_Read<int32_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadInt64( int64& out, int )", asFUNCTION( ScriptLib_FS_Read<int64_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadUInt8( uint8& out, int )", asFUNCTION( ScriptLib_FS_Read<uint8_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadUInt16( uint16& out, int )", asFUNCTION( ScriptLib_FS_Read<uint16_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadUInt32( uint32& out, int )", asFUNCTION( ScriptLib_FS_Read<uint32_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadUInt64( uint64& out, int )", asFUNCTION( ScriptLib_FS_Read<uint64_t> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadUInt32( float& out, int )", asFUNCTION( ScriptLib_FS_Read<float> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadUInt64( double& out, int )", asFUNCTION( ScriptLib_FS_Read<double> ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint TheNomad::Engine::FileSystem::ReadString( string& out, int )", asFUNCTION( ScriptLib_FS_ReadString ), asCALL_CDECL );
}