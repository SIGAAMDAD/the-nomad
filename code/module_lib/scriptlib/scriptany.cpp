#include "../module_public.h"
#include "../module_alloc.h"
#include "scriptany.h"

// We'll use the generic interface for the factories as we need the engine pointer
static void ScriptAnyFactory_Generic( asIScriptGeneric *gen )
{
	asIScriptEngine *engine = gen->GetEngine();

	*(CScriptAny **)gen->GetAddressOfReturnLocation() = new ( Mem_ClearedAlloc( sizeof( CScriptAny ) ) ) CScriptAny( engine );
}

static void ScriptAnyFactory2_Generic( asIScriptGeneric *gen )
{
	asIScriptEngine *engine = gen->GetEngine();
	void *ref = (void *)gen->GetArgAddress( 0 );
	int refType = gen->GetArgTypeId( 0 );

	*(CScriptAny **)gen->GetAddressOfReturnLocation() = new ( Mem_ClearedAlloc( sizeof( CScriptAny ) ) ) CScriptAny( engine );
}

static CScriptAny& ScriptAnyAssignment( CScriptAny *other, CScriptAny *self )
{
	return *self = *other;
}

static void ScriptAnyAssignment_Generic( asIScriptGeneric *gen )
{
	CScriptAny *other = (CScriptAny *)gen->GetArgObject( 0 );
	CScriptAny *self = (CScriptAny *)gen->GetObjectData();

	*self = *other;

	gen->SetReturnObject( self );
}

static void ScriptAny_Store_Generic( asIScriptGeneric *gen )
{
	void *ref = (void *)gen->GetArgAddress( 0 );
	int refTypeId = gen->GetArgTypeId( 0 );
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();

	self->Store( ref, refTypeId );
}

static void ScriptAny_StorePrimitive_Generic( asIScriptGeneric *gen )
{
	void *ref = gen->GetArgAddress(0);
	CScriptAny *self = (CScriptAny *)gen->GetObjectData();

    switch ( gen->GetArgTypeId( 0 ) ) {
    case asTYPEID_BOOL:
        self->Store( *(bool *)ref);
        break;
    case asTYPEID_INT8:
        self->Store( *(int8_t *)ref);
        break;
    case asTYPEID_INT16:
        self->Store( *(int16_t *)ref);
        break;
    case asTYPEID_INT32:
        self->Store( *(int32_t *)ref);
        break;
    case asTYPEID_INT64:
        self->Store( *(int64_t *)ref);
        break;
    case asTYPEID_UINT8:
        self->Store( *(uint8_t *)ref);
        break;
    case asTYPEID_UINT16:
        self->Store( *(uint16_t *)ref);
        break;
    case asTYPEID_UINT32:
        self->Store( *(uint32_t *)ref);
        break;
    case asTYPEID_UINT64:
        self->Store( *(uint64_t *)ref);
        break;
    case asTYPEID_FLOAT:
        self->Store( *(float *)ref);
        break;
    case asTYPEID_DOUBLE:
        self->Store( *(double *)ref);
        break;
    };
}

static void ScriptAny_Retrieve_Generic( asIScriptGeneric *gen )
{
	void *ref = (void *)gen->GetArgAddress(0);
	int refTypeId = gen->GetArgTypeId(0);
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();

	*(bool *)gen->GetAddressOfReturnLocation() = self->Retrieve(ref, refTypeId);
}

static void ScriptAny_RetrievePrimitive_Generic( asIScriptGeneric *gen )
{
	void *ref = gen->GetArgAddress( 0 );
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
    bool out;

    switch ( gen->GetArgTypeId( 0 ) ) {
    case asTYPEID_BOOL:
        out = self->Retrieve( *(bool *)ref);
        break;
    case asTYPEID_INT8:
        out = self->Retrieve( *(int8_t *)ref);
        break;
    case asTYPEID_INT16:
        out = self->Retrieve( *(int16_t *)ref);
        break;
    case asTYPEID_INT32:
        out = self->Retrieve( *(int32_t *)ref);
        break;
    case asTYPEID_INT64:
        out = self->Retrieve( *(int64_t *)ref);
        break;
    case asTYPEID_UINT8:
        out = self->Retrieve( *(uint8_t *)ref);
        break;
    case asTYPEID_UINT16:
        out = self->Retrieve( *(uint16_t *)ref);
        break;
    case asTYPEID_UINT32:
        out = self->Retrieve( *(uint32_t *)ref);
        break;
    case asTYPEID_UINT64:
        out = self->Retrieve( *(uint64_t *)ref);
        break;
    case asTYPEID_FLOAT:
        out = self->Retrieve( *(float *)ref);
        break;
    case asTYPEID_DOUBLE:
        out = self->Retrieve( *(double *)ref);
        break;
	default:
		out = false;
		break;
    };

	*(bool *)gen->GetAddressOfReturnLocation() = out;
}

static void ScriptAny_AddRef_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	self->AddRef();
}

static void ScriptAny_Release_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	self->Release();
}

static void ScriptAny_GetRefCount_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	*(int *)gen->GetAddressOfReturnLocation() = self->GetRefCount();
}

static void ScriptAny_SetFlag_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	self->SetFlag();
}

static void ScriptAny_GetFlag_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	*(bool *)gen->GetAddressOfReturnLocation() = self->GetFlag();
}

static void ScriptAny_EnumReferences_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	asIScriptEngine *engine = *(asIScriptEngine* *)gen->GetAddressOfArg(0);
	self->EnumReferences(engine);
}

static void ScriptAny_ReleaseAllHandles_Generic( asIScriptGeneric *gen )
{
	CScriptAny *self = (CScriptAny  *)gen->GetObjectData();
	asIScriptEngine *engine = *(asIScriptEngine* *)gen->GetAddressOfArg(0);
	self->ReleaseAllHandles(engine);
}

void RegisterScriptAny_Native( asIScriptEngine *engine )
{
	CheckASCall( engine->RegisterObjectType( "any", sizeof( CScriptAny ), asOBJ_REF | asOBJ_GC ) );

	// We'll use the generic interface for the constructor as we need the engine pointer
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f()", asFUNCTION(ScriptAnyFactory_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f(?&in) explicit", asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f(const int64&in) explicit", asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f(const double&in) explicit", asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_ADDREF, "void f()", asMETHOD(CScriptAny,AddRef), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_RELEASE, "void f()", asMETHOD(CScriptAny,Release), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "any &opAssign(any&in)", asFUNCTION(ScriptAnyAssignment), asCALL_CDECL_OBJLAST ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(?&in)", asMETHODPR(CScriptAny,Store,(void*,int),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int8&in)", asMETHODPR(CScriptAny,Store,(asINT8&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int16&in)", asMETHODPR(CScriptAny,Store,(asINT16&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int32&in)", asMETHODPR(CScriptAny,Store,(asINT32&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int64&in)", asMETHODPR(CScriptAny,Store,(asINT64&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint8&in)", asMETHODPR(CScriptAny,Store,(asBYTE&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint16&in)", asMETHODPR(CScriptAny,Store,(asWORD&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint32&in)", asMETHODPR(CScriptAny,Store,(asDWORD&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint64&in)", asMETHODPR(CScriptAny,Store,(asQWORD&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const double&in)", asMETHODPR(CScriptAny,Store,(double&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const float&in)", asMETHODPR(CScriptAny,Store,(float&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const bool&in)", asMETHODPR(CScriptAny,Store,(bool&),void), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(?&out)", asMETHODPR(CScriptAny,Retrieve,(void*,int) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int8&out)", asMETHODPR(CScriptAny,Retrieve,(asINT8&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int16&out)", asMETHODPR(CScriptAny,Retrieve,(asINT16&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int32&out)", asMETHODPR(CScriptAny,Retrieve,(asINT32&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int64&out)", asMETHODPR(CScriptAny,Retrieve,(asINT64&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint8&out)", asMETHODPR(CScriptAny,Retrieve,(asBYTE&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint16&out)", asMETHODPR(CScriptAny,Retrieve,(asWORD&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint32&out)", asMETHODPR(CScriptAny,Retrieve,(asDWORD&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint64&out)", asMETHODPR(CScriptAny,Retrieve,(asQWORD&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(double&out)", asMETHODPR(CScriptAny,Retrieve,(double&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(float&out)", asMETHODPR(CScriptAny,Retrieve,(float&) const,bool), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(bool&out)", asMETHODPR(CScriptAny,Retrieve,(bool&) const,bool), asCALL_THISCALL ) );
	// Register GC behaviours
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(CScriptAny,GetRefCount), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(CScriptAny,SetFlag), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(CScriptAny,GetFlag), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(CScriptAny,EnumReferences), asCALL_THISCALL ) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(CScriptAny,ReleaseAllHandles), asCALL_THISCALL ) );
}

void RegisterScriptAny_Generic( asIScriptEngine *engine )
{
	CheckASCall( engine->RegisterObjectType( "any", sizeof( CScriptAny ), asOBJ_REF | asOBJ_GC ) );

	// We'll use the generic interface for the constructor as we need the engine pointer
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f()", asFUNCTION(ScriptAnyFactory_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f(?&in) explicit", asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f(const int64&in) explicit", asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_FACTORY, "any@ f(const double&in) explicit", asFUNCTION(ScriptAnyFactory2_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptAny_AddRef_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour( "any", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptAny_Release_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "any &opAssign(any&in)", asFUNCTION(ScriptAnyAssignment_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(?&in)", asFUNCTION(ScriptAny_Store_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int8& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int16& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int32& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const int64& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint8& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint16& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint32& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const uint64& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "void store(const double& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const float& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "void store(const bool& in)", asFUNCTION(ScriptAny_StorePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(?&out) const", asFUNCTION(ScriptAny_Retrieve_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int8& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int16& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int32& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(int64& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint8& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint16& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint32& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(uint64& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod( "any", "bool load(double& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(float& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod( "any", "bool load(bool& out) const", asFUNCTION(ScriptAny_RetrievePrimitive_Generic), asCALL_GENERIC) );

	// Register GC behaviours
	CheckASCall( engine->RegisterObjectBehaviour("any", asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(ScriptAny_GetRefCount_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("any", asBEHAVE_SETGCFLAG, "void f()", asFUNCTION(ScriptAny_SetFlag_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("any", asBEHAVE_GETGCFLAG, "bool f()", asFUNCTION(ScriptAny_GetFlag_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("any", asBEHAVE_ENUMREFS, "void f(int&in)", asFUNCTION(ScriptAny_EnumReferences_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("any", asBEHAVE_RELEASEREFS, "void f(int&in)", asFUNCTION(ScriptAny_ReleaseAllHandles_Generic), asCALL_GENERIC) );
}

void RegisterScriptAny( asIScriptEngine *engine )
{
	if ( strstr( asGetLibraryOptions(), "AS_MAX_PORTABILITY" ) ) {
		RegisterScriptAny_Generic( engine );
	} else {
		RegisterScriptAny_Native( engine );
	}
}

CScriptAny &CScriptAny::operator=( const CScriptAny& other )
{
	// Hold on to the object type reference so it isn't destroyed too early
	if ( ( other.value.typeId & asTYPEID_MASK_OBJECT ) ) {
		asITypeInfo *ti = engine->GetTypeInfoById( other.value.typeId );
		if ( ti ) {
			ti->AddRef();
        }
	}

	FreeObject();

	value.typeId = other.value.typeId;
	if ( value.typeId & asTYPEID_OBJHANDLE ) {
		// For handles, copy the pointer and increment the reference count
		value.data.valueObj = other.value.data.valueObj;
		engine->AddRefScriptObject( value.data.valueObj, engine->GetTypeInfoById( value.typeId ) );
	}
	else if ( value.typeId & asTYPEID_MASK_OBJECT ) {
		// Create a copy of the object
		value.data.valueObj = engine->CreateScriptObjectCopy( other.value.data.valueObj, engine->GetTypeInfoById( value.typeId ) );
	}
	else {
		// Primitives can be copied directly
        memcpy( (void *)&value.data, (const void *)&other.value.data, sizeof( uint64_t ) );
	}

	return *this;
}

int CScriptAny::CopyFrom( const CScriptAny *other )
{
	if ( !other ) {
        return asINVALID_ARG;
    }

	*this = *other;
	return 0;
}

CScriptAny::CScriptAny( asIScriptEngine *engine )
{
	this->engine = engine;
	refCount = 1;
	gcFlag = false;

	value.typeId = 0;
	value.data.u64 = 0;

	// Notify the garbage collector of this object
	engine->NotifyGarbageCollectorOfNewObject( this, engine->GetTypeInfoByName( "any" ) );
}

CScriptAny::CScriptAny( void *ref, int refTypeId, asIScriptEngine *engine )
{
	this->engine = engine;
	refCount = 1;
	gcFlag = false;

	value.typeId = 0;
	value.data.u64 = 0;

	// Notify the garbage collector of this object
	engine->NotifyGarbageCollectorOfNewObject( this, engine->GetTypeInfoByName( "any" ) );

	Store(ref, refTypeId);
}

CScriptAny::~CScriptAny()
{
	FreeObject();
}

void CScriptAny::Store( void *ref, int refTypeId )
{
    // This method is not expected to be used for primitive types, except for bool, int64, or double
    if ( refTypeId < asTYPEID_BOOL || refTypeId > asTYPEID_DOUBLE ) {
        N_Error( ERR_DROP, "CScriptAny::Retrieve: only primitive types may be used with script any object" );
    }

	// Hold on to the object type reference so it isn't destroyed too early
	if ( ( refTypeId & asTYPEID_MASK_OBJECT ) ) {
		asITypeInfo *ti = engine->GetTypeInfoById( refTypeId );
		if ( ti ) {
			ti->AddRef();
        }
	}

	FreeObject();

	value.typeId = refTypeId;
	if ( value.typeId & asTYPEID_OBJHANDLE ) {
		// We're receiving a reference to the handle, so we need to dereference it
		value.data.valueObj = *(void **)ref;
		engine->AddRefScriptObject( value.data.valueObj, engine->GetTypeInfoById( value.typeId ) );
	}
	else if ( value.typeId & asTYPEID_MASK_OBJECT ) {
		// Create a copy of the object
		value.data.valueObj = engine->CreateScriptObjectCopy( ref, engine->GetTypeInfoById( value.typeId ) );
	}
	else {
		// Primitives can be copied directly
		value.data.u64 = 0;

		// Copy the primitive value
		// We receive a pointer to the value.
		int32_t size = engine->GetSizeOfPrimitiveType( value.typeId );
		memcpy( (void *)&value.data, ref, size);
	}
}

void CScriptAny::Store( bool& value ) {
    Store( &value, asTYPEID_BOOL );
}

void CScriptAny::Store( int8_t& value ) {
    Store( &value, asTYPEID_INT8 );
}

void CScriptAny::Store( int16_t& value ) {
    Store( &value, asTYPEID_INT16 );
}

void CScriptAny::Store( int32_t& value ) {
    Store( &value, asTYPEID_INT32 );
}

void CScriptAny::Store( int64_t& value ) {
    Store( &value, asTYPEID_INT64 );
}

void CScriptAny::Store( uint8_t& value ) {
    Store( &value, asTYPEID_UINT8 );
}

void CScriptAny::Store( uint16_t& value ) {
    Store( &value, asTYPEID_UINT16 );
}

void CScriptAny::Store( uint32_t& value ) {
    Store( &value, asTYPEID_UINT32 );
}

void CScriptAny::Store( uint64_t& value ) {
    Store( &value, asTYPEID_UINT64 );
}

void CScriptAny::Store( float& value ) {
    Store( &value, asTYPEID_FLOAT );
}

void CScriptAny::Store( double& value ) {
    Store( &value, asTYPEID_DOUBLE );
}

void CScriptAny::ValueAssign( void *ref, int refTypeId ) const
{
    const uint32_t refTypeSize = engine->GetTypeInfoById( refTypeId )->GetSize();
    memcpy( ref, (void *)&value.data, refTypeSize );
}

bool CScriptAny::Retrieve( void *ref, int refTypeId ) const
{
	// This method is not expected to be used for primitive types, except for bool, int64, or double
    if ( refTypeId < asTYPEID_BOOL || refTypeId > asTYPEID_DOUBLE ) {
        N_Error( ERR_DROP, "CScriptAny::Retrieve: only primitive types may be used with script any object" );
    }

	if ( refTypeId & asTYPEID_OBJHANDLE ) {
		// Is the handle type compatible with the stored value?

		// A handle can be retrieved if the stored type is a handle of same or compatible type
		// or if the stored type is an object that implements the interface that the handle refer to.
		if ( ( value.typeId & asTYPEID_MASK_OBJECT ) ) {
			// Don't allow the retrieval if the stored handle is to a const object but not the wanted handle
			if ( ( value.typeId & asTYPEID_HANDLETOCONST ) && !( refTypeId & asTYPEID_HANDLETOCONST ) ) {
				return false;
            }

			// RefCastObject will increment the refCount of the returned pointer if successful
			engine->RefCastObject( value.data.valueObj, engine->GetTypeInfoById( value.typeId ), engine->GetTypeInfoById( refTypeId ), (void **)ref );
			if ( !*(asPWORD *)ref ) {
				return false;
            }
			return true;
		}
	}
	else if ( refTypeId & asTYPEID_MASK_OBJECT ) {
		// Is the object type compatible with the stored value?

		// Copy the object into the given reference
		if ( value.typeId == refTypeId ) {
			engine->AssignScriptObject( ref, value.data.valueObj, engine->GetTypeInfoById( value.typeId ) );
			return true;
		}
	}
	else {
		// Is the primitive type compatible with the stored value?

		if ( value.typeId == refTypeId ) {
			int32_t size = engine->GetSizeOfPrimitiveType( refTypeId );
			memcpy( ref, (void *)&value.data, size );
			return true;
		}

        ValueAssign( ref, refTypeId );
	}

	return false;
}

bool CScriptAny::Retrieve( bool& value ) const {
    return Retrieve( &value, asTYPEID_BOOL );
}

bool CScriptAny::Retrieve( int8_t& value ) const {
    return Retrieve( &value, asTYPEID_INT8 );
}

bool CScriptAny::Retrieve( int16_t& value ) const {
    return Retrieve( &value, asTYPEID_INT16 );
}

bool CScriptAny::Retrieve( int32_t& value ) const {
    return Retrieve( &value, asTYPEID_INT32 );
}

bool CScriptAny::Retrieve( int64_t& value ) const {
    return Retrieve( &value, asTYPEID_INT64 );
}

bool CScriptAny::Retrieve( uint8_t& value ) const {
    return Retrieve( &value, asTYPEID_UINT8 );
}

bool CScriptAny::Retrieve( uint16_t& value ) const {
    return Retrieve( &value, asTYPEID_UINT16 );
}

bool CScriptAny::Retrieve( uint32_t& value ) const {
    return Retrieve( &value, asTYPEID_UINT32 );
}

bool CScriptAny::Retrieve( uint64_t& value ) const {
    return Retrieve( &value, asTYPEID_UINT64 );
}

bool CScriptAny::Retrieve( float& value ) const {
    return Retrieve( &value, asTYPEID_FLOAT );
}

bool CScriptAny::Retrieve( double& value ) const {
    return Retrieve( &value, asTYPEID_DOUBLE );
}

int CScriptAny::GetTypeId( void ) const
{
	return value.typeId;
}

void CScriptAny::FreeObject( void )
{
	// If it is a handle or a ref counted object, call release
	if ( value.typeId & asTYPEID_MASK_OBJECT ) {
		// Let the engine release the object
		asITypeInfo *ti = engine->GetTypeInfoById( value.typeId );
		engine->ReleaseScriptObject( value.data.valueObj, ti );

		// Release the object type info
		if ( ti ) {
			ti->Release();
        }

		value.data.valueObj = 0;
		value.typeId = 0;
	}

	// For primitives, there's nothing to do
}


void CScriptAny::EnumReferences( asIScriptEngine *inEngine )
{
	// If we're holding a reference, we'll notify the garbage collector of it
	if ( value.data.valueObj && ( value.typeId & asTYPEID_MASK_OBJECT ) ) {
		asITypeInfo *subType = engine->GetTypeInfoById( value.typeId );
		if ( ( subType->GetFlags() & asOBJ_REF ) ) {
			inEngine->GCEnumCallback( value.data.valueObj );
		}
        else if ( ( subType->GetFlags() & asOBJ_VALUE ) && ( subType->GetFlags() & asOBJ_GC ) ) {
        	// For value types we need to forward the enum callback
			// to the object so it can decide what to do
			engine->ForwardGCEnumReferences( value.data.valueObj, subType );
		}

		// The object type itself is also garbage collected
		asITypeInfo *ti = inEngine->GetTypeInfoById( value.typeId );
		if ( ti )  {
			inEngine->GCEnumCallback( ti );
        }
	}
}

void CScriptAny::ReleaseAllHandles( asIScriptEngine * /*engine*/ )
{
	FreeObject();
}

int CScriptAny::AddRef( void ) const
{
	// Increase counter and clear flag set by GC
	gcFlag = false;
    return refCount.fetch_add();
}

int CScriptAny::Release( void ) const
{
	// Decrease the ref counter
	gcFlag = false;
	if ( refCount.fetch_sub() == 0 ) {
		Mem_Free( const_cast<CScriptAny *>( this ) );
		return 0;
	}

	return refCount;
}

int CScriptAny::GetRefCount( void ) const
{
	return refCount;
}

void CScriptAny::SetFlag( void )
{
	gcFlag = true;
}

bool CScriptAny::GetFlag( void ) const
{
	return gcFlag;
}
