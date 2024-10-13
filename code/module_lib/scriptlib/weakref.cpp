#include "../module_public.h"
#include "../module_alloc.h"
#include "weakref.h"

static void ScriptWeakRefConstruct(asITypeInfo *type, void *mem)
{
	new(mem) CScriptWeakRef(type);
}

static void ScriptWeakRefConstruct2(asITypeInfo *type, void *ref, void *mem)
{
	new(mem) CScriptWeakRef(ref, type);

	// It's possible the constructor raised a script exception, in which case we
	// need to call the destructor in order to cleanup the memory before returning
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
		reinterpret_cast<CScriptWeakRef*>(mem)->~CScriptWeakRef();
}

static void ScriptWeakRefDestruct(CScriptWeakRef *obj)
{
	obj->~CScriptWeakRef();
}

static bool ScriptWeakRefTemplateCallback(asITypeInfo *ti, bool &/*dontGarbageCollect*/)
{
	asITypeInfo *subType = ti->GetSubType();

	// Weak references only work for reference types
	if( subType == 0 ) return false;
	if( !(subType->GetFlags() & asOBJ_REF) ) return false;

	// The subtype shouldn't be a handle
	if( ti->GetSubTypeId() & asTYPEID_OBJHANDLE )
		return false;

	// Make sure the type really supports weak references
	asUINT cnt = subType->GetBehaviourCount();
	for( asUINT n = 0; n < cnt; n++ )
	{
		asEBehaviours beh;
		subType->GetBehaviourByIndex(n, &beh);
		if( beh == asBEHAVE_GET_WEAKREF_FLAG )
			return true;
	}

	ti->GetEngine()->WriteMessage("weakref", 0, 0, asMSGTYPE_ERROR, "The subtype doesn't support weak references");
	return false;
}

CScriptWeakRef::CScriptWeakRef(asITypeInfo *type)
{
	m_ref  = 0;
	m_type = type;
	m_type->AddRef();
	m_weakRefFlag = 0;
}

CScriptWeakRef::CScriptWeakRef(const CScriptWeakRef &other)
{
	m_ref  = other.m_ref;
	m_type = other.m_type;
	m_type->AddRef();
	m_weakRefFlag = other.m_weakRefFlag;
	if( m_weakRefFlag )
		m_weakRefFlag->AddRef();
}

CScriptWeakRef::CScriptWeakRef(void *ref, asITypeInfo *type)
{
	m_ref  = ref;
	m_type = type;
	m_type->AddRef();

	// The given type should be the weakref template instance
	Assert( N_strcmp(type->GetName(), "weakref") == 0 ||
	        N_strcmp(type->GetName(), "const_weakref") == 0 );

	// Get the shared flag that will tell us when the object has been destroyed
	// This is threadsafe as we hold a strong reference to the object
	m_weakRefFlag = m_type->GetEngine()->GetWeakRefFlagOfScriptObject(m_ref, m_type->GetSubType());
	if( m_weakRefFlag )
		m_weakRefFlag->AddRef();
}

CScriptWeakRef::~CScriptWeakRef()
{
	if( m_type )
		m_type->Release();
	if( m_weakRefFlag )
		m_weakRefFlag->Release();
}

CScriptWeakRef &CScriptWeakRef::operator =(const CScriptWeakRef &other)
{
	// Don't do anything if it is the same reference
	if( m_ref == other.m_ref )
		return *this;

	// Must not allow changing the type
	if( m_type != other.m_type )
	{
		// We can allow a weakref to be assigned to a const_weakref
		if( !(N_strcmp(m_type->GetName(), "const_weakref") == 0 &&
			  N_strcmp(other.m_type->GetName(), "weakref") == 0 &&
			  m_type->GetSubType() == other.m_type->GetSubType()) )
		{
			  Assert( false );
			  return *this;
		}
	}

	m_ref = other.m_ref;

	if( m_weakRefFlag )
		m_weakRefFlag->Release();
	m_weakRefFlag = other.m_weakRefFlag;
	if( m_weakRefFlag )
		m_weakRefFlag->AddRef();

	return *this;
}

CScriptWeakRef &CScriptWeakRef::Set(void *newRef)
{
	// Release the previous weak ref 
	if( m_weakRefFlag )
		m_weakRefFlag->Release();

	// Retrieve the new weak ref
	m_ref = newRef;
	if( newRef )
	{
		m_weakRefFlag = m_type->GetEngine()->GetWeakRefFlagOfScriptObject(newRef, m_type->GetSubType());
		m_weakRefFlag->AddRef();
	}
	else
		m_weakRefFlag = 0;

	// Release the newRef since we're only supposed to hold a weakref
	m_type->GetEngine()->ReleaseScriptObject(newRef, m_type->GetSubType());

	return *this;
}

asITypeInfo *CScriptWeakRef::GetRefType() const
{
	return m_type->GetSubType();
}

bool CScriptWeakRef::operator==(const CScriptWeakRef &o) const
{
	if( m_ref  == o.m_ref &&
		m_type == o.m_type )
		return true;

	// TODO: If type is not the same, we should attempt to do a dynamic cast,
	//       which may change the pointer for application registered classes

	return false;
}

bool CScriptWeakRef::operator!=(const CScriptWeakRef &o) const
{
	return !(*this == o);
}

// AngelScript: used as '@obj = ref.get();'
void *CScriptWeakRef::Get() const
{
	// If we hold a null handle, then just return null
	if( m_ref == 0 || m_weakRefFlag == 0 )
		return 0;

	// Lock on the shared bool, so we can be certain it won't be changed to true
	// between the inspection of the flag and the increase of the ref count in the
	// owning object.
	m_weakRefFlag->Lock();
	if( !m_weakRefFlag->Get() )
	{
		m_type->GetEngine()->AddRefScriptObject(m_ref, m_type->GetSubType());
		m_weakRefFlag->Unlock();
		return m_ref;
	}
	m_weakRefFlag->Unlock();

	return 0;
}

bool CScriptWeakRef::Equals(void *ref) const
{
	// Release the ref since we'll not keep it
	m_type->GetEngine()->ReleaseScriptObject(ref, m_type->GetSubType());

	if( m_ref == ref )
		return true;

	return false;
}

static void ScriptWeakRefConstruct_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *reinterpret_cast<asITypeInfo**>(gen->GetAddressOfArg(0));

	ScriptWeakRefConstruct(ti, gen->GetObjectData());
}

static void ScriptWeakRefConstruct2_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *reinterpret_cast<asITypeInfo**>(gen->GetAddressOfArg(0));
	void *ref = gen->GetArgAddress(1);

	ScriptWeakRefConstruct2(ti, ref, gen->GetObjectData());
}

static void ScriptWeakRefDestruct_Generic(asIScriptGeneric *gen)
{
	CScriptWeakRef *self = reinterpret_cast<CScriptWeakRef*>(gen->GetObjectData());
	self->~CScriptWeakRef();
}

void CScriptWeakRef_Get_Generic(asIScriptGeneric *gen)
{
	CScriptWeakRef *self = reinterpret_cast<CScriptWeakRef*>(gen->GetObjectData());
	gen->SetReturnAddress(self->Get());
}

void CScriptWeakRef_Assign_Generic(asIScriptGeneric *gen)
{
	CScriptWeakRef *other = reinterpret_cast<CScriptWeakRef*>(gen->GetArgAddress(0));
	CScriptWeakRef *self = reinterpret_cast<CScriptWeakRef*>(gen->GetObjectData());
	*self = *other;
	gen->SetReturnAddress(self);
}

void CScriptWeakRef_Assign2_Generic(asIScriptGeneric *gen)
{
	void *other = gen->GetArgAddress(0);
	CScriptWeakRef *self = reinterpret_cast<CScriptWeakRef*>(gen->GetObjectData());

	// Must increase the refcount of the object, since Set() will decrease it
	// If this is not done, the object will be destroyed too early since the 
	// generic interface also automatically decreases the refcount of received handles
	gen->GetEngine()->AddRefScriptObject(other, self->GetRefType());

	self->Set(other);
	gen->SetReturnAddress(self);
}

void CScriptWeakRef_Equals_Generic(asIScriptGeneric *gen)
{
	CScriptWeakRef *other = reinterpret_cast<CScriptWeakRef*>(gen->GetArgAddress(0));
	CScriptWeakRef *self = reinterpret_cast<CScriptWeakRef*>(gen->GetObjectData());
	gen->SetReturnByte(*self == *other);
}

void CScriptWeakRef_Equals2_Generic(asIScriptGeneric *gen)
{
	void *other = gen->GetArgAddress(0);
	CScriptWeakRef *self = reinterpret_cast<CScriptWeakRef*>(gen->GetObjectData());

	// Must increase the refcount of the object, since Equals() will decrease it
	// If this is not done, the object will be destroyed too early since the 
	// generic interface also automatically decreases the refcount of received handles
	gen->GetEngine()->AddRefScriptObject(other, self->GetRefType());

	gen->SetReturnByte(self->Equals(other));
}

static void ScriptWeakRefTemplateCallback_Generic(asIScriptGeneric *gen)
{
	asITypeInfo *ti = *reinterpret_cast<asITypeInfo**>(gen->GetAddressOfArg(0));
	bool *dontGarbageCollect = *reinterpret_cast<bool**>(gen->GetAddressOfArg(1));
	*reinterpret_cast<bool*>(gen->GetAddressOfReturnLocation()) = ScriptWeakRefTemplateCallback(ti, *dontGarbageCollect);
}

void RegisterScriptWeakRef_Native(asIScriptEngine *engine)
{
	// Register a type for non-const handles
	CheckASCall( engine->RegisterObjectType("weakref<class T>", sizeof(CScriptWeakRef), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_TEMPLATE | asOBJ_APP_CLASS_DAK) );

	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ScriptWeakRefConstruct), asCALL_CDECL_OBJLAST) );
	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in, T@+) explicit", asFUNCTION(ScriptWeakRefConstruct2), asCALL_CDECL_OBJLAST) );
	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ScriptWeakRefDestruct), asCALL_CDECL_OBJLAST) );
	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(ScriptWeakRefTemplateCallback), asCALL_CDECL) );

	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "T@ opImplCast()", asMETHOD(CScriptWeakRef, Get), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "T@ get() const", asMETHODPR(CScriptWeakRef, Get, () const, void*), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "weakref<T> &opHndlAssign(const weakref<T> &in)", asMETHOD(CScriptWeakRef, operator=), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "weakref<T> &opAssign(const weakref<T> &in)", asMETHOD(CScriptWeakRef, operator=), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "bool opEquals(const weakref<T> &in) const", asMETHODPR(CScriptWeakRef, operator==, (const CScriptWeakRef &) const, bool), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "weakref<T> &opHndlAssign(T@)", asMETHOD(CScriptWeakRef, Set), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "bool opEquals(const T@+) const", asMETHOD(CScriptWeakRef, Equals), asCALL_THISCALL) );

	// Register another type for const handles
	CheckASCall( engine->RegisterObjectType("const_weakref<class T>", sizeof(CScriptWeakRef), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_TEMPLATE | asOBJ_APP_CLASS_DAK) );

	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ScriptWeakRefConstruct), asCALL_CDECL_OBJLAST) );
	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in, const T@+) explicit", asFUNCTION(ScriptWeakRefConstruct2), asCALL_CDECL_OBJLAST) );
	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ScriptWeakRefDestruct), asCALL_CDECL_OBJLAST) );
	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(ScriptWeakRefTemplateCallback), asCALL_CDECL) );

	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const T@ opImplCast() const", asMETHOD(CScriptWeakRef, Get), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const T@ get() const", asMETHODPR(CScriptWeakRef, Get, () const, void*), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opHndlAssign(const const_weakref<T> &in)", asMETHOD(CScriptWeakRef, operator=), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opAssign(const const_weakref<T> &in)", asMETHOD(CScriptWeakRef, operator=), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "bool opEquals(const const_weakref<T> &in) const", asMETHODPR(CScriptWeakRef, operator==, (const CScriptWeakRef &) const, bool), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opHndlAssign(const T@)", asMETHOD(CScriptWeakRef, Set), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "bool opEquals(const T@+) const", asMETHOD(CScriptWeakRef, Equals), asCALL_THISCALL) );

	// Allow non-const weak references to be converted to const weak references
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opHndlAssign(const weakref<T> &in)", asMETHOD(CScriptWeakRef, operator=), asCALL_THISCALL) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "bool opEquals(const weakref<T> &in) const", asMETHODPR(CScriptWeakRef, operator==, (const CScriptWeakRef &) const, bool), asCALL_THISCALL) );
}

void RegisterScriptWeakRef_Generic(asIScriptEngine *engine)
{
	// Register a type for non-const handles
	CheckASCall( engine->RegisterObjectType("weakref<class T>", sizeof(CScriptWeakRef), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_TEMPLATE | asOBJ_APP_CLASS_DAK) );

	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ScriptWeakRefConstruct_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in, T@)", asFUNCTION(ScriptWeakRefConstruct2_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(ScriptWeakRefTemplateCallback_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("weakref<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ScriptWeakRefDestruct_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "T@ opImplCast()", asFUNCTION(CScriptWeakRef_Get_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "T@ get() const", asFUNCTION(CScriptWeakRef_Get_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "weakref<T> &opHndlAssign(const weakref<T> &in)", asFUNCTION(CScriptWeakRef_Assign_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "bool opEquals(const weakref<T> &in) const", asFUNCTION(CScriptWeakRef_Equals_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "weakref<T> &opHndlAssign(T@)", asFUNCTION(CScriptWeakRef_Assign2_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("weakref<T>", "bool opEquals(const T@) const", asFUNCTION(CScriptWeakRef_Equals2_Generic), asCALL_GENERIC) );

	// Register another type for const handles
	CheckASCall( engine->RegisterObjectType("const_weakref<class T>", sizeof(CScriptWeakRef), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_TEMPLATE | asOBJ_APP_CLASS_DAK) );

	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTION(ScriptWeakRefConstruct_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_CONSTRUCT, "void f(int&in, const T@)", asFUNCTION(ScriptWeakRefConstruct2_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(ScriptWeakRefTemplateCallback_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("const_weakref<T>", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ScriptWeakRefDestruct_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const T@ opImplCast() const", asFUNCTION(CScriptWeakRef_Get_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const T@ get() const", asFUNCTION(CScriptWeakRef_Get_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opHndlAssign(const const_weakref<T> &in)", asFUNCTION(CScriptWeakRef_Assign_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "bool opEquals(const const_weakref<T> &in) const", asFUNCTION(CScriptWeakRef_Equals_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opHndlAssign(const T@)", asFUNCTION(CScriptWeakRef_Assign2_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "bool opEquals(const T@) const", asFUNCTION(CScriptWeakRef_Equals2_Generic), asCALL_GENERIC) );

	// Allow non-const weak references to be converted to const weak references
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "const_weakref<T> &opHndlAssign(const weakref<T> &in)", asFUNCTION(CScriptWeakRef_Assign_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("const_weakref<T>", "bool opEquals(const weakref<T> &in) const", asFUNCTION(CScriptWeakRef_Equals_Generic), asCALL_GENERIC) );
}

void RegisterScriptWeakRef( asIScriptEngine *pEngine )
{
	if ( strstr( asGetLibraryOptions(), "AS_MAX_PORTABILITY" ) ) {
		RegisterScriptWeakRef_Generic( pEngine );
	} else {
		RegisterScriptWeakRef_Native( pEngine );
	}
}