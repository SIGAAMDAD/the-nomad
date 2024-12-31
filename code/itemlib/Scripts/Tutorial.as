namespace itemlib::Script {
	const uint TUTORIAL_TYPE_TOOLTIP = 0;
	const uint TUTORIAL_TYPE_BINDING = 1;

	int GetKeyShader( uint nKeyNum ) {
		switch ( nKeyNum ) {
		case TheNomad::Engine::KeyNum::A: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_a" );
		case TheNomad::Engine::KeyNum::B: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_b" );
		case TheNomad::Engine::KeyNum::C: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_c" );
		case TheNomad::Engine::KeyNum::D: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_d" );
		case TheNomad::Engine::KeyNum::E: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_e" );
		case TheNomad::Engine::KeyNum::F: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_f" );
		case TheNomad::Engine::KeyNum::G: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_g" );
		case TheNomad::Engine::KeyNum::H: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_h" );
		case TheNomad::Engine::KeyNum::I: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_i" );
		case TheNomad::Engine::KeyNum::J: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_j" );
		case TheNomad::Engine::KeyNum::K: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_k" );
		case TheNomad::Engine::KeyNum::L: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_l" );
		case TheNomad::Engine::KeyNum::M: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_m" );
		case TheNomad::Engine::KeyNum::N: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_n" );
		case TheNomad::Engine::KeyNum::O: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_o" );
		case TheNomad::Engine::KeyNum::P: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_p" );
		case TheNomad::Engine::KeyNum::Q: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_q" );
		case TheNomad::Engine::KeyNum::R: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_r" );
		case TheNomad::Engine::KeyNum::S: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_s" );
		case TheNomad::Engine::KeyNum::T: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_t" );
		case TheNomad::Engine::KeyNum::U: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_u" );
		case TheNomad::Engine::KeyNum::V: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_v" );
		case TheNomad::Engine::KeyNum::W: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_w" );
		case TheNomad::Engine::KeyNum::X: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_x" );
		case TheNomad::Engine::KeyNum::Y: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_y" );
		case TheNomad::Engine::KeyNum::Z: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_z" );

		case TheNomad::Engine::KeyNum::Tab: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_tab" );
		case TheNomad::Engine::KeyNum::Space: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_space" );
		case TheNomad::Engine::KeyNum::BackSpace: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_backspace" );
		case TheNomad::Engine::KeyNum::Alt: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_alt" );
		case TheNomad::Engine::KeyNum::UpArrow: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_up" );
		case TheNomad::Engine::KeyNum::LeftArrow: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_left" );
		case TheNomad::Engine::KeyNum::RightArrow: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_right" );
		case TheNomad::Engine::KeyNum::DownArrow: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_down" );

		case TheNomad::Engine::KeyNum::BackSlash: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_backslash" );
		case TheNomad::Engine::KeyNum::Slash: return TheNomad::Engine::Renderer::RegisterShader( "icons/iconk_slash" );
		};
		return TheNomad::Engine::Renderer::RegisterShader( "white" );
	}

	class TutorialData {
		TutorialData() {
		}
		TutorialData( const string& in id, const string& in shaderName, const string& in text, uint type ) {
			Id = id;
			if ( type == TUTORIAL_TYPE_BINDING ) {
				Shader = GetKeyShader( TheNomad::Engine::KeyGetKey( shaderName ) );
			} else if ( type == TUTORIAL_TYPE_TOOLTIP ) {
				Shader = TheNomad::Engine::Renderer::RegisterShader( shaderName );
			}
			Text = text;
		}

		string Id;
		string Text;
		int Shader;
	};

	array<TutorialData> Tutorials;

	void LoadTutorialData() {
		array<json@> tutorials;
		json@ file = json();

		if ( !file.ParseFile( "modules/itemlib/DataScripts/tutorials.json" ) ) {
			GameError( "Couldn't load tutorials file!" );
		}
		if ( !file.get( "TutorialData", tutorials ) ) {
			GameError( "Couldn't load data from tutorials file!" );
		}
		@file = null;

		Tutorials.Reserve( tutorials.Count() );
		for ( uint i = 0; i < tutorials.Count(); ++i ) {
			json@ data = @tutorials[i];
			uint type;
			string typeString;
			string id;
			string shader;
			string text;

			data.get( "Type", typeString );
			data.get( "Id", id );
			data.get( "Shader", shader );
			data.get( "Text", text );

			if ( typeString == "binding" ) {
				type = TUTORIAL_TYPE_BINDING;
			} else if ( typeString == "tooltip" ) {
				type = TUTORIAL_TYPE_TOOLTIP;
			}

			Tutorials.Add( TutorialData( id, shader, text, type ) );
		}
	}

	class Tutorial : ItemScript {
		Tutorial() {
		}

		void Draw() const {
			TheNomad::SGame::PlayrObject@ player = @TheNomad::SGame::EntityManager.GetActivePlayer();

			ImGui::PushStyleColor( ImGuiCol::WindowBg, vec4( 0.25f, 0.25f, 0.25f, 0.5f ) );

			ImGui::Begin( "##TutorialPopupBackground", null, ImGui::MakeWindowFlags( ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoCollapse | ImGuiWindowFlags::NoTitleBar
				| ImGuiWindowFlags::NoScrollbar ) );
			
			ImGui::SetWindowPos( vec2( 0.0f, 480.0f * TheNomad::GameSystem::UIScale ) );
			ImGui::SetWindowSize( vec2( TheNomad::GameSystem::GPUConfig.screenWidth, TheNomad::GameSystem::GPUConfig.screenHeight ) );

			ImGui::Image( m_Data.Shader, vec2( 340.0f * TheNomad::GameSystem::UIScale, 500.0f * TheNomad::GameSystem::UIScale ),
				vec2( 72.0f * TheNomad::GameSystem::UIScale, 72.0f * TheNomad::GameSystem::UIScale ) );
			ImGui::SetWindowFontScale( 1.75f * TheNomad::GameSystem::UIScale );
			ImGui::SameLine();
			ImGui::Text( m_Data.Text );

			ImGui::NewLine();
			ImGui::SameLine( 500.0f * TheNomad::GameSystem::UIScale );
			if ( ImGui::Button( "OK" ) || TheNomad::Util::Distance( player.GetOrigin(), m_EntityData.GetOrigin() ) >= 6.0f ) {
				player.GetUI().SetTutorial( null );
			}

			ImGui::End();

			ImGui::PopStyleColor();
		}

		void OnEquip( TheNomad::SGame::EntityObject@ user ) override {
			if ( user.GetType() != TheNomad::GameSystem::EntityType::Playr ) {
				return;
			}
			TheNomad::SGame::EntityManager.GetActivePlayer().GetUI().SetTutorial( @this );
		}
		void OnDrop() override {
		}
		void OnUse( TheNomad::SGame::EntityObject@ user ) override {
		}
		void OnSpawn() override {
			// load the tutorial's text
			TheNomad::SGame::InfoSystem::ItemInfo@ info = @m_EntityData.GetItemInfo();

			for ( uint i = 0; i < Tutorials.Count(); ++i ) {
				if ( info.name == Tutorials[i].Id ) {
					@m_Data = @Tutorials[i];
					DebugPrint( "Tutorial \"" + info.name + "\" linked.\n" );
				}
			}
		}

		private TutorialData@ m_Data = null;
	};
};