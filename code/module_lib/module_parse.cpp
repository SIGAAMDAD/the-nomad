#include "module_public.h"

CModuleParse::CModuleParse( const char *filename )
    : m_pTextBuffer( NULL )
{
    union {
        void *v;
        char *c;
    } f;
    
    moduleImport.FS_LoadFile( filename, &f.v );
    if ( !f.v ) {
        moduleImport.Printf( PRINT_WARNING, "CModuleParse::CModuleParse: failed to load info file '%s'\n", filename );
        return;
    }

    m_pTextBuffer = f.c;
}

CModuleParse::CModuleParse( const CModuleParse& other ) {
    m_bFailed = other.m_bFailed;
    m_Infos = eastl::move( other.m_Infos );
    m_ValueLists = eastl::move( other.m_ValueLists );
    m_ValueMaps = eastl::move( other.m_ValueMaps );
    m_Values = eastl::move( other.m_Values );
}

CModuleParse::CModuleParse( CModuleParse&& other ) {
    m_bFailed = other.m_bFailed;
    m_Infos = other.m_Infos;
    m_ValueLists = other.m_ValueLists;
    m_ValueMaps = other.m_ValueMaps;
    m_Values = other.m_Values;
}

CModuleParse::~CModuleParse() {
    moduleImport.FS_FreeFile( m_pTextBuffer );

    for ( auto it = m_Infos.begin(); it != m_Infos.end(); it++ ) {
        delete *it;
    }
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL CModuleParse::Error( const char *fmt, ... )
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof(msg) - 1, fmt, argptr );
    va_end( argptr );

    m_bFailed = qtrue;
    COM_ParseError( "%s", msg );
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL CModuleParse::Warning( const char *fmt, ... )
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof(msg) - 1, fmt, argptr );
    va_end( argptr );

    COM_ParseWarning( "%s", msg );
}

void CModuleParse::ParseHashTable( const char **text, const char *pName, HashTable& hashTable )
{
    const char *key, *value;
    const char *rewind;

    if ( m_bFailed ) {
        return;
    }

    while ( 1 ) {
        key = COM_ParseExt( text, qfalse );

        if ( !key[0] ) {
            Error( "expected '}' at end of info array hashtable");
            return;
        }

        // end of list
        if ( key[0] == ']' ) {
            break;
        }
        // nested info
        else if ( key[0] == '{' ) {
            if ( !rewind ) {
                Error( "unnamed nested info in info definition" );
                return;
            }

            CModuleParse *parse = new CModuleParse();
            ParseInfo( text, rewind, parse );

            parse->m_Name = rewind;
            m_Infos.emplace_back( parse );
        }
        // array list
        else if ( key[0] == '[' ) {
            if ( !rewind ) {
                Error( "unnamed array list in info definition" );
                return;
            }

            ValueList valueList;
            ParseList( text, rewind, valueList );

            m_ValueLists.try_emplace( rewind, valueList );
        }
        // array hashtable
        else if ( key[0] == '(' ) {
            if ( !rewind ) {
                Error( "unnamed array hashtable in info definition" );
                return;
            }

            HashTable hashTable;
            ParseHashTable( text, rewind, hashTable );

            m_ValueMaps.try_emplace( rewind, hashTable );
        }
        else {
            value = COM_ParseExt( text, qfalse );
            if ( !value[0] ) {
                Error( "missing value for key '%s' in array hashtable", key );
                return;
            }
            hashTable.try_emplace( key, value );
        }

        rewind = key;
    }
}

void CModuleParse::ParseList( const char **text, const char *pName, ValueList& valueList )
{
    const char *tok;
    const char *rewind;
    
    if ( m_bFailed ) {
        return;
    }

    while ( 1 ) {
        tok = COM_ParseExt( text, qfalse );

        if ( !tok[0] ) {
            Error( "expected '}' at end of info array list");
            return;
        }

        // end of list
        if ( tok[0] == ']' ) {
            break;
        }
        // nested info
        else if ( tok[0] == '{' ) {
            if ( !rewind ) {
                Error( "unnamed nested info in info definition" );
                return;
            }

            CModuleParse *parse = new CModuleParse();
            ParseInfo( text, rewind, parse );
            
            parse->m_Name = rewind;
            m_Infos.emplace_back( parse );
        }
        // array list
        else if ( tok[0] == '[' ) {
            if ( !rewind ) {
                Error( "unnamed array list in info definition" );
                return;
            }

            ValueList valueList;
            ParseList( text, rewind, valueList );

            m_ValueLists.try_emplace( rewind, valueList );
        }
        // array hashtable
        else if ( tok[0] == '(' ) {
            if ( !rewind ) {
                Error( "unnamed array hashtable in info definition" );
                return;
            }

            HashTable hashTable;
            ParseHashTable( text, rewind, hashTable );

            m_ValueMaps.try_emplace( rewind, hashTable );
        }
        else {
            valueList.emplace_back( tok );
        }

        rewind = tok;
    }
}

void CModuleParse::ParseInfo( const char **text, const char *pName, CModuleParse *parse )
{
    const char *key, *value;
    const char *rewind;

    if ( m_bFailed ) {
        return;
    }

    while ( 1 ) {
        key = COM_ParseExt( text, qtrue );

        if ( !key[0] ) {
            Error( "expected '}' at end of info definition" );
            return;
        }
        
        // end of definition
        if ( key[0] == '}' ) {
            break;
        }
        // nested info
        else if ( key[0] == '{' ) {
            if ( !rewind ) {
                Error( "unnamed nested info in info definition" );
                return;
            }

            CModuleParse *parse = new CModuleParse();
            ParseInfo( text, rewind, parse );

            parse->m_Name = rewind;
            m_Infos.emplace_back( parse );
        }
        // array list
        else if ( key[0] == '[' ) {
            if ( !rewind ) {
                Error( "unnamed array list in info definition" );
                return;
            }

            ValueList valueList;
            ParseList( text, rewind, valueList );

            m_ValueLists.try_emplace( rewind, valueList );
        }
        // array hashtable
        else if ( key[0] == '(' ) {
            if ( !rewind ) {
                Error( "unnamed array hashtable in info definition" );
                return;
            }

            HashTable hashTable;
            ParseHashTable( text, rewind, hashTable );

            m_ValueMaps.try_emplace( rewind, hashTable );
        }
        else {
            value = COM_ParseExt( text, qfalse );
            if ( !value[0] ) {
                Error( "missing value for key '%s' in info definition", key );
                return;
            }
            m_Values.try_emplace( key, value );
        }
    }
}

void CModuleParse::Parse( void )
{
    const char **text;
    const char *text_p;
    const char *key, *value;
    const char *rewind;

    text_p = m_pTextBuffer;
    text = (const char **)&text_p;

    key = COM_ParseComplex( text, qtrue );
    if ( key[0] != '{' ) {
        Error( "Invalid info file, missing '{' at beginning of info file" );
        return;
    }

    while ( 1 ) {
        if ( m_bFailed ) {
            // failed somewhere, get out
            moduleImport.Printf( PRINT_INFO, "failed to parse info file.\n" );
            break;
        }

        key = COM_ParseExt( text, qtrue );
        if ( !key[0] ) {
            Error( "unexpected end of info file" );
            return;
        }
        if ( key[0] != '}' ) {
            Error( "expected '}' at end of info definition" );
            return;
        }
        
        // end of defintion
        if ( key[0] == '}' ) {
            break;
        }
        // nested info
        else if ( key[0] == '{' ) {
            if ( !rewind ) {
                Error( "unnamed nested info in info definition" );
                return;
            }

            CModuleParse *parse = new CModuleParse();
            ParseInfo( text, rewind, parse );

            parse->m_Name = rewind;
            m_Infos.emplace_back( parse );
        }
        // array list
        else if ( key[0] == '[' ) {
            if ( !rewind ) {
                Error( "unnamed array list in info definition" );
                return;
            }

            ValueList valueList;
            ParseList( text, rewind, valueList );

            m_ValueLists.try_emplace( rewind, valueList );
        }
        // array hashtable
        else if ( key[0] == '(' ) {
            if ( !rewind ) {
                Error( "unnamed array hashtable in info definition" );
                return;
            }

            HashTable hashTable;
            ParseHashTable( text, rewind, hashTable );

            m_ValueMaps.try_emplace( rewind, hashTable );
        }
        else {
            value = COM_ParseExt( text, qfalse );
            m_Values.try_emplace( key, value );
        }

        rewind = key;
    }
}

bool CModuleParse::Failed( void ) const {
    return m_bFailed;
}

CModuleParse *CModuleParse::GetInfo( const char *pName )
{
    for ( auto& it : m_Infos ) {
        if ( !N_stricmp( it->m_Name.c_str(), pName ) ) {
            return it;
        }
    }

    moduleImport.Printf( PRINT_WARNING, "CModuleParse::GetInfo: failed to find info '%s'", pName );
    return NULL;
}

UtlString& CModuleParse::GetValue( const char *pName )
{
    for ( auto it = m_Values.begin(); it != m_Values.end(); it++ ) {
        if ( !N_stricmp( it->first.c_str(), pName ) ) {
            return it->second;
        }
    }
    moduleImport.Printf( PRINT_WARNING, "CModuleParse::GetValue: no value for info key '%s'\n", pName );
    return m_EmptyString;
}

UtlMap<eastl::string, UtlString>& CModuleParse::GetHashTable( const char *pName )
{
    for ( auto it = m_ValueMaps.begin(); it != m_ValueMaps.end(); it++ ) {
        if ( !N_stricmp( it->first.c_str(), pName ) ) {
            return it->second;
        }
    }
    moduleImport.Printf( PRINT_WARNING, "CModuleParse::GetHashTable: no hashtable for info key '%s'\n", pName );
    return m_EmptyMap;
}

UtlVector<UtlString>& CModuleParse::GetArray( const char *pName )
{
    for ( auto it = m_ValueLists.begin(); it != m_ValueLists.end(); it++ ) {
        if ( !N_stricmp( it->first.c_str(), pName ) ) {
            return it->second;
        }
    }
    moduleImport.Printf( PRINT_WARNING, "CModuleParse::GetHashTable: no list for info key '%s'\n", pName );
    return m_EmptyList;
}
