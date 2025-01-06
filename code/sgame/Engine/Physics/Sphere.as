namespace TheNomad::Engine::Physics {
	class Sphere {
		Sphere() {
		}
		Sphere( const vec3& in point ) {
			m_Origin = point;
			m_nRadius = 0.0f;
		}
		Sphere( const vec3& in point, float radius ) {
			m_Origin = point;
			m_nRadius = radius;
		}

		float opIndex( uint nIndex ) const {
			return m_Origin[ nIndex ];
		}
		float& opIndex( uint nIndex ) {
			return m_Origin[ nIndex ];
		}

		bool opEquals( const Sphere& in other ) const {
			return m_Origin == other.m_Origin && m_nRadius == other.m_nRadius;
		}

		void Clear() {
			m_Origin = vec3( 0.0f );
			m_nRadius = -1.0f;
		}
		void Zero() {
			m_Origin = vec3( 0.0f );
			m_nRadius = 0.0f;
		}

		void SetOrigin( const vec3& in origin ) {
			m_Origin = origin;
		}
		void SetRadius( float radius ) {
			m_nRadius = radius;
		}

		const vec3& GetOrigin() const {
			return m_Origin;
		}
		float GetRadius() const {
			return m_nRadius;
		}

		bool IsCleared() const {
			return ( m_nRadius < 0.0f );
		}

		Sphere& Expand( float delta ) {
			m_nRadius += delta;
			return this;
		}
		bool ContainsPoint( const vec3& in point ) const {
			const vec3 p = point - m_Origin;
			if ( Util::DotProduct( p, p ) > m_nRadius * m_nRadius ) {
				return false;
			}
			return true;
		}

		private vec3 m_Origin = vec3( 0.0f );
		private float m_nRadius = 0.0f;
	};
};