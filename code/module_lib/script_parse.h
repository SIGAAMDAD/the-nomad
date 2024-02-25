#ifndef __SCRIPT_PARSE__
#define __SCRIPT_PARSE__

#pragma once

class CScriptParser
{
public:
    CScriptParser( void );

    void AddRef( void ) const;
    void Release( void ) const;

    void Load( const eastl::string& file );
private:
    ~CScriptParser();

    mutable int m_nRefCount;
};

void RegisterScriptParser();

#endif