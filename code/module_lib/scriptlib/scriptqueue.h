#ifndef __SCRIPT_QUEUE__
#define __SCRIPT_QUEUE__

#pragma once

#include "scriptarray.h"

class CScriptQueue
{
public:
    static CScriptQueue *Create( asITypeInfo *ot );
	static CScriptQueue *Create( asITypeInfo *ot, asUINT length );
	static CScriptQueue *Create( asITypeInfo *ot, asUINT length, void *defaultValue );
	static CScriptQueue *Create( asITypeInfo *ot, void *listBuffer );

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

	void  SetValue( asUINT index, void *value );

	CScriptQueue& operator=( const CScriptQueue& );

	bool operator==( const CScriptQueue& ) const;

	void InsertAt( asUINT index, void *value );
	void InsertAt( asUINT index, const CScriptQueue& arr );
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
protected:
	mutable CThreadAtomic<int32_t> refCount;
	mutable bool    gcFlag;
	asITypeInfo    *objType;
	asIScriptFunction *subTypeHandleAssignFunc;
	uint32_t         elementSize;
	int32_t         subTypeId;

	SArrayBuffer *buffer;

	// Constructors
	CScriptQueue( asITypeInfo *ot, void *initBuf ); // Called from script when initialized with list
	CScriptQueue( asUINT length, asITypeInfo *ot );
	CScriptQueue( asUINT length, void *defVal, asITypeInfo *ot );
	CScriptQueue( const CScriptQueue& other );
	virtual ~CScriptQueue();

	void DeleteBuffer( SArrayBuffer *buffer );

	void AllocBuffer( uint32_t nItems );
	void DoAllocate( int delta, uint32_t at );
	bool  Less( const void *a, const void *b, bool asc );
	void *GetArrayItemPointer(int index );
	void *GetDataPointer(void *buffer );
	void  Copy( void *dst, void *src );
	void  Swap( void *a, void *b );
	void  Precache( void );
	bool  CheckMaxSize( asUINT numElements );
	void  CreateBuffer( SArrayBuffer **buffer, asUINT numElements );
	void  CopyBuffer( SArrayBuffer *dst, SArrayBuffer *src );
	void  Construct( SArrayBuffer *buf, asUINT start, asUINT end );
	void  Destruct( SArrayBuffer *buf, asUINT start, asUINT end );
	bool  Equals( const void *a, const void *b, asIScriptContext *ctx, SArrayCache *cache ) const;
};

#endif