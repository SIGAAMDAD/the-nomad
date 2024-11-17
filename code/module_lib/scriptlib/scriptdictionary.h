#ifndef SCRIPTDICTIONARY_H
#define SCRIPTDICTIONARY_H

// The dictionary class relies on the script string object, thus the script
// string type must be registered with the engine before registering the
// dictionary type

#include "../module_public.h"
#include "scriptarray.h"
#include "../../engine/n_threads.h"
typedef string_t dictKey_t;

class CScriptDictValue;
typedef UtlMap<dictKey_t, CScriptDictValue> dictMap_t;

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

class CScriptArray;
class CScriptDictionary;

class CScriptDictValue
{
public:
	// This class must not be declared as local variable in C++, because it needs
	// to receive the script engine pointer in all operations. The engine pointer
	// is not kept as member in order to keep the size down
	CScriptDictValue( void );
	CScriptDictValue( asIScriptEngine *engine, void *value, int typeId );

	// Destructor must not be called without first calling FreeValue, otherwise a memory leak will occur
	~CScriptDictValue();

	// Replace the stored value
	void Set( asIScriptEngine *engine, void *value, int typeId );
	void Set( asIScriptEngine *engine, const asINT32& value );
	void Set( asIScriptEngine *engine, const asDWORD& value );
	void Set( asIScriptEngine *engine, const asINT64& value );
	void Set( asIScriptEngine *engine, const asQWORD& value );
	void Set( asIScriptEngine *engine, const double& value );
	void Set( asIScriptEngine *engine, CScriptDictValue& value );

	// Gets the stored value. Returns false if the value isn't compatible with the informed typeId
	bool Get( asIScriptEngine *engine, void *value, int typeId ) const;
	bool Get( asIScriptEngine *engine, asINT32& value ) const;
	bool Get( asIScriptEngine *engine, asDWORD& value ) const;
	bool Get( asIScriptEngine *engine, asINT64& value ) const;
	bool Get( asIScriptEngine *engine, asQWORD& value ) const;
	bool Get( asIScriptEngine *engine, double& value ) const;

	// Returns the address of the stored value for inspection
	const void *GetAddressOfValue( void ) const;

	// Returns the type id of the stored value
	int  GetTypeId( void ) const;

	// Free the stored value
	void FreeValue( asIScriptEngine *engine );

	// GC callback
	void EnumReferences( asIScriptEngine *engine );
protected:
	friend class CScriptDictionary;

	union {
		asINT64 m_valueInt;
		double  m_valueFlt;
		void   *m_valueObj;
	};
	int m_typeId;
};

class CScriptDictionary
{
public:
	// Factory functions
	static CScriptDictionary *Create( asIScriptEngine *engine );

	// Called from the script to instantiate a dictionary from an initialization list
	static CScriptDictionary *Create( asBYTE *buffer );

	// Reference counting
	void AddRef( void ) const;
	void Release( void ) const;

	// Reassign the dictionary
	CScriptDictionary &operator=( const CScriptDictionary &other );

	// Sets a key/value pair
	void Set( const dictKey_t &key, void *value, int typeId );
	void Set( const dictKey_t &key, const asINT32& value );
	void Set( const dictKey_t &key, const asDWORD& value );
	void Set( const dictKey_t &key, const asINT64& value );
	void Set( const dictKey_t &key, const asQWORD& value );
	void Set( const dictKey_t &key, const double& value );

	// Gets the stored value. Returns false if the value isn't compatible with the informed typeId
	bool Get( const dictKey_t &key, void *value, int typeId ) const;
	bool Get( const dictKey_t &key, asINT32& value ) const;
	bool Get( const dictKey_t &key, asDWORD& value ) const;
	bool Get( const dictKey_t &key, asINT64& value ) const;
	bool Get( const dictKey_t &key, asQWORD& value ) const;
	bool Get( const dictKey_t &key, double& value ) const;

	// Index accessors. If the dictionary is not const it inserts the value if it doesn't already exist
	// If the dictionary is const then a script exception is set if it doesn't exist and a null pointer is returned
	CScriptDictValue *operator[]( const dictKey_t &key );
	const CScriptDictValue *operator[]( const dictKey_t &key ) const;

	// Returns the type id of the stored value, or negative if it doesn't exist
	int GetTypeId( const dictKey_t &key ) const;

	// Returns true if the key is set
	bool Exists( const dictKey_t &key ) const;

	// Returns true if there are no key/value pairs in the dictionary
	bool IsEmpty( void ) const;

	// Returns the number of key/value pairs in the dictionary
	asUINT GetSize( void ) const;

	// Deletes the key
	bool Delete( const dictKey_t &key );

	// Deletes all keys
	void DeleteAll( void );

	// Get an array of all keys
	CScriptArray *GetKeys( void ) const;

	class CIterator
	{
	public:
		void operator++( void );    // Pre-increment
		void operator++( int ); // Post-increment

		// This is needed to support C++11 range-for
		CIterator& operator*( void );

		bool operator==( const CIterator& other ) const;
		bool operator!=( const CIterator& other ) const;

		// Accessors
		const dictKey_t& GetKey( void ) const;
		int              GetTypeId( void ) const;
		bool             GetValue( asINT32& value ) const;
		bool             GetValue( asDWORD& value ) const;
		bool             GetValue( asINT64& value ) const;
		bool             GetValue( asQWORD& value ) const;
		bool             GetValue( double& value ) const;
		bool             GetValue( void *value, int typeId ) const;
		const void *     GetAddressOfValue( void ) const;
	protected:
		friend class CScriptDictionary;

		CIterator( void );
		CIterator( const CScriptDictionary &dict, dictMap_t::const_iterator it );

		CIterator& operator=( const CIterator & ) { return *this; } // Not used

		dictMap_t::const_iterator m_it;
		const CScriptDictionary &m_Dict;
	};

	CIterator begin( void ) const;
	CIterator end( void ) const;
	CIterator find( const dictKey_t &key ) const;

	// Garbage collections behaviours
	int32_t GetRefCount( void );
	void SetGCFlag( void );
	bool GetGCFlag( void );
	void EnumReferences( asIScriptEngine *engine );
	void ReleaseAllReferences( asIScriptEngine *engine );
protected:
	// Since the dictionary uses the asAllocMem and asFreeMem functions to allocate memory
	// the constructors are made protected so that the application cannot allocate it
	// manually in a different way
	CScriptDictionary( asIScriptEngine *engine );
	CScriptDictionary( asBYTE *buffer );

	// We don't want anyone to call the destructor directly, it should be called through the Release method
	virtual ~CScriptDictionary();

	// Cache the object types needed
	void Init( asIScriptEngine *engine );

	// Our properties
	asIScriptEngine *m_pEngine;
	mutable CThreadAtomic<int32_t>  m_nRefCount;
	mutable bool     m_bGCFlag;
	dictMap_t        m_Dict;
};

// This function will determine the configuration of the engine
// and use one of the two functions below to register the dictionary object
void RegisterScriptDictionary( asIScriptEngine *engine );

// Call this function to register the math functions
// using native calling conventions
void RegisterScriptDictionary_Native( asIScriptEngine *engine );

// Use this one instead if native calling conventions
// are not supported on the target platform
void RegisterScriptDictionary_Generic( asIScriptEngine *engine );

#endif
