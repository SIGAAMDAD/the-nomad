#include "checkpoint.as"

namespace TheNomad::SGame {
    shared class MapSpawn {
		MapSpawn( const uvec3& in origin, uint nEntityId, TheNomad::GameSystem::EntityType nEntityType,
			MapCheckpoint@ checkpoint )
		{
			m_Checkpoint = checkpoint;
			m_Origin = origin;
			m_nEntityId = nEntityId;
			m_nEntityType = nEntityType;
			m_bUsed = false;
			
			checkpoint.AddSpawn( this );
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
		TheNomad::GameSystem::EntityType m_nEntityType;
		uint m_nEntityId;
		MapCheckpoint@ m_Checkpoint;
		bool m_bUsed;
	};
};