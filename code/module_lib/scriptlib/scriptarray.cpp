#include "../module_public.h"
#include "scriptarray.h"
#include "../module_alloc.h"

// This macro is used to avoid warnings about unused variables.
// Usually where the variables are only used in debug mode.
#define UNUSED_VAR(x) (void)(x)

typedef struct {
	uint64_t numAllocs;
	uint64_t numFrees;
	uint64_t totalBytesAllocated;
	uint64_t totalBytesFreed;
	int64_t numBuffers;
	int64_t currentBytesAllocated;
	int64_t overHeadBytes;
} alloc_stats_t;

static alloc_stats_t memstats;

static void PrintArrayMemoryStats_f( void ) {
	Con_Printf( "\nCScriptArray Memory Statistics:\n"
				"Total Allocations: %lu\n"
				"Total Deallocations: %lu\n"
				"Current Buffer Count: %li\n"
				"Total Bytes Allocated: %lu\n"
				"Total Bytes Deallocated: %lu\n"
				"Current Bytes Allocated: %li\n"
				"Overhead Data Size: %li\n"
	, memstats.numAllocs, memstats.numFrees, memstats.numBuffers, memstats.totalBytesAllocated, memstats.totalBytesFreed,
	memstats.currentBytesAllocated, memstats.overHeadBytes );
}

// Set the default memory routines
// Use the angelscript engine's memory routines by default

static asFREEFUNC_t _arrayUserFree;

// Allows the application to set which memory routines should be used by the array object
void CScriptArray::SetMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc)
{
}

static void RegisterScriptList_Native( asIScriptEngine *engine );
static void RegisterScriptList_Generic( asIScriptEngine *engine );

// We just define a number here that we assume nobody else is using for
// object type user data. The add-ons have reserved the numbers 1000
// through 1999 for this purpose, so we should be fine.

static void CleanupTypeInfoArrayCache( asITypeInfo *type )
{
	SArrayCache *cache = (SArrayCache *)type->GetUserData( ARRAY_CACHE );
	if ( cache ) {
		cache->~SArrayCache();
		Mem_Free( cache );
	}
}

CScriptArray *CScriptArray::Create( asITypeInfo *ti, asUINT length )
{
	void *mem = Mem_ClearedAlloc( sizeof( CScriptArray ) );
	if ( mem == NULL ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "Mem_ClearedAlloc() failed on CScriptArray (%lu bytes)", sizeof( CScriptArray ) ) );
		}
		return NULL;
	}

	CScriptArray *a = new ( mem ) CScriptArray( length, ti );

	return a;
}

CScriptArray *CScriptArray::Create( asITypeInfo *ti, void *initList )
{
	void *mem = Mem_ClearedAlloc( sizeof( CScriptArray ) );
	if ( mem == NULL ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "Mem_ClearedAlloc() failed on CScriptArray (%lu bytes)", sizeof( CScriptArray ) ) );
		}
		return NULL;
	}

	CScriptArray *a = new ( mem ) CScriptArray( ti, initList );

	return a;
}

CScriptArray *CScriptArray::Create( asITypeInfo *ti, asUINT length, void *defVal )
{
	void *mem = Mem_ClearedAlloc( sizeof( CScriptArray ) );
	if ( mem == NULL ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "Mem_ClearedAlloc() failed on CScriptArray (%lu bytes)", sizeof( CScriptArray ) ) );
		}
		return NULL;
	}

	CScriptArray *a = new ( mem ) CScriptArray( length, defVal, ti );

	return a;
}

CScriptArray *CScriptArray::Create( asITypeInfo *ti )
{
	return CScriptArray::Create( ti, asUINT( 0 ) );
}

// This optional callback is called when the template type is first used by the compiler.
// It allows the application to validate if the template can be instantiated for the requested
// subtype at compile time, instead of at runtime. The output argument dontGarbageCollect
// allow the callback to tell the engine if the template instance type shouldn't be garbage collected,
// i.e. no asOBJ_GC flag.
static bool ScriptListTemplateCallback( asITypeInfo *ti, bool *dontGarbageCollect = NULL )
{
	// Make sure the subtype can be instantiated with a default factory/constructor,
	// otherwise we won't be able to instantiate the elements.
	int typeId = ti->GetSubTypeId();
	if( typeId == asTYPEID_VOID )
		return false;
	if( (typeId & asTYPEID_MASK_OBJECT) && !(typeId & asTYPEID_OBJHANDLE) )
	{
		asITypeInfo *subtype = ti->GetEngine()->GetTypeInfoById(typeId);
		asDWORD flags = subtype->GetFlags();
		if( (flags & asOBJ_VALUE) && !(flags & asOBJ_POD) )
		{
			// Verify that there is a default constructor
			bool found = false;
			for( asUINT n = 0; n < subtype->GetBehaviourCount(); n++ )
			{
				asEBehaviours beh;
				asIScriptFunction *func = subtype->GetBehaviourByIndex(n, &beh);
				if( beh != asBEHAVE_CONSTRUCT ) continue;

				if( func->GetParamCount() == 0 )
				{
					// Found the default constructor
					found = true;
					break;
				}
			}

			if( !found )
			{
				// There is no default constructor
				// TODO: Should format the message to give the name of the subtype for better understanding
				ti->GetEngine()->WriteMessage("array", 0, 0, asMSGTYPE_ERROR, "The subtype has no default constructor");
				return false;
			}
		}
		else if( (flags & asOBJ_REF) )
		{
			bool found = false;

			// If value assignment for ref type has been disabled then the array
			// can be created if the type has a default factory function
			if( !ti->GetEngine()->GetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE) )
			{
				// Verify that there is a default factory
				for( asUINT n = 0; n < subtype->GetFactoryCount(); n++ )
				{
					asIScriptFunction *func = subtype->GetFactoryByIndex(n);
					if( func->GetParamCount() == 0 )
					{
						// Found the default factory
						found = true;
						break;
					}
				}
			}

			if( !found )
			{
				// No default factory
				// TODO: Should format the message to give the name of the subtype for better understanding
				ti->GetEngine()->WriteMessage("array", 0, 0, asMSGTYPE_ERROR, "The subtype has no default factory");
				return false;
			}
		}

		// If the object type is not garbage collected then the array also doesn't need to be
		if( !(flags & asOBJ_GC) && dontGarbageCollect && dontGarbageCollect != (bool *)0xd )
			*dontGarbageCollect = true;
	}
	else if( !(typeId & asTYPEID_OBJHANDLE) )
	{
		// Arrays with primitives cannot form circular references,
		// thus there is no need to garbage collect them
		if ( dontGarbageCollect && dontGarbageCollect != (bool *)0xd )
			*dontGarbageCollect = true;
	}
	else
	{
		Assert( typeId & asTYPEID_OBJHANDLE );

		// It is not necessary to set the array as garbage collected for all handle types.
		// If it is possible to determine that the handle cannot refer to an object type
		// that can potentially form a circular reference with the array then it is not
		// necessary to make the array garbage collected.
		asITypeInfo *subtype = ti->GetEngine()->GetTypeInfoById(typeId);
		asDWORD flags = subtype->GetFlags();
		if( !(flags & asOBJ_GC) )
		{
			if( (flags & asOBJ_SCRIPT_OBJECT) )
			{
				// Even if a script class is by itself not garbage collected, it is possible
				// that classes that derive from it may be, so it is not possible to know
				// that no circular reference can occur.
				if( (flags & asOBJ_NOINHERIT) )
				{
					// A script class declared as final cannot be inherited from, thus
					// we can be certain that the object cannot be garbage collected.
					if ( dontGarbageCollect && dontGarbageCollect != (bool *)0xd )
						*dontGarbageCollect = true;
				}
			}
			else
			{
				// For application registered classes we assume the application knows
				// what it is doing and don't mark the array as garbage collected unless
				// the type is also garbage collected.
				if ( dontGarbageCollect && dontGarbageCollect != (bool *)0xd )
					*dontGarbageCollect = true;
			}
		}
	}

	// The type is ok
	return true;
}

// Registers the template array type
void RegisterScriptArray(asIScriptEngine *engine)
{
	if ( !strstr( asGetLibraryOptions(), "AS_MAX_PORTABILITY" ) ) {
		RegisterScriptList_Native( engine );
	} else {
		RegisterScriptList_Generic( engine );
	}
	
	engine->RegisterDefaultArrayType( "array<T>" );
}

static void RegisterScriptList_Native( asIScriptEngine *engine )
{
	Cmd_AddCommand( "ml_debug.print_list_memory_stats", PrintArrayMemoryStats_f );

	memset( &memstats, 0, sizeof( memstats ) );

	engine->SetTypeInfoUserDataCleanupCallback( CleanupTypeInfoArrayCache, ARRAY_CACHE );

	CheckASCall( engine->RegisterObjectType( "array<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE ) );

	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION( ScriptListTemplateCallback ), asCALL_CDECL ) );

	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in)", asFUNCTIONPR( CScriptArray::Create, ( asITypeInfo * ), CScriptArray * ), asCALL_CDECL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint length) explicit", asFUNCTIONPR( CScriptArray::Create, ( asITypeInfo *, asUINT ), CScriptArray * ), asCALL_CDECL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint length, const T &in value)", asFUNCTIONPR( CScriptArray::Create, ( asITypeInfo *, asUINT, void * ), CScriptArray * ), asCALL_CDECL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_LIST_FACTORY, "array<T>@ f(int&in, int&in) {repeat T}", asFUNCTIONPR( CScriptArray::Create, ( asITypeInfo *, void * ), CScriptArray * ), asCALL_CDECL ) );

	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_ADDREF, "void f()", asMETHOD( CScriptArray, AddRef ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_RELEASE, "void f()", asMETHOD( CScriptArray, Release ), asCALL_THISCALL ) );

	CheckASCall( engine->RegisterObjectMethod( "array<T>", "T &opIndex(uint index)", asMETHODPR( CScriptArray, At, ( asUINT ), void * ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "const T &opIndex(uint index) const", asMETHODPR( CScriptArray, At, ( asUINT ) const, const void * ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "array<T> &opAssign(const array<T>&in)", asMETHOD( CScriptArray, operator= ), asCALL_THISCALL ) );

	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void InsertAt( uint, const T& in )", asMETHODPR( CScriptArray, InsertAt, ( asUINT, void * ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void InsertAt( uint, const array<T>& in )", asMETHODPR( CScriptArray, InsertAt, ( asUINT, const CScriptArray& ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Insert( uint, const T& in )", asMETHODPR( CScriptArray, InsertAt, ( asUINT, void * ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Insert( uint, const array<T>& in )", asMETHODPR( CScriptArray, InsertAt, ( asUINT, const CScriptArray& ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void InsertLast( const T& in )", asMETHOD( CScriptArray, InsertLast ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void RemoveAt( uint )", asMETHOD( CScriptArray, RemoveAt ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void RemoveLast()", asMETHOD( CScriptArray, RemoveLast ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Erase( uint )", asMETHOD( CScriptArray, RemoveAt ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Size() const", asMETHOD( CScriptArray, GetSize ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Count() const", asMETHOD( CScriptArray, GetSize ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Find( const T&in if_handle_then_const ) const", asMETHODPR( CScriptArray, Find, ( void * ) const, asUINT ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Find( uint, const T&in if_handle_then_const ) const", asMETHODPR( CScriptArray, Find, ( asUINT, void * ) const, asUINT ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Add(const T&in value)", asMETHOD( CScriptArray, InsertLast ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Clear()", asMETHOD( CScriptArray, Clear ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Resize( uint )", asMETHOD( CScriptArray, Resize ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Reserve( uint )", asMETHOD( CScriptArray, Reserve ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortAsc()", asMETHODPR( CScriptArray, SortAsc, ( void ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortAsc( uint, uint )", asMETHODPR( CScriptArray, SortAsc, ( asUINT, asUINT ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortDesc()", asMETHODPR( CScriptArray, SortDesc, ( void ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortDesc( uint, uint )", asMETHODPR( CScriptArray, SortDesc, ( asUINT, asUINT ), void ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Reverse()", asMETHOD( CScriptArray, Reverse ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterFuncdef( "bool array<T>::Less(const T&in if_handle_then_const a, const T&in if_handle_then_const b)" ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD( CScriptArray, GetRefCount ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_SETGCFLAG, "void f()", asMETHOD( CScriptArray, SetFlag ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD( CScriptArray, GetFlag ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD( CScriptArray, EnumReferences ), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD( CScriptArray, ReleaseAllHandles ), asCALL_THISCALL ) );

	//========================================================================
	// Inheriting container classes
	//

/*
	CheckASCall( engine->RegisterObjectType( "stack<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION( ScriptListTemplateCallback_Generic ), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_FACTORY, "stack<T>@ f(int&in)", asFUNCTION(ScriptListFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_FACTORY, "stack<T>@ f(int&in, uint length) explicit", asFUNCTION(ScriptListFactory2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_FACTORY, "stack<T>@ f(int&in, uint length, const T &in value)", asFUNCTION(ScriptListFactoryDefVal_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_LIST_FACTORY, "stack<T>@ f(int&in, int&in) {repeat T}", asFUNCTION(ScriptListListFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptListAddRef_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptListRelease_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "T &opIndex(uint index)", asFUNCTION(ScriptListAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "const T &opIndex(uint index) const", asFUNCTION(ScriptListAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "stack<T> &opAssign(const stack<T>&in)", asFUNCTION(ScriptListAssignment_Generic), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Push( const T& in )", asFUNCTION(ScriptListInsertLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Pop()", asFUNCTION(ScriptListRemoveLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Erase(uint index)", asFUNCTION(ScriptListRemoveAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "uint Size() const", asFUNCTION(ScriptListLength_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "uint Count() const", asFUNCTION(ScriptListLength_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "int Find(const T&in if_handle_then_const value) const", asFUNCTION(ScriptListFind_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "int Find(uint startAt, const T&in if_handle_then_const value) const", asFUNCTION(ScriptListFind2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Add(const T&in value)", asFUNCTION(ScriptListInsertLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Clear()", asFUNCTION(ScriptListClear_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Resize( uint )", asFUNCTION(ScriptListResize_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Reserve( uint )", asFUNCTION( ScriptListReserve_Generic ), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortAsc()", asFUNCTION(ScriptListSortAsc_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortAsc(uint startAt, uint count)", asFUNCTION(ScriptListSortAsc2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortDesc()", asFUNCTION(ScriptListSortDesc_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortDesc(uint startAt, uint count)", asFUNCTION(ScriptListSortDesc2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Reverse()", asFUNCTION(ScriptListReverse_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterFuncdef( "bool stack<T>::Less(const T&in if_handle_then_const a, const T&in if_handle_then_const b)" ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptListGetRefCount_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptListSetFlag_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptListGetFlag_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptListEnumReferences_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptListReleaseAllHandles_Generic), asCALL_GENERIC ) );
*/
}

CScriptArray &CScriptArray::operator=(const CScriptArray &other)
{
	// Only perform the copy if the array types are the same
	if( &other != this &&
		other.GetArrayObjectType() == GetArrayObjectType() )
	{
		Resize( other.buffer->size );
		CopyBuffer( buffer, other.buffer );
	}

	return *this;
}

CScriptArray::CScriptArray(asITypeInfo *ti, void *buf)
{
	if ( !ti ) {
		N_Error( ERR_FATAL, "CScriptArray::CScriptArray: typeInfo is NULL" );
	}
	if ( N_stricmp( ti->GetName(), "array" ) ) {
		N_Error( ERR_FATAL, "CScriptArray::CScriptArray: typeInfo isn't an array" );
	}
	// The object type should be the template instance of the array
	Assert( ti && !N_stricmp( ti->GetName(), "array" ) );

	memstats.numBuffers++;
	refCount = 1;
	gcFlag = false;
	objType = ti;
	objType->AddRef();
	buffer = NULL;
	Clear();

	memstats.overHeadBytes += sizeof( *buffer );

	Precache();

	asIScriptEngine *engine = ti->GetEngine();

	// Determine element size
	if( subTypeId & asTYPEID_MASK_OBJECT )
		elementSize = sizeof(asPWORD);
	else
		elementSize = engine->GetSizeOfPrimitiveType(subTypeId);

	// Determine the initial size from the buffer
	asUINT length = *(asUINT*)buf;

	// Make sure the array size isn't too large for us to handle
	if( !CheckMaxSize(length) )
	{
		// Don't continue with the initialization
		return;
	}

	// Copy the values of the array elements from the buffer
	if( (ti->GetSubTypeId() & asTYPEID_MASK_OBJECT) == 0 )
	{
		CreateBuffer( &buffer, length );

		// Copy the values of the primitive type into the internal buffer
		if( length > 0 )
			memcpy(buffer, (((asUINT*)buf)+1), length * elementSize);
	}
	else if( ti->GetSubTypeId() & asTYPEID_OBJHANDLE )
	{
		CreateBuffer( &buffer, length );

		// Copy the handles into the internal buffer
		if( length > 0 )
			memcpy(buffer, (((asUINT*)buf)+1), length * elementSize);

		// With object handles it is safe to clear the memory in the received buffer
		// instead of increasing the ref count. It will save time both by avoiding the
		// call the increase ref, and also relieve the engine from having to release
		// its references too
		memset((((asUINT*)buf)+1), 0, length * elementSize);
	}
	else if( ti->GetSubType()->GetFlags() & asOBJ_REF )
	{
		// Only allocate the buffer, but not the objects
		subTypeId |= asTYPEID_OBJHANDLE;
		CreateBuffer( &buffer, length );
		subTypeId &= ~asTYPEID_OBJHANDLE;

		// Copy the handles into the internal buffer
		if( length > 0 )
			memcpy(buffer, (((asUINT*)buf)+1), length * elementSize);

		// For ref types we can do the same as for handles, as they are
		// implicitly stored as handles.
		memset((((asUINT*)buf)+1), 0, length * elementSize);
	}
	else
	{
		// TODO: Optimize by calling the copy constructor of the object instead of
		//       constructing with the default constructor and then assigning the value
		// TODO: With C++11 ideally we should be calling the move constructor, instead
		//       of the copy constructor as the engine will just discard the objects in the
		//       buffer afterwards.
		CreateBuffer( &buffer, length );

		// For value types we need to call the opAssign for each individual object
		for( asUINT n = 0; n < length; n++ )
		{
			void *obj = At(n);
			asBYTE *srcObj = (asBYTE*)buf;
			srcObj += 4 + n*ti->GetSubType()->GetSize();
			engine->AssignScriptObject(obj, srcObj, ti->GetSubType());
		}
	}

	// Notify the GC of the successful creation
	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);
}

CScriptArray::CScriptArray(asUINT length, asITypeInfo *ti)
{
	if ( !ti ) {
		N_Error( ERR_FATAL, "CScriptArray::CScriptArray: typeInfo is NULL" );
	}
	if ( N_stricmp( ti->GetName(), "array" ) ) {
		N_Error( ERR_FATAL, "CScriptArray::CScriptArray: typeInfo isn't an array" );
	}
	// The object type should be the template instance of the array
	Assert( ti && !N_stricmp( ti->GetName(), "array" ) );

	memstats.numBuffers++;
	refCount = 1;
	gcFlag = false;
	objType = ti;
	objType->AddRef();
	buffer = NULL;
	Clear();

	memstats.overHeadBytes += sizeof( *buffer );

	Precache();

	// Determine element size
	if( subTypeId & asTYPEID_MASK_OBJECT )
		elementSize = sizeof(asPWORD);
	else
		elementSize = objType->GetEngine()->GetSizeOfPrimitiveType(subTypeId);

	// Make sure the array size isn't too large for us to handle
	if( !CheckMaxSize(length) )
	{
		// Don't continue with the initialization
		return;
	}

	CreateBuffer( &buffer, length );

	// Notify the GC of the successful creation
	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);
}

CScriptArray::CScriptArray(const CScriptArray &other)
{
	memstats.numBuffers++;
	refCount = 1;
	gcFlag = false;
	objType = other.objType;
	objType->AddRef();
	buffer = NULL;
	Clear();

//	g_pModuleLib->GetModule( asGetActiveContext()->GetSystemFunction()->GetModuleName() )->m_pHandle->AddArrayObject( this );

	memstats.overHeadBytes += sizeof( *buffer );

	Precache();

	elementSize = other.elementSize;

	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);

	// Copy the content
	*this = other;
}

CScriptArray::CScriptArray(asUINT length, void *defVal, asITypeInfo *ti)
{
	// The object type should be the template instance of the array
	Assert( ti && !N_stricmp( ti->GetName(), "array" ) );

	memstats.numBuffers++;
	refCount = 1;
	gcFlag = false;
	objType = ti;
	objType->AddRef();
	buffer = NULL;
	Clear();

//	g_pModuleLib->GetModule( asGetActiveContext()->GetSystemFunction()->GetModuleName() )->m_pHandle->AddArrayObject( this );

	memstats.overHeadBytes += sizeof( *buffer );

	Precache();

	// Determine element size
	if( subTypeId & asTYPEID_MASK_OBJECT )
		elementSize = sizeof(asPWORD);
	else
		elementSize = objType->GetEngine()->GetSizeOfPrimitiveType(subTypeId);

	// Make sure the array size isn't too large for us to handle
	if( !CheckMaxSize(length) )
	{
		// Don't continue with the initialization
		return;
	}

	CreateBuffer( &buffer, length );

	// Notify the GC of the successful creation
	if( objType->GetFlags() & asOBJ_GC )
		objType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objType);

	// Initialize the elements with the default value
	for( asUINT n = 0; n < buffer->size; n++ )
		SetValue(n, defVal);
}

void CScriptArray::SetValue(asUINT index, void *value)
{
	// At() will take care of the out-of-bounds checking, though
	// if called from the application then nothing will be done
	void *ptr = At(index);
	if( ptr == 0 ) return;

	if ((subTypeId & ~asTYPEID_MASK_SEQNBR) && !(subTypeId & asTYPEID_OBJHANDLE))
	{
		asITypeInfo *subType = objType->GetSubType();
		if (subType->GetFlags() & asOBJ_ASHANDLE)
		{
			// For objects that should work as handles we must use the opHndlAssign method
			// TODO: Must support alternative syntaxes as well
			// TODO: Move the lookup of the opHndlAssign method to Precache() so it is only done once
			if ( subTypeHandleAssignFunc )
			{
				// TODO: Reuse active context if existing
				asIScriptEngine* engine = objType->GetEngine();
				asIScriptContext* ctx = engine->RequestContext();
				ctx->Prepare(subTypeHandleAssignFunc);
				ctx->SetObject(ptr);
				ctx->SetArgAddress(0, value);
				// TODO: Handle errors
				ctx->Execute();
				engine->ReturnContext(ctx);
			}
			else
			{
				// opHndlAssign doesn't exist, so try ordinary value assign instead
				objType->GetEngine()->AssignScriptObject(ptr, value, subType);
			}
		}
		else
			objType->GetEngine()->AssignScriptObject(ptr, value, subType);
	}
	else if( subTypeId & asTYPEID_OBJHANDLE )
	{
		void *tmp = *(void**)ptr;
		*(void**)ptr = *(void**)value;
		objType->GetEngine()->AddRefScriptObject(*(void**)value, objType->GetSubType());
		if( tmp )
			objType->GetEngine()->ReleaseScriptObject(tmp, objType->GetSubType());
	}
	else if( subTypeId == asTYPEID_BOOL ||
			 subTypeId == asTYPEID_INT8 ||
			 subTypeId == asTYPEID_UINT8 )
		*(char*)ptr = *(char*)value;
	else if( subTypeId == asTYPEID_INT16 ||
			 subTypeId == asTYPEID_UINT16 )
		*(short*)ptr = *(short*)value;
	else if( subTypeId == asTYPEID_INT32 ||
			 subTypeId == asTYPEID_UINT32 ||
			 subTypeId == asTYPEID_FLOAT ||
			 subTypeId > asTYPEID_DOUBLE ) // enums have a type id larger than doubles
		*(int*)ptr = *(int*)value;
	else if( subTypeId == asTYPEID_INT64 ||
			 subTypeId == asTYPEID_UINT64 ||
			 subTypeId == asTYPEID_DOUBLE )
		*(double*)ptr = *(double*)value;
}

CScriptArray::~CScriptArray()
{
	Clear();
	if ( objType ) {
		objType->Release();
	}
}

asUINT CScriptArray::GetSize( void ) const
{
	if ( !buffer ) {
		return 0;
	}
	return buffer->size;
}

bool CScriptArray::IsEmpty() const
{
	if ( !buffer ) {
		return true;
	}
	return !buffer->size;
}

void CScriptArray::Reserve( asUINT nItems )
{
	if ( !CheckMaxSize( nItems ) ) {
		return;
	}

	asUINT capacity, size;

	capacity = nItems;
	size = 0;
	if ( buffer ) {
		capacity += buffer->capacity;
		size = buffer->size;
	}

	SArrayBuffer *newBuffer = (SArrayBuffer *)Mem_ClearedAlloc( sizeof( *newBuffer ) + ( capacity * elementSize ) );
	if ( buffer ) {
		memcpy( newBuffer->data, buffer->data, buffer->size * elementSize );
		Mem_Free( buffer );
	}

	buffer = newBuffer;
	buffer->size = size;
	buffer->capacity = capacity;
}

void CScriptArray::Resize(asUINT numElements)
{
	if( !CheckMaxSize(numElements) )
		return;

	DoAllocate( (int)numElements - (int)buffer->size, (asUINT)-1 );
}

void CScriptArray::RemoveRange(asUINT start, asUINT count)
{
	if ( !count )
		return;

	if( !buffer || start > buffer->size ) {
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if (ctx)
			ctx->SetException("out of range index");
		return;
	}

	// Cap count to the end of the array
	if (start + count > buffer->size) {
		count = buffer->size - start;
	}

	// Destroy the elements that are being removed
	Destruct( buffer, start, start + count);

	// Compact the elements
	// As objects in arrays of objects are not stored inline, it is safe to use memmove here
	// since we're just copying the pointers to objects and not the actual objects.
	memmove( buffer->data + start*elementSize, buffer->data + (start + count)*elementSize, (buffer->size - start - count)*elementSize);
	buffer->size -= count;
}

//
// CScriptArray::Clear: resets count
//
void CScriptArray::Clear( void )
{
	if ( !buffer ) {
		return;
	}
	
	memstats.currentBytesAllocated -= buffer->capacity * elementSize;
	Destruct( buffer, 0, buffer->size );
	
	// clear it all
	Mem_Free( buffer );
	buffer = NULL;

	memstats.numBuffers--;
}

void CScriptArray::AllocBuffer( uint32_t nItems )
{
	if ( !buffer ) {
		buffer = (SArrayBuffer *)Mem_ClearedAlloc( sizeof( *buffer ) + ( ( nItems * 4 ) * elementSize ) );
		if ( !buffer ) {
			asIScriptContext *pContext = asGetActiveContext();
			if ( pContext ) {
				pContext->SetException( va( "Mem_ClearedAlloc() failed on %lu bytes",
					sizeof( *buffer ) + ( ( nItems * 4 ) * elementSize ) ) );
			}
		}
		buffer->size = nItems;
		buffer->capacity = nItems * 4;
		Construct( buffer, 0, nItems );

		memstats.totalBytesAllocated += buffer->capacity * elementSize;
		memstats.numAllocs++;
		memstats.currentBytesAllocated += buffer->capacity * elementSize;
	}
}

//
// CScriptArray::DoAllocate: checks if the buffer is large enough to support
// nItems, if not, allocate nItems * 4 more
//
void CScriptArray::DoAllocate( int delta, uint32_t at )
{
	if ( delta < 0 ) {
		if ( -delta > (int)buffer->size ) {
			delta = -(int)buffer->size;
		}
		if ( at > buffer->size + delta ) {
			at = buffer->size + delta;
		}
	} else if ( delta > 0 ) {
		// make sure the array size isn't too large for us to handle
		if ( delta > 0 && !CheckMaxSize( buffer->size + delta ) ) {
			return;
		}
		if ( at > buffer->size ) {
			at = buffer->size;
		}
	}

	if ( delta == 0 ) {
		return;
	}

	if ( buffer->size + delta >= buffer->capacity ) {
		// allocate new space
		buffer->capacity += delta * 4;

		SArrayBuffer *buf = (SArrayBuffer *)Mem_ClearedAlloc( sizeof( *buf ) + ( buffer->capacity * elementSize ) );
		if ( buf ) {
			buf->size = buffer->size + delta;
			buf->capacity = buffer->capacity;
		} else {
			asIScriptContext *pContext = asGetActiveContext();
			if ( pContext ) {
				pContext->SetException( va( "Mem_ClearedAlloc() failed on %lu bytes",
					sizeof( *buf ) + ( buffer->capacity * elementSize ) ) );
			}
		}

		// as objects in arrays of objects are not stored inline, it is safe to use a block copy here
		// since we're just copying the pointers to objects and not the actual objects.
		memcpy( buf->data, buffer->data, at * elementSize );
		if ( at < buffer->size ) {
			memcpy( buf->data + ( at + delta ) * elementSize, buffer->data + at * elementSize, ( buffer->size - at ) * elementSize );
		}
		Construct( buf, at, at + delta );
		Mem_Free( (byte *)buffer );

		buffer = buf;
	} else if ( delta < 0 ) {
		Destruct( buffer, at, at - delta );

		// as objects in arrays of objects are not stored inline, it is safe to use a block copy here
		// since we're just copying the pointers to objects and not the actual objects.
		memmove( buffer->data + at * elementSize, buffer->data + ( at - delta ) * elementSize, ( buffer->size -
			( at - delta ) ) * elementSize );
		buffer->size += delta;
	} else {
		memmove( buffer->data + ( at + delta ) * elementSize, buffer->data + at * elementSize, ( buffer->size - at ) * elementSize );
		Construct( buffer, at, at + delta );
		buffer->size += delta;
	}
}

// internal
bool CScriptArray::CheckMaxSize( asUINT numElements )
{
	// This code makes sure the size of the buffer that is allocated
	// for the array doesn't overflow and becomes smaller than requested

	asUINT maxSize = MAX_UINT;
	if ( elementSize > 0 ) {
		maxSize /= elementSize;
	}

	if ( numElements > maxSize ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "invalid new array length, %u, integer overflow", numElements ) );
		}
		return false;
	}

	// OK
	return true;
}

asITypeInfo *CScriptArray::GetArrayObjectType() const
{
	return objType;
}

int CScriptArray::GetArrayTypeId() const
{
	return objType->GetTypeId();
}

int CScriptArray::GetElementTypeId() const
{
	return subTypeId;
}

void CScriptArray::InsertAt(asUINT index, void *value)
{
	if ( index > buffer->size ) {
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "out of range index: %u", index ) );
		}
		return;
	}
	DoAllocate( 1, index );
	SetValue( index, value );
}

void CScriptArray::InsertAt( asUINT index, const CScriptArray& arr )
{
	asUINT n, m;

	if ( index > buffer->size ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "out of range index: %u", index ) );
		}
		return;
	}

	if ( objType != arr.objType ) {
		// This shouldn't really be possible to happen when
		// called from a script, but let's check for it anyway
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( "mismatching array types" );
		}
		return;
	}

	const asUINT nItems = arr.buffer->size;
	DoAllocate( nItems, index );
	if ( &arr == this ) {
		for ( n = 0; n < arr.buffer->size; n++ ) {
			// This const cast is allowed, since we know the
			// value will only be used to make a copy of it
			void *value = const_cast<void*>(arr.At(n));
			SetValue(index + n, value);
		}
	}
	else
	{
		// The array that is being inserted is the same as this one.
		// So we should iterate over the elements before the index,
		// and then the elements after
		for ( n = 0; n < index; n++ ) {
			// This const cast is allowed, since we know the
			// value will only be used to make a copy of it
			void *value = const_cast<void*>(arr.At(n));
			SetValue(index + n, value);
		}

		for ( n = index + nItems, m = 0; n < arr.buffer->size; n++, m++ ) {
			// This const cast is allowed, since we know the
			// value will only be used to make a copy of it
			void *value = const_cast<void*>(arr.At(n));
			SetValue(index + index + m, value);
		}
	}
}

void CScriptArray::InsertLast( void *value )
{
	if ( !buffer ) {
		CreateBuffer( &buffer, 8 );
	}
	InsertAt( buffer->size, value );
}

void CScriptArray::RemoveAt( asUINT index )
{
	if( index >= buffer->size )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "out of range index: %u", index ) );
		}
		return;
	}

	DoAllocate( -1, index );
}

void CScriptArray::RemoveLast( void )
{
	RemoveAt( buffer->size - 1 );
}

void CScriptArray::DeleteBuffer( SArrayBuffer *buffer )
{
	Destruct( buffer, 0, buffer->size );
	Mem_Free( buffer );
}


// Return a pointer to the array element. Returns 0 if the index is out of bounds
const void *CScriptArray::At( asUINT index ) const
{
	if( !buffer ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( "buffer not allocated" );
		}
	}
	if ( index >= buffer->size ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "out of range index: %u", index ) );
		}
	}

	if( (subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) ) {
		return *(void**)( buffer->data + elementSize*index);
	}
	
	return (const void *)( buffer->data + ( elementSize * index ) );
}
void *CScriptArray::At(asUINT index)
{
	if( !buffer ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( "buffer not allocated" );
		}
		return NULL;
	}
	if ( index >= buffer->size ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "out of range index: %u", index ) );
		}
		return NULL;
	}

	if( (subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) ) {
		return *(void**)( buffer->data + elementSize*index);
	}
	
	return (void *)( buffer->data + ( elementSize * index ) );
}

void *CScriptArray::GetBuffer( void ) {
	return buffer->data;
}

const void *CScriptArray::GetBuffer( void ) const {
	return buffer->data;
}


// internal
void CScriptArray::CreateBuffer( SArrayBuffer **buffer, asUINT nItems )
{
	*buffer = (SArrayBuffer *)Mem_ClearedAlloc( sizeof( *buffer ) + ( nItems * elementSize ) );

	if ( *buffer ) {
		(*buffer)->size = nItems;
		(*buffer)->capacity = nItems;
		Construct( *buffer, 0, nItems );
	} else {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "Mem_ClearedAlloc() failed on %lu bytes", sizeof( *buffer ) + ( nItems * elementSize ) ) );
		}
		return;
	}
}


// internal
void CScriptArray::Construct( SArrayBuffer *buf, asUINT start, asUINT end )
{
	if( (subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) )
	{
		// Create an object using the default constructor/factory for each element
		void **max = (void**)( buf->data + end * sizeof(void*));
		void **d = (void**)( buf->data + start * sizeof(void*));

		asIScriptEngine *engine = objType->GetEngine();
		asITypeInfo *subType = objType->GetSubType();

		for( ; d < max; d++ )
		{
			*d = (void*)engine->CreateScriptObject(subType);
			if( *d == 0 )
			{
				// Set the remaining entries to null so the destructor
				// won't attempt to destroy invalid objects later
				memset(d, 0, sizeof(void*)*(max-d));

				// There is no need to set an exception on the context,
				// as CreateScriptObject has already done that
				return;
			}
		}
	}
	else
	{
		// Set all elements to zero whether they are handles or primitives
		void *d = (void*)( buf->data + start * elementSize );
		memset(d, 0, (end-start)*elementSize);
	}
}

// internal
void CScriptArray::Destruct(SArrayBuffer *buf, asUINT start, asUINT end)
{
	if( subTypeId & asTYPEID_MASK_OBJECT ) {
		asIScriptEngine *engine = objType->GetEngine();

		void **max = (void **)( buf->data + end * sizeof( void * ) );
		void **d = (void **)( buf->data + start * sizeof( void * ) );

		for( ; d < max; d++ ) {
			if( *d )
				engine->ReleaseScriptObject(*d, objType->GetSubType());
		}
	}
}


// internal
bool CScriptArray::Less(const void *a, const void *b, bool asc)
{
	if( !asc )
	{
		// Swap items
		const void *TEMP = a;
		a = b;
		b = TEMP;
	}

	if( !(subTypeId & ~asTYPEID_MASK_SEQNBR) )
	{
		// Simple compare of values
		switch( subTypeId )
		{
			#define COMPARE(T) *((T*)a) < *((T*)b)
			case asTYPEID_BOOL:   return COMPARE(bool);
			case asTYPEID_INT8:   return COMPARE(asINT8);
			case asTYPEID_INT16:  return COMPARE(asINT16);
			case asTYPEID_INT32:  return COMPARE(asINT32);
			case asTYPEID_INT64:  return COMPARE(asINT64);
			case asTYPEID_UINT8:  return COMPARE(asBYTE);
			case asTYPEID_UINT16: return COMPARE(asWORD);
			case asTYPEID_UINT32: return COMPARE(asDWORD);
			case asTYPEID_UINT64: return COMPARE(asQWORD);
			case asTYPEID_FLOAT:  return COMPARE(float);
			case asTYPEID_DOUBLE: return COMPARE(double);
			default: return COMPARE(signed int); // All enums fall in this case
			#undef COMPARE
		}
	}

	return false;
}

void CScriptArray::Reverse()
{
	asUINT size = buffer->size;

	if( size >= 2 )
	{
		asBYTE TEMP[16];

		for( asUINT i = 0; i < size / 2; i++ )
		{
			Copy(TEMP, GetArrayItemPointer(i));
			Copy(GetArrayItemPointer(i), GetArrayItemPointer(size - i - 1));
			Copy(GetArrayItemPointer(size - i - 1), TEMP);
		}
	}
}

bool CScriptArray::operator==(const CScriptArray &other) const
{
	if( objType != other.objType )
		return false;

	if( buffer->size != other.buffer->size )
		return false;

	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = objType->GetEngine()->CreateContext();
		}
	}

	// Check if all elements are equal
	bool isEqual = true;
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	for( asUINT n = 0; n < buffer->size; n++ )
		if( !Equals(At(n), other.At(n), cmpContext, cache) )
		{
			isEqual = false;
			break;
		}

	if( cmpContext )
	{
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
	}

	return isEqual;
}

// internal
bool CScriptArray::Equals(const void *a, const void *b, asIScriptContext *ctx, SArrayCache *cache) const
{
	if( !(subTypeId & ~asTYPEID_MASK_SEQNBR) )
	{
		// Simple compare of values
		switch( subTypeId )
		{
			#define COMPARE(T) *((T*)a) == *((T*)b)
			case asTYPEID_BOOL:   return COMPARE(bool);
			case asTYPEID_INT8:   return COMPARE(asINT8);
			case asTYPEID_INT16:  return COMPARE(asINT16);
			case asTYPEID_INT32:  return COMPARE(asINT32);
			case asTYPEID_INT64:  return COMPARE(asINT64);
			case asTYPEID_UINT8:  return COMPARE(asBYTE);
			case asTYPEID_UINT16: return COMPARE(asWORD);
			case asTYPEID_UINT32: return COMPARE(asDWORD);
			case asTYPEID_UINT64: return COMPARE(asQWORD);
			case asTYPEID_FLOAT:  return COMPARE(float);
			case asTYPEID_DOUBLE: return COMPARE(double);
			default: return COMPARE(signed int); // All enums fall here. TODO: update this when enums can have different sizes and types
			#undef COMPARE
		}
	}
	else
	{
		int r = 0;

		if( subTypeId & asTYPEID_OBJHANDLE )
		{
			// Allow the find to work even if the array contains null handles
			if( *(void**)a == *(void**)b ) return true;
		}

		// Execute object opEquals if available
		if( cache && cache->eqFunc )
		{
			// TODO: Add proper error handling
			r = ctx->Prepare(cache->eqFunc); Assert(r >= 0);

			if( subTypeId & asTYPEID_OBJHANDLE )
			{
				r = ctx->SetObject(*((void**)a)); Assert(r >= 0);
				r = ctx->SetArgObject(0, *((void**)b)); Assert(r >= 0);
			}
			else
			{
				r = ctx->SetObject((void*)a); Assert(r >= 0);
				r = ctx->SetArgObject(0, (void*)b); Assert(r >= 0);
			}

			r = ctx->Execute();

			if( r == asEXECUTION_FINISHED )
				return ctx->GetReturnByte() != 0;

			return false;
		}

		// Execute object opCmp if available
		if( cache && cache->cmpFunc )
		{
			// TODO: Add proper error handling
			r = ctx->Prepare(cache->cmpFunc); Assert(r >= 0);

			if( subTypeId & asTYPEID_OBJHANDLE )
			{
				r = ctx->SetObject(*((void**)a)); Assert(r >= 0);
				r = ctx->SetArgObject(0, *((void**)b)); Assert(r >= 0);
			}
			else
			{
				r = ctx->SetObject((void*)a); Assert(r >= 0);
				r = ctx->SetArgObject(0, (void*)b); Assert(r >= 0);
			}

			r = ctx->Execute();

			if( r == asEXECUTION_FINISHED )
				return (int)ctx->GetReturnDWord() == 0;

			return false;
		}
	}

	return false;
}

asUINT CScriptArray::FindByRef(void *ref) const
{
	return FindByRef(0, ref);
}

asUINT CScriptArray::FindByRef(asUINT startAt, void *ref) const
{
	// Find the matching element by its reference
	asUINT size = buffer->size;
	if( subTypeId & asTYPEID_OBJHANDLE )
	{
		// Dereference the pointer
		ref = *(void**)ref;
		for( asUINT i = startAt; i < size; i++ )
		{
			if( *(void**)At(i) == ref )
				return i;
		}
	}
	else
	{
		// Compare the reference directly
		for ( asUINT i = startAt; i < size; i++ ) {
			if ( At( i ) == ref ) {
				return i;
			}
		}
	}

	return buffer->size;
}

asUINT CScriptArray::Find( void *value ) const
{
	return Find(0, value);
}

asUINT CScriptArray::Find( asUINT startAt, void *value ) const
{
	// Check if the subtype really supports find()
	// TODO: Can't this be done at compile time too by the template callback
	SArrayCache *cache;
	asIScriptContext *cmpContext;
	bool isNested;
	asUINT ret;
	asUINT size;

	cache = NULL;
	isNested = false;
	cmpContext = NULL;
	ret = buffer->size;

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
		if( !cache || (cache->cmpFunc == 0 && cache->eqFunc == 0) )
		{
			asIScriptContext *ctx = asGetActiveContext();
			asITypeInfo* subType = objType->GetEngine()->GetTypeInfoById(subTypeId);

			// Throw an exception
			if( ctx ) {
				char tmp[4096];

				if( cache && cache->eqFuncReturnCode == asMULTIPLE_FUNCTIONS ) {
					Com_snprintf( tmp, sizeof( tmp ) - 1, "Type '%s' has multiple matching opEquals or opCmp methods", subType->GetName() );
				} else {
					Com_snprintf( tmp, sizeof( tmp ) - 1, "Type '%s' does not have a matching opEquals or opCmp method", subType->GetName() );
				}
				ctx->SetException( tmp );
			}

			return buffer->size;
		}
	}

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = objType->GetEngine()->CreateContext();
		}
	}

	// Find the matching element
	for( asUINT i = startAt; i < buffer->size; i++ ) {
		// value passed by reference
		if( Equals(At(i), value, cmpContext, cache) )
		{
			ret = i;
			break;
		}
	}

	if( cmpContext )
	{
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
	}

	return ret;
}



// internal
// Copy object handle or primitive value
// Even in arrays of objects the objects are allocated on
// the heap and the array stores the pointers to the objects
void CScriptArray::Copy(void *dst, void *src)
{
	memcpy(dst, src, elementSize);
}


// internal
// Swap two elements
// Even in arrays of objects the objects are allocated on 
// the heap and the array stores the pointers to the objects.
void CScriptArray::Swap(void* a, void* b)
{
	// [SIREngine] 3/27/24
	// this was just a byte[16] array for some reason...
	// that CAN and WILL segfault, so we're just using alloca
	void *tmp = alloca16( elementSize );

	Copy(tmp, a);
	Copy(a, b);
	Copy(b, tmp);
}


// internal
// Return pointer to array item (object handle or primitive value)
void *CScriptArray::GetArrayItemPointer(int index)
{
	return buffer->data + ( index * elementSize );
}

// internal
// Return pointer to data in buffer (object or primitive)
void *CScriptArray::GetDataPointer(void *buf)
{
	if ((subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) )
	{
		// [SIREngine] 3/27/24
		// why was this a size_t before?

		return (void *)(*(uintptr_t*)buf);
	}
	else
	{
		// Primitive is just a raw data
		return buf;
	}
}


// Sort ascending
void CScriptArray::SortAsc()
{
	Sort(0, buffer->size, true);
}

// Sort ascending
void CScriptArray::SortAsc(asUINT startAt, asUINT count)
{
	Sort(startAt, count, true);
}

// Sort descending
void CScriptArray::SortDesc()
{
	Sort(0, buffer->size, false);
}

// Sort descending
void CScriptArray::SortDesc(asUINT startAt, asUINT count)
{
	Sort(startAt, count, false);
}


// internal
void CScriptArray::Sort(asUINT startAt, asUINT count, bool asc)
{
	// Subtype isn't primitive and doesn't have opCmp
	SArrayCache *cache = (SArrayCache *)objType->GetUserData(ARRAY_CACHE);
	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		if( !cache || cache->cmpFunc == 0 )
		{
			asIScriptContext *ctx = asGetActiveContext();
			asITypeInfo* subType = objType->GetEngine()->GetTypeInfoById(subTypeId);

			// Throw an exception
			if( ctx )
			{
				char tmp[4096];

				if( cache && cache->cmpFuncReturnCode == asMULTIPLE_FUNCTIONS ) {
					Com_snprintf( tmp, sizeof( tmp ) - 1, "Type '%s' has multiple matching opCmp methods", subType->GetName() );
				} else {
					Com_snprintf( tmp, sizeof( tmp ) - 1, "Type '%s' does not have a matching opCmp method", subType->GetName() );
				}

				ctx->SetException(tmp);
			}

			return;
		}
	}

	// No need to sort
	if( count < 2 )
	{
		return;
	}

	uint32_t start = startAt;
	uint32_t end = startAt + count;

	// Check if we could access invalid item while sorting
	if( start >= buffer->size || end > buffer->size )
	{
		asIScriptContext *ctx = asGetActiveContext();

		// Throw an exception
		if( ctx )
		{
			ctx->SetException("out of range index");
		}

		return;
	}

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		asIScriptContext *cmpContext = 0;
		bool isNested = false;

		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
			cmpContext = objType->GetEngine()->RequestContext();

		// Do the sorting
		struct {
			bool               asc;
			asIScriptContext  *cmpContext;
			asIScriptFunction *cmpFunc;
			bool operator()(void *a, void *b) const
			{
				if( !asc )
				{
					// Swap items
					void *TEMP = a;
					a = b;
					b = TEMP;
				}

				int r = 0;

				// Allow sort to work even if the array contains null handles
				if( a == 0 ) return true;
				if( b == 0 ) return false;

				// Execute object opCmp
				if( cmpFunc )
				{
					CheckASCall( cmpContext->Prepare(cmpFunc) );
					CheckASCall( cmpContext->SetObject(a) );
					CheckASCall( cmpContext->SetArgObject(0, b) );
					r = cmpContext->Execute();

					if( r == asEXECUTION_FINISHED )
					{
						return (int)cmpContext->GetReturnDWord() < 0;
					}
				}

				return false;
			}
		} customLess = {asc, cmpContext, cache ? cache->cmpFunc : 0};
		eastl::sort((void**)GetArrayItemPointer(start), (void**)GetArrayItemPointer(end), customLess);

		// Clean up
		if( cmpContext )
		{
			if( isNested )
			{
				asEContextState state = cmpContext->GetState();
				cmpContext->PopState();
				if( state == asEXECUTION_ABORTED )
					cmpContext->Abort();
			}
			else
				objType->GetEngine()->ReturnContext(cmpContext);
		}
	}
	else
	{
		// TODO: Use std::sort for primitive types too

		// AGAIN, alloca instead of a static sized array
		void *tmp = alloca16( elementSize );
		for( int i = start + 1; i < end; i++ )
		{
			Copy(tmp, GetArrayItemPointer(i));

			int j = i - 1;

			while( j >= start && Less(GetDataPointer(tmp), At(j), asc) )
			{
				Copy(GetArrayItemPointer(j + 1), GetArrayItemPointer(j));
				j--;
			}

			Copy(GetArrayItemPointer(j + 1), tmp);
		}
	}
}

// Sort with script callback for comparing elements
void CScriptArray::Sort(asIScriptFunction *func, asUINT startAt, asUINT count)
{
	// No need to sort
	if (count < 2)
		return;

	// Check if we could access invalid item while sorting
	asUINT start = startAt;
	asUINT end = asQWORD(startAt) + asQWORD(count) >= (asQWORD(1)<<32) ? 0xFFFFFFFF : startAt + count;
	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if (end > buffer->size)
		end = buffer->size;

	if (start >= buffer->size)
	{
		asIScriptContext *ctx = asGetActiveContext();

		// Throw an exception
		if (ctx)
			ctx->SetException("out of range index");

		return;
	}

	// Try to reuse the active context
	cmpContext = asGetActiveContext();
	if (cmpContext)
	{
		if (cmpContext->GetEngine() == objType->GetEngine() && cmpContext->PushState() >= 0)
			isNested = true;
		else
			cmpContext = 0;
	}
	if (cmpContext == 0)
		cmpContext = objType->GetEngine()->RequestContext();

	// TODO: Security issue: If the array is accessed from the callback while the sort is going on the result may be unpredictable
	//       For example, the callback resizes the array in the middle of the sort
	//       Possible solution: set a lock flag on the array, and prohibit modifications while the lock flag is set

	// Bubble sort
	// TODO: optimize: Use an efficient sort algorithm
	for (asUINT i = start; i+1 < end; i++)
	{
		asUINT best = i;
		for (asUINT j = i + 1; j < end; j++)
		{
			cmpContext->Prepare(func);
			cmpContext->SetArgAddress(0, At(j));
			cmpContext->SetArgAddress(1, At(best));
			int r = cmpContext->Execute();
			if (r != asEXECUTION_FINISHED)
				break;
			if (*(bool*)(cmpContext->GetAddressOfReturnValue()))
				best = j;
		}

		// With Swap we guarantee that the array always sees all references
		// if the GC calls the EnumReferences in the middle of the sorting
		if( best != i )
			Swap(GetArrayItemPointer(i), GetArrayItemPointer(best));
	}

	if (cmpContext)
	{
		if (isNested)
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if (state == asEXECUTION_ABORTED)
				cmpContext->Abort();
		}
		else
			objType->GetEngine()->ReturnContext(cmpContext);
	}
}


// internal
void CScriptArray::CopyBuffer( SArrayBuffer *dst, SArrayBuffer *src )
{
	asIScriptEngine *engine = objType->GetEngine();

	if ( subTypeId & asTYPEID_OBJHANDLE ) {
		// Copy the references and increase the reference counters
		if ( dst->size > 0 && src->size > 0 ) {
			const uint32_t count = dst->size > src->size ? src->size : dst->size;

			void **max = (void**)( dst->data + count * sizeof(void*));
			void **d   = (void**)dst->data;
			void **s   = (void**)src->data;

			for ( ; d < max; d++, s++ ) {
				void *tmp = *d;
				*d = *s;
				if( *d )
					engine->AddRefScriptObject(*d, objType->GetSubType());
				// Release the old ref after incrementing the new to avoid problem incase it is the same ref
				if( tmp )
					engine->ReleaseScriptObject(tmp, objType->GetSubType());
			}
		}
	}
	else {
		if ( dst->size > 0 && src->size > 0 ) {
			const uint32_t count = dst->size > src->size ? src->size : dst->size;
			if( subTypeId & asTYPEID_MASK_OBJECT )
			{
				// Call the assignment operator on all of the objects
				void **max = (void**)( dst->data + count * sizeof(void*));
				void **d   = (void**)dst->data;
				void **s   = (void**)src->data;

				asITypeInfo *subType = objType->GetSubType();
				if (subType->GetFlags() & asOBJ_ASHANDLE)
				{
					// For objects that should work as handles we must use the opHndlAssign method
					// TODO: Must support alternative syntaxes as well
					// TODO: Move the lookup of the opHndlAssign method to Precache() so it is only done once
					if (subTypeHandleAssignFunc)
					{
						// TODO: Reuse active context if existing
						asIScriptContext* ctx = engine->RequestContext();
						for (; d < max; d++, s++)
						{
							ctx->Prepare(subTypeHandleAssignFunc);
							ctx->SetObject(*d);
							ctx->SetArgAddress(0, *s);
							// TODO: Handle errors
							ctx->Execute();
						}
						engine->ReturnContext(ctx);
					}
					else
					{
						// opHndlAssign doesn't exist, so try ordinary value assign instead
						for (; d < max; d++, s++)
							engine->AssignScriptObject(*d, *s, subType);
					}
				}
				else
					for( ; d < max; d++, s++ )
						engine->AssignScriptObject(*d, *s, subType);
			}
			else
			{
				// Primitives are copied byte for byte
				memcpy( dst->data, src->data, count*elementSize );
			}
		}
	}
}

// internal
// Precache some info
void CScriptArray::Precache( void )
{
	subTypeId = objType->GetSubTypeId();

	// Check if it is an array of objects. Only for these do we need to cache anything
	// Type ids for primitives and enums only has the sequence number part
	if( !(subTypeId & ~asTYPEID_MASK_SEQNBR) )
		return;

	// The opCmp and opEquals methods are cached because the searching for the
	// methods is quite time consuming if a lot of array objects are created.

	// First check if a cache already exists for this array type
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	if( cache ) {
		return;
	}

	// We need to make sure the cache is created only once, even
	// if multiple threads reach the same point at the same time
//	asAcquireExclusiveLock();

	// Now that we got the lock, we need to check again to make sure the
	// cache wasn't created while we were waiting for the lock
	cache = reinterpret_cast<SArrayCache*>(objType->GetUserData(ARRAY_CACHE));
	if( cache )
	{
//		asReleaseExclusiveLock();
		return;
	}

	// Create the cache
	cache = (SArrayCache *)Mem_ClearedAlloc( sizeof( SArrayCache ) );
	if( !cache ) {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException( va( "Mem_ClearedAlloc() failed on SArrayCache (%lu bytes)", sizeof( SArrayCache ) ) );
		}
//		asReleaseExclusiveLock();
		return;
	}

	// If the sub type is a handle to const, then the methods must be const too
	bool mustBeConst = (subTypeId & asTYPEID_HANDLETOCONST) ? true : false;

	asITypeInfo *subType = objType->GetEngine()->GetTypeInfoById(subTypeId);
	if ( subType ) {
		for( asUINT i = 0; i < subType->GetMethodCount(); i++ )
		{
			asIScriptFunction *func = subType->GetMethodByIndex(i);

			if( func->GetParamCount() == 1 && (!mustBeConst || func->IsReadOnly()) )
			{
				asDWORD flags = 0;
				int returnTypeId = func->GetReturnTypeId(&flags);

				// The method must not return a reference
				if( flags != asTM_NONE ) {
					continue;
				}

				// opCmp returns an int and opEquals returns a bool
				bool isCmp = false, isEq = false;
				if( returnTypeId == asTYPEID_INT32 && strcmp(func->GetName(), "opCmp") == 0 )
					isCmp = true;
				if( returnTypeId == asTYPEID_BOOL && strcmp(func->GetName(), "opEquals") == 0 )
					isEq = true;

				if( !isCmp && !isEq )
					continue;

				// The parameter must either be a reference to the subtype or a handle to the subtype
				int paramTypeId;
				func->GetParam(0, &paramTypeId, &flags);

				if( (paramTypeId & ~(asTYPEID_OBJHANDLE|asTYPEID_HANDLETOCONST)) != (subTypeId &  ~(asTYPEID_OBJHANDLE|asTYPEID_HANDLETOCONST)) )
					continue;

				if( (flags & asTM_INREF) )
				{
					if( (paramTypeId & asTYPEID_OBJHANDLE) || (mustBeConst && !(flags & asTM_CONST)) )
						continue;
				}
				else if( paramTypeId & asTYPEID_OBJHANDLE )
				{
					if( mustBeConst && !(paramTypeId & asTYPEID_HANDLETOCONST) )
						continue;
				}
				else
					continue;

				if( isCmp )
				{
					if( cache->cmpFunc || cache->cmpFuncReturnCode )
					{
						cache->cmpFunc = 0;
						cache->cmpFuncReturnCode = asMULTIPLE_FUNCTIONS;
					}
					else
						cache->cmpFunc = func;
				}
				else if( isEq )
				{
					if( cache->eqFunc || cache->eqFuncReturnCode )
					{
						cache->eqFunc = 0;
						cache->eqFuncReturnCode = asMULTIPLE_FUNCTIONS;
					}
					else
						cache->eqFunc = func;
				}
			}
		}
	}

	{
		// [SIREngine] 3/27/24
		// cache it
		char decl[4096];
		Com_snprintf( decl, sizeof( decl ) - 1, "%s& opHndlAssign( const %s& in )", subType->GetName(), subType->GetName() );
		subTypeHandleAssignFunc = subType->GetMethodByDecl( decl );
	}

	if( cache->eqFunc == 0 && cache->eqFuncReturnCode == 0 )
		cache->eqFuncReturnCode = asNO_FUNCTION;
	if( cache->cmpFunc == 0 && cache->cmpFuncReturnCode == 0 )
		cache->cmpFuncReturnCode = asNO_FUNCTION;

	// Set the user data only at the end so others that retrieve it will know it is complete
	objType->SetUserData(cache, ARRAY_CACHE);

//	asReleaseExclusiveLock();
}

// GC behaviour
void CScriptArray::EnumReferences(asIScriptEngine *engine)
{
	// TODO: If garbage collection can be done from a separate thread, then this method must be
	//       protected so that it doesn't get lost during the iteration if the array is modified

	// If the array is holding handles, then we need to notify the GC of them
	if( subTypeId & asTYPEID_MASK_OBJECT )
	{
		void **d = (void**)buffer->data;

		if ( !buffer ) {
			return;
		}

		asITypeInfo *subType = engine->GetTypeInfoById(subTypeId);
		if ((subType->GetFlags() & asOBJ_REF))
		{
			// For reference types we need to notify the GC of each instance
			for (asUINT n = 0; n < buffer->size; n++)
			{
				if (d[n])
					engine->GCEnumCallback(d[n]);
			}
		}
		else if ((subType->GetFlags() & asOBJ_VALUE) && (subType->GetFlags() & asOBJ_GC))
		{
			// For value types we need to forward the enum callback
			// to the object so it can decide what to do
			for (asUINT n = 0; n < buffer->size; n++)
			{
				if (d[n])
					engine->ForwardGCEnumReferences(d[n], subType);
			}
		}
	}
}

void CScriptArray::ReleaseAllHandles( asIScriptEngine * )
{
	Clear();
}

void CScriptArray::AddRef() const
{
	gcFlag = false;
	refCount.fetch_add();
}

void CScriptArray::Release() const
{
	gcFlag = false;
	if ( refCount.fetch_sub() == 0 ) {
		this->~CScriptArray();
		Mem_Free( const_cast<CScriptArray *>( this ) );
	}
}

int CScriptArray::GetRefCount()
{
	return refCount.load();
}

void CScriptArray::SetFlag()
{
	gcFlag = true;
}

bool CScriptArray::GetFlag()
{
	return gcFlag;
}

//--------------------------------------------
// Generic calling conventions

static void ScriptListFactory_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *(asITypeInfo**)gen->GetAddressOfArg(0);

	*reinterpret_cast<CScriptArray**>(gen->GetAddressOfReturnLocation()) = CScriptArray::Create(ti);
}

static void ScriptListFactory2_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *(asITypeInfo**)gen->GetAddressOfArg(0);
	asUINT length = gen->GetArgDWord(1);

	*reinterpret_cast<CScriptArray**>(gen->GetAddressOfReturnLocation()) = CScriptArray::Create(ti, length);
}

static void ScriptListListFactory_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *(asITypeInfo**)gen->GetAddressOfArg(0);
	void *buf = gen->GetArgAddress(1);

	*reinterpret_cast<CScriptArray**>(gen->GetAddressOfReturnLocation()) = CScriptArray::Create(ti, buf);
}

static void ScriptListFactoryDefVal_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *(asITypeInfo**)gen->GetAddressOfArg(0);
	asUINT length = gen->GetArgDWord(1);
	void *defVal = gen->GetArgAddress(2);

	*reinterpret_cast<CScriptArray**>(gen->GetAddressOfReturnLocation()) = CScriptArray::Create(ti, length, defVal);
}

static void ScriptListTemplateCallback_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *(asITypeInfo**)gen->GetAddressOfArg(0);
	bool *dontGarbageCollect = *(bool**)gen->GetAddressOfArg(1);
	*reinterpret_cast<bool*>(gen->GetAddressOfReturnLocation()) = ScriptListTemplateCallback(ti, dontGarbageCollect);
}

static void ScriptListAssignment_Generic(asIScriptGeneric *gen)
{
	CScriptArray *other = (CScriptArray*)gen->GetArgObject(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	*self = *other;
	gen->SetReturnObject(self);
}

static void ScriptListEquals_Generic(asIScriptGeneric *gen)
{
	CScriptArray *other = (CScriptArray*)gen->GetArgObject(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	gen->SetReturnByte(self->operator==(*other));
}

static void ScriptListFind_Generic(asIScriptGeneric *gen)
{
	void *value = gen->GetArgAddress(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	gen->SetReturnDWord(self->Find(value));
}

static void ScriptListFind2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	void *value = gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	gen->SetReturnDWord(self->Find(index, value));
}

static void ScriptListFindByRef_Generic(asIScriptGeneric *gen)
{
	void *value = gen->GetArgAddress(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	gen->SetReturnDWord(self->FindByRef(value));
}

static void ScriptListFindByRef2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	void *value = gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	gen->SetReturnDWord(self->FindByRef(index, value));
}

static void ScriptListAt_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();

	gen->SetReturnAddress(self->At(index));
}

static void ScriptListInsertAt_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	void *value = gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->InsertAt(index, value);
}

static void ScriptListInsertAtArray_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	CScriptArray *array = (CScriptArray*)gen->GetArgAddress(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->InsertAt(index, *array);
}

static void ScriptListRemoveAt_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->RemoveAt(index);
}

static void ScriptListRemoveRange_Generic(asIScriptGeneric *gen)
{
	asUINT start = gen->GetArgDWord(0);
	asUINT count = gen->GetArgDWord(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->RemoveRange(start, count);
}

static void ScriptListInsertLast_Generic(asIScriptGeneric *gen)
{
	void *value = gen->GetArgAddress(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->InsertLast(value);
}

static void ScriptListRemoveLast_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->RemoveLast();
}

static void ScriptListLength_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();

	gen->SetReturnDWord( self->GetSize() );
}

static void ScriptListResize_Generic(asIScriptGeneric *gen)
{
	asUINT size = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();

	self->Resize(size);
}

static void ScriptListReserve_Generic(asIScriptGeneric *gen)
{
	asUINT size = gen->GetArgDWord(0);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->Reserve(size);
}

static void ScriptListSortAsc_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->SortAsc();
}

static void ScriptListReverse_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->Reverse();
}

static void ScriptListIsEmpty_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	*reinterpret_cast<bool*>(gen->GetAddressOfReturnLocation()) = self->IsEmpty();
}

static void ScriptListSortAsc2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	asUINT count = gen->GetArgDWord(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->SortAsc(index, count);
}

static void ScriptListSortDesc_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->SortDesc();
}

static void ScriptListSortDesc2_Generic(asIScriptGeneric *gen)
{
	asUINT index = gen->GetArgDWord(0);
	asUINT count = gen->GetArgDWord(1);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->SortDesc(index, count);
}

static void ScriptListSortCallback_Generic(asIScriptGeneric *gen)
{
	asIScriptFunction *callback = (asIScriptFunction*)gen->GetArgAddress(0);
	asUINT startAt = gen->GetArgDWord(1);
	asUINT count = gen->GetArgDWord(2);
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->Sort(callback, startAt, count);
}

static void ScriptListAddRef_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->AddRef();
}

static void ScriptListRelease_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->Release();
}

static void ScriptListGetRefCount_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	*reinterpret_cast<int32_t*>(gen->GetAddressOfReturnLocation()) = self->GetRefCount();
}

static void ScriptListSetFlag_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	self->SetFlag();
}

static void ScriptListGetFlag_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	*reinterpret_cast<bool*>(gen->GetAddressOfReturnLocation()) = self->GetFlag();
}

static void ScriptListEnumReferences_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void ScriptListReleaseAllHandles_Generic(asIScriptGeneric *gen)
{
	CScriptArray *self = (CScriptArray*)gen->GetObjectData();
	asIScriptEngine *engine = *(asIScriptEngine**)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

static void ScriptListClear_Generic( asIScriptGeneric *gen ) {
	( (CScriptArray *)gen->GetObjectData() )->Clear();
}

static void RegisterScriptList_Generic( asIScriptEngine *engine )
{
	Cmd_AddCommand( "ml_debug.print_list_memory_stats", PrintArrayMemoryStats_f );

	memset( &memstats, 0, sizeof( memstats ) );

	engine->SetTypeInfoUserDataCleanupCallback( CleanupTypeInfoArrayCache, ARRAY_CACHE );

	CheckASCall( engine->RegisterObjectType( "array<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE ) );

	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION( ScriptListTemplateCallback_Generic ), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in)", asFUNCTION(ScriptListFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint length) explicit", asFUNCTION(ScriptListFactory2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint length, const T &in value)", asFUNCTION(ScriptListFactoryDefVal_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_LIST_FACTORY, "array<T>@ f(int&in, int&in) {repeat T}", asFUNCTION(ScriptListListFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptListAddRef_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptListRelease_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "T &opIndex(uint index)", asFUNCTION(ScriptListAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "const T &opIndex(uint index) const", asFUNCTION(ScriptListAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "array<T> &opAssign(const array<T>&in)", asFUNCTION(ScriptListAssignment_Generic), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void InsertAt( uint, const T& in )", asFUNCTION(ScriptListInsertAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void InsertAt( uint, const array<T>& in )", asFUNCTION(ScriptListInsertAtArray_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Insert( uint, const T& in )", asFUNCTION(ScriptListInsertAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Insert( uint, const array<T>& in )", asFUNCTION(ScriptListInsertAtArray_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void InsertLast( const T& in )", asFUNCTION(ScriptListInsertLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void RemoveAt(uint index)", asFUNCTION(ScriptListRemoveAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void RemoveLast()", asFUNCTION(ScriptListRemoveLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Erase(uint index)", asFUNCTION(ScriptListRemoveAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Size() const", asFUNCTION(ScriptListLength_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Count() const", asFUNCTION(ScriptListLength_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Find(const T&in if_handle_then_const value) const", asFUNCTION(ScriptListFind_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "uint Find(uint startAt, const T&in if_handle_then_const value) const", asFUNCTION(ScriptListFind2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Add(const T&in value)", asFUNCTION(ScriptListInsertLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Clear()", asFUNCTION(ScriptListClear_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Resize( uint )", asFUNCTION(ScriptListResize_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Reserve( uint )", asFUNCTION( ScriptListReserve_Generic ), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortAsc()", asFUNCTION(ScriptListSortAsc_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortAsc(uint startAt, uint count)", asFUNCTION(ScriptListSortAsc2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortDesc()", asFUNCTION(ScriptListSortDesc_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void SortDesc(uint startAt, uint count)", asFUNCTION(ScriptListSortDesc2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "array<T>", "void Reverse()", asFUNCTION(ScriptListReverse_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterFuncdef( "bool array<T>::Less(const T&in if_handle_then_const a, const T&in if_handle_then_const b)" ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptListGetRefCount_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptListSetFlag_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptListGetFlag_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptListEnumReferences_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "array<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptListReleaseAllHandles_Generic), asCALL_GENERIC ) );

	//========================================================================
	// Inheriting container classes
	//

	CheckASCall( engine->RegisterObjectType( "stack<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION( ScriptListTemplateCallback_Generic ), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_FACTORY, "stack<T>@ f(int&in)", asFUNCTION(ScriptListFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_FACTORY, "stack<T>@ f(int&in, uint length) explicit", asFUNCTION(ScriptListFactory2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_FACTORY, "stack<T>@ f(int&in, uint length, const T &in value)", asFUNCTION(ScriptListFactoryDefVal_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_LIST_FACTORY, "stack<T>@ f(int&in, int&in) {repeat T}", asFUNCTION(ScriptListListFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptListAddRef_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptListRelease_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "T &opIndex(uint index)", asFUNCTION(ScriptListAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "const T &opIndex(uint index) const", asFUNCTION(ScriptListAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "stack<T> &opAssign(const stack<T>&in)", asFUNCTION(ScriptListAssignment_Generic), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Push( const T& in )", asFUNCTION(ScriptListInsertLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Pop()", asFUNCTION(ScriptListRemoveLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Erase(uint index)", asFUNCTION(ScriptListRemoveAt_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "uint Size() const", asFUNCTION(ScriptListLength_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "uint Count() const", asFUNCTION(ScriptListLength_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "uint Find(const T&in if_handle_then_const value) const", asFUNCTION(ScriptListFind_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "uint Find(uint startAt, const T&in if_handle_then_const value) const", asFUNCTION(ScriptListFind2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Add(const T&in value)", asFUNCTION(ScriptListInsertLast_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Clear()", asFUNCTION(ScriptListClear_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Resize( uint )", asFUNCTION(ScriptListResize_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Reserve( uint )", asFUNCTION( ScriptListReserve_Generic ), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortAsc()", asFUNCTION(ScriptListSortAsc_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortAsc(uint startAt, uint count)", asFUNCTION(ScriptListSortAsc2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortDesc()", asFUNCTION(ScriptListSortDesc_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void SortDesc(uint startAt, uint count)", asFUNCTION(ScriptListSortDesc2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectMethod( "stack<T>", "void Reverse()", asFUNCTION(ScriptListReverse_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterFuncdef( "bool stack<T>::Less(const T&in if_handle_then_const a, const T&in if_handle_then_const b)" ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptListGetRefCount_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptListSetFlag_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptListGetFlag_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptListEnumReferences_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "stack<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptListReleaseAllHandles_Generic), asCALL_GENERIC ) );
}

END_AS_NAMESPACE
