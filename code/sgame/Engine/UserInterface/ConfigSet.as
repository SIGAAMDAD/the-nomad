namespace TheNomad::Engine::UserInterface {
    funcdef void DrawConfig();

    class ConfigSet {
        ConfigSet( const string& in name, DrawConfig@ fn ) {
            m_Name = name;
            @m_DrawFunc = @fn;
        }

        const string& GetName() const {
            return m_Name;
        }
        DrawConfig@ GetDrawFunc() {
            return @m_DrawFunc;
        }

        private DrawConfig@ m_DrawFunc = null;
        private string m_Name;
    };
};