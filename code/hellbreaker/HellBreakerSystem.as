#include "nomadmain/GameSystem/GameSystem.as"

namespace hellbreaker {
	class HellBreakerSystem : TheNomad::GameSystem::GameObject {
		HellBreakerSystem() {
		}

		void OnLoad() {
			TheNomad::GameSystem::SaveSystem::LoadSection section( GetName() );
			if ( !section.Found() ) {
				return;
			}

			m_nWave = section.LoadUInt( "wave" );
		}
		void OnSave() const {
			TheNomad::GameSystem::SaveSystem::SaveSection section( GetName() );

			section.SaveUInt( "wave", m_nWave );
		}

		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		void OnCheckpointPassed( uint ) {
		}
		void OnPlayerDeath( int ) {
			m_nMessageStartTime = TheNomad::GameSystem::GameTic;
			m_nCurrentMessage = 0;

			ConsolePrint( "=================\n" );
			ConsolePrint( "" );
		}
		void OnInit() {
			json@ data;
			
			@data = json();
			if ( !data.ParseFile( "Config/nomadmain/hellbreaker.json" ) ) {
				ConsoleWarning( "Couldn't load hellbreaker config, using default values.\n" );
				return;
			}
		}
		void OnShutdown() {
		}
		void OnRunTic() {
		}
		void OnRenderScene() {
		}
		bool OnConsoleCommand( const string& in cmd ) {
			return true;
		}
		const string& GetName() const {
			return "Hellbreaker";
		}

		private void NextMessage( uint nDuration ) {
			if ( TheNomad::GameSystem::GameTic - m_nMessageStartTime >= nDuration ) {
				m_nCurrentMessage++;
			}
		}
		private void DisplayMessage() const {
			ImGui::Begin( "##HellbreakerInitMessage", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoMove | ImGuiWindowFlags::NoResize
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground ) );
//            ImGui::SetWindowPos( vec2() );
//            ImGui::SetWindowSize( vec2() );
//

			switch ( m_nCurrentMessage ) {
			case 0:
				ImGui::Text( "You're not dead yet..." );
				NextMessage( 1500 );
				break;
			case 1:
				ImGui::Text( "HELLBREAKER" );
				ImGui::Text( "Break your way out of hell, fight through the hordes of the fallen to get right back into the fight" );
				ImGui::Text( "This is your last chance... So take it!" );
				break;
			};

			ImGui::End();
		}

		//
		// display message data
		//
		private uint m_nCurrentMessage = 0;
		private uint m_nMessageStartTime = 0;

		private uint m_nWave = 0;
		private uint m_nWaveCount = 0;

		private array<TheNomad::SGame::MobObject@> m_DeadList;

		//
		// configuration
		//
		private bool m_bRevanents = false; // use dead enemies as hellbreaker mobs
		private uint m_nMaxWaves = 0;

		int m_nBackgroundShader = FS_INVALID_HANDLE;
	};

	HellBreakerSystem@ HellBreaker = null;
};