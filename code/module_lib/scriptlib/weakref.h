#ifndef __SCRIPT_WEAKREF__
#define __SCRIPT_WEAKREF__

#pragma once

#include "../module_public.h"

class CScriptWeakRef 
{
public:
	CScriptWeakRef( asITypeInfo *type );
	CScriptWeakRef( const CScriptWeakRef& other );
	CScriptWeakRef( void *ref, asITypeInfo *type );
	~CScriptWeakRef();

	// Copy the stored value from another weakref object
	CScriptWeakRef &operator=( const CScriptWeakRef& other );

	// Compare equalness
	bool operator==( const CScriptWeakRef& o ) const;
	bool operator!=( const CScriptWeakRef& o ) const;

	// Sets a new reference
	CScriptWeakRef& Set( void *newRef );

	// Returns the object if it is still alive
	// This will increment the refCount of the returned object
	void *Get() const;

	// Returns true if the contained reference is the same
	bool Equals( void *ref ) const;

	// Returns the type of the reference held
	asITypeInfo *GetRefType() const;

protected:
	// These functions need to have access to protected
	// members in order to call them from the script engine
	friend void RegisterScriptWeakRef_Native(asIScriptEngine *engine);

	void                  *m_ref;
	asITypeInfo           *m_type;
	asILockableSharedBool *m_weakRefFlag;
};

void RegisterScriptWeakRef(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif