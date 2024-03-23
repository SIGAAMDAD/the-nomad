#ifndef __MODULE_GPUCONFIG__
#define __MODULE_GPUCONFIG__

#pragma once

#include "../module_public.h"

class CModuleGPUConfig
{
public:
    CModuleGPUConfig( void ) = default;

    CModuleGPUConfig& operator=( const CModuleGPUConfig& ) = default;

    string_t vendorString;
    string_t rendererString;
    string_t versionString;
    string_t shaderVersionString;
    string_t extensionsString;
    gpuConfig_t gpuConfig;
};

#endif