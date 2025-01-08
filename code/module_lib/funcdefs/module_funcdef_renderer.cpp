#include "../module_public.h"
#include "../module_funcdefs.hpp"
#include "module_funcdefs.h"

static void RenderScene( asDWORD x, asDWORD y, asDWORD width, asDWORD height, asDWORD flags, asDWORD time )
{
	renderSceneRef_t refdef;

	refdef.x = x;
	refdef.y = y;
	refdef.width = width;
	refdef.height = height;
	refdef.flags = flags;
	refdef.time = time;

	re.RenderScene( &refdef );
}

static void AddEntityToScene( nhandle_t sheetNum, nhandle_t spriteId, const vec3& origin, float shaderTime, float radius, float rotation,
	const vec2& scale )
{
	refEntity_t refEntity;

	refEntity.sheetNum = sheetNum;
	refEntity.spriteId = spriteId;
	memcpy( refEntity.origin, &origin, sizeof( vec3_t ) );

	refEntity.radius = radius;
	refEntity.rotation = rotation;
	memcpy( refEntity.scale, &scale, sizeof( vec2_t ) );

	re.AddEntityToScene( &refEntity );
}

nhandle_t RegisterShader( const string_t *name )
{
	return re.RegisterShader( name->c_str() );
}

nhandle_t RegisterSpriteSheet( const string_t *name, asDWORD sheetWidth, asDWORD sheetHeight, asDWORD spriteWidth, asDWORD spriteHeight )
{
	return re.RegisterSpriteSheet( name->c_str(), sheetWidth, sheetHeight, spriteWidth, spriteHeight );
}

void ScriptLib_Register_Renderer( void )
{
	SET_NAMESPACE( "TheNomad::Engine::Renderer" );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::ClearScene()", asFUNCTION( re.ClearScene ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::AddEntityToScene( int, int, const vec3& in, float, float, float, const vec2& in )",
		asFUNCTION( AddEntityToScene ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RenderScene( uint, uint, uint, uint )", asFUNCTION( RenderScene ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::AddDLightToScene( const vec3& in origin, float, const vec3& in )",
		asFUNCTION( re.AddDynamicLightToScene ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RegisterShader( const string& in )", asFUNCTION( RegisterShader ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RegisterSpriteSheet( const string& in, uint, uint, uint, uint )",
		asFUNCTION( RegisterSpriteSheet ), asCALL_CDECL );

	RESET_NAMESPACE();
}