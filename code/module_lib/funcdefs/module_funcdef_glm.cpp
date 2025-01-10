#include "../module_public.h"
#include "module_funcdefs.h"

// glm has a lot of very fuzzy template types
using vec2 = glm::vec<2, float, glm::packed_highp>;
using vec3 = glm::vec<3, float, glm::packed_highp>;
using vec4 = glm::vec<4, float, glm::packed_highp>;
using ivec2 = glm::vec<2, int, glm::packed_highp>;
using ivec3 = glm::vec<3, int, glm::packed_highp>;
using ivec4 = glm::vec<4, int, glm::packed_highp>;
using uvec2 = glm::vec<2, unsigned, glm::packed_highp>;
using uvec3 = glm::vec<3, unsigned, glm::packed_highp>;
using uvec4 = glm::vec<4, unsigned, glm::packed_highp>;

template<typename VectorType>
static void ScriptLib_ConstructVec( VectorType *thisPointer )
{ new ( thisPointer ) VectorType(); }
template<typename VectorType, typename CopyType>
static void ScriptLib_ConstructVec_CopyVec( const CopyType& other, VectorType *thisPointer )
{ new ( thisPointer ) VectorType( other ); }
template<typename VectorType, typename ValueType>
static void ScriptLib_ConstructVec_Values( VectorType *thisPointer, ValueType x )
{ new ( thisPointer ) VectorType( x ); }
template<typename VectorType, typename ValueType>
static void ScriptLib_ConstructVec_Values( VectorType *thisPointer, ValueType x, ValueType y )
{ new ( thisPointer ) VectorType( x, y ); }
template<typename VectorType, typename ValueType>
static void ScriptLib_ConstructVec_Values( VectorType *thisPointer, ValueType x, ValueType y, ValueType z )
{ new ( thisPointer ) VectorType( x, y, z ); }
template<typename VectorType, typename ValueType>
static void ScriptLib_ConstructVec_Values( VectorType *thisPointer, ValueType x, ValueType y, ValueType z, ValueType w )
{ new ( thisPointer ) VectorType( x, y, z, w ); }
template<typename VectorType>
static VectorType ScriptLib_Add( const VectorType *a, const VectorType& b )
{ return *a + b; }
template<typename VectorType>
static VectorType ScriptLib_Sub( const VectorType *a, const VectorType& b )
{ return *a - b; }
template<typename VectorType>
static VectorType ScriptLib_Mul( const VectorType *a, const VectorType& b )
{ return *a * b; }
template<typename VectorType>
static VectorType ScriptLib_Div( const VectorType *a, const VectorType& b )
{ return *a / b; }
template<typename VectorType>
static void ScriptLib_DestructVec( VectorType *thisPointer )
{ thisPointer->~VectorType(); }
template<typename VectorType>
static bool ScriptLib_Equal( const VectorType *a, const VectorType& b )
{ return *a == b; }
template<typename VectorType>
static int ScriptLib_CmpVec2( const VectorType *a, const VectorType& b )
{
	int cmp = 0;
	if ( a->x < b.x && a->y < b.y ) {
		cmp = -1;
	} else if ( b.x < a->x && b.y < a->y ) {
		cmp = 1;
	}
	return cmp;
}
template<typename VectorType>
static int ScriptLib_CmpVec3( const VectorType *a, const VectorType& b )
{
	int cmp = 0;
	if ( a->x < b.x && a->y < b.y && a->z < b.z ) {
		cmp = -1;
	} else if ( b.x < a->x && b.y < a->y && b.z < a->z ) {
		cmp = 1;
	}
	return cmp;
}
template<typename VectorType>
static int ScriptLib_CmpVec4( const VectorType *a, const VectorType& b )
{
	int cmp = 0;
	if ( a->r < b.r && a->g < b.g && a->b < b.b && a->a < b.a ) {
		cmp = -1;
	} else if ( b.r < a->r && b.g < a->g && b.b < a->b && b.a < a->a ) {
		cmp = 1;
	}
	return cmp;
}

template<typename VectorType>
static void ScriptLib_Register_GLM_Vec2( void )
{
	eastl::fixed_string<char, 128, false> str;
	const char *name;

	if constexpr ( std::is_same<VectorType, vec2>() ) {
		name = "vec2";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( float )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, float> ), ( VectorType *, float ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( float, float )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, float> ), ( VectorType *, float, float ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "float x", offsetof( VectorType, x ) );
		REGISTER_OBJECT_PROPERTY( name, "float y", offsetof( VectorType, y ) );
		
		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const vec2& in ) const", asFUNCTIONPR( glm::operator==, ( const vec2&, const vec2& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const vec2& in ) const", asFUNCTIONPR( ScriptLib_CmpVec2, ( const vec2 *, const vec2& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "float& opIndex( int )", asMETHODPR( vec2, operator[], ( int ), float& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const float& opIndex( int ) const", asMETHODPR( vec2, operator[], ( int ) const, const float& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "vec2 opAdd( const vec2& in ) const", asFUNCTIONPR( ScriptLib_Add<vec2>, ( const vec2 *, const vec2& ), vec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec2 opSub( const vec2& in ) const", asFUNCTIONPR( ScriptLib_Sub<vec2>, ( const vec2 *, const vec2& ), vec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec2 opMul( const vec2& in ) const", asFUNCTIONPR( ScriptLib_Mul<vec2>, ( const vec2 *, const vec2& ), vec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec2 opDiv( const vec2& in ) const", asFUNCTIONPR( ScriptLib_Div<vec2>, ( const vec2 *, const vec2& ), vec2 ), asCALL_CDECL_OBJFIRST );
	}
	else if constexpr ( std::is_same<VectorType, ivec2>() ) {
		name = "ivec2";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( int )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, int> ), ( VectorType *, int ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( int, int )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, int> ), ( VectorType *, int, int ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "int x", offsetof( VectorType, x ) );
		REGISTER_OBJECT_PROPERTY( name, "int y", offsetof( VectorType, y ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const ivec2& in ) const", asFUNCTIONPR( glm::operator==, ( const ivec2&, const ivec2& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const ivec2& in ) const", asFUNCTIONPR( ScriptLib_CmpVec2, ( const ivec2 *, const ivec2& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "int& opIndex( int )", asMETHODPR( ivec2, operator[], ( int ), int& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const int& opIndex( int ) const", asMETHODPR( ivec2, operator[], ( int ) const, const int& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "ivec2 opAdd( const ivec2& in ) const", asFUNCTIONPR( ScriptLib_Add<ivec2>, ( const ivec2 *, const ivec2& ), ivec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec2 opSub( const ivec2& in ) const", asFUNCTIONPR( ScriptLib_Sub<ivec2>, ( const ivec2 *, const ivec2& ), ivec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec2 opMul( const ivec2& in ) const", asFUNCTIONPR( ScriptLib_Mul<ivec2>, ( const ivec2 *, const ivec2& ), ivec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec2 opDiv( const ivec2& in ) const", asFUNCTIONPR( ScriptLib_Div<ivec2>, ( const ivec2 *, const ivec2& ), ivec2 ), asCALL_CDECL_OBJFIRST );
	}
	else if constexpr ( std::is_same<VectorType, uvec2>() ) {
		name = "uvec2";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( uint )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, unsigned> ), ( VectorType *, unsigned ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( uint, uint )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, unsigned> ), ( VectorType *, unsigned, unsigned ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "uint x", offsetof( VectorType, x ) );
		REGISTER_OBJECT_PROPERTY( name, "uint y", offsetof( VectorType, y ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const uvec2& in ) const", asFUNCTIONPR( glm::operator==, ( const uvec2&, const uvec2& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const uvec2& in ) const", asFUNCTIONPR( ScriptLib_CmpVec2, ( const uvec2 *, const uvec2& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "uint& opIndex( int )", asMETHODPR( uvec2, operator[], ( int ), unsigned& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const uint& opIndex( int ) const", asMETHODPR( uvec2, operator[], ( int ) const, const unsigned& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "uvec2 opAdd( const uvec2& in ) const", asFUNCTIONPR( ScriptLib_Add<uvec2>, ( const uvec2 *, const uvec2& ), uvec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec2 opSub( const uvec2& in ) const", asFUNCTIONPR( ScriptLib_Sub<uvec2>, ( const uvec2 *, const uvec2& ), uvec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec2 opMul( const uvec2& in ) const", asFUNCTIONPR( ScriptLib_Mul<uvec2>, ( const uvec2 *, const uvec2& ), uvec2 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec2 opDiv( const uvec2& in ) const", asFUNCTIONPR( ScriptLib_Div<uvec2>, ( const uvec2 *, const uvec2& ), uvec2 ), asCALL_CDECL_OBJFIRST );
	}

	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( ScriptLib_ConstructVec<VectorType> ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const vec2& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, vec2> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const vec3& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, vec3> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const vec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, vec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const ivec2& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, ivec2> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const ivec3& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, ivec3> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const ivec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, ivec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const uvec2& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, uvec2> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const uvec3& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, uvec3> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const uvec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, uvec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_DESTRUCT, "void f()", asFUNCTION( ScriptLib_DestructVec<VectorType> ), asCALL_CDECL_OBJFIRST );

	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const vec2& in )", name ), asMETHODPR( VectorType, operator=, ( const vec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const ivec2& in )", name ), asMETHODPR( VectorType, operator=, ( const ivec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const uvec2& in )", name ), asMETHODPR( VectorType, operator=, ( const uvec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const vec2& in )", name ), asMETHODPR( VectorType, operator+=, ( const vec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const ivec2& in )", name ), asMETHODPR( VectorType, operator+=, ( const ivec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const uvec2& in )", name ), asMETHODPR( VectorType, operator+=, ( const uvec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const vec2& in )", name ), asMETHODPR( VectorType, operator-=, ( const vec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const ivec2& in )", name ), asMETHODPR( VectorType, operator-=, ( const ivec2& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const uvec2& in )", name ), asMETHODPR( VectorType, operator-=, ( const uvec2& ), VectorType& ), asCALL_THISCALL );
}

template<typename VectorType>
static void ScriptLib_Register_GLM_Vec3( void )
{
	const char *name;

	if constexpr ( std::is_same<VectorType, vec3>() ) {
		name = "vec3";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( float )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, float> ), ( VectorType *, float ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( float, float, float )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, float> ), ( VectorType *, float, float, float ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "float x", offsetof( VectorType, x ) );
		REGISTER_OBJECT_PROPERTY( name, "float y", offsetof( VectorType, y ) );
		REGISTER_OBJECT_PROPERTY( name, "float z", offsetof( VectorType, z ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const vec3& in ) const", asFUNCTIONPR( glm::operator==, ( const vec3&, const vec3& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const vec3& in ) const", asFUNCTIONPR( ScriptLib_CmpVec3, ( const vec3 *, const vec3& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "float& opIndex( int )", asMETHODPR( vec3, operator[], ( int ), float& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const float& opIndex( int ) const", asMETHODPR( vec3, operator[], ( int ) const, const float& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "vec3 opAdd( const vec3& in ) const", asFUNCTIONPR( ScriptLib_Add<vec3>, ( const vec3 *, const vec3& ), vec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec3 opSub( const vec3& in ) const", asFUNCTIONPR( ScriptLib_Sub<vec3>, ( const vec3 *, const vec3& ), vec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec3 opMul( const vec3& in ) const", asFUNCTIONPR( ScriptLib_Mul<vec3>, ( const vec3 *, const vec3& ), vec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec3 opDiv( const vec3& in ) const", asFUNCTIONPR( ScriptLib_Div<vec3>, ( const vec3 *, const vec3& ), vec3 ), asCALL_CDECL_OBJFIRST );
	}
	else if constexpr ( std::is_same<VectorType, ivec3>() ) {
		name = "ivec3";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( int )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, int> ), ( VectorType *, int ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( int, int, int )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, int> ), ( VectorType *, int, int, int ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "int x", offsetof( VectorType, x ) );
		REGISTER_OBJECT_PROPERTY( name, "int y", offsetof( VectorType, y ) );
		REGISTER_OBJECT_PROPERTY( name, "int z", offsetof( VectorType, z ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const ivec3& in ) const", asFUNCTIONPR( glm::operator==, ( const ivec3&, const ivec3& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const ivec3& in ) const", asFUNCTIONPR( ScriptLib_CmpVec3, ( const ivec3 *, const ivec3& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "int& opIndex( int )", asMETHODPR( ivec3, operator[], ( int ), int& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const int& opIndex( int ) const", asMETHODPR( ivec3, operator[], ( int ) const, const int& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "ivec3 opAdd( const ivec3& in ) const", asFUNCTIONPR( ScriptLib_Add<ivec3>, ( const ivec3 *, const ivec3& ), ivec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec3 opSub( const ivec3& in ) const", asFUNCTIONPR( ScriptLib_Sub<ivec3>, ( const ivec3 *, const ivec3& ), ivec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec3 opMul( const ivec3& in ) const", asFUNCTIONPR( ScriptLib_Mul<ivec3>, ( const ivec3 *, const ivec3& ), ivec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec3 opDiv( const ivec3& in ) const", asFUNCTIONPR( ScriptLib_Div<ivec3>, ( const ivec3 *, const ivec3& ), ivec3 ), asCALL_CDECL_OBJFIRST );
	}
	else if constexpr ( std::is_same<VectorType, uvec3>() ) {
		name = "uvec3";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( uint )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, unsigned> ), ( VectorType *, unsigned ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( uint, uint, uint )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, unsigned> ), ( VectorType *, unsigned, unsigned, unsigned ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "uint x", offsetof( VectorType, x ) );
		REGISTER_OBJECT_PROPERTY( name, "uint y", offsetof( VectorType, y ) );
		REGISTER_OBJECT_PROPERTY( name, "uint z", offsetof( VectorType, z ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const uvec3& in ) const", asFUNCTIONPR( glm::operator==, ( const uvec3&, const uvec3& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const uvec3& in ) const", asFUNCTIONPR( ScriptLib_CmpVec3, ( const uvec3 *, const uvec3& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "uint& opIndex( int )", asMETHODPR( uvec3, operator[], ( int ), unsigned& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const uint& opIndex( int ) const", asMETHODPR( uvec3, operator[], ( int ) const, const unsigned& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "uvec3 opAdd( const uvec3& in ) const", asFUNCTIONPR( ScriptLib_Add<uvec3>, ( const uvec3 *, const uvec3& ), uvec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec3 opSub( const uvec3& in ) const", asFUNCTIONPR( ScriptLib_Sub<uvec3>, ( const uvec3 *, const uvec3& ), uvec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec3 opMul( const uvec3& in ) const", asFUNCTIONPR( ScriptLib_Mul<uvec3>, ( const uvec3 *, const uvec3& ), uvec3 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec3 opDiv( const uvec3& in ) const", asFUNCTIONPR( ScriptLib_Div<uvec3>, ( const uvec3 *, const uvec3& ), uvec3 ), asCALL_CDECL_OBJFIRST );
	}

	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( ScriptLib_ConstructVec<VectorType> ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const vec3& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, vec3> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const vec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, vec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const ivec3& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, ivec3> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const ivec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, ivec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const uvec3& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, uvec3> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const uvec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, uvec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_DESTRUCT, "void f()", asFUNCTION( ScriptLib_DestructVec<VectorType> ), asCALL_CDECL_OBJFIRST );

	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const vec3& in )", name ), asMETHODPR( VectorType, operator=, ( const vec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const ivec3& in )", name ), asMETHODPR( VectorType, operator=, ( const ivec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const uvec3& in )", name ), asMETHODPR( VectorType, operator=, ( const uvec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const vec3& in )", name ), asMETHODPR( VectorType, operator+=, ( const vec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const ivec3& in )", name ), asMETHODPR( VectorType, operator+=, ( const ivec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const uvec3& in )", name ), asMETHODPR( VectorType, operator+=, ( const uvec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const vec3& in )", name ), asMETHODPR( VectorType, operator-=, ( const vec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const ivec3& in )", name ), asMETHODPR( VectorType, operator-=, ( const ivec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const uvec3& in )", name ), asMETHODPR( VectorType, operator-=, ( const uvec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const vec3& in )", name ), asMETHODPR( VectorType, operator*=, ( const vec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const ivec3& in )", name ), asMETHODPR( VectorType, operator*=, ( const ivec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const uvec3& in )", name ), asMETHODPR( VectorType, operator*=, ( const uvec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const vec3& in )", name ), asMETHODPR( VectorType, operator/=, ( const vec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const ivec3& in )", name ), asMETHODPR( VectorType, operator/=, ( const ivec3& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const uvec3& in )", name ), asMETHODPR( VectorType, operator/=, ( const uvec3& ), VectorType& ), asCALL_THISCALL );
}

template<typename VectorType>
static void ScriptLib_Register_GLM_Vec4( void )
{
	const char *name;

	if constexpr ( std::is_same<VectorType, vec4>() ) {
		name = "vec4";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( float )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, float> ), ( VectorType *, float ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( float, float, float, float )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, float> ), ( VectorType *, float, float, float, float ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "float r", offsetof( VectorType, r ) );
		REGISTER_OBJECT_PROPERTY( name, "float g", offsetof( VectorType, g ) );
		REGISTER_OBJECT_PROPERTY( name, "float b", offsetof( VectorType, b ) );
		REGISTER_OBJECT_PROPERTY( name, "float a", offsetof( VectorType, a ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const vec4& in ) const", asFUNCTIONPR( glm::operator==, ( const vec4&, const vec4& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const vec4& in ) const", asFUNCTIONPR( ScriptLib_CmpVec4, ( const vec4 *, const vec4& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "float& opIndex( int )", asMETHODPR( vec4, operator[], ( int ), float& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const float& opIndex( int ) const", asMETHODPR( vec4, operator[], ( int ) const, const float& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "vec4 opAdd( const vec4& in ) const", asFUNCTIONPR( ScriptLib_Add<vec4>, ( const vec4 *, const vec4& ), vec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec4 opSub( const vec4& in ) const", asFUNCTIONPR( ScriptLib_Sub<vec4>, ( const vec4 *, const vec4& ), vec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec4 opMul( const vec4& in ) const", asFUNCTIONPR( ScriptLib_Mul<vec4>, ( const vec4 *, const vec4& ), vec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "vec4 opDiv( const vec4& in ) const", asFUNCTIONPR( ScriptLib_Div<vec4>, ( const vec4 *, const vec4& ), vec4 ), asCALL_CDECL_OBJFIRST );
	}
	else if constexpr ( std::is_same<VectorType, ivec4>() ) {
		name = "ivec4";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( int )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, int> ), ( VectorType *, int ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( int, int, int, int )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, int> ), ( VectorType *, int, int, int, int ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "int r", offsetof( VectorType, r ) );
		REGISTER_OBJECT_PROPERTY( name, "int g", offsetof( VectorType, g ) );
		REGISTER_OBJECT_PROPERTY( name, "int b", offsetof( VectorType, b ) );
		REGISTER_OBJECT_PROPERTY( name, "int a", offsetof( VectorType, a ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const ivec4& in ) const", asFUNCTIONPR( glm::operator==, ( const ivec4&, const ivec4& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const ivec4& in ) const", asFUNCTIONPR( ScriptLib_CmpVec4, ( const ivec4 *, const ivec4& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "int& opIndex( int )", asMETHODPR( ivec4, operator[], ( int ), int& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const int& opIndex( int ) const", asMETHODPR( ivec4, operator[], ( int ) const, const int& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "ivec4 opAdd( const ivec4& in ) const", asFUNCTIONPR( ScriptLib_Add<ivec4>, ( const ivec4 *, const ivec4& ), ivec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec4 opSub( const ivec4& in ) const", asFUNCTIONPR( ScriptLib_Sub<ivec4>, ( const ivec4 *, const ivec4& ), ivec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec4 opMul( const ivec4& in ) const", asFUNCTIONPR( ScriptLib_Mul<ivec4>, ( const ivec4 *, const ivec4& ), ivec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "ivec4 opDiv( const ivec4& in ) const", asFUNCTIONPR( ScriptLib_Div<ivec4>, ( const ivec4 *, const ivec4& ), ivec4 ), asCALL_CDECL_OBJFIRST );
	}
	else if constexpr ( std::is_same<VectorType, uvec4>() ) {
		name = "uvec4";
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( uint )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, unsigned> ), ( VectorType *, unsigned ), void ), asCALL_CDECL_OBJFIRST );
		REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( uint, uint, uint, uint )", asFUNCTIONPR( ( ScriptLib_ConstructVec_Values<VectorType, unsigned> ), ( VectorType *, unsigned, unsigned, unsigned, unsigned ), void ), asCALL_CDECL_OBJFIRST );

		REGISTER_OBJECT_PROPERTY( name, "uint r", offsetof( VectorType, r ) );
		REGISTER_OBJECT_PROPERTY( name, "uint g", offsetof( VectorType, g ) );
		REGISTER_OBJECT_PROPERTY( name, "uint b", offsetof( VectorType, b ) );
		REGISTER_OBJECT_PROPERTY( name, "uint a", offsetof( VectorType, a ) );

		REGISTER_METHOD_FUNCTION( name, "bool opEquals( const uvec4& in ) const", asFUNCTIONPR( glm::operator==, ( const uvec4&, const uvec4& ), bool ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "int opCmp( const uvec4& in ) const", asFUNCTIONPR( ScriptLib_CmpVec4, ( const uvec4 *, const uvec4& ), int ), asCALL_CDECL_OBJFIRST );

		REGISTER_METHOD_FUNCTION( name, "uint& opIndex( int )", asMETHODPR( uvec4, operator[], ( int ), unsigned& ), asCALL_THISCALL );
		REGISTER_METHOD_FUNCTION( name, "const uint& opIndex( int ) const", asMETHODPR( uvec4, operator[], ( int ) const, const unsigned& ), asCALL_THISCALL );

		REGISTER_METHOD_FUNCTION( name, "uvec4 opAdd( const uvec4& in ) const", asFUNCTIONPR( ScriptLib_Add<uvec4>, ( const uvec4 *, const uvec4& ), uvec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec4 opSub( const uvec4& in ) const", asFUNCTIONPR( ScriptLib_Sub<uvec4>, ( const uvec4 *, const uvec4& ), uvec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec4 opMul( const uvec4& in ) const", asFUNCTIONPR( ScriptLib_Mul<uvec4>, ( const uvec4 *, const uvec4& ), uvec4 ), asCALL_CDECL_OBJFIRST );
		REGISTER_METHOD_FUNCTION( name, "uvec4 opDiv( const uvec4& in ) const", asFUNCTIONPR( ScriptLib_Div<uvec4>, ( const uvec4 *, const uvec4& ), uvec4 ), asCALL_CDECL_OBJFIRST );
	}

	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( ScriptLib_ConstructVec<VectorType> ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const vec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, vec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const ivec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, ivec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_CONSTRUCT, "void f( const uvec4& in )", asFUNCTION( ( ScriptLib_ConstructVec_CopyVec<VectorType, uvec4> ) ), asCALL_CDECL_OBJLAST );
	REGISTER_OBJECT_BEHAVIOUR( name, asBEHAVE_DESTRUCT, "void f()", asFUNCTION( ScriptLib_DestructVec<VectorType> ), asCALL_CDECL_OBJFIRST );

	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const vec4& in )", name ), asMETHODPR( VectorType, operator=, ( const vec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const ivec4& in )", name ), asMETHODPR( VectorType, operator=, ( const ivec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAssign( const uvec4& in )", name ), asMETHODPR( VectorType, operator=, ( const uvec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const vec4& in )", name ), asMETHODPR( VectorType, operator+=, ( const vec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const ivec4& in )", name ), asMETHODPR( VectorType, operator+=, ( const ivec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opAddAssign( const uvec4& in )", name ), asMETHODPR( VectorType, operator+=, ( const uvec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const vec4& in )", name ), asMETHODPR( VectorType, operator-=, ( const vec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const ivec4& in )", name ), asMETHODPR( VectorType, operator-=, ( const ivec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opSubAssign( const uvec4& in )", name ), asMETHODPR( VectorType, operator-=, ( const uvec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const vec4& in )", name ), asMETHODPR( VectorType, operator*=, ( const vec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const ivec4& in )", name ), asMETHODPR( VectorType, operator*=, ( const ivec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opMulAssign( const uvec4& in )", name ), asMETHODPR( VectorType, operator*=, ( const uvec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const vec4& in )", name ), asMETHODPR( VectorType, operator/=, ( const vec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const ivec4& in )", name ), asMETHODPR( VectorType, operator/=, ( const ivec4& ), VectorType& ), asCALL_THISCALL );
	REGISTER_METHOD_FUNCTION( name, va( "%s& opDivAssign( const uvec4& in )", name ), asMETHODPR( VectorType, operator/=, ( const uvec4& ), VectorType& ), asCALL_THISCALL );
}

void ScriptLib_Register_GLM( void )
{
	REGISTER_OBJECT_TYPE( "vec2", vec2, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );
	REGISTER_OBJECT_TYPE( "vec3", vec3, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );
	REGISTER_OBJECT_TYPE( "vec4", vec4, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );

	REGISTER_OBJECT_TYPE( "uvec2", uvec2, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );
	REGISTER_OBJECT_TYPE( "uvec3", uvec3, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );
	REGISTER_OBJECT_TYPE( "uvec4", uvec4, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );

	REGISTER_OBJECT_TYPE( "ivec2", ivec2, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );
	REGISTER_OBJECT_TYPE( "ivec3", ivec3, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );
	REGISTER_OBJECT_TYPE( "ivec4", ivec4, asOBJ_VALUE | asOBJ_APP_CLASS_CDAK | asOBJ_POD );

	ScriptLib_Register_GLM_Vec2<vec2>();
	ScriptLib_Register_GLM_Vec2<uvec2>();
	ScriptLib_Register_GLM_Vec2<ivec2>();

	ScriptLib_Register_GLM_Vec3<vec3>();
	ScriptLib_Register_GLM_Vec3<uvec3>();
	ScriptLib_Register_GLM_Vec3<ivec3>();

	ScriptLib_Register_GLM_Vec4<vec4>();
	ScriptLib_Register_GLM_Vec4<uvec4>();
	ScriptLib_Register_GLM_Vec4<ivec4>();
}