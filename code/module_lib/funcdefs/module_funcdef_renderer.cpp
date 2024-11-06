#include "../module_public.h"
#include "module_funcdefs.h"

static void ScriptLib_RenderScene()
{
	
}

void ScriptLib_Register_Renderer( void )
{
	SET_NAMESPACE( "TheNomad::Engine::Renderer" );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RenderScene(  )", asFUNCTION( re.RenderScene ), asCALL_CDECL );

	RESET_NAMESPACE();
}