#ifndef __MODULE_POLYQUAD__
#define __MODULE_POLYQUAD__

#pragma once

#include "module_polyvert.h"

class CModulePolyQuad
{
public:
    CModulePolyQuad( void );
    CModulePolyQuad( const CModulePolyQuad& );
    ~CModulePolyQuad();

    CModulePolyQuad& operator=( const CModulePolyQuad& );

    CModulePolyVert& operator[]( int index );
    const CModulePolyVert& operator[]( int index ) const;

    int Count( void ) const;

    CModulePolyVert& Get( int index );
    const CModulePolyVert& Get( int index ) const;
private:
    CModulePolyVert m_PolyVerts[4];
};


GDR_INLINE CModulePolyQuad::CModulePolyQuad( void ) {
}

GDR_INLINE CModulePolyQuad::CModulePolyQuad( const CModulePolyQuad& other ) {
    *this = other;
}

GDR_INLINE CModulePolyQuad::~CModulePolyQuad() {
}

GDR_INLINE CModulePolyQuad& CModulePolyQuad::operator=( const CModulePolyQuad& other ) {
    memcpy( this, eastl::addressof( other ), sizeof( *this ) );
    return *this;
}

GDR_INLINE CModulePolyVert& CModulePolyQuad::operator[]( int index ) {
    return Get( index );
}

GDR_INLINE const CModulePolyVert& CModulePolyQuad::operator[]( int index ) const {
    return Get( index );
}

GDR_INLINE int CModulePolyQuad::Count( void ) const {
    return arraylen( m_PolyVerts );
}

GDR_INLINE CModulePolyVert& CModulePolyQuad::Get( int index ) {
    return m_PolyVerts[ index ];
}

GDR_INLINE const CModulePolyVert& CModulePolyQuad::Get( int index ) const {
    return m_PolyVerts[ index ];
}

#endif