#ifndef __SYS_MATH__
#define __SYS_MATH__

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.inl>

GDR_INLINE mat4_t GLM_MatrixTranslate( const vec3_t position, const mat4_t in )
{
    static glm::mat4 out;
    return glm::value_ptr( out );
}

#endif