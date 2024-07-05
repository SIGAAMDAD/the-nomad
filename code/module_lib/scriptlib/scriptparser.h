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

	int GetRefCount( void ) const;
	void SetFlag( void );
	bool GetFlag( void ) const;
	void EnumReferences( asIScriptEngine *pEngine );
	void ReleaseAllHandles( asIScriptEngine *pEngine );
	void AddRef( void ) const;
	void Release( void ) const;

	bool Load( const string_t& fileName );
	bool IsLoaded( void ) const { return pBuffer != NULL; }

	const char *Parse( bool allowLineBreaks );
	const char *ParseComplex( bool allowLineBreaks );

	void Rewind( void );

	bool MatchToken( const char *token );
	
	bool Parse1DMatrix( int x, int *m );
	bool Parse2DMatrix( int x, int y, int *m );
	bool Parse3DMatrix( int x, int y, int z, int *m );

	bool Parse1DMatrix( int x, float *m );
	bool Parse2DMatrix( int x, int y, float *m );
	bool Parse3DMatrix( int x, int y, int z, float *m );

	int32_t GetInt( const string_t& name );
	uint32_t GetUInt( const string_t& name );
	float GetFloat( const string_t& name );
	string_t GetString( const string_t& name );
	CScriptParser *GetObject( const string_t& name );

	void Error( const string_t& msg );
	void Warning( const string_t& msg );
	void BeginParseSession( const string_t& fileName );
	uint64_t GetCurrentParseLine( void ) const;
	bool SkipBracedSection( int depth );
private:
	void Error( const char *fmt, ... ) GDR_ATTRIBUTE((format(printf, 2, 3)));
	void Warning( const char *fmt, ... ) GDR_ATTRIBUTE((format(printf, 2, 3)));

	void SkipRestOfLine( void );
	const char *SkipWhitespace( const char *data, bool *hasNewLines );
	void Clear( void );
	const char *FindVar( const char *name );

	CScriptParser( void );
	CScriptParser( const string_t& fileName );
	~CScriptParser();

	char szToken[MAX_TOKEN_CHARS];
	char szParsename[MAX_TOKEN_CHARS];

	const char **pText;
	const char *pTextPointer;
	char *pBuffer;
	uint32_t nLength;

	uint64_t nLines;
	uint64_t nTokenLine;

	// for complex parser
	tokenType_t nTokenType;

	mutable CThreadAtomic<int> nRefCount;
	mutable qboolean bGCFlag;
};

void RegisterScriptParser( asIScriptEngine *pEngine );

#endif