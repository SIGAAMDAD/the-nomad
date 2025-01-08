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

    const glm::vec3& operator[]( const int index ) const;
	glm::vec3& operator[]( const int index );

    // returns translated bounds
	CModuleBoundBox operator+( const glm::vec3& t ) const;

    // translate the bounds
	CModuleBoundBox& operator+=( const glm::vec3& t );

	CModuleBoundBox operator+( const CModuleBoundBox& a ) const;
	CModuleBoundBox& operator+=( const CModuleBoundBox& a );
	CModuleBoundBox operator-( const CModuleBoundBox& a ) const;
	CModuleBoundBox& operator-=( const CModuleBoundBox& a );
    
    bool IntersectsEntity( uint32_t *nEntityNumber ) const;

    void MakeBounds( const glm::vec3& origin );

    // exact compare, no epsilon
    bool Compare( const CModuleBoundBox& a ) const;

    // compare with epsilon
	bool Compare( const CModuleBoundBox& a, const float epsilon ) const;

    // exact compare, no epsilon
	bool operator==( const CModuleBoundBox& a ) const;

    // exact compare, no epsilon
	bool operator!=( const CModuleBoundBox& a ) const;

    // inside out bounds
	void Clear( void );

    // single point at origin
	void Zero( void );

    // returns center of bounds
	glm::vec3 GetCenter( void ) const;

    // returns the radius relative to the bounds origin
	float GetRadius( void ) const;

    // returns the radius relative to the given center
	float GetRadius( const glm::vec3& center ) const;

    // returns the volume of the bounds
	float GetVolume( void ) const;

    // returns true if bounds are inside out
	bool IsCleared( void ) const;

    // add the point, returns true if the bounds expanded
	bool AddPoint( const glm::vec3& v );

    // add the bounds, returns true if the bounds expanded
	bool AddBounds( const CModuleBoundBox& a );

    // return intersection of this bounds with the given bounds
	CModuleBoundBox Intersect( const CModuleBoundBox& a ) const;

    // intersect this bounds with the given bounds
	CModuleBoundBox& IntersectSelf( const CModuleBoundBox& a );

    // return bounds expanded in all directions with the given value
	CModuleBoundBox Expand( const float d ) const;

    // expand bounds in all directions with the given value
	CModuleBoundBox& ExpandSelf( const float d );

    bool IntersectsSphere( const glm::vec3& p, float radius ) const;

    // includes touching
	bool ContainsPoint( const glm::vec3& p ) const;

    // includes touching
	bool IntersectsBounds( const CModuleBoundBox& a ) const;

	bool LineIntersection( const glm::vec3& start, const glm::vec3& end ) const;
	
    // intersection point is start + dir * scale
	bool RayIntersection( const glm::vec3& start, const glm::vec3& dir, float& scale ) const;

    const bbox_t ToPOD( void ) const;

    glm::vec3 mins;
	glm::vec3 maxs;
    float width;
    float height;
};

#endif