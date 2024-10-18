//
// Script JSON.
//
#ifndef _ScriptJSON_h_
#define _ScriptJSON_h_

#include "../module_public.h"
#include "scriptarray.h"

typedef string_t jsonKey_t;
using json = nlohmann::json;

BEGIN_AS_NAMESPACE

class CScriptArray;

enum CScriptJsonType
{
    OBJECT_VALUE,
    ARRAY_VALUE,
    BOOLEAN_VALUE,
    STRING_VALUE,
    NUMBER_VALUE,
    REAL_VALUE,
    NULL_VALUE
};

class CScriptJson
{
public:
	// Factory functions
	static CScriptJson *Create(asIScriptEngine *engine);
	static CScriptJson *Create(asIScriptEngine *engine, const json& js);

	// Reference counting
	void AddRef() const;
	void Release() const;

	// Reassign the json
	CScriptJson& operator=( bool other );
	CScriptJson& operator=( asDWORD other );
	CScriptJson& operator=( asINT32 other );
	CScriptJson& operator=( float other );
	CScriptJson& operator=( const string_t& other );
	CScriptJson& operator=( const CScriptArray& other );
	CScriptJson& operator=( const CScriptJson& other );

	// Sets a key/value pair
	void Set(const jsonKey_t& key, const bool *value);
	void Set(const jsonKey_t& key, const asINT32 *value);
	void Set(const jsonKey_t& key, const asINT64 *value);
	void Set(const jsonKey_t& key, const asDWORD *value);
	void Set(const jsonKey_t& key, const asQWORD *value);
	void Set(const jsonKey_t& key, const float *value);
	void Set(const jsonKey_t& key, const string_t& value);
	void Set(const jsonKey_t& key, const CScriptArray  &value);
	void Set(const jsonKey_t& key, const CScriptJson& value);

	// Gets the stored value. Returns false if the value isn't compatible
	bool Get(const jsonKey_t& key, bool *value) const;
	bool Get(const jsonKey_t& key, asINT32 *value) const;
	bool Get(const jsonKey_t& key, asINT64 *value) const;
	bool Get(const jsonKey_t& key, asDWORD *value) const;
	bool Get(const jsonKey_t& key, asQWORD *value) const;
	bool Get(const jsonKey_t& key, float *value) const;
	bool Get(const jsonKey_t& key, string_t& value) const;
	bool Get(const jsonKey_t& key, CScriptArray& value) const;
	bool Get(const jsonKey_t& key, CScriptJson& value) const;

    bool			GetBool( void );
    string_t		GetString( void );
    asINT32			GetNumber( void );
	asDWORD			GetUNumber( void );
    float			GetReal( void );
    CScriptArray	*GetArray( void );
	CScriptJson		*GetObject( void );

	// Index accessors. If the json is not const it inserts the value if it doesn't already exist
	// If the json is const then a script exception is set if it doesn't exist and a null pointer is returned
	CScriptJson *operator[](const jsonKey_t &key);
	const CScriptJson *operator[](const jsonKey_t &key) const;

	CScriptJson *operator[](const uint32_t key);
	const CScriptJson *operator[](const uint32_t key) const;

	// Returns true if the key is set
	bool Exists(const jsonKey_t &key) const;

	// Returns true if there are no key/value pairs in the json
	bool IsEmpty() const;

	// Returns the number of key/value pairs in the json
	asUINT GetSize() const;

	// Deletes all keys
	void Clear();

	// Get Value type
	CScriptJsonType Type();

	int GetRefCount();

    json             js_info = { 0 };

private:
	// Since the dictionary uses the asAllocMem and asFreeMem functions to allocate memory
	// the constructors are made protected so that the application cannot allocate it
	// manually in a different way
	CScriptJson(asIScriptEngine *engine);

	// We don't want anyone to call the destructor directly, it should be called through the Release method
    ~CScriptJson();

	// Our properties
	asIScriptEngine *engine;
	mutable CThreadAtomic<int>      refCount;
};

// This function will determine the configuration of the engine
// and use one of the two functions below to register the dictionary object
void RegisterScriptJson(asIScriptEngine *engine);

END_AS_NAMESPACE

#endif