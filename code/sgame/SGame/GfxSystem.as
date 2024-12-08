#include "Engine/Renderer/LocalEntity.as"
#include "Engine/Renderer/Particle.as"

namespace TheNomad::SGame {
	class GfxSystem : TheNomad::GameSystem::GameObject {
		GfxSystem() {
		}

		void OnInit() {
			InitLocalEntities();
		}
		void OnShutdown() {
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
		}
		void OnPlayerDeath( int ) {
		}
		void OnCheckpointPassed( uint ) {
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
			m_LocalEnts.Clear();
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
			TheNomad::Engine::Renderer::LocalEntity@ ent = null;

			// walk the list backwards, so any new local entities generated
			// (trails, marks, etc) will be present this frame
			@ent = @m_ActiveLocalEnts.m_Prev;
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

		private void InitLocalEntities() {
			const uint numGfx = 512;
			m_LocalEnts.Resize( numGfx );

			@m_ActiveLocalEnts.m_Next = @m_ActiveLocalEnts;
			@m_ActiveLocalEnts.m_Prev = @m_ActiveLocalEnts;
			@m_FreeLocalEnts = @m_LocalEnts[0];

			for ( uint i = 0; i < m_LocalEnts.Count() - 1; i++ ) {
				@m_LocalEnts[i].m_Next = @m_LocalEnts[ i + 1 ];
			}
		}

		private TheNomad::Engine::Renderer::LocalEntity@ AllocLocalEntity() {
			TheNomad::Engine::Renderer::LocalEntity@ ent = null;

			if ( @m_FreeLocalEnts is null ) {
				// no free polys, so free the one at the end of the chain
				// remove the oldest active entity
				FreeLocalEntity( @m_ActiveLocalEnts.m_Prev );
			}

			@ent = @m_FreeLocalEnts;
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


		//
		// GfxSystem::Bleed: this is a spurt of blood when an entity gets hit
		//
		TheNomad::Engine::Renderer::LocalEntity@ Bleed( const vec3& in origin ) {
			TheNomad::Engine::Renderer::LocalEntity@ ent = null;

			if ( sgame_Blood.GetInt() == 0 ) {
				return null;
			}

			float x, y;
			const uint randX = Util::PRandom();
			const uint randY = Util::PRandom();

			@ent = AllocLocalEntity();

			x = origin.x - ( 1.25f / ( randX == 0 ? 1 : randX ) );
			y = origin.y - ( 1.25f / ( randY == 0 ? 1 : randY ) );

			ent.m_nLifeTime = 1500;
			ent.m_Velocity = vec3( 0.05f, -0.03f, 0.0f );

			ent.m_Origin = vec3( x, y - 0.2f, 0.0f );
			ent.m_hShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/bloodSplatter0" );
			ent.m_bGravity = true;

			return @ent;
		}

		void AddBloodSplatter( const vec3& in origin, int facing ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			TheNomad::Engine::Renderer::LocalEntity@ ent = @AllocLocalEntity();

			vec2 scale = vec2( 1.0f );
			if ( facing == FACING_LEFT ) {
				scale.x = -scale.x;
			}
			vec3 accel = vec3( 0.1f, 0.4f, 1.8f );

			ent.Spawn( origin, accel, 180, TheNomad::Engine::Renderer::RegisterShader( "gfx/bloodSplatter0" ),
				scale, true, 0.0f );

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

		void SmokePuff( const vec3& in origin, const vec3& in vel ) {
		}

		void AddWaterWake( const vec3& in origin, uint lifeTime = 1200, float scale = 2.5f, float grow = 0.05f ) {
			if ( !sgame_EnableParticles.GetBool() ) {
				return;
			}

			AllocLocalEntity().Spawn( origin, vec3( 0.0f ), lifeTime, TheNomad::Engine::Renderer::RegisterShader( "wake" ),
				scale, false, grow );
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
			
			ent.Spawn( origin, vec3( 0.0f ), 1250, FS_INVALID_HANDLE,
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

		private bool m_bAllowGfx = true;
	};
	
	GfxSystem@ GfxManager = null;
};