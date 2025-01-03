const float INFINITY = 1e30f;

namespace TheNomad::Engine::Physics {
	//
	// Bounds:
	// a rewritten version of GameSystem::Bounds, since that version is currently bugged,
	// and this is probably a better place to stick the class definition
	//
	class Bounds {
		Bounds() {
		}
		Bounds( const vec3& in mins, const vec3& in maxs ) {
			m_nMins = mins;
			m_nMaxs = maxs;
			m_nWidth = m_nMaxs.x - m_nMins.x;
			m_nHeight = m_nMaxs.y - m_nMins.y;
		}
		Bounds( float width, float height, const vec3& in origin ) {
			m_nWidth = width;
			m_nHeight = height;
			MakeBounds( origin );
		}

		Bounds& opAssign( const Bounds& in bounds ) {
			m_nMins = bounds.m_nMins;
			m_nMaxs = bounds.m_nMaxs;
			m_nWidth = bounds.m_nWidth;
			m_nHeight = bounds.m_nHeight;

			return this;
		}
		bool opEquals( const Bounds& in bounds ) const {
			return m_nMins == bounds.m_nMins && m_nMaxs == bounds.m_nMaxs;
		}

		void Clear() {
			m_nWidth = INFINITY;
			m_nHeight = INFINITY;
			m_nMins = vec3( INFINITY );
			m_nMaxs = vec3( INFINITY );
		}
		void Zero() {
			m_nWidth = 0.0f;
			m_nHeight = 0.0f;
			m_nMins = vec3( 0.0f );
			m_nMaxs = vec3( 0.0f );
		}

		vec3 GetCenter() const {
			return vec3( ( m_nMaxs.x + m_nMins.x ) * 0.5f, ( m_nMaxs.y + m_nMins.y ) * 0.5f, ( m_nMaxs.z + m_nMins.z ) * 0.5f );
		}
		float GetRadius( const vec3& in center ) const {
			float b0, b1;

			float total = 0.0f;
			// loop0
			{
				b0 = abs( center.x - m_nMins.x );
				b1 = abs( m_nMaxs.x - center.x );
				if ( b0 > b1 ) {
					total += b0 * b0;
				} else {
					total += b1 * b1;
				}
			}
			// loop1
			{
				b0 = abs( center.y - m_nMins.y );
				b1 = abs( m_nMaxs.y - center.y );
				if ( b0 > b1 ) {
					total += b0 * b0;
				} else {
					total += b1 * b1;
				}
			}
			// loop2
			{
				b0 = abs( center.z - m_nMins.z );
				b1 = abs( m_nMaxs.z - center.z );
				if ( b0 > b1 ) {
					total += b0 * b0;
				} else {
					total += b1 * b1;
				}
			}
			return sqrt( total );
		}
		float GetRadius() const {
			float b0, b1;

			float total = 0.0f;
			// loop0
			{
				b0 = abs( m_nMins.x );
				b1 = abs( m_nMaxs.x );
				if ( b0 > b1 ) {
					total += b0 * b0;
				} else {
					total += b1 * b1;
				}
			}
			// loop1
			{
				b0 = abs( m_nMins.y );
				b1 = abs( m_nMaxs.y );
				if ( b0 > b1 ) {
					total += b0 * b0;
				} else {
					total += b1 * b1;
				}
			}
			// loop2
			{
				b0 = abs( m_nMins.z );
				b1 = abs( m_nMaxs.z );
				if ( b0 > b1 ) {
					total += b0 * b0;
				} else {
					total += b1 * b1;
				}
			}
			return sqrt( total );
		}
		bool IsCleared() const {
			return m_nMins.x > m_nMaxs.x;
		}

		bool AddPoint( const vec3& in p ) {
			bool expanded = false;

			if ( p.x < m_nMins.x ) {
				m_nMins.x = p.x;
				expanded = true;
			}
			if ( p.x > m_nMaxs.x ) {
				m_nMaxs.x = p.x;
				expanded = true;
			}
			if ( p.y < m_nMins.y ) {
				m_nMins.y = p.y;
				expanded = true;
			}
			if ( p.y > m_nMaxs.y ) {
				m_nMaxs.y = p.y;
				expanded = true;
			}
			if ( p.z < m_nMins.z ) {
				m_nMins.z = p.z;
				expanded = true;
			}
			if ( p.z > m_nMaxs.z ) {
				m_nMaxs.z = p.z;
				expanded = true;
			}
			return expanded;
		}
		bool AddBounds( const Bounds& in other ) {
			bool expanded = false;
			if ( other.m_nMins.x < m_nMins.x ) {
				m_nMins.x = other.m_nMins.x;
				expanded = true;
			}
			if ( other.m_nMins.y < m_nMins.y ) {
				m_nMins.y = other.m_nMins.y;
				expanded = true;
			}
			if ( other.m_nMins.z < m_nMins.z ) {
				m_nMins.z = other.m_nMins.z;
				expanded = true;
			}
			if ( other.m_nMaxs.x > m_nMaxs.x ) {
				m_nMaxs.x = other.m_nMaxs.x;
				expanded = true;
			}
			if ( other.m_nMaxs.y > m_nMaxs.y ) {
				m_nMaxs.y = other.m_nMaxs.y;
				expanded = true;
			}
			if ( other.m_nMaxs.z > m_nMaxs.z ) {
				m_nMaxs.z = other.m_nMaxs.z;
				expanded = true;
			}
			return expanded;
		}

		bool LineIntersection( const vec3& in start, const vec3& in end ) const {
			vec3 ld;
			const vec3 center = ( m_nMins + m_nMaxs ) * 0.5f;
			const vec3 extents = m_nMaxs - center;
			vec3 lineDir = 0.5f;
			lineDir.x *= ( end.x - start.x );
			lineDir.y *= ( end.y - start.y );
			lineDir.z *= ( end.z - start.z );
			const vec3 lineCenter = start + lineDir;
			const vec3 dir = lineCenter - center;

			ld.x = abs( lineDir.x );
			if ( abs( dir.x ) > extents.x + ld.x ) {
				return false;
			}

			ld.y = abs( lineDir.y );
			if ( abs( dir.y ) > extents.y + ld.y ) {
				return false;
			}

			ld.z = abs( lineDir.z );
			if ( abs( dir.z ) > extents.z + ld.z ) {
				return false;
			}

			vec3 cross;
			Util::CrossProduct( lineDir, dir, cross );

			if ( abs( cross.x ) > extents.y * ld.z + extents.z * ld.y ) {
				return false;
			}
			if ( abs( cross.y ) > extents.x * ld.z + extents.z * ld.x ) {
				return false;
			}
			if ( abs( cross.z ) > extents.z * ld.y + extents.y * ld.x ) {
				return false;
			}

			return true;
		}
		bool IntersectsSphere( const vec3& in p, float radius ) const {
			if ( p.x - radius > m_nMaxs.x || p.x + radius < m_nMins.x ||
				p.y - radius > m_nMaxs.y || p.y + radius < m_nMins.y )
			{
				return false;
			}
			return true;
		}
		bool IntersectsPoint( const vec3& in p ) const {
//			if ( p.z > m_nMaxs.z || p.z < m_nMins.z ) {
//				return false;
//			}
			if ( p.x < m_nMins.x || p.y < m_nMins.y || p.z < m_nMins.z
				|| p.x > m_nMaxs.x || p.y > m_nMaxs.y || p.z > m_nMaxs.z )
			{
				return false;
			}
			return true;
		}
		bool IntersectsBounds( const Bounds& other ) const {
			if ( other.m_nMins.z > m_nMaxs.z || other.m_nMaxs.z < m_nMins.z ) {
				return false;
			}
			if ( other.m_nMaxs.x < m_nMins.x || other.m_nMaxs.y < m_nMins.y
				|| other.m_nMins.x > m_nMaxs.x || other.m_nMins.y > m_nMaxs.y )
			{
				return false;
			}
			return true;
		}

		void MakeBounds( const vec3& in origin ) {
			m_nMins.x = origin.x;
			m_nMins.y = origin.y;
			m_nMins.z = origin.z;

			m_nMaxs.x = origin.x + m_nWidth;
			m_nMaxs.y = origin.y + m_nHeight;
			m_nMaxs.z = origin.z + m_nHeight;
		}

		vec3 m_nMins = vec3( 0.0f );
		vec3 m_nMaxs = vec3( 0.0f );
		float m_nWidth = 0.0f;
		float m_nHeight = 0.0f;
	};
};