#ifndef __MODULE_JSON__
#define __MODULE_JSON__

#pragma once

class CModuleJsonObject
{
public:
    CModuleJsonObject( void );
    CModuleJsonObject( const string_t *fileName );
    CModuleJsonObject( const CModuleJsonObject& );
    ~CModuleJsonObject() = default;

    CModuleJsonObject& operator=( const CModuleJsonObject& ) = default;
};

#endif