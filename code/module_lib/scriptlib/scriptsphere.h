/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __SCRIPT_SPHERE__
#define __SCRIPT_SPHERE__

#pragma once

#include "../module_public.h"

/*
===============================================================================

	Sphere

===============================================================================
*/

class CModuleSphere {
public:
	CModuleSphere( void );
	explicit CModuleSphere( const glm::vec3& point );
	explicit CModuleSphere( const glm::vec3& point, const float r );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	CModuleSphere		operator+( const glm::vec3& t ) const;				// returns tranlated sphere
	CModuleSphere &		operator+=( const glm::vec3& t );					// translate the sphere
	CModuleSphere		operator+( const CModuleSphere &s ) const;
	CModuleSphere &		operator+=( const CModuleSphere &s );

	bool			Compare( const CModuleSphere &a ) const;							// exact compare, no epsilon
	bool			Compare( const CModuleSphere &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const CModuleSphere &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const CModuleSphere &a ) const;						// exact compare, no epsilon

	void			Clear( void );									// inside out sphere
	void			Zero( void );									// single point at origin
	void			SetOrigin( const glm::vec3& o );					// set origin of sphere
	void			SetRadius( const float r );						// set square radius

	const glm::vec3& 	GetOrigin( void ) const;						// returns origin of sphere
	float			GetRadius( void ) const;						// returns sphere radius
	bool			IsCleared( void ) const;						// returns true if sphere is inside out

	bool			AddPoint( const glm::vec3& p );					// add the point, returns true if the sphere expanded
	bool			AddSphere( const CModuleSphere &s );					// add the sphere, returns true if the sphere expanded
	CModuleSphere		Expand( const float d ) const;					// return bounds expanded in all directions with the given value
	CModuleSphere &		ExpandSelf( const float d );					// expand bounds in all directions with the given value
	CModuleSphere		Translate( const glm::vec3& translation ) const;
	CModuleSphere &		TranslateSelf( const glm::vec3& translation );

//	float			PlaneDistance( const idPlane &plane ) const;
//	int				PlaneSide( const idPlane &plane, const float epsilon = ON_EPSILON ) const;

	bool			ContainsPoint( const glm::vec3& p ) const;			// includes touching
	bool			IntersectsSphere( const CModuleSphere &s ) const;	// includes touching
	bool			LineIntersection( const glm::vec3& start, const glm::vec3& end ) const;
					// intersection points are (start + dir * scale1) and (start + dir * scale2)
	bool			RayIntersection( const glm::vec3& start, const glm::vec3& dir, float &scale1, float &scale2 ) const;

					// Tight sphere for a point set.
	void			FromPoints( const glm::vec3 *points, const int numPoints );
					// Most tight sphere for a translation.
	void			FromPointTranslation( const glm::vec3& point, const glm::vec3& translation );
	void			FromSphereTranslation( const CModuleSphere &sphere, const glm::vec3& start, const glm::vec3& translation );
					// Most tight sphere for a rotation.
	void			FromPointRotation( const glm::vec3& point, const idRotation &rotation );
	void			FromSphereRotation( const CModuleSphere &sphere, const glm::vec3& start, const idRotation &rotation );

	void			AxisProjection( const glm::vec3& dir, float &min, float &max ) const;

private:
	glm::vec3			origin;
	float			radius;
};

extern CModuleSphere	sphere_zero;

GDR_INLINE CModuleSphere::CModuleSphere( void ) {
}

GDR_INLINE CModuleSphere::CModuleSphere( const glm::vec3& point ) {
	origin = point;
	radius = 0.0f;
}

GDR_INLINE CModuleSphere::CModuleSphere( const glm::vec3& point, const float r ) {
	origin = point;
	radius = r;
}

GDR_INLINE float CModuleSphere::operator[]( const int index ) const {
	return ((float *) &origin)[index];
}

GDR_INLINE float &CModuleSphere::operator[]( const int index ) {
	return ((float *) &origin)[index];
}

GDR_INLINE CModuleSphere CModuleSphere::operator+( const glm::vec3& t ) const {
	return CModuleSphere( origin + t, radius );
}

GDR_INLINE CModuleSphere &CModuleSphere::operator+=( const glm::vec3& t ) {
	origin += t;
	return *this;
}

GDR_INLINE bool CModuleSphere::Compare( const CModuleSphere &a ) const {
	return ( origin.Compare( a.origin ) && radius == a.radius );
}

GDR_INLINE bool CModuleSphere::Compare( const CModuleSphere &a, const float epsilon ) const {
	return ( origin.Compare( a.origin, epsilon ) && idMath::Fabs( radius - a.radius ) <= epsilon );
}

GDR_INLINE bool CModuleSphere::operator==( const CModuleSphere &a ) const {
	return Compare( a );
}

GDR_INLINE bool CModuleSphere::operator!=( const CModuleSphere &a ) const {
	return !Compare( a );
}

GDR_INLINE void CModuleSphere::Clear( void ) {
	origin.Zero();
	radius = -1.0f;
}

GDR_INLINE void CModuleSphere::Zero( void ) {
	origin.Zero();
	radius = 0.0f;
}

GDR_INLINE void CModuleSphere::SetOrigin( const glm::vec3& o ) {
	origin = o;
}

GDR_INLINE void CModuleSphere::SetRadius( const float r ) {
	radius = r;
}

GDR_INLINE const glm::vec3& CModuleSphere::GetOrigin( void ) const {
	return origin;
}

GDR_INLINE float CModuleSphere::GetRadius( void ) const {
	return radius;
}

GDR_INLINE bool CModuleSphere::IsCleared( void ) const {
	return ( radius < 0.0f );
}

GDR_INLINE bool CModuleSphere::AddPoint( const glm::vec3& p ) {
	if ( radius < 0.0f ) {
		origin = p;
		radius = 0.0f;
		return true;
	}
	else {
		float r = ( p - origin ).LengthSqr();
		if ( r > radius * radius ) {
			r = idMath::Sqrt( r );
			origin += ( p - origin ) * 0.5f * (1.0f - radius / r );
			radius += 0.5f * ( r - radius );
			return true;
		}
		return false;
	}
}

GDR_INLINE bool CModuleSphere::AddSphere( const CModuleSphere &s ) {
	if ( radius < 0.0f ) {
		origin = s.origin;
		radius = s.radius;
		return true;
	}
	else {
		float r = ( s.origin - origin ).LengthSqr();
		if ( r > ( radius + s.radius ) * ( radius + s.radius ) ) {
			r = idMath::Sqrt( r );
			origin += ( s.origin - origin ) * 0.5f * (1.0f - radius / ( r + s.radius ) );
			radius += 0.5f * ( ( r + s.radius ) - radius );
			return true;
		}
		return false;
	}
}

GDR_INLINE CModuleSphere CModuleSphere::Expand( const float d ) const {
	return CModuleSphere( origin, radius + d );
}

GDR_INLINE CModuleSphere &CModuleSphere::ExpandSelf( const float d ) {
	radius += d;
	return *this;
}

GDR_INLINE CModuleSphere CModuleSphere::Translate( const glm::vec3& translation ) const {
	return CModuleSphere( origin + translation, radius );
}

GDR_INLINE CModuleSphere &CModuleSphere::TranslateSelf( const glm::vec3& translation ) {
	origin += translation;
	return *this;
}

GDR_INLINE bool CModuleSphere::ContainsPoint( const glm::vec3& p ) const {
	if ( ( p - origin ).LengthSqr() > radius * radius ) {
		return false;
	}
	return true;
}

GDR_INLINE bool CModuleSphere::IntersectsSphere( const CModuleSphere &s ) const {
	float r = s.radius + radius;
	if ( ( s.origin - origin ).LengthSqr() > r * r ) {
		return false;
	}
	return true;
}

GDR_INLINE void CModuleSphere::FromPointTranslation( const glm::vec3& point, const glm::vec3& translation ) {
	origin = point + 0.5f * translation;
	radius = idMath::Sqrt( 0.5f * translation.LengthSqr() );
}

GDR_INLINE void CModuleSphere::FromSphereTranslation( const CModuleSphere &sphere, const glm::vec3& start, const glm::vec3& translation ) {
	origin = start + sphere.origin + 0.5f * translation;
	radius = idMath::Sqrt( 0.5f * translation.LengthSqr() ) + sphere.radius;
}

GDR_INLINE void CModuleSphere::FromPointRotation( const glm::vec3& point, const idRotation &rotation ) {
	glm::vec3 end = rotation * point;
	origin = ( point + end ) * 0.5f;
	radius = idMath::Sqrt( 0.5f * ( end - point ).LengthSqr() );
}

GDR_INLINE void CModuleSphere::FromSphereRotation( const CModuleSphere &sphere, const glm::vec3& start, const idRotation &rotation ) {
	glm::vec3 end = rotation * sphere.origin;
	origin = start + ( sphere.origin + end ) * 0.5f;
	radius = idMath::Sqrt( 0.5f * ( end - sphere.origin ).LengthSqr() ) + sphere.radius;
}

GDR_INLINE void CModuleSphere::AxisProjection( const glm::vec3& dir, float &min, float &max ) const {
	float d;
	d = dir * origin;
	min = d - radius;
	max = d + radius;
}

#endif /* !__SCRIPT_SPHERE__ */
