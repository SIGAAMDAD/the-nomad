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

    CModuleLinkEntity& operator=( const CModuleLinkEntity& );

    void SetOrigin( const glm::vec3& origin );
    void SetBounds( const CModuleBoundBox& bounds );
    void Update( void );

    const glm::vec3& GetOrigin( void );
    const CModuleBoundBox& GetBounds( void );

    // internal
    void ToLinkEntity( linkEntity_t *ent );

    CModuleBoundBox m_Bounds;
    glm::vec3 m_Origin;
    uint32_t m_nEntityId;
    uint32_t m_nEntityType;
    uint32_t m_nEntityNumber;

    qboolean m_bLinked;
    
    linkEntity_t handle;
};

CModuleLinkEntity::CModuleLinkEntity( void ) {
    memset( this, 0, sizeof( *this ) );
    m_bLinked = qfalse;
    ToLinkEntity( &handle );
}

CModuleLinkEntity::CModuleLinkEntity( const glm::vec3& origin, const CModuleBoundBox& bounds, uint32_t nEntityId, uint32_t nEntityType )
{
    m_Origin = origin;
    m_Bounds = bounds;
    m_nEntityType = nEntityType;
    m_nEntityId = nEntityId;
    m_nEntityNumber = g_world->NumEntities();
    m_bLinked = qtrue;
    ToLinkEntity( &handle );
    g_world->LinkEntity( &handle );
}

CModuleLinkEntity::~CModuleLinkEntity() {
    if ( m_bLinked ) {
        g_world->UnlinkEntity( &handle );
    }
}

CModuleLinkEntity& CModuleLinkEntity::operator=( const CModuleLinkEntity& other ) {
    memcpy( this, eastl::addressof( other ), sizeof( *this ) );
    return *this;
}

void CModuleLinkEntity::Update( void ) {
    ToLinkEntity( &handle );
}

void CModuleLinkEntity::SetOrigin( const glm::vec3& origin ) {
    VectorCopy( m_Origin, origin );
}

void CModuleLinkEntity::SetBounds( const CModuleBoundBox& bounds ) {
    m_Bounds = bounds;
}

const glm::vec3& CModuleLinkEntity::GetOrigin( void ) {
    return m_Origin;
}

const CModuleBoundBox& CModuleLinkEntity::GetBounds( void ) {
    return m_Bounds;
}

void CModuleLinkEntity::ToLinkEntity( linkEntity_t *ent ) {
    VectorCopy( ent->origin, m_Origin );
    VectorCopy( ent->bounds.mins, m_Bounds.mins );
    VectorCopy( ent->bounds.maxs, m_Bounds.maxs );

    ent->id = m_nEntityId;
    ent->type = m_nEntityType;
    ent->entityNumber = m_nEntityNumber;
}

#endif