#include "scriptarray.h"
#include "scriptstack.h"

const asPWORD STACK_CACHE = 1001;

static void CleanupTypeInfoArrayCache( asITypeInfo *type )
{
	SArrayCache *cache = (SArrayCache *)type->GetUserData( STACK_CACHE );
	if ( cache ) {
		cache->~SArrayCache();
		userFree( cache );
	}
}

CScriptStack* CScriptStack::Create( asITypeInfo *ti, asUINT length )
{
    static void *mem = alloca16( sizeof( CScriptStack ) );
	static CScriptStack *a = new(mem) CScriptStack(length, ti);

	return a;
}

CScriptStack* CScriptStack::Create(asITypeInfo *ti, void *initList)
{
    static void *mem = alloca16( sizeof( CScriptStack ) );
	static CScriptStack *a = new(mem) CScriptStack(ti, initList);
	return a;
}

CScriptStack* CScriptStack::Create(asITypeInfo *ti, asUINT length, void *defVal)
{
	static void *mem = alloca16( sizeof( CScriptStack ) );
	static CScriptStack *a = new(mem) CScriptStack(length, defVal, ti);

	return a;
}

CScriptStack* CScriptStack::Create(asITypeInfo *ti)
{
    static CScriptStack *data = CScriptStack::Create( ti, asUINT( 0 ) );
    return data;
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
				ti->GetEngine()->WriteMessage("stack", 0, 0, asMSGTYPE_ERROR, "The subtype has no default constructor");
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
				ti->GetEngine()->WriteMessage("stack", 0, 0, asMSGTYPE_ERROR, "The subtype has no default factory");
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


CScriptStack& CScriptStack::operator=( const CScriptStack& other )
{
	// Only perform the copy if the array types are the same
	if ( &other != this && other.GetArrayObjectType() == GetArrayObjectType() ) {
		Resize( other.buffer->size );
		CopyBuffer( buffer, other.buffer );
	}

	return *this;
}

CScriptStack::CScriptStack(asITypeInfo *ti, void *buf)
{
	if ( !ti ) {
		N_Error( ERR_FATAL, "CScriptStack::CScriptStack: typeInfo is NULL" );
	}
	if ( N_stricmp( ti->GetName(), "array" ) ) {
		N_Error( ERR_FATAL, "CScriptStack::CScriptStack: typeInfo isn't a stack" );
	}
	// The object type should be the template instance of the stack
	Assert( ti && !N_stricmp( ti->GetName(), "stack" ) );

	refCount = 1;
	gcFlag = false;
	objType = ti;
	objType->AddRef();
	buffer = NULL;
	Clear();

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

CScriptStack::CScriptStack( asUINT length, asITypeInfo *ti )
{
	if ( !ti ) {
		N_Error( ERR_FATAL, "CScriptStack::CScriptStack: typeInfo is NULL" );
	}
	if ( N_stricmp( ti->GetName(), "stack" ) ) {
		N_Error( ERR_FATAL, "CScriptStack::CScriptStack: typeInfo isn't a stack" );
	}
	// The object type should be the template instance of the stack
	Assert( ti && !N_stricmp( ti->GetName(), "stack" ) );

	refCount = 1;
	gcFlag = false;
	objType = ti;
	objType->AddRef();
	buffer = NULL;
	Clear();

	Precache();

	// Determine element size
	if ( subTypeId & asTYPEID_MASK_OBJECT ) {
		elementSize = sizeof(asPWORD);
    } else {
		elementSize = objType->GetEngine()->GetSizeOfPrimitiveType(subTypeId);
    }

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

void CScriptStack::SetValue( asUINT index, void *value )
{
	// At() will take care of the out-of-bounds checking, though
	// if called from the application then nothing will be done
	void *ptr = At(index);

	if ( ptr == 0 ) {
        return;
    }

	if ((subTypeId & ~asTYPEID_MASK_SEQNBR) && !(subTypeId & asTYPEID_OBJHANDLE)) {
		asITypeInfo *subType = objType->GetSubType();
		if (subType->GetFlags() & asOBJ_ASHANDLE) {
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
	else if( subTypeId & asTYPEID_OBJHANDLE ) {
		void *tmp = *(void**)ptr;
		*(void**)ptr = *(void**)value;
		objType->GetEngine()->AddRefScriptObject(*(void**)value, objType->GetSubType());
		if( tmp ) {
			objType->GetEngine()->ReleaseScriptObject(tmp, objType->GetSubType());
        }
	}
	else if( subTypeId == asTYPEID_BOOL || subTypeId == asTYPEID_INT8 || subTypeId == asTYPEID_UINT8 ) {
		*(char*)ptr = *(char*)value;
    }
	else if( subTypeId == asTYPEID_INT16 || subTypeId == asTYPEID_UINT16 ) {
		*(short*)ptr = *(short*)value;
    }
	else if( subTypeId == asTYPEID_INT32 || subTypeId == asTYPEID_UINT32 ||  subTypeId == asTYPEID_FLOAT
        || subTypeId > asTYPEID_DOUBLE ) // enums have a type id larger than doubles
    {
		*(int*)ptr = *(int*)value;
    }
	else if( subTypeId == asTYPEID_INT64 || subTypeId == asTYPEID_UINT64 || subTypeId == asTYPEID_DOUBLE ) {
		*(double*)ptr = *(double*)value;
    }
}


CScriptStack::~CScriptStack()
{
	Clear();
	if ( objType ) {
		objType->Release();
	}
}

asUINT CScriptStack::GetSize() const
{
	return buffer->size;
}

bool CScriptStack::IsEmpty() const
{
	return !buffer->size;
}

void CScriptStack::Reserve(asUINT nItems)
{
	if ( !CheckMaxSize( nItems ) ) {
		return;
	}

	if ( buffer->capacity < nItems ) {
		DoAllocate( nItems, buffer->size );
	}
}

void CScriptStack::Resize(asUINT numElements)
{
	if( !CheckMaxSize(numElements) )
		return;

	DoAllocate( (int)numElements - (int)buffer->size, (asUINT)-1 );
}

void CScriptStack::RemoveRange(asUINT start, asUINT count)
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
// CScriptStack::Clear: resets count
//
void CScriptStack::Clear( void )
{
	if ( !buffer ) {
		return;
	}
	
	Destruct( buffer, 0, buffer->size );
	
	// clear it all
	buffer->size = 0;
	memset( buffer->data, 0, buffer->capacity );
}

void CScriptStack::AllocBuffer( uint32_t nItems )
{
	if ( !buffer ) {
        static void *data = m_Allocator.allocate( sizeof( *buffer ) - 1 + ( ( nItems * 4 ) * elementSize ), 16 );
        buffer = (SArrayBuffer *)data;
		if ( !buffer ) {
			asIScriptContext *pContext = asGetActiveContext();
			if ( pContext ) {
				pContext->SetException( "out of memory" );
			}
			return;
		}
		buffer->size = nItems;
		buffer->capacity = nItems * 4;
		Construct( buffer, 0, nItems );
	}
}

//
// CScriptStack::DoAllocate: checks if the buffer is large enough to support
// nItems, if not, allocate nItems * 4 more
//
void CScriptStack::DoAllocate( int delta, uint32_t at )
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

		static SArrayBuffer *buf = (SArrayBuffer *)m_Allocator.allocate( sizeof( *buf ) + ( buffer->capacity * elementSize ), 16 );
		if ( buf ) {
			buf->size = buffer->size;
			buf->capacity = buffer->capacity;
		} else {
			asIScriptContext *pContext = asGetActiveContext();
			if ( pContext ) {
				pContext->SetException( "out of memory" );
			}
			return;
		}

		// as objects in arrays of objects are not stored inline, it is safe to use a block copy here
		// since we're just copying the pointers to objects and not the actual objects.
		memcpy( buf->data, buffer->data, at * elementSize );
		if ( at < buffer->size ) {
			memcpy( buf->data + ( at + delta ) * elementSize, buffer->data + at * elementSize, ( buffer->size - at ) * elementSize );
		}
		Construct( buf, at, at + delta );

        buffer = NULL;
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
bool CScriptStack::CheckMaxSize( asUINT numElements )
{
	// This code makes sure the size of the buffer that is allocated
	// for the array doesn't overflow and becomes smaller than requested

	asUINT maxSize = MAX_UINT;
	if( elementSize > 0 )
		maxSize /= elementSize;

	if( numElements > maxSize )
	{
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("array integer size overflow");

		return false;
	}

	// OK
	return true;
}

asITypeInfo *CScriptStack::GetArrayObjectType() const
{
	return objType;
}

int CScriptStack::GetArrayTypeId() const
{
	return objType->GetTypeId();
}

int CScriptStack::GetElementTypeId() const
{
	return subTypeId;
}

void CScriptStack::InsertAt(asUINT index, void *value)
{
	if ( index > buffer->size ) {
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("out of range index");
		return;
	}
	DoAllocate( 1, index );
	SetValue( index, value );
}

void CScriptStack::InsertAt(asUINT index, const CScriptStack &arr)
{
	asUINT n, m;

	if (index > buffer->size)
	{
		asIScriptContext *ctx = asGetActiveContext();
		if (ctx)
			ctx->SetException("out of range index");
		return;
	}

	if (objType != arr.objType)
	{
		// This shouldn't really be possible to happen when
		// called from a script, but let's check for it anyway
		asIScriptContext *ctx = asGetActiveContext();
		if (ctx)
			ctx->SetException("mismatching array types");
		return;
	}

	const asUINT nItems = arr.GetSize();
	DoAllocate( nItems, index );
	if ( &arr == this ) {
		for ( n = 0; n < arr.GetSize(); n++ ) {
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

		for ( n = index + nItems, m = 0; n < arr.GetSize(); n++, m++ ) {
			// This const cast is allowed, since we know the
			// value will only be used to make a copy of it
			void *value = const_cast<void*>(arr.At(n));
			SetValue(index + index + m, value);
		}
	}
}

void CScriptStack::InsertLast(void *value)
{
	InsertAt(buffer->size, value);
}

void CScriptStack::RemoveAt(asUINT index)
{
	if( index >= buffer->size )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("out of range index");
		return;
	}

	DoAllocate( -1, index );
}

void CScriptStack::RemoveLast( void )
{
	RemoveAt( buffer->size - 1 );
}

void CScriptStack::DeleteBuffer( SArrayBuffer *buffer )
{
	Destruct( buffer, 0, buffer->size );
    buffer = NULL;
}


// Return a pointer to the array element. Returns 0 if the index is out of bounds
const void *CScriptStack::At(asUINT index) const
{
	if( !buffer ) {
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("index access attempt on empty array");
		return NULL;
	}
	if ( index >= buffer->size ) {
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("out of range index");
		return NULL;
	}

	if( (subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) ) {
		return *(void**)( buffer->data + elementSize*index);
	}
	
	return (const void *)( buffer->data + ( elementSize * index ) );
}
void *CScriptStack::At(asUINT index)
{
	if( !buffer ) {
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("index access attempt on empty array");
		return NULL;
	}
	if ( index >= buffer->size ) {
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("out of range index");
		return NULL;
	}

	if( (subTypeId & asTYPEID_MASK_OBJECT) && !(subTypeId & asTYPEID_OBJHANDLE) ) {
		return *(void**)( buffer->data + elementSize*index);
	}
	
	return (void *)( buffer->data + ( elementSize * index ) );
}

void *CScriptStack::GetBuffer( void ) {
	return buffer->data;
}

const void *CScriptStack::GetBuffer( void ) const {
	return buffer->data;
}


// internal
void CScriptStack::CreateBuffer( SArrayBuffer **buffer, asUINT nItems )
{
    static void *mem = m_Allocator.allocate( sizeof( *buffer ) - 1 + ( nItems * elementSize ), 16 );
	*buffer = (SArrayBuffer *)mem;

	if ( *buffer ) {
		(*buffer)->size = nItems;
		(*buffer)->capacity = nItems;
		Construct( *buffer, 0, nItems );
	} else {
		asIScriptContext *ctx = asGetActiveContext();
		if ( ctx ) {
			ctx->SetException("out of memory");
		}
		return;
	}
}


// internal
void CScriptStack::Construct( SArrayBuffer *buf, asUINT start, asUINT end )
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
void CScriptStack::Destruct(SArrayBuffer *buf, asUINT start, asUINT end)
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
bool CScriptStack::Less(const void *a, const void *b, bool asc)
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

void CScriptStack::Reverse()
{
	asUINT size = GetSize();

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

bool CScriptStack::operator==(const CScriptStack &other) const
{
	if( objType != other.objType )
		return false;

	if( GetSize() != other.GetSize() )
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
	SArrayCache *cache = reinterpret_cast<SArrayCache*>(objType->GetUserData( STACK_CACHE ));
	for( asUINT n = 0; n < GetSize(); n++ )
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
bool CScriptStack::Equals(const void *a, const void *b, asIScriptContext *ctx, SArrayCache *cache) const
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

int CScriptStack::FindByRef(void *ref) const
{
	return FindByRef(0, ref);
}

int CScriptStack::FindByRef(asUINT startAt, void *ref) const
{
	// Find the matching element by its reference
	asUINT size = GetSize();
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
		for( asUINT i = startAt; i < size; i++ )
		{
			if( At(i) == ref )
				return i;
		}
	}

	return -1;
}

// GC behaviour
void CScriptStack::ReleaseAllHandles( asIScriptEngine * ) {
    Clear();
}

void CScriptStack::AddRef( void ) const {
	gcFlag = false;
	refCount.fetch_add();
}

void CScriptStack::Release( void ) const {
	gcFlag = false;
	if( refCount.fetch_sub() == 0 ) {
		this->~CScriptStack();
	}
}

int CScriptStack::Find(void *value) const {
	return Find(0, value);
}

int CScriptStack::Find(asUINT startAt, void *value) const
{
	// Check if the subtype really supports find()
	// TODO: Can't this be done at compile time too by the template callback
	SArrayCache *cache;
	asIScriptContext *cmpContext;
	bool isNested;
	int ret;
	asUINT size;

	cache = NULL;
	isNested = false;
	cmpContext = NULL;
	ret = -1;

	if( subTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		cache = reinterpret_cast<SArrayCache*>(objType->GetUserData( STACK_CACHE ));
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
				ctx->SetException(tmp);
			}

			return -1;
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

	size = GetSize();

	// Find the matching element
	for( asUINT i = startAt; i < size; i++ ) {
		// value passed by reference
		if( Equals(At(i), value, cmpContext, cache) )
		{
			ret = (int)i;
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
void CScriptStack::Copy(void *dst, void *src)
{
	memcpy(dst, src, elementSize);
}


// internal
// Swap two elements
// Even in arrays of objects the objects are allocated on 
// the heap and the array stores the pointers to the objects.
void CScriptStack::Swap(void* a, void* b)
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
void *CScriptStack::GetArrayItemPointer(int index)
{
	return buffer->data + ( index * elementSize );
}

// internal
// Return pointer to data in buffer (object or primitive)
void *CScriptStack::GetDataPointer(void *buf)
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
void CScriptStack::SortAsc()
{
	Sort(0, GetSize(), true);
}

// Sort ascending
void CScriptStack::SortAsc(asUINT startAt, asUINT count)
{
	Sort(startAt, count, true);
}

// Sort descending
void CScriptStack::SortDesc()
{
	Sort(0, GetSize(), false);
}

// Sort descending
void CScriptStack::SortDesc(asUINT startAt, asUINT count)
{
	Sort(startAt, count, false);
}


// internal
void CScriptStack::Sort(asUINT startAt, asUINT count, bool asc)
{
	// Subtype isn't primitive and doesn't have opCmp
	SArrayCache *cache = (SArrayCache *)objType->GetUserData( STACK_CACHE );
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
void CScriptStack::Sort(asIScriptFunction *func, asUINT startAt, asUINT count)
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

void RegisterScriptStack( asIScriptEngine *pEngine )
{

}
