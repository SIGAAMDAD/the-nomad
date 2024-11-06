#include "../module_public.h"
#include "../../game/g_world.h"
#include "module_funcdefs.h"

void ScriptLib_Register_Game( void )
{
	SET_NAMESPACE( "TheNomad::GameSystem" );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetCheckpointData( uvec3& out, uvec2& out, uint )",
		asFUNCTION( G_GetCheckpointData ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetSpawnData( uvec3& out, uint& out, uint& out, uint, uint& out )",
		asFUNCTION( G_GetSpawnData ), asCALL_CDECL );

	RESET_NAMESPACE();
}