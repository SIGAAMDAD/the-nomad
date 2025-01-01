namespace TheNomad::Engine::UserInterface {
	class ConfigButtonVar : ConfigVar {
		ConfigButtonVar() {
		}
		ConfigButtonVar( const string& in name, const string& in id, bool bValue ) {
			m_bValue = bValue;

			super( name, id );
		}

		void Draw() {
			ImGui::TableNextColumn();

			ImGui::Text( m_Name );

			ImGui::SameLine();

			ImGui::TableNextColumn();

			if ( ImGui::RadioButton( ( m_bValue ? "YES" : "NO" ) + "##"  + m_Name + m_Id, m_bValue ) ) {
				m_bValue = !m_bValue;
			}

			ImGui::TableNextRow();
		}
		void Save() {
			TheNomad::Engine::CvarSet( m_Id, m_bValue ? "1" : "0" );
		}

		private bool m_bValue = false;
	};
};