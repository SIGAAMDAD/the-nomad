#ifndef __MODULE_STRINGFACTORY__
#define __MODULE_STRINGFACTORY__

#pragma once

#include "module_public.h"
#include "../engine/n_threads.h"

using string_constant_t = eastl::basic_string<char, CHunkAllocator<h_high>>;

namespace eastl {
	// for some reason, the eastl doesn't support eastl::hash<eastl::fixed_string>
	template<> struct hash<string_constant_t> {
		size_t operator()( const string_constant_t& str ) const {
			const unsigned char *p = (const unsigned char *)str.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};
};

class CModuleStringFactory : public asIStringFactory
{
public:
//	using CStringCache = eastl::unordered_map<string_t, int32_t, eastl::hash<string_t>, eastl::equal_to<string_t>, CModuleAllocator, true>;
	
	CModuleStringFactory( void ) {
	}
	~CModuleStringFactory() {
		m_StringCache.clear();
	}
	
	const void *GetStringConstant( const char *pData, asUINT nLength ) {
		PROFILE_FUNCTION();

		CThreadAutoLock<CThreadMutex> lock( m_hLock );
		
		const string_constant_t str( pData, nLength );
		auto it = m_StringCache.find( str );
		if ( it != m_StringCache.end() ) {
			it->second++;
		} else {
			it = m_StringCache.insert( eastl::unordered_map<string_constant_t, int32_t>::value_type( str, 1 ) ).first;
		}
		
		return (const void *)&it->first;
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
		const string_constant_t& data = *(const string_constant_t *)pStr;
		auto it = m_StringCache.find( data );
		if ( it == m_StringCache.end() ) {
			Con_Printf( COLOR_RED "[ERROR] ReleaseStringConstant: invalid string '%s'\n", data.c_str() );
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
		PROFILE_FUNCTION();

		if ( !pStr ) {
			Assert( pStr );
			return asERROR;
		}
		
		CThreadAutoLock<CThreadMutex> lock( *const_cast<CThreadMutex *>( &m_hLock ) );
		const string_constant_t *data = (const string_constant_t *)pStr;
		if ( nLength ) {
			*nLength = data->size();
		}
		if ( pData ) {
			memcpy( pData, data->data(), data->size() );
		}
		return asSUCCESS;
	}
	
	CThreadMutex m_hLock;
	eastl::unordered_map<string_constant_t, int32_t> m_StringCache;
};

extern CModuleStringFactory *g_pStringFactory;

#endif