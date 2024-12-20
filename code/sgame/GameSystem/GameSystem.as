#include "GameSystem/GameObject.as"
#include "GameSystem/SaveSystem/LoadSection.as"
#include "GameSystem/SaveSystem/SaveSection.as"
#include "Engine/UserInterface/FontCache.as"
#include "Engine/ResourceCache.as"
#include "Engine/Renderer/RenderEntity.as"
#include "Engine/FileSystem/FileSystem.as"
#include "SGame/Cvars.as"

namespace TheNomad::GameSystem {
	enum JoystickAxis {
		Side = 0,
		Forward,
		Up,
		Roll,
		Yaw,
		Pitch,

		MaxJoystickAxis
	};

	const float TIMESTEP = 1.0f / 60.0f;
	const uint SKIP_TICS = 1000.0f / 60.0f;
	const uint MAX_FRAMESKIP = 10;

    class CampaignManager : GameObject {
		CampaignManager() {
		}

		void OnInit() {
			// cache redundant calculations
			GetGPUGameConfig( m_GPUConfig );

			// for 1024x768 virtualized screen
			m_nUIScale = m_GPUConfig.screenHeight * ( 1.0f / 768.0f );
			if ( m_GPUConfig.screenWidth * 768.0f > m_GPUConfig.screenHeight * 1024.0f ) {
				// wide screen
				m_nUIBias = 0.5f * ( m_GPUConfig.screenWidth - ( m_GPUConfig.screenHeight * ( 1024.0f / 768.0f ) ) );
			} else {
				// no wide screen
				m_nUIBias = 0.0f;
			}

			m_nHalfScreenX = m_GPUConfig.screenWidth * 0.5f;
			m_nHalfScreenY = m_GPUConfig.screenHeight * 0.5f;
		}
		void OnShutdown() {
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
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		void OnRenderScene() {
		}
		const string& GetName() const {
			return "CampaignManager";
		}

		const ivec2 GetScreenSize() const {
			return ivec2( m_GPUConfig.screenWidth, m_GPUConfig.screenHeight );
		}

		int GetHalfScreenWidth() const {
			return m_nHalfScreenX;
		}
		int GetHalfScreenHeight() const {
			return m_nHalfScreenY;
		}
		float GetUIScale() const {
			return m_nUIScale;
		}
		float GetUIBias() const {
			return m_nUIBias;
		}
		void SetMsec( uint msec ) {
			DeltaTic = ( msec - GameTic ) * TIMESTEP;
			GameTic = msec;

			// if we want framerate dependant physics simply replace the time-step code above with the stuff under this comment
//			DeltaTic = msec;
		}

		TheNomad::Engine::Renderer::GPUConfig& GetGPUConfig() {
			return m_GPUConfig;
		}

		void SetJoystickAxis( int side, int forward, int up, int roll, int yaw, int pitch ) {
			m_JoystickSide = side;
		}
		int GetJoystickAxis( JoystickAxis axis ) const {
			switch ( axis ) {
			case JoystickAxis::Side: return m_JoystickSide;
			case JoystickAxis::Forward: return m_JoystickForward;
			case JoystickAxis::Up: return m_JoystickUp;
			case JoystickAxis::Roll: return m_JoystickRoll;
			case JoystickAxis::Yaw: return m_JoystickYaw;
			case JoystickAxis::Pitch: return m_JoystichPitch;
			default:
				GameError( "CampaignManager::GetJoystickAxis: bad axis " + uint( axis ) );
			};
			return 0;
		}

		void SetMousePos( const ivec2& in mousePos ) {
			m_MousePos = mousePos;
		}
		ivec2& GetMousePos() {
			return m_MousePos;
		}
		const ivec2& GetMousePos() const {
			return m_MousePos;
		}

		void SetLoadGame( bool bIsLoadActive ) {
			m_bIsLoadGameActive = bIsLoadActive;
		}
		bool IsLoadActive() const {
			return m_bIsLoadGameActive;
		}

		private TheNomad::Engine::Renderer::GPUConfig m_GPUConfig;

		private bool m_bIsLoadGameActive = false;

		// input
		private ivec2 m_MousePos = ivec2( 0 );
		private int m_JoystickSide = 0;
		private int m_JoystickForward = 0;
		private int m_JoystickUp = 0;
		private int m_JoystickRoll = 0;
		private int m_JoystickYaw = 0;
		private int m_JoystichPitch = 0;

		// rendering
		private float m_nUIBias = 0;
		private float m_nUIScale = 0;

		private int m_nHalfScreenX = 0;
		private int m_nHalfScreenY = 0;
	};

    array<GameObject@> GameSystems;

	GameObject@ AddSystem( GameObject@ SystemHandle ) {
		ConsolePrint( "Added GameObject System \"" + SystemHandle.GetName() + "\"\n" );
		GameSystems.Add( @SystemHandle );
		SystemHandle.OnInit();
		return @SystemHandle;
	}

	class RayCast {
		RayCast() {
		}

		void Cast() {
			CastRay( m_Start, m_Origin, m_nEntityNumber, m_nLength, m_nAngle, m_Flags );
		}

		vec3 m_Start = vec3( 0.0f );
		vec3 m_Origin = vec3( 0.0f );
	    uint m_nEntityNumber = 0;
		float m_nLength = 0.0f;
		float m_nAngle = 0.0f;
	    uint m_Flags = 0; // unused for now
	};

	CampaignManager@ GameManager = null;
	uint GameTic = 0;
	float DeltaTic = 0.0f;
};