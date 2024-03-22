#ifndef __MODULE_POLYVERT__
#define __MODULE_POLYVERT__

#pragma once

#include "../module_public.h"

class CModulePolyVert
{
public:
    CModulePolyVert( void );
    CModulePolyVert( const CModulePolyVert& );
    CModulePolyVert( const glm::vec3& origin, const glm::vec3& worldPos, const glm::vec2& texCoords, const color4ub_t& color );
    ~CModulePolyVert();

    CModulePolyVert& operator=( const CModulePolyVert& );

    const glm::vec2& GetTexCoords( void ) const;
    const glm::vec3& GetOrigin( void ) const;
    const glm::vec3& GetWorldPos( void ) const;
    const color4ub_t& GetColor( void ) const;

    void Clear( void );
    void Set( const glm::vec3& origin, const glm::vec3& worldPos, const glm::vec2& texCoords, const color4ub_t& color );
    void Set( const CModulePolyVert& );

    glm::vec3 m_Origin;
    glm::vec3 m_WorldPos;
    glm::vec2 m_TexCoords;
    color4ub_t m_Color;
};

CModulePolyVert::CModulePolyVert( void ) {
    Clear();
}

CModulePolyVert::CModulePolyVert( const glm::vec3& origin, const glm::vec3& worldPos,
    const glm::vec2& texCoords, const color4ub_t& color )
{
    Set( origin, worldPos, texCoords, color );
}

CModulePolyVert::CModulePolyVert( const CModulePolyVert& other ) {
    *this = other;
}

CModulePolyVert::~CModulePolyVert() {
}

CModulePolyVert& CModulePolyVert::operator=( const CModulePolyVert& other ) {
    return *(CModulePolyVert *)memcpy( this, eastl::addressof( other ), sizeof( *this ) );
}

const glm::vec2& CModulePolyVert::GetTexCoords( void ) const {
    return m_TexCoords;
}

const glm::vec3& CModulePolyVert::GetOrigin( void ) const {
    return m_Origin;
}

const glm::vec3& CModulePolyVert::GetWorldPos( void ) const {
    return m_WorldPos;
}

const color4ub_t& CModulePolyVert::GetColor( void ) const {
    return m_Color;
}

void CModulePolyVert::Clear( void ) {
    memset( this, 0, sizeof( *this ) );
}

void CModulePolyVert::Set( const CModulePolyVert& other ) {
    memcpy( this, eastl::addressof( other ), sizeof( *this ) );
}

void CModulePolyVert::Set( const glm::vec3& origin, const glm::vec3& worldPos,
    const glm::vec2& texCoords, const color4ub_t& color )
{
    m_Origin = origin;
    m_WorldPos = worldPos;
    m_TexCoords = texCoords;
    m_Color = color;
}

#endif