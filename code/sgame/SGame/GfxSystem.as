#include "Engine/Renderer/LocalEntity.as"
#include "Engine/Renderer/Particle.as"

namespace TheNomad::SGame {
	const uint GFX_LOW_AMOUNT = 128;
	const uint GFX_MEDIUM_AMOUNT = 256;
	const uint GFX_HIGH_AMOUNT = 512;

	class GfxSystem : TheNomad::GameSystem::GameObject {
		GfxSystem() {
		}

		void OnInit() {
		}
		void OnShutdown() {
			m_LocalEnts.Clear();
		}
		void OnPlayerDeath( int ) {
		}
		void OnCheckpointPassed( uint ) {
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
//			InitLocalEntities();
		}
		void OnLevelStart() {
			InitLocalEntities();
			CacheGfx();
		}
		void OnLevelEnd() {
			ClearLocalEntities();
		}
		const string& GetName() const override {
			return "GfxManager";
		}
		void OnRenderScene() {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

		#if _NOMAD_DEBUG
			if ( sgame_DebugMode.GetBool() ) {
				TheNomad::Engine::ProfileBlock block( "GfxSystem::OnRenderScene" );
			}
		#endif

			TheNomad::Engine::Renderer::LocalEntity@ next = null;

			// walk the list backwards, so any new local entities generated
			// (trails, marks, etc) will be present this frame
			TheNomad::Engine::Renderer::LocalEntity@ ent = @m_ActiveLocalEnts.m_Prev;
			for ( ; ent !is @m_ActiveLocalEnts; @ent = next ) {
				// grab next now, so if the local entity is freed we
				// still have it
				@next = ent.m_Prev;

				uint endTime = ent.m_nEndTime;
				if ( !EntityManager.GetActivePlayer().InReflex() ) {
					endTime *= TheNomad::GameSystem::DeltaTic;
				}
				if ( TheNomad::GameSystem::GameDeltaTic > endTime ) {
					FreeLocalEntity( ent );
					continue;
				}

				ent.RunTic();
				@ent = null;
			}
			@ent = null;
		}

		private void ClearLocalEntities() {
			// clear all references
			for ( uint i = 0; i < m_LocalEnts.Count(); i++ ) {
				@m_LocalEnts[i].m_Next =
				@m_LocalEnts[i].m_Prev =
					null;
			}

			@m_ActiveLocalEnts.m_Next =
			@m_ActiveLocalEnts.m_Prev =
				null;
			@m_FreeLocalEnts = null;

			@m_SmokeTrail = null;
			@m_SmokePuff = null;
			@m_SmokeLanding = null;
			@m_BloodSpurt = null;
		}

		private void InitLocalEntities() {
			uint maxGfx = GFX_MEDIUM_AMOUNT;

			switch ( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) ) {
			case 0:
				maxGfx = GFX_LOW_AMOUNT;
				break;
			case 1:
				maxGfx = GFX_MEDIUM_AMOUNT;
				break;
			case 2:
				maxGfx = GFX_HIGH_AMOUNT;
				break;
			default:
				ConsoleWarning( "invalid sgame_GfxDetail '" + TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) + "', setting to 1\n" );
				TheNomad::Engine::CvarSet( "sgame_GfxDetail", "1" );
				maxGfx = GFX_MEDIUM_AMOUNT;
				break;
			};
			// only resize if we're changing qualities
			if ( !TheNomad::GameSystem::IsRespawnActive ) {
				if ( maxGfx != m_LocalEnts.Count() ) {
					ConsolePrint( "Initializing LocalEntity list...\n" );
					m_LocalEnts.Resize( maxGfx );
				} else {
					return;
				}
			}

			@m_ActiveLocalEnts.m_Next =
			@m_ActiveLocalEnts.m_Prev =
				@m_ActiveLocalEnts;
			@m_FreeLocalEnts = m_LocalEnts[0];

			for ( uint i = 0; i < m_LocalEnts.Count() - 1; i++ ) {
				@m_LocalEnts[i].m_Next = m_LocalEnts[ i + 1 ];
			}
		}

		private TheNomad::Engine::Renderer::LocalEntity@ AllocLocalEntity() {
			if ( m_FreeLocalEnts is null ) {
				// no free polys, so free the one at the end of the chain
				// remove the oldest active entity
				FreeLocalEntity( m_ActiveLocalEnts.m_Prev );
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = m_FreeLocalEnts;
			@m_FreeLocalEnts = m_FreeLocalEnts.m_Next;

			// link into active list
			@ent.m_Next = m_ActiveLocalEnts.m_Next;
			@ent.m_Prev = m_ActiveLocalEnts;
			@m_ActiveLocalEnts.m_Next.m_Prev = ent;
			@m_ActiveLocalEnts.m_Next = ent;

			return ent;
		}
		//
		// FreeLocalEntity: only LocalEntity when its finished should ever call this, or AllocLocalEntity
		//
		private void FreeLocalEntity( TheNomad::Engine::Renderer::LocalEntity@ ent ) {
			if ( ent.m_Prev is null ) {
				GameError( "GfxManager::FreeLocalEntity: not active" );
			}

			// remove from doubly linked list
			@ent.m_Prev.m_Next = ent.m_Next;
			@ent.m_Next.m_Prev = ent.m_Prev;

			// the free list is only singly linked
			@ent.m_Next = m_FreeLocalEnts;
			@m_FreeLocalEnts = ent;
		}

		void AddBloodSplatter( const vec3& in origin, int facing ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();

			const uint row = ( Util::PRandom() & 3 ) + 2;
			vec2 scale = vec2( 1.0f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}

			ent.Spawn( origin, vec3( 0.0f ), 180, FS_INVALID_HANDLE, scale, true, m_BloodSpurt, uvec2( 0, row ) );
			ent.m_EffectAnimation.Load( 30, false, 14, false );
		}

		void SmokeCloud( const vec3& in origin ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();
			ent.Spawn( origin, vec3( 0.0f ), 1800, m_hDustScreenShader );
		}

		//
		// AddDebrisCloud: we can force constant repositioning by clouding up the fighting area with a sizeable debris
		// cloud
		//
		void AddDebrisCloud( const vec3& in origin, float velocity ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}
			const uint numSmokeClouds = floor( velocity ) * 2;
			vec3 vel = vec3( 0.0f );
			TheNomad::Engine::Renderer::LocalEntity@ ent = null;

			for ( uint i = 0; i < numSmokeClouds; ++i ) {
				@ent = AllocLocalEntity();

				vel.x = ( Util::PRandom() & 25 ) * 0.001f;
				if ( ( Util::PRandom() & 1 ) == 0 ) {
					vel.x = -vel.x;
				}
				vel.y = ( Util::PRandom() & 25 ) * 0.001f;
				if ( ( Util::PRandom() & 1 ) == 0 ) {
					vel.y = -vel.y;
				}

				ent.Spawn( origin, vel, 7500, m_hDustScreenShader );
			}
		}

		void AddWaterWake( const vec3& in origin, uint lifeTime = 200, float scale = 2.5f ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();
			ent.Spawn( origin, vec3( 0.0f ), lifeTime, m_hWaterWakeShader, scale, false );
		}

		void AddBulletHole( const vec3& in origin ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();
			ent.Spawn( origin, vec3( 0.0f ), 50000, m_hBulletHoleShader, vec2( 2.0f ), false );
		}

		void AddLanding( const vec3& in origin ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();

			ent.Spawn( origin, vec3( 0.0f ), 600, FS_INVALID_HANDLE, vec2( 3.5f ), false, m_SmokeLanding );
			ent.m_EffectAnimation.Load( 40, false, 16, false );
		}

		void AddDustPuff( const vec3& in origin, int facing ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();

			vec2 scale = vec2( 2.5f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}
			
			ent.Spawn( origin, vec3( 0.0f ), 400, FS_INVALID_HANDLE, scale, false, m_SmokePuff );
			ent.m_EffectAnimation.Load( 90, false, 9, false );
		}

		void AddDustTrail( const vec3& in origin, int facing ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();

			vec2 scale = vec2( 5.5f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}
			
			ent.Spawn( origin, vec3( 0.0f ), 720, FS_INVALID_HANDLE, scale, false, m_SmokeTrail );
			ent.m_EffectAnimation.Load( 20, false, 40, false );
		}

		void AddExplosionGfx( const vec3& in origin ) {
		}

		void AddMuzzleFlash( const vec3& in origin ) {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 || TheNomad::GameSystem::IsRespawnActive ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = AllocLocalEntity();

			ent.Spawn( origin, vec3( 0.0f ), 200, FS_INVALID_HANDLE );
			ent.m_Flags |= TheNomad::Engine::Renderer::LOCALENT_NODRAW | TheNomad::Engine::Renderer::LOCALENT_LIGHT_SOURCE;
		}

		void CacheGfx() {
			// NOTE: don't mess with the load order
			m_hDustScreenShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/env/dustScreen" );
			m_hBulletHoleShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/env/bullet_hole" );
			@m_SmokeTrail = TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/smokeTrail", 750, 1200, 150, 150 );
			@m_SmokePuff = TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/smokePuff", 576, 64, 64, 64 );
			@m_SmokeLanding = TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/landing", 4032, 60, 252, 60 );
//			@m_FlameBall = @TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/flameBall", 288, 192, 96, 48 );
			@m_BloodSpurt = TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/spurt", 1540, 836, 110, 92 );
			m_hWaterWakeShader = TheNomad::Engine::Renderer::RegisterShader( "wake" );
		}

		// runtime cache
		private array<TheNomad::Engine::Renderer::LocalEntity> m_LocalEnts;
		private TheNomad::Engine::Renderer::LocalEntity m_ActiveLocalEnts;
		private TheNomad::Engine::Renderer::LocalEntity@ m_FreeLocalEnts = null;

		// data cache
		private SpriteSheet@ m_SmokeTrail = null;
		private SpriteSheet@ m_SmokePuff = null;
		private SpriteSheet@ m_SmokeLanding = null;
		private SpriteSheet@ m_BloodSpurt = null;
//		private SpriteSheet@ m_FlameBall = null;
		private int m_hWaterWakeShader = FS_INVALID_HANDLE;
		private int m_hDustScreenShader = FS_INVALID_HANDLE;
		private int m_hBulletHoleShader = FS_INVALID_HANDLE;
	};
	
	GfxSystem@ GfxManager = null;
};