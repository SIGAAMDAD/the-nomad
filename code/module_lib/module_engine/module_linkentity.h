#ifndef __MODULE_LINK_ENTITY__
#define __MODULE_LINK_ENTITY__

#pragma once

#include "../../game/g_world.h"
#include "../module_public.h"
#include "module_bbox.h"

class CModuleLinkEntity
{
public:
    CModuleLinkEntity( void );
    CModuleLinkEntity( const glm::vec3& origin, const CModuleBoundBox& bounds, uint32_t nEntityId, uint32_t nEntityType );
    ~CModuleLinkEntity();

    void SetOrigin( const glm::vec3& origin );
    void SetBounds( const CModuleBoundBox& bounds );

    const glm::vec3& GetOrigin( void ) const;
    const CModuleBoundBox& GetBounds( void ) const;

    // internal
    void ToLinkEntity( linkEntity_t *ent );

    CModuleBoundBox m_Bounds;
    glm::vec3 m_Origin;
    uint32_t m_nEntityId;
    uint32_t m_nEntityType;

    linkEntity_t handle;
};

CModuleLinkEntity::CModuleLinkEntity( void ) {
    memset( this, 0, sizeof( *this ) );
    ToLinkEntity( &handle );
    g_world.LinkEntity( &handle );
}

CModuleLinkEntity::CModuleLinkEntity( const glm::vec3& origin, const CModuleBoundBox& bounds, uint32_t nEntityId, uint32_t nEntityType ) {
    m_Origin = origin;
    m_Bounds = bounds;
    m_nEntityType = nEntityType;
    m_nEntityId = nEntityId;
    ToLinkEntity( &handle );
    g_world.LinkEntity( &handle );
}

CModuleLinkEntity::~CModuleLinkEntity() {
    g_world.UnlinkEntity( &handle );
}

void CModuleLinkEntity::SetOrigin( const glm::vec3& origin ) {
    VectorCopy( m_Origin, origin );
}

void CModuleLinkEntity::SetBounds( const CModuleBoundBox& bounds ) {
    m_Bounds = bounds;
}

const glm::vec3& CModuleLinkEntity::GetOrigin( void ) const {
    return m_Origin;
}

const CModuleBoundBox& CModuleLinkEntity::GetBounds( void ) const {
    return m_Bounds;
}

void CModuleLinkEntity::ToLinkEntity( linkEntity_t *ent ) {
    VectorCopy( ent->origin, m_Origin );
    VectorCopy( ent->bounds.mins, m_Bounds.mins );
    VectorCopy( ent->bounds.maxs, m_Bounds.maxs );
    ent->id = m_nEntityId;
    ent->type = m_nEntityType;
}

#endif