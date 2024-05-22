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

    void Clear( void );
    void MakeBounds( const glm::vec3& origin );

    const bbox_t ToPOD( void ) const;

    glm::vec3 mins;
    glm::vec3 maxs;
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

void CModuleBoundBox::Clear( void ) {
    AxisClear( (vec3_t *)this );
}

void CModuleBoundBox::MakeBounds( const glm::vec3& origin ) {
    mins[0] = origin[0] - ( width / 2 );
	mins[1] = origin[1] - ( height / 2 );
	mins[2] = origin[2] + ( height / 2 );

	maxs[0] = origin[0] + ( width / 2 );
	maxs[1] = origin[1] + ( height / 2 );
	maxs[2] = origin[2] + ( height / 2 );
}

const bbox_t CModuleBoundBox::ToPOD( void ) const {
    return { .mins = { mins[0], mins[1], mins[2] }, .maxs = { maxs[0], maxs[1], maxs[2] } };
}

#endif