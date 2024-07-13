namespace TheNomad::Engine::UserInterface {
    class ConfigButtonVar : ConfigVar {
        ConfigButtonVar() {
        }
        ConfigButtonVar( const string& in name, bool bValue, uint flags ) {

        }

        void Draw() {
            if ( ImGui::RadioButton( name,  ) ) {
                
            }
        }

        private bool m_bValue = false;
    };
};