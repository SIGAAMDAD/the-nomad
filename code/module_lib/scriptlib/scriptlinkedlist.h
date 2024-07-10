#ifndef __SCRIPTLINKEDLIST_H__
#define __SCRIPTLINKEDLIST_H__

#include "../module_public.h"
#include "../../engine/n_threads.h"
#include "scriptarray.h"

class CScriptLinkedListNode
{
public:
	CScriptLinkedListNode( void );
	CScriptLinkedListNode( asIScriptEngine *engine, void *value, int typeId );
	~CScriptLinkedListNode();

	void Set( asIScriptEngine *engine, void *value, int typeId );
	void Set( asIScriptEngine *engine, const asINT32& value );
	void Set( asIScriptEngine *engine, const asDWORD& value );
	void Set( asIScriptEngine *engine, const asINT64& value );
	void Set( asIScriptEngine *engine, const asQWORD& value );
	void Set( asIScriptEngine *engine, const double& value );
	void Set( asIScriptEngine *engine, CScriptDictValue& value );

	bool Get( asIScriptEngine *engine, void *value, int typeId ) const;
	bool Get( asIScriptEngine *engine, asINT32& value ) const;
	bool Get( asIScriptEngine *engine, asDWORD& value ) const;
	bool Get( asIScriptEngine *engine, asINT64& value ) const;
	bool Get( asIScriptEngine *engine, asQWORD& value ) const;
	bool Get( asIScriptEngine *engine, double& value ) const;

    CScriptLinkedList *Next( void );
    CScriptLinkedList *Prev( void );

	const void *GetAddressOfValue( void ) const;

	int  GetTypeId( void ) const;
	void FreeValue( asIScriptEngine *engine );
	void EnumReferences( asIScriptEngine *engine );
protected:
	friend class CScriptLinkedList;

	union {
		asINT64 m_valueInt;
		double  m_valueFlt;
		void   *m_valueObj;
	};
	int m_typeId;
};

class CScriptLinkedList
{
public:
	static void SetMemoryFunctions( asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc );

	static CScriptLinkedList *Create( asITypeInfo *ot );
	static CScriptLinkedList *Create( asITypeInfo *ot, asUINT length );
	static CScriptLinkedList *Create( asITypeInfo *ot, asUINT length, void *defaultValue );
	static CScriptLinkedList *Create( asITypeInfo *ot, void *listBuffer );

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

	CScriptLinkedList& operator=( const CScriptLinkedList& );

	bool operator==( const CScriptLinkedList& ) const;

	void InsertAt( asUINT index, void *value );
	void InsertAt( asUINT index, const CScriptLinkedList& arr );
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
	mutable CThreadAtomic<int> refCount;
	mutable bool    gcFlag;
	asITypeInfo    *objType;
	asIScriptFunction *subTypeHandleAssignFunc;
	uint32_t         elementSize;
	int32_t         subTypeId;

	UtlList<CScriptLinkedList *> items;

	// Constructors
	CScriptLinkedList( asITypeInfo *ot, void *initBuf ); // Called from script when initialized with list
	CScriptLinkedList( asUINT length, asITypeInfo *ot );
	CScriptLinkedList( asUINT length, void *defVal, asITypeInfo *ot );
	CScriptLinkedList( const CScriptLinkedList& other );
	virtual ~CScriptLinkedList();

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

void RegisterScriptArray(asIScriptEngine *engine);

#endif
