namespace TheNomad::SGame {
	//
	// HudOverlay: a hud element that doesn't use a spritesheet, most usually a fullscreen opaque image
	//
	class HudOverlay {
		HudOverlay() {
		}

		void Init( const string& in shader, const vec2& in origin, const vec2& in size ) {
			m_WindowID = "##" + shader;
			m_Origin = origin;
			m_Size = size;
			m_hShader = TheNomad::Engine::Renderer::RegisterShader( shader );
		}
		
		void Draw() const {
			//TheNomad::Engine::Renderer::DrawImage( origin.x, origin.y, size.x, size.y, 0, 0, 1, 1, hShader );
			ImGui::Begin( m_WindowID, null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoBackground | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			ImGui::SetWindowSize( m_Size );
			ImGui::SetWindowPos( m_Origin );
			ImGui::Image( m_hShader, m_Origin, m_Size );
			ImGui::End();
		}

		private string m_WindowID;
		private vec2 m_Origin = vec2( 0.0f, 0.0f );
		private vec2 m_Size = vec2( 0.0f, 0.0f );
		private int m_hShader = FS_INVALID_HANDLE;
	};
};