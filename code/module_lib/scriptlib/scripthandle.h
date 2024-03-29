#ifndef SCRIPTHANDLE_H
#define SCRIPTHANDLE_H

#include "../module_public.h"

class CScriptHandle 
{
public:
	// Constructors
	CScriptHandle( void );
	CScriptHandle(const CScriptHandle &other);
	CScriptHandle(void *ref, asITypeInfo *type);
	~CScriptHandle();

	// Copy the stored value from another any object
	CScriptHandle &operator=(const CScriptHandle &other);

	// Set the reference
	void Set(void *ref, asITypeInfo *type);

	// Compare equalness
	bool operator==(const CScriptHandle &o) const;
	bool operator!=(const CScriptHandle &o) const;
	bool Equals(void *ref, int typeId) const;

	// Dynamic cast to desired handle type
	void Cast(void **outRef, int typeId);

	// Returns the type of the reference held
	asITypeInfo *GetType( void ) const;
	int          GetTypeId( void ) const;

	// Get the reference
	void *GetRef( void );
	const void *GetRef( void ) const;

protected:
	// These functions need to have access to protected
	// members in order to call them from the script engine
	friend void Construct(CScriptHandle *self, void *ref, int typeId);
	friend void RegisterScriptHandle_Native(asIScriptEngine *engine);
	friend void CScriptHandle_AssignVar_Generic(asIScriptGeneric *gen);

	void ReleaseHandle( void );
	void AddRefHandle( void );

	// These shouldn't be called directly by the 
	// application as they requires an active context
	CScriptHandle(void *ref, int typeId);
	CScriptHandle &Assign(void *ref, int typeId);

	void        *m_pRef;
	asITypeInfo *m_pType;
};

void RegisterScriptHandle(asIScriptEngine *engine);

#endif
