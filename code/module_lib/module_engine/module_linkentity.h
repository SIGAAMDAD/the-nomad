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
	CModuleLinkEntity( const glm::vec3& origin, const CModuleBoundBox& bounds, uint32_t nEntityId, uint32_t nEntityType, uint32_t nEntityNumber );
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

#endif