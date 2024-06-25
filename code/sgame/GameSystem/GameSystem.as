#include "GameSystem/SaveSystem/LoadSection.as"
#include "GameSystem/SaveSystem/SaveSection.as"
#include "Engine/UserInterface/FontCache.as"
#include "Engine/ResourceCache.as"
#include "Engine/Renderer/RenderEntity.as"
#include "Engine/FileSystem/FileSystem.as"

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

    class CampaignManager : GameObject {
		CampaignManager() {
		}

		void OnInit() {
			m_nGameMsec = 0;
			m_nDeltaTics = 0;

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
		}
		void OnShutdown() {
		}

		bool OnConsoleCommand( const string& in cmd ) {
			return false;
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
		
		uint GetDeltaTics() const {
			return m_nDeltaTics;
		}
		uint GetGameTic() const {
			return m_nGameTic;
		}
		float GetUIScale() const {
			return m_nUIScale;
		}
		float GetUIBias() const {
			return m_nUIBias;
		}
		uint GetGameMsec() const {
			return m_nGameMsec;
		}
		void SetMsec( uint msec ) {
			m_nDeltaTics = msec - m_nGameTic;
			m_nGameTic = msec;
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

		void SetMousePos( const uvec2& in mousePos ) {
			m_MousePos = mousePos;
		}
		uvec2& GetMousePos() {
			return m_MousePos;
		}
		const uvec2& GetMousePos() const {
			return m_MousePos;
		}

		private TheNomad::Engine::Renderer::GPUConfig m_GPUConfig;

		// input
		private uvec2 m_MousePos = uvec2( 0 );
		private int m_JoystickSide = 0;
		private int m_JoystickForward = 0;
		private int m_JoystickUp = 0;
		private int m_JoystickRoll = 0;
		private int m_JoystickYaw = 0;
		private int m_JoystichPitch = 0;

		// timing
		private uint m_nDeltaTics = 0;
		private uint m_nGameMsec = 0;
		private uint m_nGameTic = 0;

		// rendering
		private float m_nUIBias = 0;
		private float m_nUIScale = 0;
	};

    array<GameObject@> GameSystems;

	GameObject@ AddSystem( GameObject@ SystemHandle ) {
		ConsolePrint( "Added GameObject System \"" + SystemHandle.GetName() + "\"\n" );
		GameSystems.Add( @SystemHandle );
		return @SystemHandle;
	}

	class RayCast {
		RayCast() {
		}

		void Cast() {
			CastRay( m_Start, m_End, m_Origin, m_nEntityNumber, m_nLength, m_nAngle, m_Flags );
		}

		vec3 m_Start = vec3( 0.0f );
		vec3 m_End = vec3( 0.0f );
		vec3 m_Origin = vec3( 0.0f );
	    uint32 m_nEntityNumber = 0;
		float m_nLength = 0.0f;
		float m_nAngle = 0.0f;
	    uint32 m_Flags = 0; // unused for now
	};

	CampaignManager@ GameManager = null;
};