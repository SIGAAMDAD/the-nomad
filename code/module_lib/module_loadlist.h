#ifndef __LOADLIST_H__
#define __LOADLIST_H__

#pragma once

#include "module_public.h"

#define MODULE_FLAG_ALL_DEPS_ACTIVE 0x0001
#define MODULE_FLAG_ALL_DEPS_LOADED 0x0002
#define MODULE_FLAG_VALID           0x0004
#define MODULE_FLAG_ACTIVE          0x0008

typedef struct {
	char szName[MAX_NPATH];
	char szID[MAX_NPATH];
	uint32_t flags;
	uint32_t numDependencies;
	uint64_t *dependencies;
} module_t;

class CModuleLoadList
{
public:
	inline uint32_t GetModuleIndex( const module_t *mod ) const {
		return mod - m_pModList;
	}
    inline bool IsLoaded( const char *name ) const
    { return FindModule( name ) != NULL; }
    inline bool IsLoadedBefore( const module_t *a, const module_t *b ) const
    { return ( a < b ); }
    inline bool IsLoadedAfter( const module_t *a, const module_t *b ) const
    { return ( a > b ); }
    inline bool IsDependedOn( const module_t *base, const module_t *dep ) const {
		for ( uint32_t i = 0; i < base->numDependencies; i++ ) {
		}
		return false;
    }
    
    void PrintList( void ) const;
    
    bool Load( const CModuleInfo *pLoadList, uint64_t nModCount );
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
            if ( m_pModList[i].name == name ) {
                return &m_pModList[i];
            }
        }
        return NULL;
    }
    
    static inline CModuleLoadList *Get( void )
    { return g_pLoadList; }
private:
    module_t *m_pModList;
    uint32_t m_nModCount;
    
    static CModuleLoadList *g_pLoadList;
};

#endif