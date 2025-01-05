#include "SGame/MapSpawn.as"

namespace TheNomad::SGame {
    class MapCheckpoint {
		MapCheckpoint( const uvec3& in origin, const uvec2& in areaLock, uint nIndex ) {
			m_Origin = origin;
			m_AreaLock = areaLock;
			m_nIndex = nIndex;
		}
		MapCheckpoint() {
		}
		~MapCheckpoint() {
			for ( uint i = 0; i < m_Spawns.Count(); ++i ) {
				@m_Spawns[i] = null;
			}
			EntityManager.RemoveEntity( @m_Entity );
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
			m_nTime = TheNomad::GameSystem::GameTic - nLevelTime;
		}

		void InitEntity() {
			@m_Entity = @EntityManager.Spawn( TheNomad::GameSystem::EntityType::Wall, WALL_CHECKPOINT_ID,
				vec3( m_Origin.x, m_Origin.y, m_Origin.z ), vec2( 0.0f )
			);
			m_Entity.GetBounds().m_nWidth = 1.0f;
			m_Entity.GetBounds().m_nHeight = 1.0f;
			m_Entity.GetBounds().MakeBounds( m_Entity.GetOrigin() );
			m_Entity.EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/bonfire" ), 0.01f, 0xff );
			cast<WallObject@>( @m_Entity ).SetData( @this );
		}

		array<MapSpawn@> m_Spawns;
		Animation m_Animation = Animation( 150, false, 4, false );
		EntityObject@ m_Entity = null;
		uvec3 m_Origin = uvec3( 0 );
		uvec2 m_AreaLock = uvec2( 0 );
		uint m_nTime = 0;
		uint m_nIndex = 0;
		bool m_bPassed = false;
	};
};