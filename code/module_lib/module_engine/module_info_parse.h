#ifndef __MODULE_INFO_PARSE__
#define __MODULE_INFO_PARSE__

#pragma once

#include "../module_public.h"

class CModuleInfoParser
{
public:
    CModuleInfoParser( void );
    CModuleInfoParser( const char *fileName );
    ~CModuleInfoParser();

    void Parse( const char *fileName );
private:
    char *m_pBuffer;
    char m_szInfos[MAX_INFO_KEY];
};

#endif