#ifndef __MODULE_CMDARGS__
#define __MODULE_CMDARGS__

#pragma once

#include "../module_public.h"

class CModuleCmdArgs
{
public:
    CModuleCmdArgs( void );
    CModuleCmdArgs( const string_t& text );
    ~CModuleCmdArgs();

    int32_t Argc( void ) const;
    const string_t& Argv( int32_t arg ) const;
    const string_t& Args( int32_t start = 1, int32_t end = -1 ) const;

    
private:
    static const int32_t ;
};

#endif