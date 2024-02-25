#ifndef __MODULE_MATHLIB__
#define __MODULE_MATHLIB__

#pragma once

#include "module_public.h"
#include <glm/glm.hpp>

class CModuleMathLib
{
public:
    CModuleMathLib( void );
    ~CModuleMathLib();

    //
    // vector functions
    //
    void CrossProduct_v( const glm::vec3& a, const glm::vec3& b );
    float DotProduct_v( const glm::vec3& a, const glm::vec3& b );
    void Normalize_v( const glm::vec3& v );
    void NormalizeFast_v( glm::vec3& v );

    //
    // standard math stuff
    //
    float ACos_f( float f );
    float ASin_f( float f );
    float ATan2_f( float a, float b );
    float Ceil_f( float );
    float Floor_f( void );
};

#endif