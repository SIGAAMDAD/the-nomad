#ifndef __MODULE_PARSE__
#define __MODULE_PARSE__

#pragma once

#include "module_public.h"

class CModuleParse
{
public:
    using HashTable = UtlMap<eastl::string, UtlString>;
    using ValueList = UtlVector<UtlString>;

    CModuleParse( const char *filename );
    CModuleParse( const CModuleParse & );
    CModuleParse( CModuleParse && );
    ~CModuleParse();

    CModuleParse *GetInfo( const char *pName );
    UtlString& GetValue( const char *pName );
    UtlMap<eastl::string, UtlString>& GetHashTable( const char *pName );
    UtlVector<UtlString>& GetArray( const char *pName );

    bool Failed( void ) const;

    static inline UtlMap<eastl::string, UtlString> m_EmptyMap{};
    static inline UtlVector<UtlString> m_EmptyList{};
    static inline UtlString m_EmptyString{ "" };
protected:
    CModuleParse( void  ) = default;

    // these three will recurse off each other
    void ParseHashTable( const char **text, const char *pName, HashTable& hashTable );
    void ParseList( const char **text, const char *pName, ValueList& valueList );
    void ParseInfo( const char **text, const char *pName, CModuleParse *parse );
    
    void Parse( void );

    void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL Error( const char *fmt, ... );
    void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL Warning( const char *fmt, ... );

    char *m_pTextBuffer;
    qboolean m_bFailed;

    UtlString m_Name;
    UtlVector<CModuleParse *> m_Infos;
    UtlMap<eastl::string, UtlString> m_Values;

    // this is ugly as fuck
    UtlMap<eastl::string, UtlVector<UtlString>> m_ValueLists;
    UtlMap<eastl::string, UtlMap<eastl::string, UtlString>> m_ValueMaps;
};

#endif