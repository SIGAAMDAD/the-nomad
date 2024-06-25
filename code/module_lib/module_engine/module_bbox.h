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
    memset( this, 0, sizeof( *this ) );
}

float CModuleBoundBox::GetRadius( void ) const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for ( i = 0; i < 3; i++ ) {
		b0 = (float)fabs( mins[i] );
		b1 = (float)fabs( maxs[i] );
		if ( b0 > b1 ) {
			total += b0 * b0;
		} else {
			total += b1 * b1;
		}
	}
	return sqrtf( total );
}

float CModuleBoundBox::GetRadius( const glm::vec3& center ) const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for ( i = 0; i < 3; i++ ) {
		b0 = (float)fabs( center[i] - mins[i] );
		b1 = (float)fabs( maxs[i] - center[i] );
		if ( b0 > b1 ) {
			total += b0 * b0;
		} else {
			total += b1 * b1;
		}
	}
	return sqrtf( total );
}


bool CModuleBoundBox::LineIntersection( const glm::vec3& start, const glm::vec3& end ) const {
	float ld[3];
	glm::vec3 center = ( mins + maxs ) * 0.5f;
	glm::vec3 extents = maxs - center;
	glm::vec3 lineDir = 0.5f * ( end - start );
	glm::vec3 lineCenter = start + lineDir;
	glm::vec3 dir = lineCenter - center;

	ld[0] = fabs( lineDir[0] );
	if ( fabs( dir[0] ) > extents[0] + ld[0] ) {
		return false;
	}

	ld[1] = fabs( lineDir[1] );
	if ( fabs( dir[1] ) > extents[1] + ld[1] ) {
		return false;
	}

	ld[2] = fabs( lineDir[2] );
	if ( fabs( dir[2] ) > extents[2] + ld[2] ) {
		return false;
	}

	glm::vec3 cross = glm::cross( lineDir,  dir );

	if ( fabs( cross[0] ) > extents[1] * ld[2] + extents[2] * ld[1] ) {
		return false;
	}

	if ( fabs( cross[1] ) > extents[0] * ld[2] + extents[2] * ld[0] ) {
		return false;
	}

	if ( fabs( cross[2] ) > extents[0] * ld[1] + extents[1] * ld[0] ) {
		return false;
	}

	return true;
}

bool CModuleBoundBox::RayIntersection( const glm::vec3& start, const glm::vec3& dir, float &scale ) const {
	int i, ax0, ax1, ax2, side, inside;
	float f;
	glm::vec3 hit;

	ax0 = -1;
	inside = 0;
	for ( i = 0; i < 3; i++ ) {
		if ( start[i] < mins[i] ) {
			side = 0;
		}
		else if ( start[i] > maxs[i] ) {
			side = 1;
		}
		else {
			inside++;
			continue;
		}
		if ( dir[i] == 0.0f ) {
			continue;
		}
		f = ( start[i] - ( &mins )[side][i] );
		if ( ax0 < 0 || fabs( f ) > fabs( scale * dir[i] ) ) {
			scale = - ( f / dir[i] );
			ax0 = i;
		}
	}

	if ( ax0 < 0 ) {
		scale = 0.0f;
		// return true if the start point is inside the bounds
		return ( inside == 3 );
	}

	ax1 = (ax0+1)%3;
	ax2 = (ax0+2)%3;
	hit[ax1] = start[ax1] + scale * dir[ax1];
	hit[ax2] = start[ax2] + scale * dir[ax2];

	return ( hit[ax1] >= mins[ax1] && hit[ax1] <= maxs[ax1] &&
				hit[ax2] >= mins[ax2] && hit[ax2] <= maxs[ax2] );
}

void CModuleBoundBox::MakeBounds( const glm::vec3& origin ) {
    /*
    mins[0] = origin[0] - ( width / 2 );
	mins[1] = origin[1] - ( height / 2 );
	mins[2] = origin[2] + ( height / 2 );

	maxs[0] = origin[0] + ( width / 2 );
	maxs[1] = origin[1] + ( height / 2 );
	maxs[2] = origin[2] + ( height / 2 );
    */

    mins[0] = origin[0];
    mins[1] = origin[1];
    mins[2] = origin[2] + ( height / 2 );

    maxs[0] = origin[0] + width;
    maxs[1] = origin[1] + height;
    maxs[2] = origin[2] - ( height / 2 );
}

const bbox_t CModuleBoundBox::ToPOD( void ) const {
    return { .mins = { mins[0], mins[1], mins[2] }, .maxs = { maxs[0], maxs[1], maxs[2] } };
}

#endif