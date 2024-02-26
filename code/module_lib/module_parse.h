#ifndef __MODULE_PARSE__
#define __MODULE_PARSE__

#pragma once

#include "module_public.h"

class CModuleParse
{
public:
    using HashTable = eastl::unordered_map<eastl::string, eastl::string>;
    using ValueList = UtlVector<eastl::string>;

    CModuleParse( const char *filename );
    CModuleParse( const CModuleParse & );
    CModuleParse( CModuleParse && );
    ~CModuleParse();

    CModuleParse *GetInfo( const char *pName );
    eastl::string& GetValue( const char *pName );
    HashTable& GetHashTable( const char *pName );
    ValueList& GetArray( const char *pName );

    bool Failed( void ) const;

    static inline HashTable m_EmptyMap{};
    static inline ValueList m_EmptyList{};
    static inline eastl::string m_EmptyString{ "" };
protected:
    CModuleParse( void  ) = default;

    // these three will recurse off each other
    void ParseHashTable( const char **text, const eastl::string& pName, HashTable& hashTable );
    void ParseList( const char **text, const eastl::string& pName, ValueList& valueList );
    void ParseInfo( const char **text, const eastl::string& pName, CModuleParse *parse );
    
    void Parse( void );

    void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL Error( const char *fmt, ... );
    void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL Warning( const char *fmt, ... );

    char *m_pTextBuffer;
    qboolean m_bFailed;

    eastl::string m_Name;
    eastl::vector<CModuleParse *> m_Infos;
    HashTable m_Values;

    // this is ugly
    eastl::unordered_map<eastl::string, ValueList> m_ValueLists;
    eastl::unordered_map<eastl::string, HashTable> m_ValueMaps;
};

#endif