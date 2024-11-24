#include "../module_public.h"
#include "../../game/g_world.h"
#include "module_funcdefs.h"
#include "../../ui/ui_string_manager.h"
#include "../module_engine/module_bbox.h"
#include "../module_engine/module_linkentity.h"
#include "../module_engine/module_gpuconfig.h"
#include <glm/gtc/type_ptr.hpp>

CModuleBoundBox::CModuleBoundBox( void ) {
    memset( this, 0, sizeof( *this ) );
}

CModuleBoundBox::CModuleBoundBox( float w, float h, const glm::vec3& origin ) {
    memset( this, 0, sizeof( *this ) );
    width = w;
    height = h;
    MakeBounds( origin );
}

CModuleBoundBox& CModuleBoundBox::operator=( const CModuleBoundBox& other ) {
    memcpy( this, eastl::addressof( other ), sizeof( *this ) );
    return *this;
}

void CModuleBoundBox::Clear( void ) {
    memset( this, 0, sizeof( *this ) );
}

float CModuleBoundBox::GetRadius( void ) const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for ( i = 0; i < 3; i++ ) {
		b0 = (float)fabs( mins[i] );
		b1 = (float)fabs( maxs[i] );
		if ( b0 > b1 ) {
			total += b0 * b0;
		} else {
			total += b1 * b1;
		}
	}
	return sqrtf( total );
}

float CModuleBoundBox::GetRadius( const glm::vec3& center ) const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for ( i = 0; i < 3; i++ ) {
		b0 = (float)fabs( center[i] - mins[i] );
		b1 = (float)fabs( maxs[i] - center[i] );
		if ( b0 > b1 ) {
			total += b0 * b0;
		} else {
			total += b1 * b1;
		}
	}
	return sqrtf( total );
}

bool CModuleBoundBox::LineIntersection( const glm::vec3& start, const glm::vec3& end ) const {
	float ld[3];
	glm::vec3 center = ( mins + maxs ) * 0.5f;
	glm::vec3 extents = maxs - center;
	glm::vec3 lineDir = 0.5f * ( end - start );
	glm::vec3 lineCenter = start + lineDir;
	glm::vec3 dir = lineCenter - center;

	ld[0] = fabs( lineDir[0] );
	if ( fabs( dir[0] ) > extents[0] + ld[0] ) {
		return false;
	}

	ld[1] = fabs( lineDir[1] );
	if ( fabs( dir[1] ) > extents[1] + ld[1] ) {
		return false;
	}

	ld[2] = fabs( lineDir[2] );
	if ( fabs( dir[2] ) > extents[2] + ld[2] ) {
		return false;
	}

	glm::vec3 cross = glm::cross( lineDir,  dir );

	if ( fabs( cross[0] ) > extents[1] * ld[2] + extents[2] * ld[1] ) {
		return false;
	}

	if ( fabs( cross[1] ) > extents[0] * ld[2] + extents[2] * ld[0] ) {
		return false;
	}

	if ( fabs( cross[2] ) > extents[0] * ld[1] + extents[1] * ld[0] ) {
		return false;
	}

	return true;
}

bool CModuleBoundBox::ContainsPoint( const glm::vec3& p ) const
{
	if ( p[0] < mins[0] || p[1] < mins[1] || p[2] < mins[2]
		|| p[0] > maxs[0] || p[1] > maxs[1] || p[2] > maxs[2] )
	{
		return false;
	}
	return true;
}

bool CModuleBoundBox::RayIntersection( const glm::vec3& start, const glm::vec3& dir, float& scale ) const {
	int i, ax0, ax1, ax2, side, inside;
	float f;
	glm::vec3 hit;

	ax0 = -1;
	inside = 0;
	for ( i = 0; i < 3; i++ ) {
		if ( start[i] < mins[i] ) {
			side = 0;
		}
		else if ( start[i] > maxs[i] ) {
			side = 1;
		}
		else {
			inside++;
			continue;
		}
		if ( dir[i] == 0.0f ) {
			continue;
		}
		f = ( start[i] - ( &mins )[side][i] );
		if ( ax0 < 0 || fabs( f ) > fabs( scale * dir[i] ) ) {
			scale = - ( f / dir[i] );
			ax0 = i;
		}
	}

	if ( ax0 < 0 ) {
		scale = 0.0f;
		// return true if the start point is inside the bounds
		return ( inside == 3 );
	}

	ax1 = (ax0+1)%3;
	ax2 = (ax0+2)%3;
	hit[ax1] = start[ax1] + scale * dir[ax1];
	hit[ax2] = start[ax2] + scale * dir[ax2];

	return ( hit[ax1] >= mins[ax1] && hit[ax1] <= maxs[ax1] &&
				hit[ax2] >= mins[ax2] && hit[ax2] <= maxs[ax2] );
}

void CModuleBoundBox::MakeBounds( const glm::vec3& origin ) {
    /*
    mins[0] = origin[0] - ( width / 2 );
	mins[1] = origin[1] - ( height / 2 );
	mins[2] = origin[2] + ( height / 2 );

	maxs[0] = origin[0] + ( width / 2 );
	maxs[1] = origin[1] + ( height / 2 );
	maxs[2] = origin[2] + ( height / 2 );
    */

    mins[0] = origin[0];
    mins[1] = origin[1];
    mins[2] = origin[2] + ( height / 2 );

    maxs[0] = origin[0] + width;
    maxs[1] = origin[1] + height;
    maxs[2] = origin[2] - ( height / 2 );
}

const bbox_t CModuleBoundBox::ToPOD( void ) const {
    return { { mins[0], mins[1], mins[2] }, { maxs[0], maxs[1], maxs[2] } };
}

static void GetString( const string_t& key, string_t& value )
{
	const stringHash_t *hash;

	hash = strManager->ValueForKey( key.c_str() );
	value = hash->value;
}

static CModuleBoundBox BoundBoxConstruct( void )
{ return CModuleBoundBox(); }
static CModuleBoundBox BoundBoxValueConstruct( float width, float height, const glm::vec3& origin )
{ return CModuleBoundBox( width, height, origin ); }
static void BoundBoxDestruct( CModuleBoundBox& bounds )
{ bounds.~CModuleBoundBox(); }
static CModuleLinkEntity LinkEntityConstruct( void )
{ return CModuleLinkEntity(); }

static CModuleLinkEntity LinkEntityCopyConstruct( const glm::vec3& origin, const CModuleBoundBox& bounds, uint32_t nEntityId, uint32_t nEntityType )
{ return CModuleLinkEntity( origin, bounds, nEntityId, nEntityType ); }
static void LinkEntityDestruct( CModuleLinkEntity& entity )
{ entity.~CModuleLinkEntity(); }

static void BeginSaveSection( const string_t& name )
{ g_pArchiveHandler->BeginSaveSection( g_pModuleLib->GetCurrentHandle()->GetName().c_str(), name.c_str() ); }
static void EndSaveSection( void )
{ g_pArchiveHandler->EndSaveSection(); }
static nhandle_t FindSaveSection( const string_t& name )
{ return g_pArchiveHandler->GetSection( name.c_str() ); }

static void SaveInt8( const string_t& name, int8_t value )
{ g_pArchiveHandler->SaveChar( name.c_str(), value ); }
static void SaveInt16( const string_t& name, int16_t value )
{ g_pArchiveHandler->SaveShort( name.c_str(), value ); }
static void SaveInt32( const string_t& name, int32_t value )
{ g_pArchiveHandler->SaveInt( name.c_str(), value ); }
static void SaveInt64( const string_t& name, int64_t value )
{ g_pArchiveHandler->SaveLong( name.c_str(), value ); }
static void SaveUInt8( const string_t& name, uint8_t value )
{ g_pArchiveHandler->SaveByte( name.c_str(), value ); }
static void SaveUInt16( const string_t& name, uint16_t value )
{ g_pArchiveHandler->SaveUShort( name.c_str(), value ); }
static void SaveUInt32( const string_t& name, uint32_t value )
{ g_pArchiveHandler->SaveUInt( name.c_str(), value ); }
static void SaveUInt64( const string_t& name, uint64_t value )
{ g_pArchiveHandler->SaveULong( name.c_str(), value ); }
static void SaveFloat( const string_t& name, float value )
{ g_pArchiveHandler->SaveFloat( name.c_str(), value ); }
static void SaveArray( const string_t& name, const CScriptArray *pData )
{ g_pArchiveHandler->SaveArray( name.c_str(), pData ); }
static void SaveString( const string_t& name, const string_t& value )
{ g_pArchiveHandler->SaveString( name.c_str(), eastl::addressof( value ) ); }
static void SaveVec2( const string_t& name, const glm::vec2& value )
{ g_pArchiveHandler->SaveVec2( name.c_str(), (const vec_t *)glm::value_ptr( value ) ); }
static void SaveVec3( const string_t& name, const glm::vec3& value )
{ g_pArchiveHandler->SaveVec3( name.c_str(), (const vec_t *)glm::value_ptr( value ) ); }
static void SaveVec4( const string_t& name, const glm::vec4& value )
{ g_pArchiveHandler->SaveVec4( name.c_str(), (const vec_t *)glm::value_ptr( value ) ); }

static int8_t LoadInt8( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadChar( name.c_str(), hSection ); }
static int16_t LoadInt16( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadShort( name.c_str(), hSection ); }
static int32_t LoadInt32( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadInt( name.c_str(), hSection ); }
static int64_t LoadInt64( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadLong( name.c_str(), hSection ); }
static uint8_t LoadUInt8( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadByte( name.c_str(), hSection ); }
static uint16_t LoadUInt16( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadUShort( name.c_str(), hSection ); }
static uint32_t LoadUInt32( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadUInt( name.c_str(), hSection ); }
static uint64_t LoadUInt64( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadULong( name.c_str(), hSection ); }
static float LoadFloat( const string_t& name, nhandle_t hSection )
{ return g_pArchiveHandler->LoadFloat( name.c_str(), hSection ); }
static void LoadString( const string_t& name, string_t& value, nhandle_t hSection )
{ g_pArchiveHandler->LoadString( name.c_str(), eastl::addressof( value ), hSection ); }
static void LoadVec2( const string_t& name, glm::vec2& value, nhandle_t hSection )
{ g_pArchiveHandler->LoadVec2( name.c_str(), (vec_t *)glm::value_ptr( value ), hSection ); }
static void LoadVec3( const string_t& name, glm::vec3& value, nhandle_t hSection )
{ g_pArchiveHandler->LoadVec3( name.c_str(), (vec_t *)glm::value_ptr( value ), hSection ); }
static void LoadVec4( const string_t& name, glm::vec4& value, nhandle_t hSection )
{ g_pArchiveHandler->LoadVec4( name.c_str(), (vec_t *)glm::value_ptr( value ), hSection ); }
static void LoadArray( const string_t& name, CScriptArray *pData, nhandle_t hSection )
{ g_pArchiveHandler->LoadArray( name.c_str(), pData, hSection ); }

static void GetGPUConfig( CModuleGPUConfig *config )
{
	config->gpuConfig = gi.gpuConfig;
	config->extensionsString = gi.gpuConfig.extensions_string;
	config->rendererString = gi.gpuConfig.renderer_string;
	config->shaderVersionString = gi.gpuConfig.shader_version_str;
	config->versionString = gi.gpuConfig.version_string;
}

static void CastRay( const glm::vec3& start, glm::vec3& origin, uint32_t& nEntityNumber, float length, float angle, uint32_t flags )
{
	ray_t ray;

	ray.angle = angle;
	ray.flags = flags;
	ray.length = length;
	VectorCopy( ray.start, start );

	g_world->CastRay( &ray );

	nEntityNumber = ray.entityNumber;
	VectorCopy( origin, ray.origin );
}

static bool CheckWallHit( const glm::vec3& vec, dirtype_t nDir )
{ return g_world->CheckWallHit( (const vec_t *)glm::value_ptr( vec ), nDir ); }

static void GetSkinData( const string_t& name, string_t& description, string_t& displayText,
	glm::uvec2& torsoSheetSize, glm::uvec2& torsoSpriteSize,
	glm::uvec2& armsSheetSize, glm::uvec2& armsSpriteSize,
	glm::uvec2& legsSheetSize, glm::uvec2& legsSpriteSize
)
{
	description.resize( MAX_DESCRIPTION_LENGTH );
	displayText.resize( MAX_DISPLAY_NAME_LENGTH );
	G_GetSkinData(
		name.c_str(), description.data(), displayText.data(),
		(uvec_t *)glm::value_ptr( torsoSheetSize ), (uvec_t *)glm::value_ptr( torsoSpriteSize ),
		(uvec_t *)glm::value_ptr( armsSheetSize ), (uvec_t *)glm::value_ptr( armsSpriteSize ),
		(uvec_t *)glm::value_ptr( legsSheetSize ), (uvec_t *)glm::value_ptr( legsSpriteSize )
	);
}

static nhandle_t LoadMap( const string_t& name )
{ return G_LoadMap( name.c_str() ); }

static void GetTileData( CScriptArray *tiles )
{
	tiles->Resize( gi.mapCache.info.numLevels );
	for ( uint32_t i = 0; i < gi.mapCache.info.numLevels; i++ ) {
		( (CScriptArray *)tiles->At( i ) )->Resize( gi.mapCache.info.numTiles );
		G_GetTileData( (uint64_t *)( (CScriptArray *)tiles->At( i ) )->GetBuffer(), i );
	}
}

void ScriptLib_Register_Game( void )
{
	SET_NAMESPACE( "TheNomad::GameSystem" );
	
	REGISTER_OBJECT_TYPE( "BBox", CModuleBoundBox, asOBJ_VALUE );
	REGISTER_OBJECT_BEHAVIOUR( "TheNomad::GameSystem::BBox", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( BoundBoxConstruct ), asCALL_CDECL_OBJFIRST );
	REGISTER_OBJECT_BEHAVIOUR( "TheNomad::GameSystem::BBox", asBEHAVE_CONSTRUCT, "void f( float, float, const vec3& in )",
		asFUNCTION( BoundBoxValueConstruct ), asCALL_CDECL_OBJFIRST );
	REGISTER_OBJECT_BEHAVIOUR( "TheNomad::GameSystem::BBox", asBEHAVE_DESTRUCT, "void f()", asFUNCTION( BoundBoxDestruct ), asCALL_CDECL_OBJFIRST );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::BBox", "float m_nWidth", offsetof( CModuleBoundBox, width ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::BBox", "float m_nHeight", offsetof( CModuleBoundBox, height ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::BBox", "vec3 m_Mins", offsetof( CModuleBoundBox, mins ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::BBox", "vec3 m_Maxs", offsetof( CModuleBoundBox, maxs ) );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::BBox", "TheNomad::GameSystem::BBox& opAssign( const TheNomad::GameSystem::BBox& in )",
		asMETHODPR( CModuleBoundBox, operator=, ( const CModuleBoundBox& ), CModuleBoundBox& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::BBox", "void MakeBounds( const vec3& in )",
		asMETHODPR( CModuleBoundBox, MakeBounds, ( const glm::vec3& ), void ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::BBox", "bool LineIntersection( const vec3& in, const vec3& in )",
		asMETHODPR( CModuleBoundBox, LineIntersection, ( const glm::vec3&, const glm::vec3& ) const, bool ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::BBox", "bool ContainsPoint( const vec3& in )",
		asMETHODPR( CModuleBoundBox, ContainsPoint, ( const glm::vec3& ) const, bool ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::BBox", "bool RayIntersection( const vec3& in, const vec3& in, float )",
		asMETHODPR( CModuleBoundBox, RayIntersection, ( const glm::vec3&, const glm::vec3&, float& ) const, bool ), asCALL_THISCALL );

	REGISTER_OBJECT_TYPE( "LinkEntity", CModuleLinkEntity, asOBJ_VALUE );
	REGISTER_OBJECT_BEHAVIOUR( "TheNomad::GameSystem::LinkEntity", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( LinkEntityConstruct ), asCALL_CDECL_OBJFIRST );
	REGISTER_OBJECT_BEHAVIOUR( "TheNomad::GameSystem::LinkEntity", asBEHAVE_CONSTRUCT, "void f( const vec3& in, const BBox& in, uint, uint )",
		asFUNCTION( LinkEntityCopyConstruct ), asCALL_CDECL_OBJFIRST );
	REGISTER_OBJECT_BEHAVIOUR( "TheNomad::GameSystem::LinkEntity", asBEHAVE_DESTRUCT, "void f()", asFUNCTION( LinkEntityDestruct ), asCALL_CDECL_OBJFIRST );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::LinkEntity", "vec3 m_Origin", offsetof( CModuleLinkEntity, m_Origin ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::LinkEntity", "uint32 m_nEntityId", offsetof( CModuleLinkEntity, m_nEntityId ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::LinkEntity", "uint32 m_nEntityType", offsetof( CModuleLinkEntity, m_nEntityType ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::LinkEntity", "uint32 m_nEntityNumber", offsetof( CModuleLinkEntity, m_nEntityNumber ) );
	REGISTER_OBJECT_PROPERTY( "TheNomad::GameSystem::LinkEntity", "BBox m_Bounds", offsetof( CModuleLinkEntity, m_Bounds ) );
	
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::LinkEntity", "TheNomad::GameSystem::LinkEntity& opAssign( const TheNomad::GameSystem::LinkEntity& in )",
		asMETHODPR( CModuleLinkEntity, operator=, ( const CModuleLinkEntity& ), CModuleLinkEntity& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::LinkEntity", "void SetOrigin( const vec3& in )",
		asMETHODPR( CModuleLinkEntity, SetOrigin, ( const glm::vec3& ), void ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::LinkEntity", "void SetBounds( const BBox& in )",
		asMETHODPR( CModuleLinkEntity, SetBounds, ( const CModuleBoundBox& ), void ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::LinkEntity", "const vec3& GetOrigin( void ) const",
		asMETHODPR( CModuleLinkEntity, GetOrigin, ( void ), const glm::vec3& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::LinkEntity", "const BBox& GetBounds( void ) const",
		asMETHODPR( CModuleLinkEntity, GetBounds, ( void ), const CModuleBoundBox& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( "TheNomad::GameSystem::LinkEntity", "void Update()",
		asMETHODPR( CModuleLinkEntity, Update, ( void ), void ), asCALL_THISCALL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetString( const string& in, string& out )", asFUNCTION( GetString ), asCALL_CDECL );

//		g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void TheNomad::GameSystem::SetCameraPos( const vec2& in, float, float )",
//			asFUNCTION( SetCameraPos ), asCALL_GENERIC );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::BeginSaveSection( const string& in )", asFUNCTION( BeginSaveSection ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::EndSaveSection()", asFUNCTION( EndSaveSection ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::GameSystem::FindSaveSection( const string& in )", asFUNCTION( FindSaveSection ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt8( const string& in, int8 )", asFUNCTION( SaveInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt16( const string& in, int16 )", asFUNCTION( SaveInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt32( const string& in, int32 )", asFUNCTION( SaveInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt64( const string& in, int64 )", asFUNCTION( SaveInt64 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt8( const string& in, uint8 )", asFUNCTION( SaveUInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt16( const string& in, uint16 )", asFUNCTION( SaveUInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt32( const string& in, uint32 )", asFUNCTION( SaveUInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt64( const string& in, uint64 )", asFUNCTION( SaveUInt64 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveChar( const string& in, int8 )", asFUNCTION( SaveInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveShort( const string& in, int16 )", asFUNCTION( SaveInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveInt( const string& in, int32 )", asFUNCTION( SaveInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveLong( const string& in, int64 )", asFUNCTION( SaveInt64 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<int8>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<int16>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<int32>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<int64>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<uint8>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<uint16>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<uint32>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<uint64>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<float>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveArray( const string& in, const array<string>& in )", asFUNCTION( SaveArray ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveByte( const string& in, uint8 )", asFUNCTION( SaveUInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUShort( const string& in, uint16 )", asFUNCTION( SaveUInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveUInt( const string& in, uint32 )", asFUNCTION( SaveUInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveULong( const string& in, uint64 )", asFUNCTION( SaveUInt64 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveFloat( const string& in, float )", asFUNCTION( SaveFloat ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveDecimal( const string& in, float )", asFUNCTION( SaveFloat ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveString( const string& in, const string& in )", asFUNCTION( SaveString ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveVec2( const string& in, const vec2& in )", asFUNCTION( SaveVec2 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveVec3( const string& in, const vec3& in )", asFUNCTION( SaveVec3 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SaveVec4( const string& in, const vec4& in )", asFUNCTION( SaveVec4 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "int8 TheNomad::GameSystem::LoadInt8( const string& in, int )", asFUNCTION( LoadInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int16 TheNomad::GameSystem::LoadInt16( const string& in, int )", asFUNCTION( LoadInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int32 TheNomad::GameSystem::LoadInt32( const string& in, int )", asFUNCTION( LoadInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int64 TheNomad::GameSystem::LoadInt64( const string& in, int )", asFUNCTION( LoadInt64 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "uint8 TheNomad::GameSystem::LoadUInt8( const string& in, int )", asFUNCTION( LoadUInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint16 TheNomad::GameSystem::LoadUInt16( const string& in, int )", asFUNCTION( LoadUInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint32 TheNomad::GameSystem::LoadUInt32( const string& in, int )", asFUNCTION( LoadUInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::GameSystem::LoadUInt64( const string& in, int )", asFUNCTION( LoadUInt64 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "int8 TheNomad::GameSystem::LoadChar( const string& in, int )", asFUNCTION( LoadInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int16 TheNomad::GameSystem::LoadShort( const string& in, int )", asFUNCTION( LoadInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int32 TheNomad::GameSystem::LoadInt( const string& in, int )", asFUNCTION( LoadInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int64 TheNomad::GameSystem::LoadLong( const string& in, int )", asFUNCTION( LoadInt64 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "uint8 TheNomad::GameSystem::LoadByte( const string& in, int )", asFUNCTION( LoadUInt8 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint16 TheNomad::GameSystem::LoadUShort( const string& in, int )", asFUNCTION( LoadUInt16 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint32 TheNomad::GameSystem::LoadUInt( const string& in, int )", asFUNCTION( LoadUInt32 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::GameSystem::LoadULong( const string& in, int )", asFUNCTION( LoadUInt64 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "float TheNomad::GameSystem::LoadFloat( const string& in, int )", asFUNCTION( LoadFloat ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "float TheNomad::GameSystem::LoadDecimal( const string& in, int )", asFUNCTION( LoadFloat ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadString( const string& in, string& out, int )", asFUNCTION( LoadString ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadVec2( const string& in, vec2& out, int )", asFUNCTION( LoadVec2 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadVec3( const string& in, vec3& out, int )", asFUNCTION( LoadVec3 ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::LoadVec4( const string& in, vec4& out, int )", asFUNCTION( LoadVec4 ), asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetGPUGameConfig( TheNomad::Engine::Renderer::GPUConfig& out )", asFUNCTION( GetGPUConfig ),
		asCALL_CDECL );
		
	// these are here because if they change in the map editor or the engine, it'll break sgame
	REGISTER_ENUM_TYPE( "EntityType" );
	REGISTER_ENUM_VALUE( "EntityType", "Playr", ET_PLAYR );
	REGISTER_ENUM_VALUE( "EntityType", "Mob", ET_MOB );
	REGISTER_ENUM_VALUE( "EntityType", "Bot", ET_BOT );
	REGISTER_ENUM_VALUE( "EntityType", "Item", ET_ITEM );
	REGISTER_ENUM_VALUE( "EntityType", "Wall", ET_WALL );
	REGISTER_ENUM_VALUE( "EntityType", "Weapon", ET_WEAPON );
	REGISTER_ENUM_VALUE( "EntityType", "NumEntityTypes", NUMENTITYTYPES );

	REGISTER_ENUM_TYPE( "GameDifficulty" );
	REGISTER_ENUM_VALUE( "GameDifficulty", "Easy", DIF_EASY );
	REGISTER_ENUM_VALUE( "GameDifficulty", "Normal", DIF_NORMAL );
	REGISTER_ENUM_VALUE( "GameDifficulty", "Hard", DIF_HARD );
	REGISTER_ENUM_VALUE( "GameDifficulty", "VeryHard", DIF_VERY_HARD );
	REGISTER_ENUM_VALUE( "GameDifficulty", "Insane", DIF_INSANE );
	REGISTER_ENUM_VALUE( "GameDifficulty", "Meme", DIF_MEME );
	REGISTER_ENUM_VALUE( "GameDifficulty", "NumDifficulties", NUMDIFS );
		
	REGISTER_ENUM_TYPE( "DirType" );
	REGISTER_ENUM_VALUE( "DirType", "North", DIR_NORTH );
	REGISTER_ENUM_VALUE( "DirType", "NorthEast", DIR_NORTH_EAST );
	REGISTER_ENUM_VALUE( "DirType", "East", DIR_EAST );
	REGISTER_ENUM_VALUE( "DirType", "SouthEast", DIR_SOUTH_EAST );
	REGISTER_ENUM_VALUE( "DirType", "South", DIR_SOUTH );
	REGISTER_ENUM_VALUE( "DirType", "SouthWest", DIR_SOUTH_WEST );
	REGISTER_ENUM_VALUE( "DirType", "West", DIR_WEST );
	REGISTER_ENUM_VALUE( "DirType", "NorthWest", DIR_NORTH_WEST );
	REGISTER_ENUM_VALUE( "DirType", "Inside", DIR_NULL );
	REGISTER_ENUM_VALUE( "DirType", "NumDirs", NUMDIRS );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::CastRay( const vec3& in, vec3& out, uint32& out, float, float, uint32 )", asFUNCTION( CastRay ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "bool TheNomad::GameSystem::CheckWallHit( const vec3& in, TheNomad::GameSystem::DirType )", asFUNCTION( CheckWallHit ),
		asCALL_CDECL );

	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetSkinData( const string& in, string& out, string& out, uvec2& out, uvec2& out, uvec2& out, uvec2& out, uvec2& out, uvec2& out )",
		asFUNCTION( GetSkinData ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetCheckpointData( uvec3& out, uvec2& out, uint )", asFUNCTION( G_GetCheckpointData ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetSpawnData( uvec3& out, uint& out, uint& out, uint, uint& out )", asFUNCTION( G_GetSpawnData ),
		asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::GetTileData( array<array<uint64>>@ )", asFUNCTION( GetTileData ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::SetActiveMap( int, uint& out, uint& out, uint& out, int& out, int& out )",
		asFUNCTION( G_SetActiveMap ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "int TheNomad::GameSystem::LoadMap( const string& in )", asFUNCTION( LoadMap ), asCALL_CDECL );

	RESET_NAMESPACE();
}