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

		void InitEntity() {
			@m_Entity = @EntityManager.Spawn( TheNomad::GameSystem::EntityType::Wall, -1,
				vec3( m_Origin.x, m_Origin.y, m_Origin.z ), vec2( 0.0f )
			);
			m_Entity.EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/bonfire" ), 1.0f, 0xff );
		}

		void Draw() {
			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = vec3( m_Origin.x, m_Origin.y, m_Origin.z );
			refEntity.scale = vec2( 1.75f );
			if ( !m_bPassed ) {
				m_Animation.Run();
				refEntity.sheetNum = TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/checkpoint", 128, 32, 32, 32 ).GetShader();
				refEntity.spriteId = m_Animation.GetFrame();
			} else {
				refEntity.sheetNum = -1;
				refEntity.spriteId = TheNomad::Engine::Renderer::RegisterShader( "gfx/completed_checkpoint" );
			}

			refEntity.Draw();
		}

		void Load( json@ data ) {
		}

		array<MapSpawn@> m_Spawns;
		Animation m_Animation = Animation( 150, false, 4, false );
		EntityObject@ m_Entity = null;
		uvec3 m_Origin = uvec3( 0 );
		uvec2 m_AreaLock = uvec2( 0 );
		uint m_nTime = 0;
		bool m_bPassed = false;
	};
};