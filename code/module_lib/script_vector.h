#ifndef __SCRIPT_VECTOR__
#define __SCRIPT_VECTOR__

#pragma once

struct SArrayBuffer;
struct SArrayCache;

class CScriptArray
{
public:
	// Set the memory functions that should be used by all CScriptArrays
	static void SetMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc);

	// Factory functions
	static CScriptArray *Create(asITypeInfo *ot);
	static CScriptArray *Create(asITypeInfo *ot, uint32_t length);
	static CScriptArray *Create(asITypeInfo *ot, uint32_t length, void *defaultValue);
	static CScriptArray *Create(asITypeInfo *ot, void *listBuffer);

	// Memory management
	void AddRef() const;
	void Release() const;

	// Type information
	asITypeInfo *GetArrayObjectType() const;
	int          GetArrayTypeId() const;
	int          GetElementTypeId() const;

	// Get the current size
	uint32_t GetSize() const;

	// Returns true if the array is empty
	bool   IsEmpty() const;

	// Pre-allocates memory for elements
	void   Reserve(uint32_t maxElements);

	// Resize the array
	void   Resize(uint32_t numElements);

	// Get a pointer to an element. Returns 0 if out of bounds
	void       *At(uint32_t index);
	const void *At(uint32_t index) const;

	// Set value of an element. 
	// The value arg should be a pointer to the value that will be copied to the element.
	// Remember, if the array holds handles the value parameter should be the 
	// address of the handle. The refCount of the object will also be incremented
	void  SetValue(uint32_t index, void *value);

	// Copy the contents of one array to another (only if the types are the same)
	CScriptArray &operator=(const CScriptArray&);

	// Compare two arrays
	bool operator==(const CScriptArray &) const;

	// Array manipulation
	void InsertAt( uint32_t index, void *value);
	void InsertAt( uint32_t index, const CScriptArray &arr);
	void InsertLast( void *value);
	void RemoveAt( uint32_t index);
	void RemoveLast( void );
	void RemoveRange(uint32_t start, uint32_t count);
	void SortAsc( void );
	void SortDesc( void );
	void SortAsc(uint32_t startAt, uint32_t count);
	void SortDesc(uint32_t startAt, uint32_t count);
	void Sort(uint32_t startAt, uint32_t count, bool asc);
	void Sort(asIScriptFunction *less, uint32_t startAt, uint32_t count);
	void Reverse();
	int  Find(void *value) const;
	int  Find(uint32_t startAt, void *value) const;
	int  FindByRef(void *ref) const;
	int  FindByRef(uint32_t startAt, void *ref) const;

	// Return the address of internal buffer for direct manipulation of elements
	void *GetBuffer();

	// GC methods
	int  GetRefCount();
	void SetFlag();
	bool GetFlag();
	void EnumReferences(asIScriptEngine *engine);
	void ReleaseAllHandles(asIScriptEngine *engine);

protected:
	mutable int     refCount;
	mutable bool    gcFlag;
	asITypeInfo    *objType;
	SArrayBuffer   *buffer;
	int             elementSize;
	int             subTypeId;

	// Constructors
	CScriptArray(asITypeInfo *ot, void *initBuf); // Called from script when initialized with list
	CScriptArray(uint32_t length, asITypeInfo *ot);
	CScriptArray(uint32_t length, void *defVal, asITypeInfo *ot);
	CScriptArray(const CScriptArray &other);
	virtual ~CScriptArray();

	bool  Less(const void *a, const void *b, bool asc);
	void *GetArrayItemPointer(int index);
	void *GetDataPointer(void *buffer);
	void  Copy(void *dst, void *src);
	void  Swap(void *a, void *b);
	void  Precache();
	bool  CheckMaxSize(uint32_t numElements);
	void  Resize(int delta, uint32_t at);
	void  CreateBuffer(SArrayBuffer **buf, uint32_t numElements);
	void  DeleteBuffer(SArrayBuffer *buf);
	void  CopyBuffer(SArrayBuffer *dst, SArrayBuffer *src);
	void  Construct(SArrayBuffer *buf, uint32_t start, uint32_t end);
	void  Destruct(SArrayBuffer *buf, uint32_t start, uint32_t end);
	bool  Equals(const void *a, const void *b, asIScriptContext *ctx, SArrayCache *cache) const;
};

void RegisterScriptArray(asIScriptEngine *engine, bool defaultArray);

#endif