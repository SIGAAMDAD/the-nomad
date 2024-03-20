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
		
		private const vec4& GetRageColor( float rage ) const {
			return ( rage >= 70.0f ) ? TheNomad::GameSystem::ColorMagenta : TheNomad::GameSystem::ColorRed;
		}
		
		private const vec4& GetHealthColor( float health ) const {
			if ( health < 60.0f && health > 35.0f ) {
				return TheNomad::GameSystem::ColorYellow;
			} else if ( health < 10.0f ) {
				return vec4( 0.5f, 0.35f, 0.05f, 1.0f ); // brown
			} else {
				return TheNoamd::GameSystem::ColorGreen;
			}
		}
		
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
			const WeaponObject@ weapon = @GetPLayerObject().GetCurrentWeapon();
			
			ImGui::SetCursorScreenPos( vec2( screenSize.x - ( 256.0f * scale ), screenSize.y - ( 16.0f * scale ) ) );
			
			DrawRect( ImGui::GetCursorScreenPos(), vec2( 32.0f, 32.0f ), weapon.GetIconShader(), @weapon.Sprite() );
		}
		
		private void DrawMouseReticle() const {
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const vec2 mousePos = TheNomad::GameSystem::GameManager.GetMousePos();
			
			DrawRect( mousePos, vec2( 16.0f, 16.0f ), m_MouseShader, @m_MouseSpriteSheet[m_MouseSpriteIndex] );
		}
		
		private void DrawStatusBars() const {
			const float health = GetPlayerObject().GetHealth();
			const float rage = GetPlayerObject().GetRage();
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			const vec2 screenSize = TheNomad::GameSystem::GameManager.GetGPUConfig().GetScreenSize();
			
			ImGui::SetCursorScreenPos( vec2( 16.0f * scale, 16.0f * scale ) );
			
			//
			// display health
			//
			const vec4& healthColor = GetHealthColor();
			ImGui::PushStyleColor( ImGuiCol_FrameBg, healthColor );
			ImGui::PushStyleColor( ImGuiCol_FrameBgActive, healthColor );
			ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, healthColor );
			ImGui::ProgressBar( health, 256 * scale, 64 * scale, "##DisplayPlayerHealth" );
			ImGui::PopStyleColor( 3 );
			
			//
			// display rage meter
			//
			const vec4& rageColor = GetRageColor();
			ImGui::PushStyleColor( ImGuiCol_FrameBg, rageColor );
			ImGui::PushStyleColor( ImGuiCol_FrameBgActive, rageColor );
			ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, rageColor );
			ImGui::ProgressBar( rage, 256 * scale, 64 * scale, "##DisplayPlayerRage" );
			ImGui::PopStyleColor( 3 );
		}
		
		void Draw() const {
			if ( sgame_ToggleHUD.GetInt() == 0 ) {
				return; // don't draw it
			}
			const ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove |
				ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMouseInputs | ImGuiWindowFlags::NoBackground |
				ImGuiWindowFlags::NoBringToFrontOnFocus );
			const vec2 screenSize = TheNomad::GameSystem::GameManager.GetGPUConfig().GetScreenSize();
			TheNomad::Engine::Renderer::RenderSceneRef SceneRef;
			
			ImGui::Begin( "##PlayerHeadsUpDisplay", null windowFlags );
			
			ImGui::SetWindowSize( screenSize );
			ImGui::SetWindowPos( vec2( 0.0f, 0.0f ) );
			
			SceneRef.x = 0;
			SceneRef.y = 0;
			SceneRef.width = screenSize.x;
			SceneRef.height = screenSize.y;
			SceneRef.flags = RSF_ORTHO_TYPE_SCREENSPACE | RSF_NOWORLDMODEL;
			
			TheNomad::Engine::ClearScene();
			
			DrawStatusBars();
			DrawMouseReticle();
			DrawWeaponStatus();
			
			TheNomad::Engine::Renderer::RenderScene( SceneRef );
			
			ImGui::End();
		}
		
		private bool m_bChangedWeapon = false;
	};
	
	PlayerDisplayUI@ HUDManager;
};