#include "spawn.as"

namespace TheNomad::SGame {
    shared class MapCheckpoint {
		MapCheckpoint( const uvec3& in origin ) {
			m_Origin = origin;
			m_bPassed = false;
		}
		MapCheckpoint() {
		}
		
		void AddSpawn( MapSpawn@ spawn ) {
			m_Spawns.push_back( spawn );
		}
		void Activate() {
			for ( uint i = 0; i < m_Spawns.size(); i++ ) {
				m_Spawns[i].Activate(); 
			}
		}
		
		uvec3 m_Origin;
		bool m_bPassed;
		array<MapSpawn@> m_Spawns;
	};
};