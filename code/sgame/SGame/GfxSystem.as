#include "Engine/Renderer/LocalEntity.as"
#include "Engine/Renderer/Particle.as"

namespace TheNomad::SGame {
	class GfxSystem : TheNomad::GameSystem::GameObject {
		GfxSystem() {
		}

		void OnInit() {
		}
		void OnShutdown() {
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
			InitLocalEntities();
		}
		void OnPlayerDeath( int ) {
		}
		void OnCheckpointPassed( uint ) {
		}
		void OnLevelStart() {
			InitLocalEntities();
		}
		void OnLevelEnd() {
			ClearLocalEntities();
		}
		const string& GetName() const override {
			return "GfxManager";
		}
		void OnRenderScene() {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_EnableParticles" ) == 0 ) {
				return;
			}

			TheNomad::Engine::ProfileBlock block( "GfxSystem::OnRenderScene" );

			TheNomad::Engine::Renderer::LocalEntity@ next = null;

			// walk the list backwards, so any new local entities generated
			// (trails, marks, etc) will be present this frame
			TheNomad::Engine::Renderer::LocalEntity@ ent = @m_ActiveLocalEnts.m_Prev;
			for ( ; @ent !is @m_ActiveLocalEnts; @ent = @next ) {
				// grab next now, so if the local entity is freed we
				// still have it
				@next = @ent.m_Prev;

				if ( TheNomad::GameSystem::GameManager.GetGameTic() - ent.m_nStartTime >= ent.m_nLifeTime ) {
					FreeLocalEntity( @ent );
					continue;
				}

				ent.RunTic();
			}
		}

		private void ClearLocalEntities() {
			@m_ActiveLocalEnts.m_Next =
			@m_ActiveLocalEnts.m_Prev =
				@m_ActiveLocalEnts;
			@m_FreeLocalEnts = null;

			// clear all references
			for ( uint i = 0; i < m_LocalEnts.Count(); i++ ) {
				@m_LocalEnts[i].m_Next =
				@m_LocalEnts[i].m_Prev =
					null;
			}
			m_LocalEnts.Clear();
		}

		private void InitLocalEntities() {
			uint maxGfx = 256;

			switch ( sgame_GfxDetail.GetInt() ) {
			case 0:
				maxGfx = 128;
				break;
			case 1:
				maxGfx = 256;
				break;
			case 2:
				maxGfx = 512;
				break;
			default:
				ConsoleWarning( "invalid sgame_GfxDetail '" + sgame_GfxDetail.GetInt() + "', setting to 1\n" );
				TheNomad::Engine::CvarSet( "sgame_GfxDetail", "1" );
				maxGfx = 256;
				break;
			};
			// only resize if we're changing qualities
			if ( maxGfx != m_LocalEnts.Count() ) {
				m_LocalEnts.Resize( maxGfx );
			} else {
				return;
			}

			@m_ActiveLocalEnts.m_Next =
			@m_ActiveLocalEnts.m_Prev =
				@m_ActiveLocalEnts;
			@m_FreeLocalEnts = @m_LocalEnts[0];

			for ( uint i = 0; i < m_LocalEnts.Count() - 1; i++ ) {
				@m_LocalEnts[i].m_Next = @m_LocalEnts[ i + 1 ];
			}
		}

		private TheNomad::Engine::Renderer::LocalEntity@ AllocLocalEntity() {
			if ( @m_FreeLocalEnts is null ) {
				// no free polys, so free the one at the end of the chain
				// remove the oldest active entity
				FreeLocalEntity( @m_ActiveLocalEnts.m_Prev );
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = @m_FreeLocalEnts;
			@m_FreeLocalEnts = @m_FreeLocalEnts.m_Next;

			// link into active list
			@ent.m_Next = @m_ActiveLocalEnts.m_Next;
			@ent.m_Prev = @m_ActiveLocalEnts;
			@m_ActiveLocalEnts.m_Next.m_Prev = @ent;
			@m_ActiveLocalEnts.m_Next = @ent;

			ent.m_nStartTime = TheNomad::GameSystem::GameManager.GetGameTic();

			return @ent;
		}
		//
		// FreeLocalEntity: only LocalEntity when its finished should ever call this, or AllocLocalEntity
		//
		private void FreeLocalEntity( TheNomad::Engine::Renderer::LocalEntity@ ent ) {
			if ( @ent.m_Prev is null ) {
				GameError( "GfxManager::FreeLocalEntity: not active" );
			}

			// remove from doubly linked list
			@ent.m_Prev.m_Next = @ent.m_Next;
			@ent.m_Next.m_Prev = @ent.m_Prev;

			// the free list is only singly linked
			@ent.m_Next = @m_FreeLocalEnts;
			@m_FreeLocalEnts = @ent;
		}

		void AddBloodSplatter( const vec3& in origin, int facing ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = @AllocLocalEntity();

			const uint row = ( Util::PRandom() & 3 ) + 2;

			vec2 scale = vec2( 1.0f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}
//			vec3 accel = vec3( 0.1f, 0.4f, 1.8f );

			ent.Spawn( origin, vec3( 0.0f ), 180, FS_INVALID_HANDLE,
				scale, true, 0.0f,
				@TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/spurt", 1540, 836, 110, 92 ),
				uvec2( 0, row ) );
			
			ent.m_EffectAnimation.Load( 30, false, 14, false );

			/*
			ent.Spawn( origin, vec3( 0.0f ), 1200, FS_INVALID_HANDLE,
				scale, false, 0.0f, @TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/splatter", 750, 900, 150, 150 ),
				uvec2( 0 ) );
			
			ent.m_EffectAnimation.Load( 40, false, 30, false );
			*/
			/*
			ent.Spawn( origin, vec3( 0.0f ), 800, FS_INVALID_HANDLE,
				scale, false, 0.0f,
				@TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/spurt", 1540, 837, 110, 93 ),
				uvec2( 0, ( Util::PRandom() & 3 ) + 2 ) );
			
			ent.m_EffectAnimation.Load( 50, false, 14, false );
			*/
		}

		void SmokeCloud( const vec3& in origin ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			AllocLocalEntity().Spawn( origin, vec3( 0.0f ), 5000, TheNomad::Engine::Renderer::RegisterShader( "gfx/env/dustScreen" ) );
		}

		void FlameBall( const vec3& in origin ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = @AllocLocalEntity();

			ent.Spawn( origin, vec3( 0.0f ), 500, FS_INVALID_HANDLE,
				vec2( 1.5f ), false, 0.0f,
				@TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/flameBall", 288, 192, 96, 48 ) );
			
			ent.m_EffectAnimation.Load( 20, false, 10, false );
		}

		void AddWaterWake( const vec3& in origin, uint lifeTime = 1200, float scale = 2.5f, float grow = 0.05f ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			AllocLocalEntity().Spawn( origin, vec3( 0.0f ), lifeTime, TheNomad::Engine::Renderer::RegisterShader( "wake" ),
				scale, false, grow );
		}

		void AddBulletHole( const vec3& in origin ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}
		}

		void AddDustPuff( const vec3& in origin, int facing ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = @AllocLocalEntity();

			vec2 scale = vec2( 2.5f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}
			
			ent.Spawn( origin, vec3( 0.0f ), 400, FS_INVALID_HANDLE,
				scale, false, 0.0f,
				@TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/smokePuff", 576, 64, 64, 64 ) );
			
			ent.m_EffectAnimation.Load( 90, false, 9, false );
		}

		void AddDustTrail( const vec3& in origin, int facing ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = @AllocLocalEntity();

			vec2 scale = vec2( 5.5f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}
			
			ent.Spawn( origin, vec3( 0.0f ), 1220, FS_INVALID_HANDLE,
				scale, false, 0.0f,
				@TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/env/smokeTrail", 750, 1200, 150, 150 ) );
			
			ent.m_EffectAnimation.Load( 20, false, 40, false );
		}
		
		void AddDustCloud( const vec3& in origin, const vec3& in accel ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			for ( uint i = 0; i < 24; i++ ) {
				float offsetX = 0.0f;
				if ( ( Util::PRandom() & 1 ) == 0 ) {
					const uint rand = Util::PRandom() & 3;
					offsetX += 2.0f / ( rand == 0 ? 1.0f : float( rand ) );
//					offsetX = -( accel.x * ( 1.0f / ( rand == 0 ? 1.0f : float( rand ) ) ) );
				} else {
					const uint rand = Util::PRandom() & 3;
					offsetX += rand;
//					offsetX = ( accel.x * ( 1.0f / ( rand == 0 ? 1.0f : float( rand ) ) ) );
				}

				float offsetY = 0.0f;
				if ( ( Util::PRandom() & 1 ) == 0 ) {
					const uint rand = Util::PRandom() & 3;
					offsetY -= 2.0f / ( rand == 0 ? 1.0f : float( rand ) );
//					offsetY = -accel.y + ( 1.0f / ( rand == 0 ? 1.0f : float( rand ) ) );
				} else {
					const uint rand = Util::PRandom() & 3;
					offsetY += 2.0f / ( rand == 0 ? 1.0f : float( rand ) );
//					offsetY = ( accel.y * ( 1.0f / ( rand == 0 ? 1.0f : float( rand ) ) ) );
				}

				AllocLocalEntity().Spawn( vec3( origin.x + offsetX, origin.y + offsetY, origin.z ), vec3( 0.0f ), 1600,
					TheNomad::Engine::Renderer::RegisterShader( "gfx/env/smokePuff" ), 1.0f, true );
			}
		}

		void AddExplosionGfx( const vec3& in origin ) {
		}

		private array<TheNomad::Engine::Renderer::LocalEntity> m_LocalEnts;
		private TheNomad::Engine::Renderer::LocalEntity m_ActiveLocalEnts;
		private TheNomad::Engine::Renderer::LocalEntity@ m_FreeLocalEnts = null;
	};
	
	GfxSystem@ GfxManager = null;
};