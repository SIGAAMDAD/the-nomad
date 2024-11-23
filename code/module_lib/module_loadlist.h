#ifndef __LOADLIST_H__
#define __LOADLIST_H__

#pragma once

#include "module_public.h"
#include "../game/g_archive.h"

#define MODULE_FLAG_ALL_DEPS_ACTIVE 0x0001
#define MODULE_FLAG_ALL_DEPS_LOADED 0x0002
#define MODULE_FLAG_VALID           0x0004
#define MODULE_FLAG_ACTIVE          0x0008
#define MODULE_FLAG_REQUIRED		0x0010
#define MODULE_FLAG_BADVERSION		0x0020

typedef struct module_s {
	char szName[MAX_NPATH];
	char szID[MAX_NPATH];
	uint32_t flags;
	uint32_t numDependencies;
	uint32_t *dependencies;
	version_t modVersion;
	version_t gameVersion;

	module_s& operator=( module_s& other );
	module_s& operator=( const module_s& other );
	bool operator==( const module_s& other ) const;
	bool operator!=( const module_s& other ) const;
	bool operator<( const module_s& other ) const;
	bool operator>( const module_s& other ) const;
} module_t;

inline module_s& module_s::operator=( module_s& other )
{
	N_strncpyz( szID, other.szID, sizeof( szID ) );
	N_strncpyz( szName, other.szName, sizeof( szName ) );
	flags = other.flags;
	numDependencies = other.numDependencies;
	dependencies = other.dependencies;
	return *this;
}
inline module_s& module_s::operator=( const module_s& other )
{
	N_strncpyz( szID, other.szID, sizeof( szID ) );
	N_strncpyz( szName, other.szName, sizeof( szName ) );
	flags = other.flags;
	numDependencies = other.numDependencies;
	dependencies = other.dependencies;
	return *this;
}

class CModuleLoadList
{
public:
	inline uint32_t GetModuleIndex( const module_t *mod ) const
	{ return mod - m_pModList; }
	inline bool IsLoaded( const char *name ) const
	{ return FindModule( name ) != NULL; }
	inline bool IsLoadedBefore( const module_t *a, const module_t *b ) const
	{ return ( a < b ); }
	inline bool IsLoadedAfter( const module_t *a, const module_t *b ) const
	{ return ( a > b ); }
	inline bool IsRequired( const module_t *mod ) const
	{ return mod->flags & MODULE_FLAG_REQUIRED; }
	inline bool IsDependedOn( const module_t *base, const module_t *dep ) const {
		for ( uint32_t i = 0; i < base->numDependencies; i++ ) {
		}
		return false;
	}
	
	void PrintList_f( void ) const;
	
	void Load( const CModuleInfo *pLoadList, uint64_t nModCount );
	void Resort( void );
	
	inline module_t *GetList( void )
	{ return m_pModList; }
	inline uint32_t NumMods( void ) const
	{ return m_nModCount; }
	
	inline const module_t *FindModule( const char *name ) const
	{ return FindModule( name ); }
	inline module_t *FindModule( const char *name )
	{
		uint64_t i;
		
		for ( i = 0; i < m_nModCount; i++ ) {
			if ( !N_stricmp( m_pModList[i].szID, name ) ) {
				return &m_pModList[i];
			}
		}
		return NULL;
	}
	
	static void Init( const CModuleInfo *pModList, uint64_t nModCount );
	static inline CModuleLoadList *Get( void )
	{ return g_pLoadList; }
private:
	module_t *m_pModList;
	uint32_t m_nModCount;
	
	static CModuleLoadList *g_pLoadList;
};

inline bool module_s::operator==( const module_s& other ) const
{ return N_strcmp( szID, other.szID ) == 0; }
inline bool module_s::operator!=( const module_s& other ) const
{ return N_strcmp( szID, other.szID ) != 0; }
inline bool module_s::operator<( const module_s& other ) const
{ return CModuleLoadList::Get()->IsLoadedAfter( this, eastl::addressof( other ) ); }
inline bool module_s::operator>( const module_s& other ) const
{ return CModuleLoadList::Get()->IsLoadedBefore( this, eastl::addressof( other ) ); }

#endif