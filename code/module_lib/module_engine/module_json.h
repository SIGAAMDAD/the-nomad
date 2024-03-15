#ifndef __MODULE_JSON__
#define __MODULE_JSON__

#pragma once

#include "../module_public.h"

class CModuleJsonObject
{
public:
    using JsonObject = nlohmann::json;

    CModuleJsonObject( void );
    CModuleJsonObject( const string_t *fileName );
    CModuleJsonObject( const CModuleJsonObject& );
    ~CModuleJsonObject() = default;

    CModuleJsonObject& operator=( const CModuleJsonObject& ) = default;
};

#endif