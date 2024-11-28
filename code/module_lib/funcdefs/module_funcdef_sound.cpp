#include "module_funcdefs.h"
#include "../../sound/snd_local.h"
#include <glm/gtc/type_ptr.hpp>

static nhandle_t SndRegisterSfx( const string_t& npath )
{
	return Snd_RegisterSfx( npath.c_str() );
}

static nhandle_t SndRegisterTrack( const string_t& npath )
{
	return Snd_RegisterTrack( npath.c_str() );
}

static nhandle_t PushListener( uint32_t nEntityNumber )
{
	return s_SoundWorld->PushListener( nEntityNumber );
}

static void SetEmitterPosition( nhandle_t hEmitter, const glm::vec3& origin, float forward, float up, float speed )
{
	s_SoundWorld->SetEmitterPosition( hEmitter, glm::value_ptr( origin ), forward, up, speed );
}

static void SetEmitterVolume( nhandle_t hEmitter, float nVolume )
{
	s_SoundWorld->SetEmitterVolume( hEmitter, nVolume );
}

static void SetEmitterAudioMask( nhandle_t hEmitter, uint32_t nListenerMask )
{
	s_SoundWorld->SetEmitterAudioMask( hEmitter, nListenerMask );
}

static void PlayEmitterSound( nhandle_t hEmitter, float nVolume, uint32_t nListenerMask, sfxHandle_t hSfx )
{
	s_SoundWorld->PlayEmitterSound( hEmitter, nVolume, nListenerMask, hSfx );
}

static nhandle_t RegisterEmitter( uint32_t nEntityNumber )
{
	return s_SoundWorld->RegisterEmitter( nEntityNumber );
}

static void RemoveEmitter( nhandle_t hEmitter )
{
	s_SoundWorld->RemoveEmitter( hEmitter );
}

void ScriptLib_Register_Sound( void )
{
	SET_NAMESPACE( "TheNomad::Engine::SoundSystem" );

	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterSfx( const string& in npath )", asFUNCTION( SndRegisterSfx ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterTrack( const string& in npath )", asFUNCTION( SndRegisterTrack ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::PushListener( uint nEntityNumber )", asFUNCTION( PushListener ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::SetEmitterPosition( int hEmitter, const vec3& in origin, float forward, float up, float speed )",
		asFUNCTION( SetEmitterPosition ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::SetEmitterVolume( int hEmitter, float nVolume )", asFUNCTION( SetEmitterVolume ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::SetEmitterAudioMask( int hEmitter, uint nListenerMask )",
		asFUNCTION( SetEmitterAudioMask ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::PlayEmitterSound( int hEmitter, float nVolume, uint nListenerMask, int hSfx )",
		asFUNCTION( PlayEmitterSound ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterEmitter( uint nEntityNumber )", asFUNCTION( RegisterEmitter ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::RemoveEmitter( int hEmitter )", asFUNCTION( RemoveEmitter ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::PlaySfx( int )", asFUNCTION( Snd_PlaySfx ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::AddLoopingTrack( int )", asFUNCTION( Snd_AddLoopingTrack ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::ClearLoopingTracks()", asFUNCTION( Snd_ClearLoopingTracks ), asCALL_CDECL );

	RESET_NAMESPACE();
}