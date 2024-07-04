#ifndef __SCRIPT_PARSER__
#define __SCRIPT_PARSER__

#pragma once

#include "scriptarray.h"

class CScriptParser
{
public:
	static CScriptParser *Create( void );
	static CScriptParser *Create( const string_t& fileName );

	CScriptParser& operator=( const CScriptParser& other );

	int GetRefCount( void );
	void SetFlag( void );
	bool GetFlag( void );
	void EnumReferences( asIScriptEngine *pEngine );
	void ReleaseAllHandles( asIScriptEngine *pEngine );
	void AddRef( void ) const;
	void Release( void ) const;

	bool Load( const string_t& fileName );
	bool IsLoaded( void ) const { return pBuffer != NULL; }

	string_t Parse( void );
	string_t ParseExt( bool allowLineBreaks );

	void Rewind( void );

	int32_t GetInt( const string_t& name );
	uint32_t GetUInt( const string_t& name );
	float GetFloat( const string_t& name );
	string_t GetString( const string_t& name );
	CScriptParser *GetObject( const string_t& name );
private:
	void Clear( void );
	const char *FindVar( const char *name );

	CScriptParser( void );
	CScriptParser( const string_t& fileName );
	~CScriptParser();

	const char **pText;
	const char *pTextPointer;
	char *pBuffer;
	uint32_t nLength;

	mutable CThreadAtomic<int> nRefCount;
	mutable qboolean bGCFlag;
};

#endif