#include "SGame/DisplayNotification.as"
#include "SGame/PlayerSystem/HudOverlay.as"

namespace TheNomad::SGame {
	class PlayerDisplayUI {
		PlayerDisplayUI() {
		}
		
		void Init( PlayrObject@ parent ) {
			@m_Parent = @parent;
			
			CacheUI();
		}
		
		private void CacheUI() {
			const vec2 screenSize = vec2( TheNomad::GameSystem::GPUConfig.screenWidth, TheNomad::GameSystem::GPUConfig.screenHeight );

			m_nStatusBarWidth = 350.0f * TheNomad::GameSystem::UIScale;
			m_nStatusBarHeight = 40.0f * TheNomad::GameSystem::UIScale;
			
			m_nStatusBarFontScale = 2.0f * TheNomad::GameSystem::UIScale;
			m_nStatusBarStretchAmount = 256.0f * TheNomad::GameSystem::UIScale;
			
			//
			// create overlays
			//

			m_DashScreen.Init( "gfx/hud/dash_screen", vec2( 0.0f ), screenSize );
			m_BloodScreenSplatter.Init( "gfx/hud/blood_screen", vec2( 0.0f ), screenSize );
			m_BulletTimeBlurScreen.Init( "gfx/hud/bullet_time_blur", vec2( 0.0f ), screenSize );
		}
		
		private void DrawLoadoutStatus() {
			ImGui::Begin( "##LoadoutInventoryStatus", null,  ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoScrollbar ) );
			
			ImGui::SetWindowPos( vec2( 940.0f * TheNomad::GameSystem::UIScale, 580.0f * TheNomad::GameSystem::UIScale ) );
			ImGui::SetWindowSize( vec2( 300.0f * TheNomad::GameSystem::UIScale, 120.0f * TheNomad::GameSystem::UIScale ) );
			
			// draw arm usage
			const int handsUsed = m_Parent.GetHandsUsed();
			ImGui::RadioButton( "##HandsUsedLeft", handsUsed == LEFT_ARM || handsUsed == BOTH_ARMS );
			ImGui::SameLine();
			ImGui::RadioButton( "##HandsUsedRight", handsUsed == RIGHT_ARM || handsUsed == BOTH_ARMS );

			// draw weapon icon
			const WeaponObject@ weapon = m_Parent.GetCurrentWeapon();
			if ( weapon !is null ) {
				ImGui::Image( weapon.GetWeaponInfo().hIconShader,
					vec2( 950.0f * TheNomad::GameSystem::UIScale, 600.0f * TheNomad::GameSystem::UIScale ),
					vec2( 128.0f * TheNomad::GameSystem::UIScale, 128.0f * TheNomad::GameSystem::UIScale ) );
			} else {
				ImGui::Text( "NOTHING EQUIPPED" );
			}

			ImGui::End();
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

		private void DrawHealthStatus() {
			const float health = m_Parent.GetHealth();

			ImGui::Begin( "##HealthBarFilled", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowPos( vec2( 0.0f, 0.0f ) );
			ImGui::SetWindowSize( vec2( m_nStatusBarWidth, m_nStatusBarHeight ) );
			ImGui::SameLine( m_nStatusBarStretchAmount );
			ImGui::SetWindowFontScale( m_nStatusBarFontScale );
			ImGui::Text( "HEALTH" );
			ImGui::SetWindowFontScale( 1.0f );
			ImGui::End();
			
			{
				vec4 healthColor;
				if ( health < 60.0f && health > 35.0f ) {
					healthColor =  colorYellow;
				} else if ( health >= 10.0f && health <= 35.0f ) {
					healthColor = colorRed;	
				} else if ( health < 10.0f ) {
					healthColor = vec4( 0.5f, 0.35f, 0.05f, 1.0f ); // brown
				} else {
					healthColor = colorGreen;
				}
				
				ImGui::PushStyleColor( ImGuiCol::WindowBg, healthColor );
				ImGui::Begin( "##HealthBar", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
					| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoTitleBar
					| ImGuiWindowFlags::NoScrollbar ) );
				ImGui::SetWindowPos( vec2( 0.0f, 0.0f ) );
				ImGui::SetWindowSize( vec2( ( float( ( 250.0f * ( health * 0.01f ) ) * TheNomad::GameSystem::UIScale ) ), m_nStatusBarHeight ) );
				ImGui::End();
				ImGui::PopStyleColor( 1 );
				if ( health < 30.0f ) {
					m_Shake.Start( 2000, 20.5f, 20.5f );
					Util::HapticRumble( 0, 0.20f, 100 );
					
					// draw a red overlay if we're really low on health
					m_BloodScreenSplatter.Draw();
				}
			}

			if ( TheNomad::GameSystem::GameTic > m_nHealthBarEndTime ) {
				m_nHealthBarEndTime = 0;
			}
		}

		private void DrawRageStatus() {
			const float rage = m_Parent.GetRage();

			ImGui::Begin( "##RageBarFilled", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowPos( vec2( 0.0f, 44.0f * TheNomad::GameSystem::UIScale ) );
			ImGui::SetWindowSize( vec2( m_nStatusBarWidth, m_nStatusBarHeight ) );
			ImGui::SetWindowFontScale( m_nStatusBarFontScale );
			ImGui::SameLine( m_nStatusBarStretchAmount );
			ImGui::Text( "RAGE" );
			ImGui::SetWindowFontScale( 1.0f );
			ImGui::End();

			{
				ImGui::PushStyleColor( ImGuiCol::WindowBg, vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
				ImGui::Begin( "##RageBar", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
					| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoTitleBar
					| ImGuiWindowFlags::NoScrollbar ) );
				ImGui::SetWindowPos( vec2( 0.0f, 44.0f * TheNomad::GameSystem::UIScale ) );
				ImGui::SetWindowSize( vec2( ( float( ( 250 * ( rage * 0.01f ) ) * TheNomad::GameSystem::UIScale ) ), m_nStatusBarHeight ) );
				ImGui::End();
				ImGui::PopStyleColor( 1 );
			}

			if ( TheNomad::GameSystem::GameTic > m_nRageBarEndTime ) {
				m_nRageBarEndTime = 0;
			}
		}
		
		void Draw() {
			m_Shake.OnRunTic();

			if ( m_Tutorial !is null ) {
				m_Tutorial.Draw();
			}

			if ( ( m_Parent.Flags & PF_DASHING ) != 0 )	{
				m_DashScreen.Draw();
			}
			if ( ( m_Parent.Flags & PF_REFLEX ) != 0 ) {
				m_BulletTimeBlurScreen.Draw();
			}
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_ToggleHUD" ) == 0 ) {
				return; // don't draw it
			}
			
			TheNomad::Engine::UserInterface::SetActiveFont( TheNomad::Engine::UserInterface::Font_RobotoMono );
			
			if ( m_nHealthBarEndTime != 0 ) {
				DrawHealthStatus();
			}
			if ( m_nRageBarEndTime != 0 ) {
				DrawRageStatus();
			}
			DrawLoadoutStatus();
			DrawMouseReticle();
		}
		
		void ShowStatusBars() {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_ToggleHUD" ) == 0 ) {
				m_nHealthBarEndTime = 0;
				m_nRageBarEndTime = 0;
				return;
			}
			m_nHealthBarEndTime = TheNomad::GameSystem::GameTic + 5000;
			m_nRageBarEndTime = TheNomad::GameSystem::GameTic + 5000;
		}
		void ShowRageBar() {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_ToggleHUD" ) == 0 ) {
				m_nRageBarEndTime = 0;
				return;
			}
			m_nRageBarEndTime = TheNomad::GameSystem::GameTic + 5000;
		}
		void ShowHealthBar() {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_ToggleHUD" ) == 0 ) {
				m_nHealthBarEndTime = 0;
				return;
			}
			m_nHealthBarEndTime = TheNomad::GameSystem::GameTic + 5000;
		}
		bool InCombat() const {
			return TheNomad::GameSystem::GameTic < m_nHealthBarEndTime;
		}
		void SetTutorial( itemlib::Script::Tutorial@ popup ) {
			@m_Tutorial = popup;
		}
		
		private PlayrObject@ m_Parent = null;
		private itemlib::Script::Tutorial@ m_Tutorial = null;
		
		// cached ui values
		private float m_nStatusBarWidth = 0.0f;
		private float m_nStatusBarHeight = 0.0f;
		private float m_nStatusBarFontScale = 0.0f;
		private float m_nStatusBarStretchAmount = 0.0f;
		
		private float m_nReflexAmount = 0.0f;

		private uint m_nHealthBarEndTime = 0;
		private uint m_nRageBarEndTime = 0;
		private uint m_nWeaponStatusEndTime = 0;
		
		private HudOverlay m_BloodScreenSplatter;
		private HudOverlay m_ParryScreenFlash;
		private HudOverlay m_DashScreen;
		private HudOverlay m_BulletTimeBlurScreen;

		ScreenShake m_Shake;
	};
};