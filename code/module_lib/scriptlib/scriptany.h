#ifndef __SCRIPT_ANY__
#define __SCRIPT_ANY__

#pragma once

#include "../module_public.h"

class CScriptAny
{
public:
	// Constructors
	CScriptAny( asIScriptEngine *engine );
	CScriptAny( void *ref, int refTypeId, asIScriptEngine *engine );

	// Memory management
	int AddRef( void ) const;
	int Release( void ) const;

	// Copy the stored value from another any object
	CScriptAny& operator=( const CScriptAny& );
	int CopyFrom( const CScriptAny *other );

	// Store the value, either as variable type, integer number, or real number
	void Store( void *ref, int refTypeId );
    void Store( bool& value );
    void Store( int8_t& value );
    void Store( int16_t& value );
    void Store( int32_t& value );
	void Store( int64_t& value );
    void Store( uint8_t& value );
    void Store( uint16_t& value );
    void Store( uint32_t& value );
	void Store( uint64_t& value );
    void Store( float& value );
	void Store( double& value );

	// Retrieve the stored value, either as variable type, integer number, or real number
	bool Retrieve( void *ref, int refTypeId ) const;
    bool Retrieve( bool& value ) const;
	bool Retrieve( int8_t& value ) const;
    bool Retrieve( int16_t& value ) const;
    bool Retrieve( int32_t& value ) const;
	bool Retrieve( int64_t& value ) const;
    bool Retrieve( uint8_t& value ) const;
    bool Retrieve( uint16_t& value ) const;
    bool Retrieve( uint32_t& value ) const;
	bool Retrieve( uint64_t& value ) const;
    bool Retrieve( float& value ) const;
	bool Retrieve( double& value ) const;

	// Get the type id of the stored value
	int  GetTypeId( void ) const;

	// GC methods
	int  GetRefCount( void ) const;
	void SetFlag( void );
	bool GetFlag( void ) const;
	void EnumReferences( asIScriptEngine *engine );
	void ReleaseAllHandles( asIScriptEngine *engine );
protected:
	virtual ~CScriptAny();
	void FreeObject( void );

    void ValueAssign( void *ref, int refTypeId ) const;

	mutable CThreadAtomic<int32_t> refCount;
	mutable qboolean gcFlag;
	asIScriptEngine *engine;

	// The structure for holding the values
    struct valueStruct {
        union {
            bool b;
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint64_t u64;
            int8_t s8;
            int16_t s16;
            int32_t s32;
            int64_t s64;
            float flt32;
            double flt64;
            void *valueObj;
        } data;
        int typeId;
    };

	valueStruct value;
};

void RegisterScriptAny( asIScriptEngine *engine );

END_AS_NAMESPACE

#endif