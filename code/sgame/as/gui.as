namespace TheNomad::GUI {
    class WidgetConfig {
        WidgetConfig( const vec2& in pos, const vec2& in size, const string& in name, uint32 flags, bool closable ) {
            m_Position = pos;
            m_Size = size;
            m_Name = name;
            m_Flags = flags;
            m_bClosable = closable;
        }

        WidgetConfig& opAssign( const WidgetConfig& in other ) {
            m_Name = other.m_Name;
            m_Position = other.m_Position;
            m_Size = other.m_Size;
            m_Flags = other.m_Flags;
            m_bClosable = other.m_bClosable;
            return this;
        }

        vec2 m_Position;
        vec2 m_Size;
        string m_Name;
        uint32 m_Flags;
        bool m_bClosable;
    };

    interface Widget {
        void Draw();

        string label;
        int flags;
    };

    class Table {
        Table( const string& in label, int nColumns ) {
            m_Label = label;
            m_nColumns = nColumns;
        }

        void Add() {
            
        }

        void Draw() {

        }

        private string m_Label;
        private int m_nColumns;
    };

    class Button {
        Button( const string& in label ) {
            m_Label = label;
        }

        bool IsPressed() const {
            return m_bPressed;
        }

        void Draw() {
            m_bPressed = ImGui::Button( m_Label );
        }

        private bool m_bPressed;
        private string m_Label;
    };

    class Window {
        Window( const WidgetConfig& in config ) {
            m_Config = config;
            m_bOpen = true;
        }

        Window& opAssign( const WidgetConfig& in config ) {
            m_Config = config;
            return this;
        }

        void AddButton( const Button& in button ) {
            m_Buttons.push_back( button );
        }

        const Button& GetButton( const string& in name ) const {
        }

        void ForceClose() {
            m_bOpen = false;
        }

        void Draw() {
            if ( !m_bOpen && m_Config.m_bClosable ) {
                return; // not open
            }

            ImGui::Begin( name, m_Config.m_bClosable ? &m_bOpen : null, m_Config.m_Flags );
            ImGui::End();
        }

        private WidgetConfig m_Config;
        private bool m_bOpen;
    };
};