#include "module_funcdefs.h"
#include "../module_engine/module_bbox.h"

static int StrCmp( const string_t& str1, const string_t& str2 )
{ return strcmp( str1.c_str(), str2.c_str() ); }
static int StrICmp( const string_t& str1, const string_t& str2 )
{ return N_stricmp( str1.c_str(), str2.c_str() ); }

static asIScriptObject *AllocateExternalScriptObject( const string_t& nameSpace, const string_t& name )
{
	asIScriptObject *pObject;
	asIScriptFunction *pFactory;
	asITypeInfo *pTypeInfo;
	char szFactoryName[ 1024 ];

	memset( szFactoryName, 0, sizeof( szFactoryName ) );
	snprintf( szFactoryName, sizeof( szFactoryName ) - 1, "%s::%s@ %s()", nameSpace.c_str(), name.c_str(), name.c_str() );

	pTypeInfo = g_pModuleLib->GetScriptModule()->GetTypeInfoByDecl( va( "%s::%s", nameSpace.c_str(), name.c_str() ) );
	if ( !pTypeInfo ) {
		Con_Printf( COLOR_RED "ERROR: invalid script class type \"%s::%s\"!\n", nameSpace.c_str(), name.c_str() );
		return NULL;
	}

	pFactory = g_pModuleLib->GetScriptModule()->GetFunctionByDecl( szFactoryName );
	if ( !pFactory ) {
		Con_Printf( COLOR_RED "ERROR: script class \"%s::%s\" has no default factory!\n", nameSpace.c_str(), name.c_str() );
		return NULL;
	}

	g_pModuleLib->GetScriptContext()->PushState();
	g_pModuleLib->GetScriptContext()->Prepare( pFactory );
	g_pModuleLib->GetScriptContext()->Execute();
	g_pModuleLib->GetScriptContext()->Unprepare();
	g_pModuleLib->GetScriptContext()->PopState();

	pObject = *(asIScriptObject **)g_pModuleLib->GetScriptContext()->GetAddressOfReturnValue();
	pObject->AddRef();

	return pObject;
}

static bool BoundsIntersect( const CModuleBoundBox *a, const CModuleBoundBox *b ) {
	const bbox_t a2 = a->ToPOD();
	const bbox_t b2 = b->ToPOD();
	return BoundsIntersect( &a2, &b2 );
}

static bool BoundsIntersectPoint( const CModuleBoundBox *bbox, const glm::vec3 *point ) {
	const bbox_t a = bbox->ToPOD();
	return BoundsIntersectPoint( &a, (const float *)point );
}

static bool BoundsIntersectSphere( const CModuleBoundBox *bbox, const glm::vec3 *point, float radius ) {
	const bbox_t a = bbox->ToPOD();
	return BoundsIntersectSphere( &a, (const float *)point,radius  );
}

void ScriptLib_Register_Util( void )
{
	SET_NAMESPACE( "TheNomad::Util" );

	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StrCmp( const string& in, const string& in )", asFUNCTION( StrCmp ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::Util::StrICmp( const string& in, const string& in )", asFUNCTION( StrICmp ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION(
		"float TheNomad::Util::Distance( const vec2& in, const vec2& in )", asFUNCTIONPR( disBetweenOBJ, ( const glm::vec2&, const glm::vec2& ), float ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION(
		"float TheNomad::Util::Distance( const ivec2& in, const ivec2& in )", asFUNCTIONPR( disBetweenOBJ, ( const glm::ivec2&, const glm::ivec2& ), int ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION(
		"float TheNomad::Util::Distance( const vec3& in, const vec3& in )", asFUNCTIONPR( disBetweenOBJ, ( const glm::vec3&, const glm::vec3& ), float ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION(
		"float TheNomad::Util::Distance( const uvec3& in, const uvec3& in )", asFUNCTIONPR( disBetweenOBJ, ( const uvec3_t, const uvec3_t ), unsigned ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION(
		"float TheNomad::Util::Distance( const ivec3& in, const ivec3& in )", asFUNCTIONPR( disBetweenOBJ, ( const ivec3_t, const ivec3_t ), int ),
		asCALL_GENERIC );
	REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::BoundsIntersect( const TheNomad::GameSystem::BBox& in, const TheNomad::GameSystem::BBox& in )",
		asFUNCTIONPR( BoundsIntersect, ( const CModuleBoundBox *, const CModuleBoundBox * ), bool ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::BoundsIntersectPoint( const TheNomad::GameSystem::BBox& in, const vec3& in )",
		asFUNCTIONPR( BoundsIntersectPoint, ( const CModuleBoundBox *, const glm::vec3 * ), bool ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "bool TheNomad::Util::BoundsIntersectSphere( const TheNomad::GameSystem::BBox& in, const vec3& in, float )",
		asFUNCTIONPR( BoundsIntersectSphere, ( const CModuleBoundBox *, const glm::vec3 *, float ), bool ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "ref@ TheNomad::Util::AllocateExternalScriptClass( const string& in nameSpace, const string& in name )",
		asFUNCTION( AllocateExternalScriptObject ), asCALL_CDECL );

	RESET_NAMESPACE();
}