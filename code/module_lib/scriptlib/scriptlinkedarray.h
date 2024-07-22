#ifndef __SCRIPT_LINKED_ARRAY__
#define __SCRIPT_LINKED_ARRAY__

#pragma once

#include "scriptarray.h"

class CScriptLinkedArray
{
public:
    static CScriptLinkedArray *Create( asITypeInfo *ot );
	static CScriptLinkedArray *Create( asITypeInfo *ot, asUINT length );
	static CScriptLinkedArray *Create( asITypeInfo *ot, asUINT length, void *defaultValue );
	static CScriptLinkedArray *Create( asITypeInfo *ot, void *listBuffer );

	void AddRef( void ) const;
	void Release( void ) const;

	asITypeInfo *GetArrayObjectType() const;
	int          GetArrayTypeId() const;
	int          GetElementTypeId() const;

	asUINT GetSize( void ) const;
	bool   IsEmpty( void ) const;

	void   Reserve( asUINT maxElements );
	void   Resize( asUINT numElements );

	void       *At( asUINT index );
	const void *At( asUINT index ) const;

	// Set value of an element. 
	// The value arg should be a pointer to the value that will be copied to the element.
	// Remember, if the array holds handles the value parameter should be the 
	// address of the handle. The refCount of the object will also be incremented
	void  SetValue( asUINT index, void *value );

	CScriptLinkedArray& operator=( const CScriptLinkedArray& );

	bool operator==( const CScriptLinkedArray& ) const;

	void InsertAt( asUINT index, void *value );
	void InsertAt( asUINT index, const CScriptLinkedArray& arr );
	void InsertLast( void *value );
	void RemoveAt( asUINT index );
	void RemoveLast( void );
	void RemoveRange( asUINT start, asUINT count );
	void SortAsc( void );
	void SortDesc( void );
	void SortAsc( asUINT startAt, asUINT count );
	void SortDesc( asUINT startAt, asUINT count );
	void Sort( asUINT startAt, asUINT count, bool asc );
	void Sort( asIScriptFunction *less, asUINT startAt, asUINT count );
	void Reverse( void );
	int Find( void *value ) const;
	int Find( asUINT startAt, void *value ) const;
	int FindByRef( void *ref ) const;
	int FindByRef( asUINT startAt, void *ref ) const;

	void *GetBuffer( void );
	const void *GetBuffer( void ) const;

	void Clear( void );

	int  GetRefCount( void );
	void SetFlag( void );
	bool GetFlag( void );
	void EnumReferences( asIScriptEngine *pEngine );
	void ReleaseAllHandles( asIScriptEngine *pEngine );
private:
    CScriptArray *mpBase;

    struct ListElement {
        ListElement *pNext;
    };
};

#endif