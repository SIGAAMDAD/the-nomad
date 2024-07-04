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
        FS_FreeFile( pBuffer );
        pText = NULL;
        nLength = 0;
        pBuffer = NULL;
    }
}

CScriptParser& CScriptParser::operator=( const CScriptParser& other )
{
    pBuffer = other.pBuffer;
    pText = other.pText;
    nLength = other.nLength;

    return *this;
}

bool CScriptParser::Load( const string_t& fileName )
{
    nLength = FS_LoadFile( fileName.c_str(), (void **)&pBuffer );
    if ( !nLength || !pBuffer ) {
        Con_Printf( COLOR_YELLOW "Error loading parse file '%s'\n", fileName.c_str() );
        return false;
    }

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
    tok = COM_ParseExt( pText, qfalse );
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
    tok = COM_ParseExt( pText, qfalse );
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
    tok = COM_ParseExt( pText, qfalse );
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
    tok = COM_ParseExt( pText, qfalse );
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
    tok = COM_ParseExt( pText, qfalse );
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

const char *CScriptParser::FindVar( const char *name )
{
    const char *tok;

    tok = NULL;
    while ( 1 ) {
        tok = COM_ParseExt( pText, qtrue );
        if ( !tok[0] ) {
            COM_ParseError( "unexpected end of parse file" );
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
