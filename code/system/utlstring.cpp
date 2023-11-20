//====== Copyright 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "utlstring.h"
#include <ctype.h>


//-----------------------------------------------------------------------------
// Base class, containing simple memory management
//-----------------------------------------------------------------------------
CUtlBinaryBlock::CUtlBinaryBlock( uint64_t growSize, uint64_t initSize ) 
{
	MEM_ALLOC_CREDIT();
	m_Memory.Init( growSize, initSize );

	m_nActualLength = 0;
}

CUtlBinaryBlock::CUtlBinaryBlock( void* pMemory, uint64_t nSizeInBytes, uint64_t nInitialLength ) : m_Memory( (unsigned char*)pMemory, nSizeInBytes )
{
	m_nActualLength = nInitialLength;
}

CUtlBinaryBlock::CUtlBinaryBlock( const void* pMemory, uint64_t nSizeInBytes ) : m_Memory( (const unsigned char*)pMemory, nSizeInBytes )
{
	m_nActualLength = nSizeInBytes;
}

CUtlBinaryBlock::CUtlBinaryBlock( const CUtlBinaryBlock& src )
{
	Set( src.Get(), src.Length() );
}

void CUtlBinaryBlock::Get( void *pValue, uint64_t nLen ) const
{
	Assert( nLen > 0 );
	if ( m_nActualLength < nLen )
	{
		nLen = m_nActualLength;
	}

	if ( nLen > 0 )
	{
		memcpy( pValue, m_Memory.Base(), nLen );
	}
}

void CUtlBinaryBlock::SetLength( uint64_t nLength )
{
	MEM_ALLOC_CREDIT();
	Assert( !m_Memory.IsReadOnly() );

	m_nActualLength = nLength;
	if ( nLength > m_Memory.NumAllocated() )
	{
		uint64_t nOverFlow = nLength - m_Memory.NumAllocated();
		m_Memory.Grow( nOverFlow );

		// If the reallocation failed, clamp length
		if ( nLength > m_Memory.NumAllocated() )
		{
			m_nActualLength = m_Memory.NumAllocated();
		}
	}

#ifdef _NOMAD_DEBUG
	if ( m_Memory.NumAllocated() > m_nActualLength )
	{
		memset( ( ( char * )m_Memory.Base() ) + m_nActualLength, 0xEB, m_Memory.NumAllocated() - m_nActualLength );
	}
#endif
}


void CUtlBinaryBlock::Set( const void *pValue, uint64_t nLen )
{
	Assert( !m_Memory.IsReadOnly() );

	if ( !pValue )
	{
		nLen = 0;
	}

	SetLength( nLen );

	if ( m_nActualLength )
	{
		if ( ( ( const char * )m_Memory.Base() ) >= ( ( const char * )pValue ) + nLen ||
			 ( ( const char * )m_Memory.Base() ) + m_nActualLength <= ( ( const char * )pValue ) )
		{
			memcpy( m_Memory.Base(), pValue, m_nActualLength );
		}
		else
		{
			memmove( m_Memory.Base(), pValue, m_nActualLength );
		}
	}
}


CUtlBinaryBlock &CUtlBinaryBlock::operator=( const CUtlBinaryBlock &src )
{
	Assert( !m_Memory.IsReadOnly() );
	Set( src.Get(), src.Length() );
	return *this;
}


bool CUtlBinaryBlock::operator==( const CUtlBinaryBlock &src ) const
{
	if ( src.Length() != Length() )
		return false;

	return !memcmp( src.Get(), Get(), Length() );
}


//-----------------------------------------------------------------------------
// Simple string class. 
//-----------------------------------------------------------------------------
CUtlString::CUtlString()
{
}

CUtlString::CUtlString( const char *pString )
{
	Set( pString );
}

CUtlString::CUtlString( const CUtlString& string )
{
	Set( string.Get() );
}

// Attaches the string to external memory. Useful for avoiding a copy
CUtlString::CUtlString( void* pMemory, uint64_t nSizeInBytes, uint64_t nInitialLength ) : m_Storage( pMemory, nSizeInBytes, nInitialLength )
{
}

CUtlString::CUtlString( const void* pMemory, uint64_t nSizeInBytes ) : m_Storage( pMemory, nSizeInBytes )
{
}


//-----------------------------------------------------------------------------
// Purpose: Set directly and don't look for a null terminator in pValue.
//-----------------------------------------------------------------------------
void CUtlString::SetDirect( const char *pValue, uint64_t nChars )
{
	if ( nChars > 0 )
	{
		m_Storage.SetLength( nChars+1 );
		m_Storage.Set( pValue, nChars );
		m_Storage[nChars] = 0;
	}
	else
	{
		m_Storage.SetLength( 0 );
	}
}


void CUtlString::Set( const char *pValue )
{
	Assert( !m_Storage.IsReadOnly() );
	uint64_t nLen = pValue ? V_strlen(pValue) + 1 : 0;
	m_Storage.Set( pValue, nLen );
}


// Returns strlen
uint64_t CUtlString::Length() const
{
	return m_Storage.Length() ? m_Storage.Length() - 1 : 0;
}

// Sets the length (used to serialize into the buffer )
void CUtlString::SetLength( uint64_t nLen )
{
	Assert( !m_Storage.IsReadOnly() );

	// Add 1 to account for the NULL
	m_Storage.SetLength( nLen > 0 ? nLen + 1 : 0 );
}

const char *CUtlString::Get( ) const
{
	if ( m_Storage.Length() == 0 )
	{
		return "";
	}

	return reinterpret_cast< const char* >( m_Storage.Get() );
}

// Converts to c-strings
CUtlString::operator const char*() const
{
	return Get();
}

char *CUtlString::Get()
{
	Assert( !m_Storage.IsReadOnly() );

	if ( m_Storage.Length() == 0 )
	{
		// In general, we optimise away small mallocs for empty strings
		// but if you ask for the non-const bytes, they must be writable
		// so we can't return "" here, like we do for the const version - jd
		m_Storage.SetLength( 1 );
		m_Storage[ 0 ] = '\0';
	}

	return reinterpret_cast< char* >( m_Storage.Get() );
}

void CUtlString::Purge()
{
	m_Storage.Purge();
}


void CUtlString::ToLower()
{
	for( uint64_t nLength = Length() - 1; nLength >= 0; nLength-- )
	{
		m_Storage[ nLength ] = tolower( m_Storage[ nLength ] );
	}
}


CUtlString &CUtlString::operator=( const CUtlString &src )
{
	Assert( !m_Storage.IsReadOnly() );
	m_Storage = src.m_Storage;
	return *this;
}

CUtlString &CUtlString::operator=( const char *src )
{
	Assert( !m_Storage.IsReadOnly() );
	Set( src );
	return *this;
}

bool CUtlString::operator==( const CUtlString &src ) const
{
	return m_Storage == src.m_Storage;
}

bool CUtlString::operator==( const char *src ) const
{
	return ( strcmp( Get(), src ) == 0 );
}

CUtlString &CUtlString::operator+=( const CUtlString &rhs )
{
	Assert( !m_Storage.IsReadOnly() );

	const uint64_t lhsLength( Length() );
	const uint64_t rhsLength( rhs.Length() );
	const uint64_t requestedLength( lhsLength + rhsLength );

	SetLength( requestedLength );
	const uint64_t allocatedLength( Length() );
	const uint64_t copyLength( allocatedLength - lhsLength < rhsLength ? allocatedLength - lhsLength : rhsLength );
	memcpy( Get() + lhsLength, rhs.Get(), copyLength );
	m_Storage[ allocatedLength ] = '\0';

	return *this;
}

CUtlString &CUtlString::operator+=( const char *rhs )
{
	Assert( !m_Storage.IsReadOnly() );

	const uint64_t lhsLength( Length() );
	const uint64_t rhsLength( V_strlen( rhs ) );
	const uint64_t requestedLength( lhsLength + rhsLength );

	SetLength( requestedLength );
	const uint64_t allocatedLength( Length() );
	const uint64_t copyLength( allocatedLength - lhsLength < rhsLength ? allocatedLength - lhsLength : rhsLength );
	memcpy( Get() + lhsLength, rhs, copyLength );
	m_Storage[ allocatedLength ] = '\0';

	return *this;
}

CUtlString &CUtlString::operator+=( char c )
{
	Assert( !m_Storage.IsReadOnly() );

	uint64_t nLength = Length();
	SetLength( nLength + 1 );
	m_Storage[ nLength ] = c;
	m_Storage[ nLength+1 ] = '\0';
	return *this;
}

CUtlString &CUtlString::operator+=( uint64_t rhs )
{
	Assert( !m_Storage.IsReadOnly() );
	Assert( sizeof( rhs ) == 4 );

	char tmpBuf[ 12 ];	// Sufficient for a signed 32 bit integer [ -2147483648 to +2147483647 ]
	V_snprintf( tmpBuf, sizeof( tmpBuf ), "%d", rhs );
	tmpBuf[ sizeof( tmpBuf ) - 1 ] = '\0';

	return operator+=( tmpBuf );
}

CUtlString &CUtlString::operator+=( double rhs )
{
	Assert( !m_Storage.IsReadOnly() );

	char tmpBuf[ 256 ];	// How big can doubles be???  Dunno.
	V_snprintf( tmpBuf, sizeof( tmpBuf ), "%lg", rhs );
	tmpBuf[ sizeof( tmpBuf ) - 1 ] = '\0';

	return operator+=( tmpBuf );
}

bool CUtlString::MatchesPattern( const CUtlString &Pattern, uint64_t nFlags ) const
{
	const char *pszSource = String();
	const char *pszPattern = Pattern.String();
	bool	bExact = true;

	while( 1 )
	{
		if ( ( *pszPattern ) == 0 )
		{
			return ( (*pszSource ) == 0 );
		}

		if ( ( *pszPattern ) == '*' )
		{
			pszPattern++;

			if ( ( *pszPattern ) == 0 )
			{
				return true;
			}

			bExact = false;
			continue;
		}

		uint64_t nLength = 0;

		while( ( *pszPattern ) != '*' && ( *pszPattern ) != 0 )
		{
			nLength++;
			pszPattern++;
		}

		while( 1 )
		{
			const char *pszStartPattern = pszPattern - nLength;
			const char *pszSearch = pszSource;

			for( uint64_t i = 0; i < nLength; i++, pszSearch++, pszStartPattern++ )
			{
				if ( ( *pszSearch ) == 0 )
				{
					return false;
				}

				if ( ( *pszSearch ) != ( *pszStartPattern ) )
				{
					break;
				}
			}

			if ( pszSearch - pszSource == nLength )
			{
				break;
			}

			if ( bExact == true )
			{
				return false;
			}

			if ( ( nFlags & PATTERN_DIRECTORY ) != 0 )
			{
				if ( ( *pszPattern ) != '/' && ( *pszSource ) == '/' )
				{
					return false;
				}
			}

			pszSource++;
		}

		pszSource += nLength;
	}
}


uint64_t CUtlString::Format( const char *pFormat, ... )
{
	Assert( !m_Storage.IsReadOnly() );

	char tmpBuf[ 4096 ];	//< Nice big 4k buffer, as much memory as my first computer had, a Radio Shack Color Computer

	va_list marker;

	va_start( marker, pFormat );
#ifdef _WIN32
	uint64_t len = _vsnprintf( tmpBuf, sizeof( tmpBuf ) - 1, pFormat, marker );
#elif POSIX
	uint64_t len = vsnprintf( tmpBuf, sizeof( tmpBuf ) - 1, pFormat, marker );
#else
#error "define vsnprintf type."
#endif
	va_end( marker );

	// Len > maxLen represents an overflow on POSIX, < 0 is an overflow on windows
	if( len < 0 || len >= sizeof( tmpBuf ) - 1 )
	{
		len = sizeof( tmpBuf ) - 1;
		tmpBuf[sizeof( tmpBuf ) - 1] = 0;
	}

	Set( tmpBuf );

	return len;
}

//-----------------------------------------------------------------------------
// Strips the trailing slash
//-----------------------------------------------------------------------------
void CUtlString::StripTrailingSlash()
{
	if ( IsEmpty() )
		return;

	uint64_t nLastChar = Length() - 1;
	char c = m_Storage[ nLastChar ];
	if ( c == '\\' || c == '/' )
	{
		m_Storage[ nLastChar ] = 0;
		m_Storage.SetLength( m_Storage.Length() - 1 );
	}
}

CUtlString CUtlString::Slice( uint64_t nStart, uint64_t nEnd ) const
{
	if ( nStart < 0 )
		nStart = Length() - (-nStart % Length());
	else if ( nStart >= Length() )
		nStart = Length();

	if ( nEnd == INT32_MAX )
		nEnd = Length();
	else if ( nEnd < 0 )
		nEnd = Length() - (-nEnd % Length());
	else if ( nEnd >= Length() )
		nEnd = Length();
	
	if ( nStart >= nEnd )
		return CUtlString( "" );

	const char *pIn = String();

	CUtlString ret;
	ret.m_Storage.SetLength( nEnd - nStart + 1 );
	char *pOut = (char*)ret.m_Storage.Get();

	memcpy( ret.m_Storage.Get(), &pIn[nStart], nEnd - nStart );
	pOut[nEnd - nStart] = 0;

	return ret;
}

// Grab a substring starting from the left or the right side.
CUtlString CUtlString::Left( uint64_t nChars ) const
{
	return Slice( 0, nChars );
}

CUtlString CUtlString::Right( uint64_t nChars ) const
{
	return Slice( -nChars );
}

CUtlString CUtlString::Replace( char cFrom, char cTo ) const
{
	CUtlString ret = *this;
	uint64_t len = ret.Length();
	for ( uint64_t i=0; i < len; i++ )
	{
		if ( ret.m_Storage[i] == cFrom )
			ret.m_Storage[i] = cTo;
	}

	return ret;
}

CUtlString CUtlString::AbsPath( const char *pStartingDir ) const
{
	char szNew[MAX_PATH];
	V_MakeAbsolutePath( szNew, sizeof( szNew ), this->String(), pStartingDir );
	return CUtlString( szNew );
}

CUtlString CUtlString::UnqualifiedFilename() const
{
	const char *pFilename = V_UnqualifiedFileName( this->String() );
	return CUtlString( pFilename );
}

CUtlString CUtlString::DirName() const
{
	CUtlString ret( this->String() );
	V_StripLastDir( (char*)ret.m_Storage.Get(), ret.m_Storage.Length() );
	V_StripTrailingSlash( (char*)ret.m_Storage.Get() );
	return ret;
}

CUtlString CUtlString::PathJoin( const char *pStr1, const char *pStr2 )
{
	char szPath[MAX_PATH];
	V_ComposeFileName( pStr1, pStr2, szPath, sizeof( szPath ) );
	return CUtlString( szPath );
}


CUtlString CUtlString::operator+( const char *pOther ) const
{
	CUtlString s = *this;
	s += pOther;
	return s;
}


//-----------------------------------------------------------------------------
// Purpose: concatenate the provided string to our current content
//-----------------------------------------------------------------------------
void CUtlString::Append( const char *pchAddition )
{
	CUtlString s = *this;
	s += pchAddition;
}
