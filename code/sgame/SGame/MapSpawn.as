namespace TheNomad::SGame {
    class MapSpawn {
		MapSpawn( const uvec3& in origin, uint nEntityId, TheNomad::GameSystem::EntityType nEntityType ) {
			@m_Checkpoint = null;
			m_Origin = origin;
			m_nEntityId = nEntityId;
			m_nEntityType = nEntityType;
			m_bUsed = false;
		}
		MapSpawn() {
		}
		
		void Activate() {
			if ( m_bUsed ) {
				return; // ensure no recursions
			}
			
			m_bUsed = true;
		}
		
		uvec3 m_Origin;
		MapCheckpoint@ m_Checkpoint;
		TheNomad::GameSystem::EntityType m_nEntityType;
		uint m_nEntityId;
		bool m_bUsed;
	};
};