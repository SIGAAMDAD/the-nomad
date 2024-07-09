namespace TheNomad::Engine::UserInterface {
    enum DisplayType {
        Slider,
        Input,
        Angle
    };

    enum ValueType {
        Decimal,
        Integral,
        String,

        Float = Decimal,
        Integer = Integral,
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

        void Set( float min, float max, float value, DisplayType type = DisplayType::Slider ) {
            m_nMin = min;
            m_nMax = max;
            m_Value = value;
            m_DisplayType = type;
        }
        void Set( int min, int max, int value, DisplayType type = DisplayType::Slider ) {
            m_nMin = float( min );
            m_nMax = float( max );
            m_Value = value;
            m_DisplayType = type;
        }
        void Set( const string& in value ) {
            m_DisplayType = DisplayType::Input;
        }

        void SetDisplayType( DisplayType type ) {
            m_DisplayType = type;
        }

        void Draw() {
            switch ( m_DisplayType ) {
            case DisplayType::Slider:
                break;
            };
        }

        void Save() {
        }
        
        private DisplayType m_DisplayType = DisplayType::Slider;
        private ValueType m_ValueType = ValueType::Decimal;
        private any m_Value = 0;
        private any m_nMin = 0;
        private any m_nMax = 0;
        private TheNomad::ConVar@ m_EngineData = null;
    };
};