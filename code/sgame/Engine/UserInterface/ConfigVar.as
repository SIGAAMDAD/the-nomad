namespace TheNomad::Engine::UserInterface {
    //
    // ConfigVar: the bridge between ImGui and the modder
    // only primitives will work here, strings are the exception
    //
    class ConfigVar : ConfigVarBase {
        ConfigVar() {
        }
        ConfigVar( const string& in name, const string& in id, const string& in value, uint flags ) {
            ConVar tmp;

            m_Id = id;
            m_Name = name;

            // TODO: ensure we don't add duplicate cvars
        #ifdef _NOMAD_DEBUG
            TheNomad::CvarManager.AddCvar( @tmp, id, value, flags, true );
        #else
            TheNomad::CvarManager.AddCvar( @tmp, id, value, flags, false );
        #endif
        }

        protected string m_Id;
        protected string m_Name;
        protected bool m_bModified = false;
    };
};