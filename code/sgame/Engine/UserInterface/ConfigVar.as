namespace TheNomad::Engine::UserInterface {
    enum DisplayType {
        Slider,
        Input,
        Angle
    };

    //
    // ConfigVar: the bridge between ImGui and the modder
    // only primitives will work here, strings are the exception
    //
    class ConfigVar {
        ConfigVar( const string& in name ) {
            m_Name = name;
        }
        
        void SetDefaultValue( const string& in str ) {
            m_Data.store( str );
        }
        void SetDefaultValue( float f ) {
            m_Data.store( f );
        }
        void SetDefaultValue( int i ) {
            m_Data.store( i );
        }
        void SetDefaultValue( uint u ) {
            m_Data.store( u );
        }

        void SetDisplayType( DisplayType type ) {
            m_DisplayType = type;
        }

        void Draw() {
            
        }

        private string m_Name;
        private any m_Data;
        private DisplayType m_DisplayType;
    };
};