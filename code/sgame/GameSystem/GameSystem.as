#include "GameSystem/GameObject.as"
#include "GameSystem/SaveSystem/LoadSection.as"
#include "GameSystem/SaveSystem/SaveSection.as"
#include "Engine/UserInterface/FontCache.as"
#include "Engine/ResourceCache.as"
#include "Engine/Renderer/RenderEntity.as"
#include "Engine/FileSystem/FileSystem.as"
#include "SGame/Cvars.as"

namespace TheNomad::GameSystem {
    array<GameObject@> GameSystems;

	GameObject@ AddSystem( GameObject@ SystemHandle ) {
		ConsolePrint( "Added GameObject System \"" + SystemHandle.GetName() + "\"\n" );
		GameSystems.Add( @SystemHandle );
		SystemHandle.OnInit();
		return @SystemHandle;
	}

	const uint RAYFLAG_WALLPIERCING = 0x0001;

	class RayCast {
		RayCast() {
		}

		void Cast( const vec3& in endDefault = vec3( 0.0f ), bool inDegress = false ) {
			vec3 end;
			if ( endDefault != Vec3Origin ) {
				end = endDefault;
			} else {
				end.x = m_Start.x + ( m_nLength * cos( m_nAngle ) );
				end.y = m_Start.y + ( m_nLength * sin( m_nAngle ) );

				end.z = m_Start.z * sin( m_nAngle );
			}
			const float dx = abs( end.x - m_Start.x );
			const float dy = abs( end.y - m_Start.y );
			const float sx = m_Start.x < end.x ? 0.5f : -0.5f;
			const float sy = m_Start.y < end.y ? 0.5f : -0.5f;
			float err = ( dx > dy ? dx : -dy ) * 0.5f;
			
			m_Origin = m_Start;
			float angle = m_nAngle;
			if ( m_nOwner2 != ENTITYNUM_INVALID ) {
				angle = Util::RAD2DEG( angle );
				if ( angle < 0.0f ) {
					angle += 360.0f;
				}
			}

			TheNomad::SGame::EntityObject@ activeEnts = @TheNomad::SGame::EntityManager.GetActiveEnts();
			TheNomad::SGame::EntityObject@ ent = null;
			for ( ;; ) {
				for ( @ent = @activeEnts.m_Next; @ent !is @activeEnts; @ent = @ent.m_Next ) {
					if ( ent.GetEntityNum() != m_nOwner && ent.GetEntityNum() != m_nOwner2 && ent.GetBounds().IntersectsPoint( m_Origin )
						&& !ent.CheckFlags( TheNomad::SGame::EntityFlags::Dead ) )
					{
						m_nEntityNumber = ent.GetEntityNum();
						return;
					}
				}
				if ( ( sy == -0.5f && m_Origin.y <= end.y ) || ( sy == 0.5f && m_Origin.y >= end.y )
					|| ( sx == -0.5f && m_Origin.x <= end.x || ( sx == 0.5f && m_Origin.x >= end.x ) ) )
				{
					m_nEntityNumber = ENTITYNUM_INVALID;
					return;
				}

				const TheNomad::GameSystem::DirType rayDir = Util::InverseDir( Util::Angle2Dir( angle ) );
				if ( TheNomad::GameSystem::CheckWallHit( m_Origin, rayDir ) ) {
					m_nEntityNumber = ENTITYNUM_WALL;
					return;
				}

				const float e2 = err;
				if ( e2 > -dx ) {
					err -= dy;
					m_Origin.x += sx;
				}
				if ( e2 < dy ) {
					err += dx;
					m_Origin.y += sy;
				}
			}
		}

		vec3 m_Start = vec3( 0.0f );
		vec3 m_Origin = vec3( 0.0f );
		uint m_nEntityNumber = ENTITYNUM_INVALID;
		uint m_nOwner = ENTITYNUM_INVALID;
		uint m_nOwner2 = ENTITYNUM_INVALID;
		float m_nLength = 0.0f;
		float m_nAngle = 0.0f;
//	    uint m_Flags = 0; // unused for now
	};

	// constants, its a lot faster just to keep these globals
	// I don't care about the whole OOP bullshit, this is
	// much faster and much less boilerplate-driven
	float TIMESTEP = 1.0f / 60.0f;
	bool IsLoadGameActive = false;
	bool IsRespawnActive = false;
 	uint GameTic = 0;
	float GameDeltaTic = 0.0f;
 	float DeltaTic = 0.0f;
	float ReflexTic = 0.0f;
	int HalfScreenWidth = 0;
	int HalfScreenHeight = 0;
	float UIScale = 0.0f;
	float UIBias = 0.0f;
	ivec2 MousePosition = ivec2( 0 );
	TheNomad::Engine::Renderer::GPUConfig GPUConfig;

	void Init() {
		// cache redundant calculations
		GetGPUGameConfig( GPUConfig );

		// for 1280x720 virtualized screen
		UIScale = GPUConfig.screenHeight * ( 1.0f / 720.0f );
		if ( GPUConfig.screenWidth * 720.0f > GPUConfig.screenHeight * 1280.0f ) {
			// wide screen
			UIBias = 0.5f * ( GPUConfig.screenWidth - ( GPUConfig.screenHeight * ( 1280.0f / 720.0f ) ) );
		} else {
			// no wide screen
			UIBias = 0.0f;
		}

		HalfScreenWidth = GPUConfig.screenWidth * 0.5f;
		HalfScreenHeight = GPUConfig.screenHeight * 0.5f;
	}

	void RespawnPlayer() {
		// clear out all the current entities, then respawn the player
		TheNomad::SGame::EntityManager.OnLevelEnd();
		TheNomad::SGame::EntityManager.OnLevelStart();
		TheNomad::SGame::GfxManager.Respawn();

		TheNomad::SGame::ScreenData.InitPlayers();
		TheNomad::SGame::ScreenData.GetPlayer().InitHUD();
	}
};