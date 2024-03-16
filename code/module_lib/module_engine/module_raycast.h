#ifndef __MODULE_RAYCAST__
#define __MODULE_RAYCAST__

#pragma once

#include "../module_public.h"

class CModuleRayCast
{
public:
    CModuleRayCast( void ) = default;
    ~CModuleRayCast() = default;

    CModuleRayCast& operator=( const CModuleRayCast& ) = default;

    glm::vec3 start;
	glm::vec3 end;
	glm::vec3 origin;
    uint32_t entityNumber;
//	float speed;
	float length;
	float angle;
    uint32_t flags; // unused for now
};

#endif