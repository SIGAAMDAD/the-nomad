#ifndef __MODULE_BBOX__
#define __MODULE_BBOX__

#pragma once

#include "../module_public.h"

class CModuleBoundBox
{
public:
    CModuleBoundBox( void );
    CModuleBoundBox( float w, float h, const glm::vec3& origin );

    CModuleBoundBox& operator=( const CModuleBoundBox& );

    void MakeBounds( const glm::vec3& origin );

    glm::vec2 mins;
    glm::vec2 maxs;
    float width;
    float height;
};

CModuleBoundBox::CModuleBoundBox( void ) {
    memset( this, 0, sizeof( *this ) );
}

CModuleBoundBox::CModuleBoundBox( float w, float h, const glm::vec3& origin ) {
    memset( this, 0, sizeof( *this ) );
    width = w;
    height = h;
    MakeBounds( origin );
}

CModuleBoundBox& CModuleBoundBox::operator=( const CModuleBoundBox& other ) {
    memcpy( this, eastl::addressof( other ), sizeof( *this ) );
    return *this;
}

void CModuleBoundBox::MakeBounds( const glm::vec3& origin ) {
    mins[0] = origin[0] - height;
	mins[1] = origin[1] - width;
	mins[2] = origin[2];

	maxs[0] = origin[0] + height;
	maxs[1] = origin[1] + width;
	maxs[2] = origin[2];
}

#endif