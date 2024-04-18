#include "SGame/DisplayNotification.as"
#include "SGame/HudOverlay.as"

namespace TheNomad::SGame {
	class PlayerDisplayUI {
		PlayerDisplayUI() {
			@m_WeaponIcons = SpriteSheet(  ); // TODO: THIS

			m_DisplayStr.reserve( 24 );
		}
		
		void Init( PlayrObject@ parent ) {
			@m_Parent = @parent;
			
			// init shaders
			m_BloodScreenSplatter.origin = vec2( 0.0f, 0.0f );
			m_BloodScreenSplatter.size = vec2( 0.0f, 0.0f );
			m_BloodScreenSplatter.hShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/blood_screen" );
			
			m_HealthBar.origin = vec2( 0.0f, 0.0f );
			m_HealthBar.size.y = 72.0f;
			m_HealthBar.color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
			m_HealthBar.hShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/health_bar" );
			
			m_HealthBarEmpty.origin = vec2( 0.0f, 0.0f );
//			m_HealthBarEmpty.hShader = TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/health_empty" );
		}
		
		private const vec4 GetRageColor( float rage ) const {
			return ( rage >= 70.0f ) ? colorMagenta : colorRed;
		}
		
		private const vec4 GetHealthColor( float health ) const {
			if ( health < 60.0f && health > 35.0f ) {
				return colorYellow;
			} else if ( health < 10.0f ) {
				return vec4( 0.5f, 0.35f, 0.05f, 1.0f ); // brown
			} else {
				return colorGreen;
			}
		}
		
		private void DrawWeaponStatus() const {
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const ivec2 screenSize = TheNomad::GameSystem::GameManager.GetScreenSize();
			const WeaponObject@ weapon;
			const SpriteSheet@ sheet;
			const vec2[]@ texCoords;
			int hShader;
			vec2 pos, size;
			
			@weapon = @m_Parent.GetCurrentWeapon();
			hShader = weapon.GetShader();

			return;

			size = weapon.GetSpriteSheet().GetSpriteSize();
			pos.x = ( screenSize.x - 8 ) - ( 256 * scale );
			pos.y = ( screenSize.y - 8 ) - ( 72 * scale );

			@sheet = @weapon.GetSpriteSheet();
			if ( @sheet !is null ) {
				@texCoords = @sheet[ weapon.GetSpriteIndex() ].GetTexCoords();
				TheNomad::Engine::Renderer::DrawImage( pos.x, pos.y, size.x, size.y,
					texCoords[0][0], texCoords[0][1], texCoords[2][0], texCoords[2][1],
					hShader );
			}
			else {
				// standalone shader
				TheNomad::Engine::Renderer::DrawImage( pos.x, pos.y, size.x, size.y, 0, 0, 1, 1, hShader );
			}
		}
		
		private void DrawMouseReticle() const {
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const ivec2 screenSize = TheNomad::GameSystem::GameManager.GetScreenSize();
			int hShader;
			
		}
		
		private void DrawHealthBar() {
			int hShader;
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const ivec2 screenSize = TheNomad::GameSystem::GameManager.GetScreenSize();
			const float health = m_Parent.GetHealth();
			
			if ( health < 10.0f ) {
				// draw a red overlay if we're really low on health
				m_BloodScreenSplatter.Draw();
			}

			if ( Engine::CvarVariableInteger( "g_paused" ) == 1 ) {
				return;
			}

			m_HealthBar.size.x = ( 256 * ( health * 0.01f ) ) * scale;
			
			ImGui::PushStyleColor( ImGuiCol::FrameBg, vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol::FrameBgActive, vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );

			ImGui::Begin( "##HealthBar", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowPos( m_HealthBar.origin );
			ImGui::SetWindowSize( vec2( 500 * scale, 42 * scale ) );
			ImGui::SetWindowFontScale( 2.0f * scale );
			ImGui::ProgressBar( health, vec2( 350 * scale, 26 * scale ) );
			ImGui::SameLine();
			m_DisplayStr = formatFloat( health, "", 4, 2 );
			m_DisplayStr += "%";
			ImGui::Text( m_DisplayStr );
			ImGui::SetWindowFontScale( 1.0f );
			ImGui::End();

			ImGui::PopStyleColor( 3 );
		}

		private void DrawRageBar() {
			int hShader;
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const ivec2 screenSize = TheNomad::GameSystem::GameManager.GetScreenSize();
			const float rage = m_Parent.GetRage();


		}
		
		private void DrawStatusBars() const {
			DrawHealthBar();
		}
		
		void Draw() const {
			if ( sgame_ToggleHUD.GetInt() == 0 ) {
				return; // don't draw it
			}
			
			uint time;
			const vec2 screenSize = vec2( TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
				TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight );
			
			
			
			TheNomad::Engine::Renderer::ClearScene();
			
			DrawStatusBars();
			DrawMouseReticle();
			DrawWeaponStatus();
			
			TheNomad::Engine::Renderer::RenderScene( 0, 0, uint( screenSize.x ), uint( screenSize.y ),
				RSF_ORTHO_TYPE_SCREENSPACE | RSF_NOWORLDMODEL, 0 );
		}

		private HudOverlay m_HealthBar;
		private HudOverlay m_HealthBarEmpty;
		private HudOverlay m_BloodScreenSplatter;
		
		private string m_DisplayStr;
		
		private SpriteSheet@ m_WeaponIcons = null;
		private PlayrObject@ m_Parent = null;
	};
};