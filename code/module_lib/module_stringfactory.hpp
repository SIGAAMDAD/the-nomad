#ifndef __MODULE_STRINGFACTORY__
#define __MODULE_STRINGFACTORY__

#pragma once

#include "module_public.h"
#include "../engine/n_threads.h"

class CModuleStringFactory;

static CModuleStringFactory *s_pStringFactory;

class CModuleStringFactory : public asIStringFactory
{
public:
	using CStringCache = eastl::unordered_map<string_t, int32_t, eastl::hash<string_t>, eastl::equal_to<string_t>, CModuleAllocator, true>;
	
	CModuleStringFactory( void ) {
	}
	~CModuleStringFactory() {
		Assert( m_StringCache.empty() );
	}
	
	const void *GetStringConstant( const char *pData, asUINT nLength ) {
		CThreadAutoLock<CThreadMutex> lock( m_hLock );
		
		const eastl::string str( pData, nLength );
		auto it = m_StringCache.find( str );
		if ( it != m_StringCache.end() ) {
			it->second++;
		} else {
			it = m_StringCache.insert( CStringCache::value_type( str, 1 ) ).first;
		}
		
		return (const void *)&it->first;
	}
	int ReleaseStringConstant( const void *pStr ) {
		int ret;
		
		if ( !pStr ) {
			Assert( pStr );
			return asERROR;
		}
		
		ret = asSUCCESS;
		
		CThreadAutoLock<CThreadMutex> lock( m_hLock );
		auto it = m_StringCache.find( (const char *)pStr );
		if ( it == m_StringCache.end() ) {
			Con_Printf( COLOR_RED "[ERROR] ReleaseStringConstant: invalid string '%s'\n", (const char *)pStr );
			ret = asERROR;
		} else {
			it->second--;
			if ( !it->second ) {
				m_StringCache.erase( it );
			}
		}
		
		return ret;
	}
	int GetRawStringData( const void *pStr, char *pData, asUINT *nLength ) const {
		if ( !pStr ) {
			Assert( pStr );
			return asERROR;
		}
		
		CThreadAutoLock<CThreadMutex> lock( *const_cast<CThreadMutex *>( &m_hLock ) );
		const string_t *data = (const string_t *)pStr;
		if ( nLength ) {
			*nLength = data->size();
		}
		if ( pData ) {
			memcpy( pData, data->data(), data->size() );
		}
		return asSUCCESS;
	}
	
	CThreadMutex m_hLock;
	CStringCache m_StringCache;
};

CModuleStringFactory *GetStringFactorySingleton( void )
{
	if ( !s_pStringFactory ) {
		CThreadMutex mutex;
		
		CThreadAutoLock<CThreadMutex> lock( mutex );
		if ( !s_pStringFactory ) {
			s_pStringFactory = new CModuleStringFactory();
		}
	}
	return s_pStringFactory;
}

struct CStringCacheCleaner
{
	~CStringCacheCleaner() {
		if ( s_pStringFactory ) {
			if ( s_pStringFactory->m_StringCache.empty() ) {
				delete s_pStringFactory;
				s_pStringFactory = NULL;
			}
		}
	}
};

static CStringCacheCleaner s_StringCacheCleaner;

#endif