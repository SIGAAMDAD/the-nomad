namespace TheNomad::Engine::UserInterface {
	class ConfigButtonVar : ConfigVar {
		ConfigButtonVar() {
		}
		ConfigButtonVar( const string& in name, const string& in id, bool bValue ) {
			m_bValue = bValue;

			super( name, id );
		}

		void Draw() {
			ImGui::Text( m_Name );
			ImGui::SameLine();

			ImGui::TableNextRow();
		}
		void Save() {
		}

		private bool m_bValue = false;
	};
};