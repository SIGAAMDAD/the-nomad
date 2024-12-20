#include "SGame/DisplayNotification.as"
#include "SGame/PlayerSystem/HudOverlay.as"

namespace TheNomad::SGame {
	class PlayerDisplayUI {
		PlayerDisplayUI() {
		}
		
		void Init( PlayrObject@ parent ) {
			const ivec2 screenSize = TheNomad::GameSystem::GameManager.GetScreenSize();

			@m_Parent = @parent;
			
			CacheUI();
		}
		
		private void CacheUI() {
			m_nStatusBarWidth = 350.0f * m_nUIScale;
			m_nStatusBarHeight = float( m_ScreenSize.y );
			
			m_nStatusBarFontScale = 2.0f * m_nUIScale;
			m_nStatusBarStretchAmount = 256.0f * m_nUIScale;
			
			//
			// create overlays
			//

			m_DashScreen.origin = vec2( 0.0f, 0.0f );
			m_DashScreen.size = vec2( m_ScreenSize.x, m_ScreenSize.y );
			m_DashScreen.hShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/dash_screen" );

			m_BloodScreenSplatter.origin = vec2( 0.0f, 0.0f );
			m_BloodScreenSplatter.size = vec2( m_ScreenSize.x, m_ScreenSize.y );
			m_BloodScreenSplatter.hShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/blood_screen" );
		}
		
		private const vec4 GetRageColor( float rage ) const {
			return ( rage >= 70.0f ) ? colorMagenta : colorRed;
		}
		
		private const vec4 GetHealthColor( float health ) const {
			if ( health < 60.0f && health > 35.0f ) {
				return colorYellow;
			} else if ( health >= 10.0f && health <= 35.0f ) {
				return colorRed;	
			} else if ( health < 10.0f ) {
				return vec4( 0.5f, 0.35f, 0.05f, 1.0f ); // brown
			} else {
				return colorGreen;
			}
		}
		
		private void DrawWeaponStatus() {
		}
		
		private void DrawMouseReticle() const {
		}

		private void DrawJumpKitStatus() const {
			ImGui::Begin( "##JumpKitStatus", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoScrollbar ) );
			
			ImGui::SetWindowPos( vec2() );
			ImGui::SetWindowSize( vec2() );

			ImGui::End();
		}

		private void DrawHealthBarFilled() const {
			ImGui::PushStyleColor( ImGuiCol::FrameBg, colorBlack );
			ImGui::PushStyleColor( ImGuiCol::FrameBgActive, colorBlack );
			ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, colorBlack );

			ImGui::SetWindowFontScale( m_nStatusBarFontScale );

			ImGui::PushStyleColor( ImGuiCol::Text, vec4( 0.0f ) );
			ImGui::DragFloat( "HEALTHFILLED", 0.0f );
			ImGui::PopStyleColor();

			ImGui::SameLine( m_nStatusBarStretchAmount );
			ImGui::Text( "HEALTH" );

			ImGui::SetWindowFontScale( 1.0f );

			ImGui::PopStyleColor( 3 );
		}

		private void DrawRageBarFilled() const {
			ImGui::PushStyleColor( ImGuiCol::FrameBg, colorBlack );
			ImGui::PushStyleColor( ImGuiCol::FrameBgActive, colorBlack );
			ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, colorBlack );

			ImGui::SetWindowFontScale( m_nStatusBarFontScale );

			ImGui::PushStyleColor( ImGuiCol::Text, vec4( 0.0f ) );
			ImGui::DragFloat( "RAGEFILLED", 0.0f );
			ImGui::PopStyleColor();

			ImGui::SameLine( m_nStatusBarStretchAmount );
			ImGui::Text( "RAGE" );

			ImGui::SetWindowFontScale( 1.0f );

			ImGui::PopStyleColor( 3 );
		}
		
		private void DrawStatusBars() {	
			const float rage = m_Parent.GetRage();
			const float health = m_Parent.GetHealth();
			
			ImGui::Begin( "##StatusBarsFilled", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowPos( vec2( 0.0f, 0.0f ) );
			ImGui::SetWindowSize( vec2( m_nStatusBarWidth, m_nStatusBarHeight ) );
			DrawHealthBarFilled();
			DrawRageBarFilled();
			ImGui::End();

			ImGui::Begin( "##HealthBar", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowPos( vec2( 0.0f, 0.0f ) );
			ImGui::SetWindowSize( vec2( ( float( ( 350.0f * ( health * 0.01f ) ) * m_nUIScale ) ), m_nStatusBarHeight ) );
			
			{
				if ( health < 30.0f ) {
					m_Shake.Start( 2000, 20.5f, 20.5f );
					Util::HapticRumble( m_Parent.GetPlayerIndex(), 0.20f, 100 );
					
					// draw a red overlay if we're really low on health
					m_BloodScreenSplatter.Draw();
				}

				const vec4 healthColor = GetHealthColor( health );

				ImGui::PushStyleColor( ImGuiCol::FrameBg, healthColor );
				ImGui::PushStyleColor( ImGuiCol::FrameBgActive, healthColor );
				ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, healthColor );

				ImGui::SetWindowFontScale( m_nStatusBarFontScale );

				ImGui::PushStyleColor( ImGuiCol::Text, vec4( 0.0f ) );
				ImGui::DragFloat( "HEALTH", 0.0f );
				ImGui::PopStyleColor();

				ImGui::SetWindowFontScale( 1.0f );

				ImGui::PopStyleColor( 3 );
			}
			
			ImGui::End();

			ImGui::Begin( "##RageBar", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowPos( vec2( 0.0f, 42.0f * m_nUIScale ) );
			ImGui::SetWindowSize( vec2( ( float( ( 350 * ( rage * 0.01f ) ) * m_nUIScale ) ), m_nStatusBarHeight ) );
			
			{
				ImGui::PushStyleColor( ImGuiCol::FrameBg, vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
				ImGui::PushStyleColor( ImGuiCol::FrameBgActive, vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
				ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );

				ImGui::SetWindowFontScale( m_nStatusBarFontScale );

				ImGui::PushStyleColor( ImGuiCol::Text, vec4( 0.0f ) );
				ImGui::DragFloat( "RAGE", 0.0f );
				ImGui::PopStyleColor();

				ImGui::SetWindowFontScale( 1.0f );

				ImGui::PopStyleColor( 3 );
			}
			
			ImGui::End();
			
			if ( TheNomad::GameSystem::GameTic - m_nStatusBarStartTime >= 5000 ) {
				m_nStatusBarStartTime = 0;
			}
		}
		
		void Draw() {
			if ( !sgame_ToggleHUD.GetBool() ) {
				return; // don't draw it
			}
			
			TheNomad::Engine::UserInterface::SetActiveFont( TheNomad::Engine::UserInterface::Font_RobotoMono );
			
			if ( m_nStatusBarStartTime != 0 ) {
				DrawStatusBars();
			}
			if ( m_nWeaponStatusStartTime != 0 ) {
				DrawWeaponStatus();
			}
			if ( m_nDashStartTime != 0 )	{
				m_DashScreen.Draw();
				if ( TheNomad::GameSystem::GameTic - m_nDashStartTime <= 1500 ) {
					m_nDashStartTime = 0;
				}
			}
			DrawMouseReticle();

			m_Shake.OnRunTic();
		}
		
		void ShowStatusBars() {
			if ( !sgame_ToggleHUD.GetBool() ) {
				m_nStatusBarStartTime = 0;
				return;
			}
			m_nStatusBarStartTime = TheNomad::GameSystem::GameTic;
		}
		void ShowDashMarks() {
			if ( !sgame_ToggleHUD.GetBool() ) {
				m_nDashStartTime = 0;
				return;
			}
			m_nDashStartTime = TheNomad::GameSystem::GameTic;
		}
		
		private PlayrObject@ m_Parent = null;
		
		// cached ui values
		private ivec2 m_ScreenSize = TheNomad::GameSystem::GameManager.GetScreenSize();
		private float m_nUIScale = TheNomad::GameSystem::GameManager.GetUIScale();
		private float m_nStatusBarWidth = 0.0f;
		private float m_nStatusBarHeight = 0.0f;
		private float m_nStatusBarFontScale = 0.0f;
		private float m_nStatusBarStretchAmount = 0.0f;
		
		private uint m_nStatusBarStartTime = 0;
		private uint m_nWeaponStatusStartTime = 0;
		private uint m_nDashStartTime = 0;
		
		private HudOverlay m_BloodScreenSplatter;
		private HudOverlay m_ParryScreenFlash;
		private HudOverlay m_DashScreen;

		private ScreenShake m_Shake;
	};
};