#include "SGame/MapSpawn.as"

namespace TheNomad::SGame {
    class MapCheckpoint {
		MapCheckpoint( const uvec3& in origin ) {
			m_Origin = origin;
		}
		MapCheckpoint() {
		}
		
		void AddSpawn( MapSpawn@ spawn ) {
			m_Spawns.Add( @spawn );
		}
		void Activate() {
			for ( uint i = 0; i < m_Spawns.Count(); i++ ) {
				m_Spawns[i].Activate(); 
			}
			m_bPassed = true;
		}
		
		uvec3 m_Origin = uvec3( 0 );
		bool m_bPassed = false;
		array<MapSpawn@> m_Spawns;
	};
};