#include "scriptparser.h"

CScriptParser *CScriptParser::Create( void ) {
    void *mem = Mem_ClearedAlloc( sizeof( CScriptParser ) );
    if ( !mem ) {
        asIScriptContext *pContext = asGetActiveContext();
        if ( pContext ) {
            pContext->SetException( "out of memory" );
        }
        return NULL;
    }
    CScriptParser *parser = new ( mem ) CScriptParser();
    return parser;
}

CScriptParser *CScriptParser::Create( const string_t& fileName ) {
    void *mem = Mem_ClearedAlloc( sizeof( CScriptParser ) );
    if ( !mem ) {
        asIScriptContext *pContext = asGetActiveContext();
        if ( pContext ) {
            pContext->SetException( "out of memory" );
        }
        return NULL;
    }
    CScriptParser *parser = new ( mem ) CScriptParser( fileName );
    return parser;
}

CScriptParser::CScriptParser( void ) {
    nRefCount.store( 1 );
	bGCFlag = false;
    pBuffer = NULL;
    nLength = 0;
}

CScriptParser::CScriptParser( const string_t& fileName ) {
    nRefCount.store( 1 );
	bGCFlag = false;
    pBuffer = NULL;
    nLength = 0;

    Load( fileName );
}

CScriptParser::~CScriptParser() {
    if ( pBuffer && nRefCount.load() <= 1 ) {
        Mem_Free( pBuffer );
        memset( this, 0, sizeof( *this ) );
    }
}

CScriptParser& CScriptParser::operator=( const CScriptParser& other )
{
    pBuffer = other.pBuffer;
    pText = other.pText;
    nLength = other.nLength;

    memcpy( szToken, other.szToken, sizeof( other.szToken ) );
    memcpy( szParsename, other.szParsename, sizeof( other.szParsename ) );
    nLines = other.nLines;
    nTokenLine = other.nTokenLine;
    nTokenType = other.nTokenType;

    return *this;
}


void CScriptParser::BeginParseSession( const string_t& fileName )
{
	nLines = 1;
	nTokenLine = 0;
	Com_snprintf( szParsename, sizeof( szParsename ) - 1, "%s", fileName.c_str() );
}


uint64_t CScriptParser::GetCurrentParseLine( void ) const
{
	if ( nTokenLine ) {
		return nTokenLine;
	}

	return nLines;
}

void CScriptParser::Error( const string_t& string )
{
	Con_Printf( COLOR_RED "ERROR: %s, line %lu: %s\n", szParsename, GetCurrentParseLine(), string.c_str() );
}

void CScriptParser::Warning( const string_t& string )
{
	Con_Printf( COLOR_YELLOW "WARNING: %s, line %lu: %s\n", szParsename, GetCurrentParseLine(), string.c_str() );
}


/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
const char *CScriptParser::SkipWhitespace( const char *data, bool *hasNewLines ) {
	int c;

	while ( ( c = *data ) <= ' ' ) {
		if ( !c ) {
			return NULL;
		}
		if ( c == '\n' ) {
			nLines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

const char *CScriptParser::Parse( bool allowLineBreaks )
{
	int c = 0, len;
	bool hasNewLines = false;
	const char *data;

	data = *pText;
	len = 0;
	szToken[0] = '\0';
	nTokenLine = 0;

	// make sure incoming data is valid
	if ( !data ) {
		*pText = NULL;
		return szToken;
	}

	while ( 1 ) {
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data ) {
			*pText = NULL;
			return szToken;
		}
		if ( hasNewLines && !allowLineBreaks ) {
			*pText = data;
			return szToken;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' ) {
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c == '/' && data[1] == '*' ) {
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) ) {
				if ( *data == '\n' ) {
					nLines++;
				}
				data++;
			}
			if ( *data ) {
				data += 2;
			}
		}
		else {
			break;
		}
	}

	// token starts on this line
	nTokenLine = nLines;

	// handle quoted strings
	if ( c == '"' )
	{
		data++;
		while ( 1 )
		{
			c = *data;
			if ( c == '"' || c == '\0' )
			{
				if ( c == '"' )
					data++;
				szToken[ len ] = '\0';
				*pText = data;
				return szToken;
			}
			data++;
			if ( c == '\n' )
			{
				nLines++;
			}
			if ( len < arraylen( szToken )-1 )
			{
				szToken[ len ] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do {
		if ( len < arraylen( szToken )-1 ) {
			szToken[ len ] = c;
			len++;
		}
		data++;
		c = *data;
	} while ( c > ' ' );

	szToken[ len ] = '\0';

	*pText = data;
	return szToken;
}
	

/*
==============
COM_ParseComplex
==============
*/
const char *CScriptParser::ParseComplex( bool allowLineBreaks )
{
	static const byte is_separator[ 256 ] =
	{
	// \0 . . . . . . .\b\t\n . .\r . .
		1,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,
	//  . . . . . . . . . . . . . . . .
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//    ! " # $ % & ' ( ) * + , - . /
		1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // excl. '-' '.' '/'
	//  0 1 2 3 4 5 6 7 8 9 : ; < = > ?
		0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	//  @ A B C D E F G H I J K L M N O
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  P Q R S T U V W X Y Z [ \ ] ^ _
		0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0, // excl. '\\' '_'
	//  ` a b c d e f g h i j k l m n o
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  p q r s t u v w x y z { | } ~ 
		0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1
	};

	int c, len, shift;
	const byte *str;

	str = (byte*)*pText;
	len = 0; 
	shift = 0; // token line shift relative to nLines
	nTokenType = TK_GENEGIC;
	
__reswitch:
	switch ( *str )
	{
	case '\0':
		nTokenType = TK_EOF;
		break;

	// whitespace
	case ' ':
	case '\t':
		str++;
		while ( (c = *str) == ' ' || c == '\t' )
			str++;
		goto __reswitch;

	// newlines
	case '\n':
	case '\r':
	nLines++;
		if ( *str == '\r' && str[1] == '\n' )
			str += 2; // CR+LF
		else
			str++;
		if ( !allowLineBreaks ) {
			nTokenType = TK_NEWLINE;
			break;
		}
		goto __reswitch;

	// comments, single slash
	case '/':
		// until end of line
		if ( str[1] == '/' ) {
			str += 2;
			while ( (c = *str) != '\0' && c != '\n' && c != '\r' )
				str++;
			goto __reswitch;
		}

		// comment
		if ( str[1] == '*' ) {
			str += 2;
			while ( (c = *str) != '\0' && ( c != '*' || str[1] != '/' ) ) {
				if ( c == '\n' || c == '\r' ) {
					nLines++;
					if ( c == '\r' && str[1] == '\n' ) // CR+LF?
						str++;
				}
				str++;
			}
			if ( c != '\0' && str[1] != '\0' ) {
				str += 2;
			} else {
				// FIXME: unterminated comment?
			}
			goto __reswitch;
		}

		// single slash
		szToken[ len++ ] = *str++;
		break;
	
	// quoted string?
	case '"':
		str++; // skip leading '"'
		//nTokenLine = nLines;
		while ( (c = *str) != '\0' && c != '"' ) {
			if ( c == '\n' || c == '\r' ) {
				nLines++; // FIXME: unterminated quoted string?
				shift++;
			}
			if ( len < MAX_TOKEN_CHARS-1 ) // overflow check
				szToken[ len++ ] = c;
			str++;
		}
		if ( c != '\0' ) {
			str++; // skip ending '"'
		} else {
			// FIXME: unterminated quoted string?
		}
		nTokenType = TK_QUOTED;
		break;

	// single tokens:
	case '+': case '`':
	/*case '*':*/ case '~':
	case '{': case '}':
	case '[': case ']':
	case '?': case ',':
	case ':': case ';':
	case '%': case '^':
		szToken[ len++ ] = *str++;
		break;

	case '*':
		szToken[ len++ ] = *str++;
		nTokenType = TK_MATCH;
		break;

	case '(':
		szToken[ len++ ] = *str++;
		nTokenType = TK_SCOPE_OPEN;
		break;

	case ')':
		szToken[ len++ ] = *str++;
		nTokenType = TK_SCOPE_CLOSE;
		break;

	// !, !=
	case '!':
		szToken[ len++ ] = *str++;
		if ( *str == '=' ) {
			szToken[ len++ ] = *str++;
			nTokenType = TK_NEQ;
		}
		break;

	// =, ==
	case '=':
		szToken[ len++ ] = *str++;
		if ( *str == '=' ) {
			szToken[ len++ ] = *str++;
			nTokenType = TK_EQ;
		}
		break;

	// >, >=
	case '>':
		szToken[ len++ ] = *str++;
		if ( *str == '=' ) {
			szToken[ len++ ] = *str++;
			nTokenType = TK_GTE;
		} else {
			nTokenType = TK_GT;
		}
		break;

	//  <, <=
	case '<':
		szToken[ len++ ] = *str++;
		if ( *str == '=' ) {
			szToken[ len++ ] = *str++;
			nTokenType = TK_LTE;
		} else {
			nTokenType = TK_LT;
		}
		break;

	// |, ||
	case '|':
		szToken[ len++ ] = *str++;
		if ( *str == '|' ) {
			szToken[ len++ ] = *str++;
			nTokenType = TK_OR;
		}
		break;

	// &, &&
	case '&':
		szToken[ len++ ] = *str++;
		if ( *str == '&' ) {
			szToken[ len++ ] = *str++;
			nTokenType = TK_AND;
		}
		break;

	// rest of the charset
	default:
		szToken[ len++ ] = *str++;
		while ( !is_separator[ (c = *str) ] ) {
			if ( len < MAX_TOKEN_CHARS-1 )
				szToken[ len++ ] = c;
			str++;
		}
		nTokenType = TK_STRING;
		break;

	} // switch ( *str )

	nTokenLine = nLines - shift;
	szToken[ len ] = '\0';
	*pText = ( char * )str;
	return szToken;
}

bool CScriptParser::MatchToken( const char *match ) {
	const char *token;

	token = Parse( true );
	if ( strcmp( token, match ) ) {
        return false;
	}
    return true;
}


/*
=================
SkipBracedSection

The next token should be an open brace or set depth to 1 if already parsed it.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
bool CScriptParser::SkipBracedSection( int depth ) {
	const char			*token;

	do {
		token = Parse( true );
		if ( token[1] == 0 ) {
			if ( token[0] == '{' ) {
				depth++;
			}
			else if ( token[0] == '}' ) {
				depth--;
			}
		}
	} while ( depth && *pText );

	return ( depth == 0 );
}


/*
=================
SkipRestOfLine
=================
*/
void CScriptParser::SkipRestOfLine( void ) {
	const char *p;
	int		c;

	p = *pText;

	if ( !*p )
		return;

	while ( (c = *p) != '\0' ) {
		p++;
		if ( c == '\n' ) {
			nLines++;
			break;
		}
	}

	*pText = p;
}

bool CScriptParser::Parse1DMatrix( int x, float *m ) {
	const char	*token;
	int		i;

    if ( !MatchToken( "(" ) ) {
        return false;
    }

	for (i = 0 ; i < x; i++) {
		token = Parse( true );
		m[i] = N_atof( token );
	}

	if ( !MatchToken( ")" ) ) {
        return false;
    }
    return true;
}

bool CScriptParser::Parse2DMatrix( int x, int y, float *m ) {
	int		i;

    if ( !MatchToken( "(" ) ) {
        return false;
    }

	for ( i = 0 ; i < y ; i++ ) {
		if ( !Parse1DMatrix( x, m + i * x ) ) {
            return false;
        }
	}

    if ( !MatchToken( ")" ) ) {
        return false;
    }
    return true;
}

bool CScriptParser::Parse3DMatrix( int x, int y, int z, float *m ) {
	int		i;

	if ( !MatchToken( "(" ) ) {
        return false;
    }

	for ( i = 0 ; i < z ; i++) {
		if ( !Parse2DMatrix( x, y, m + i * x * y ) ) {
            return false;
        }
	}

	if ( !MatchToken( ")" ) ) {
        return false;
    }
    return true;
}

bool CScriptParser::Load( const string_t& fileName )
{
    char *pTempBuffer;

    BeginParseSession( fileName );

    nLength = FS_LoadFile( fileName.c_str(), (void **)&pTempBuffer );
    if ( !nLength || !pTempBuffer ) {
        Con_Printf( COLOR_YELLOW "Error loading parse file '%s'\n", fileName.c_str() );
        return false;
    }

    // move it off the hunk stack
    pBuffer = (char *)Mem_Alloc( nLength );
    memcpy( pBuffer, pTempBuffer, nLength );

    FS_FreeFile( pTempBuffer );

    return true;
}

int32_t CScriptParser::GetInt( const string_t& name )
{
    const char *text_p, *tok;

    text_p = pBuffer;
    pText = (const char **)&text_p;

    if ( ( tok = FindVar( name.c_str() ) ) == NULL ) {
        return 0;
    }
    tok = Parse( false );
    if ( !tok[0] ) {
        COM_ParseError( "missing value for parameter '%s'", name.c_str() );
        Clear();
        return 0;
    }

    return atoi( tok );
}

uint32_t CScriptParser::GetUInt( const string_t& name )
{
    const char *text_p, *tok;

    text_p = pBuffer;
    pText = (const char **)&text_p;

    if ( ( tok = FindVar( name.c_str() ) ) == NULL ) {
        return 0;
    }
    tok = Parse( false );
    if ( !tok[0] ) {
        COM_ParseError( "missing value for parameter '%s'", name.c_str() );
        Clear();
        return 0;
    }

    return (uint32_t)atol( tok );
}

float CScriptParser::GetFloat( const string_t& name )
{
    const char *text_p, *tok;

    text_p = pBuffer;
    pText = (const char **)&text_p;

    if ( ( tok = FindVar( name.c_str() ) ) == NULL ) {
        return 0.0f;
    }
    tok = Parse( false );
    if ( !tok[0] ) {
        COM_ParseError( "missing value for parameter '%s'", name.c_str() );
        Clear();
        return 0.0f;
    }

    return atof( tok );
}

string_t CScriptParser::GetString( const string_t& name )
{
    const char *text_p, *tok;

    text_p = pBuffer;
    pText = (const char **)&text_p;

    if ( ( tok = FindVar( name.c_str() ) ) == NULL ) {
        return "";
    }
    tok = Parse( false );
    if ( !tok[0] ) {
        COM_ParseError( "missing value for parameter '%s'", name.c_str() );
        Clear();
        return "";
    }

    return tok;
}

CScriptParser *CScriptParser::GetObject( const string_t& name )
{
    const char *text_p, *tok;
    CScriptParser *obj;

    text_p = pBuffer;
    pText = (const char **)&text_p;

    if ( ( tok = FindVar( name.c_str() ) ) == NULL ) {
        return NULL;
    }
    tok = Parse( true );
    if ( !tok[0] ) {
        COM_ParseError( "missing object '%s'", name.c_str() );
        Clear();
        return NULL;
    } else if ( tok[0] != '{' ) {
        COM_ParseError( "expected '{' at beginning of object definition, instead got '%s'", tok );
        Clear();
        return NULL;
    }

    obj = CScriptParser::Create();
    return eastl::addressof( *obj = *this );
}

void CScriptParser::Rewind( void ) {
    pTextPointer = pBuffer;
    pText = (const char **)&pTextPointer;
}

void CScriptParser::Clear( void ) {
    Release();
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) CScriptParser::Error( const char *fmt, ... )
{
    va_list argptr;
    char string[1024];

    va_start( argptr, fmt );
    N_vsnprintf( string, sizeof( string ) - 1, fmt, argptr );
    va_end( argptr );

    Con_Printf( COLOR_RED "ERROR: %s, line %lu: %s\n", szParsename, GetCurrentParseLine(), string );
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) CScriptParser::Warning( const char *fmt, ... )
{
    va_list argptr;
    char string[1024];

    va_start( argptr, fmt );
    N_vsnprintf( string, sizeof( string ) - 1, fmt, argptr );
    va_end( argptr );

    Con_Printf( COLOR_YELLOW "WARNING: %s, line %lu: %s\n", szParsename, GetCurrentParseLine(), string );
}

const char *CScriptParser::FindVar( const char *name )
{
    const char *tok;

    tok = NULL;
    while ( 1 ) {
        tok = Parse( true );
        if ( !tok[0] ) {
            Error( "unexpected end of parse file" );
            Clear();
            break;
        } else if ( tok[0] == '}' ) {
            break; // not in current scope
        } else if ( !N_stricmp( name, tok ) ) {
            break;
        }
    }

    return tok;
}

int CScriptParser::GetRefCount( void ) const {
    return nRefCount.load();
}

void CScriptParser::SetFlag( void ) {
    bGCFlag = qtrue;
}

bool CScriptParser::GetFlag( void ) const {
    return bGCFlag;
}

void CScriptParser::EnumReferences( asIScriptEngine *pEngine ) {
}

void CScriptParser::ReleaseAllHandles( asIScriptEngine *pEngine ) {
    Clear();
}

void CScriptParser::AddRef( void ) const {
    bGCFlag = false;
	nRefCount.fetch_add();
}

void CScriptParser::Release( void ) const {
    bGCFlag = false;
	if ( nRefCount.fetch_sub() == 0 ) {
		// When reaching 0 no more references to this instance
		// exists and the object should be destroyed
		this->~CScriptParser();
		Mem_Free( const_cast<CScriptParser *>( this ) );
	}
}

static void ScriptParser_Factory( asIScriptGeneric *pGeneric ) {
    *(CScriptParser **)pGeneric->GetAddressOfReturnLocation() = CScriptParser::Create();
}

static void ScriptParser_FactoryParse( asIScriptGeneric *pGeneric ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(CScriptParser **)pGeneric->GetAddressOfReturnLocation() = CScriptParser::Create( *fileName );
}

static void ScriptParser_AddRef( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->AddRef();
}

static void ScriptParser_Release( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->Release();
}

static void ScriptParser_GetRefCount( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnDWord( ( (CScriptParser *)pGeneric->GetObjectData() )->GetRefCount() );
}

static void ScriptParser_GetFlag( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnDWord( ( (CScriptParser *)pGeneric->GetObjectData() )->GetFlag() );
}

static void ScriptParser_SetFlag( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->SetFlag();
}

static void ScriptParser_ReleaseAllHandles( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->ReleaseAllHandles( pGeneric->GetEngine() );
}

static void ScriptParser_Load( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnDWord( ( (CScriptParser *)pGeneric->GetObjectData() )->Load( *(const string_t *)pGeneric->GetArgObject( 0 ) ) );
}

static void ScriptParser_IsLoaded( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnDWord( ( (CScriptParser *)pGeneric->GetObjectData() )->IsLoaded() );
}

static void ScriptParser_GetInt( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnDWord( ( (CScriptParser *)pGeneric->GetObjectData() )->GetInt( *(const string_t *)pGeneric->GetArgObject( 0 ) ) );
}

static void ScriptParser_GetUInt( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnDWord( ( (CScriptParser *)pGeneric->GetObjectData() )->GetUInt( *(const string_t *)pGeneric->GetArgObject( 0 ) ) );
}

static void ScriptParser_GetFloat( asIScriptGeneric *pGeneric ) {
    pGeneric->SetReturnFloat( ( (CScriptParser *)pGeneric->GetObjectData() )->GetFloat( *(const string_t *)pGeneric->GetArgObject( 0 ) ) );
}

static void ScriptParser_GetString( asIScriptGeneric *pGeneric ) {
    new ( pGeneric->GetAddressOfReturnLocation() )
        string_t( ( (CScriptParser *)pGeneric->GetObjectData() )->GetString( *(const string_t *)pGeneric->GetArgObject( 0 ) ) );
}

static void ScriptParser_GetObject( asIScriptGeneric *pGeneric ) {
    *(CScriptParser **)pGeneric->GetAddressOfReturnLocation() = ( ( (CScriptParser *)pGeneric->GetObjectData() )->GetObject(
        *(const string_t *)pGeneric->GetArgObject( 0 ) ) );
}

static void ScriptParser_Rewind( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->Rewind();
}

static void ScriptParser_Clear( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->Release();
}

static void ScriptParser_OpAssign( asIScriptGeneric *pGeneric ) {
    *( (CScriptParser *)pGeneric->GetObjectData() ) = *(CScriptParser *)pGeneric->GetArgObject( 0 );
}

static void ScriptParser_Parse( asIScriptGeneric *pGeneric ) {
    new ( pGeneric->GetAddressOfReturnLocation() )
        string_t( ( (CScriptParser *)pGeneric->GetObjectData() )->Parse( pGeneric->GetArgDWord( 0 ) ) );
}

static void ScriptParser_Parse1DMatrix( asIScriptGeneric *pGeneric ) {
    CScriptParser *obj = (CScriptParser *)pGeneric->GetObjectData();
    CScriptArray *matrix = (CScriptArray *)pGeneric->GetArgObject( 1 );
    pGeneric->SetReturnDWord( obj->Parse1DMatrix( pGeneric->GetArgDWord( 0 ), (float *)matrix->GetBuffer() ) );
}

static void ScriptParser_Parse2DMatrix( asIScriptGeneric *pGeneric ) {
    CScriptParser *obj = (CScriptParser *)pGeneric->GetObjectData();
    CScriptArray *matrix = (CScriptArray *)pGeneric->GetArgObject( 1 );
    pGeneric->SetReturnDWord( obj->Parse2DMatrix( pGeneric->GetArgDWord( 0 ), pGeneric->GetArgDWord( 1 ),
        (float *)matrix->GetBuffer() ) );
}

static void ScriptParser_Parse3DMatrix( asIScriptGeneric *pGeneric ) {
    CScriptParser *obj = (CScriptParser *)pGeneric->GetObjectData();
    CScriptArray *matrix = (CScriptArray *)pGeneric->GetArgObject( 1 );
    pGeneric->SetReturnDWord( obj->Parse3DMatrix( pGeneric->GetArgDWord( 0 ), pGeneric->GetArgDWord( 1 ),
        pGeneric->GetArgDWord( 2 ), (float *)matrix->GetBuffer() ) );
}

static void ScriptParser_Warning( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->Warning( *(const string_t *)pGeneric->GetArgObject( 0 ) );
}

static void ScriptParser_Error( asIScriptGeneric *pGeneric ) {
    ( (CScriptParser *)pGeneric->GetObjectData() )->Error( *(const string_t *)pGeneric->GetArgObject( 0 ) );
}

void RegisterScriptParser( asIScriptEngine *pEngine )
{
    CheckASCall( pEngine->RegisterObjectType( "parser", sizeof( CScriptParser ), asOBJ_REF ) );
    
    CheckASCall( pEngine->RegisterObjectBehaviour( "parser", asBEHAVE_FACTORY, "parser@ f()", asFUNCTION( ScriptParser_Factory ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectBehaviour( "parser", asBEHAVE_FACTORY, "parser@ f( const string& in )", asFUNCTION( ScriptParser_FactoryParse ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectBehaviour( "parser", asBEHAVE_RELEASE, "void f()", asFUNCTION( ScriptParser_Release ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectBehaviour( "parser", asBEHAVE_ADDREF, "void f()", asFUNCTION( ScriptParser_AddRef ), asCALL_GENERIC ) );

    CheckASCall( pEngine->RegisterObjectMethod( "parser", "parser& opAssign( const parser& in )", asFUNCTION( ScriptParser_OpAssign ), asCALL_GENERIC ) );
    
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "bool Load( const string& in )", asFUNCTION( ScriptParser_Load ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "bool IsLoaded( const string& in )", asFUNCTION( ScriptParser_IsLoaded ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "void Rewind()", asFUNCTION( ScriptParser_Rewind ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "void Clear()", asFUNCTION( ScriptParser_Clear ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "void Warning( const string& in )", asFUNCTION( ScriptParser_Warning ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "void Error( const string& in )", asFUNCTION( ScriptParser_Error ), asCALL_GENERIC ) );

    CheckASCall( pEngine->RegisterObjectMethod( "parser", "int GetInt( const string& in )", asFUNCTION( ScriptParser_GetInt ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "uint GetUInt( const string& in )", asFUNCTION( ScriptParser_GetUInt ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "float GetFloat( const string& in )", asFUNCTION( ScriptParser_GetFloat ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "string GetString( const string& in )", asFUNCTION( ScriptParser_GetString ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "parser@ GetObject( const string& in )", asFUNCTION( ScriptParser_GetObject ), asCALL_GENERIC ) );
    CheckASCall( pEngine->RegisterObjectMethod( "parser", "string Parse( bool allowLineBreaks = false )", asFUNCTION( ScriptParser_Parse ), asCALL_GENERIC ) );
}
