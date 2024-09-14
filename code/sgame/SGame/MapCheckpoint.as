#include "SGame/MapSpawn.as"

namespace TheNomad::SGame {
    class MapCheckpoint {
		MapCheckpoint( const uvec3& in origin, const uvec2& in areaLock ) {
			m_Origin = origin;
			m_AreaLock = areaLock;
		}
		MapCheckpoint() {
		}
		
		void AddSpawn( MapSpawn@ spawn ) {
			m_Spawns.Add( @spawn );
		}
		void Activate( uint nLevelTime ) {
			if ( m_bPassed ) {
				return;
			}
			for ( uint i = 0; i < m_Spawns.Count(); i++ ) {
				m_Spawns[i].Activate(); 
			}
			m_Spawns.Clear();
			m_bPassed = true;
			m_nTime = TheNomad::GameSystem::GameManager.GetGameTic() - nLevelTime;
		}

		void Load( json@ data ) {
		}
		
		array<MapSpawn@> m_Spawns;
		uvec3 m_Origin = uvec3( 0 );
		uvec2 m_AreaLock = uvec2( 0 );
		uint m_nTime = 0;
		bool m_bPassed = false;
	};
};