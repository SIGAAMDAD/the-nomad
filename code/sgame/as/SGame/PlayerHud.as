namespace TheNomad::SGame {
	class DisplayNotification {
		void Draw() {
			
		}
	};
	class PlayerDisplayUI {
		PlayerDisplayUI() {
		}
		
		void NotifyChangedWeapon() {
			m_bChangedWeapon = true;
			
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
		/*
		
		private void DrawRect( const vec2& in pos, const vec2& in size, int hShader, const Sprite@ sprite ) const {
			TheNomad::Engine::Renderer::PolyVert[] verts( 4 );
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const float sizeX = size.x / 2;
			const float sizeY = size.y / 2;
			
			verts[0].xyz = vec3( pos.x + ( sizeX * scale ), pos.y, 0.0f );
			verts[1].xyz = vec3( pos.x + ( sizeX * scale ), pos.y + ( sizeY * scale ), 0.0f );
			verts[2].xyz = vec3( pos.x, pos.y + ( sizeY * scale ), 0.0f );
			verts[3].xyz = vec3( pos.x, pos.y, 0.0f );
			
			for ( uint i = 0; i < 4; i++ ) {
				verts[i].uv = sprite[i];
			}
			
			TheNomad::Engine::Renderer::AddPolyToScene( hShader, verts );
		}
		
		private void DrawWeaponStatus() const {
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const vec2 screenSize = TheNomad::GameSystem::GameManager.GetGPUConfig().GetScreenSize();
			const WeaponObject@ weapon = @GetPlayerObject().GetCurrentWeapon();
			
			ImGui::SetCursorScreenPos( vec2( screenSize.x - ( 256.0f * scale ), screenSize.y - ( 16.0f * scale ) ) );
			
			DrawRect( ImGui::GetCursorScreenPos(), vec2( 32.0f, 32.0f ), weapon.GetIconShader(), @weapon.Sprite() );
		}
		
		private void DrawMouseReticle() const {
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const vec2 mousePos = TheNomad::GameSystem::GameManager.GetMousePos();
			
			DrawRect( mousePos, vec2( 16.0f, 16.0f ), m_MouseShader, @m_MouseSpriteSheet[m_MouseSpriteIndex] );
		}
		*/
		
		private void DrawStatusBars() const {
			const float health = GetPlayerObject().GetHealth();
			const float rage = GetPlayerObject().GetRage();
//			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const vec2 screenSize = vec2( TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
				TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight );
//			const ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove |
//				ImGuiWindowFlags::NoMouseInputs | ImGuiWindowFlags::NoBackground |
//				ImGuiWindowFlags::NoBringToFrontOnFocus | ImGuiWindowFlags::AlwaysAutoResize );
			
//			ImGui::Begin( "DisplayPlayerHealth" );
//			ImGui::SetWindowSize( vec2( 256, 256 ) );
//			ImGui::SetWindowPos( vec2( 0.0f ) );
//			ImGui::SetWindowFontScale( 1.5f );
//			ImGui::Text( "HEALTH" );
			
			//
			// display health
			//
//			const vec4 healthColor = GetHealthColor( health );
//			ImGui::PushStyleColor( ImGuiCol::FrameBg, healthColor );
//			ImGui::PushStyleColor( ImGuiCol::FrameBgActive, healthColor );
//			ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, healthColor );
//			ImGui::Text( "HEALTH METER" );
//			ImGui::ProgressBar( health );
//			ImGui::PopStyleColor( 3 );

			ImGui::End();
			
			//
			// display rage meter
			//

//			ImGui::Begin( "##DisplayPlayerRage", null, windowFlags );
//			ImGui::SetWindowPos( vec2( 16.0f, 64.0f ) );

//			const vec4 rageColor = GetRageColor( rage );
//			ImGui::PushStyleColor( ImGuiCol::FrameBg, rageColor );
//			ImGui::PushStyleColor( ImGuiCol::FrameBgActive, rageColor );
//			ImGui::PushStyleColor( ImGuiCol::FrameBgHovered, rageColor );
//			ImGui::Text( "RAGE METER" );
//			ImGui::ProgressBar( rage );
//			ImGui::PopStyleColor( 3 );

//			ImGui::End();
		}
		
		void Draw() const {
			if ( sgame_ToggleHUD.GetInt() == 0 ) {
				return; // don't draw it
			}
			const vec2 screenSize = vec2( TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
				TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight );
			
			TheNomad::Engine::Renderer::ClearScene();
			
			DrawStatusBars();
//			DrawMouseReticle();
//			DrawWeaponStatus();
			
			TheNomad::Engine::Renderer::RenderScene( 0, 0, screenSize.x, screenSize.y, RSF_ORTHO_TYPE_SCREENSPACE | RSF_NOWORLDMODEL, 0 );
		}
		
		private bool m_bChangedWeapon = false;
	};
	
	PlayerDisplayUI@ HUDManager;
};