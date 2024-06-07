#ifndef __MODULE_STRINGFACTORY__
#define __MODULE_STRINGFACTORY__

#pragma once

#include "module_public.h"
#include "../engine/n_threads.h"
#include <EASTL/unordered_set.h>

#define USE_STRINGCACHE_MAP

#ifndef USE_STRINGCACHE_MAP
struct string_data_t {
	string_data_t( const char *pData, asUINT nLength )
		: data( pData, nLength ), refCount( 0 )
	{
	}
	string_data_t( const string_data_t& str, int32_t nRefCount )
		: data( EASTL_MOVE( str.data ) ), refCount( nRefCount )
	{ }
	string_data_t( const string_t& str )
		: data( str )
	{ }

	inline const char *c_str( void ) const {
		return data.c_str();
	}

	operator string_t&( void ) {
		return data;
	}
	operator const string_t&( void ) const {
		return data;
	}

	inline bool operator==( const string_t& str ) const {
		return data == str;
	}
	inline bool operator!=( const string_t& str ) const {
		return data != str;
	}

	string_t data;
	mutable int32_t refCount;
};
#endif

#ifndef USE_STRINGCACHE_MAP
namespace std {
	template<> struct hash<string_t> {
		size_t operator()( const string_t& str ) const {
			const unsigned char *p = (const unsigned char *)str.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};

	template<> struct hash<string_data_t> {
		size_t operator()( const string_data_t& str ) const {
			const unsigned char *p = (const unsigned char *)str.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};
};
#endif

#ifdef USE_STRINGCACHE_MAP
using CStringCache = eastl::unordered_map<string_t, int32_t, eastl::hash<string_t>,
	eastl::equal_to<string_t>, eastl::allocator, true>;
using string_hash_t = string_t;
#else
using string_allocator_t = memory::memory_pool<memory::node_pool, memory::virtual_memory_allocator>;
using CStringCache = memory::unordered_set<string_data_t, string_allocator_t>;
using string_hash_t = string_data_t;
#endif

class CModuleStringFactory : public asIStringFactory
{
public:
	#ifndef USE_STRINGCACHE_MAP
	CModuleStringFactory( void )
		: m_DataCache( memory::set_node_size<string_data_t>::value, 64_MB ), m_StringCache( m_DataCache )
	{
	}
	#else
	CModuleStringFactory( void )
	{ }
	#endif
	~CModuleStringFactory() {
		m_StringCache.clear();
	}
	
	const void *GetStringConstant( const char *pData, asUINT nLength ) {
		PROFILE_FUNCTION();

		CThreadAutoLock<CThreadMutex> lock( m_hLock );

		const string_hash_t str( pData, nLength );
		auto it = m_StringCache.find( str );
		if ( it != m_StringCache.end() ) {
		#ifdef USE_STRINGCACHE_MAP
			it->second++;
		#else
			it->refCount++;
		#endif
		} else {
			it = m_StringCache.insert( CStringCache::value_type( str, 1 ) ).first;
		}
	#ifdef USE_STRINGCACHE_MAP
		return (const void *)&it->first;
	#else
		return (const void *)&( *it );
	#endif
	}
	int ReleaseStringConstant( const void *pStr ) {
		PROFILE_FUNCTION();

		int ret;
		
		if ( !pStr ) {
			Assert( pStr );
			return asERROR;
		}
		
		ret = asSUCCESS;
		
		CThreadAutoLock<CThreadMutex> lock( m_hLock );

		const string_hash_t& data = *(const string_t *)pStr;

		auto it = m_StringCache.find( data );
		if ( it == m_StringCache.end() ) {
			Con_Printf( COLOR_RED "[ERROR] ReleaseStringConstant: invalid string '%s'\n", data.c_str() );
			ret = asERROR;
		} else {
		#ifdef USE_STRINGCACHE_MAP
			it->second--;
			if ( !it->second ) {
				m_StringCache.erase( it );
			}
		#else
			it->refCount--;
			if ( !it->refCount ) {
				m_StringCache.erase( it );
			}
		#endif
		}
		
		return ret;
	}
	int GetRawStringData( const void *pStr, char *pData, asUINT *nLength ) const {
		PROFILE_FUNCTION();

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
#ifndef USE_STRINGCACHE_MAP
	string_allocator_t m_DataCache;
#endif
	CStringCache m_StringCache;
};

extern CModuleStringFactory *g_pStringFactory;

#endif