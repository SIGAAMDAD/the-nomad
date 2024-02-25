using string;

namespace TheNomad {
    class ConVar
    {
        ConVar() {
        }
        ConVar( const string@ name, const string@ value, int64 iValue, float fValue, uint flags ) {
            m_Name = name;
            m_Value = value;
            m_IntValue = iValue;
            m_FloatValue = fValue;
            m_Flags = flags;
        }
        ConVar( const ConVar@ other ) {
            m_Name = other.m_Name;
            m_Value = other.m_Value;
            m_IntValue = other.m_IntValue;
            m_FloatValue = other.m_FloatValue;
            m_Flags = other.m_Flags;
        }

        uint GetFlags( void ) const {
            return m_Flags;
        }

        private string m_Name;
        private string m_Value;
        private int64 m_IntValue;
        private float m_FloatValue;
        private uint m_Flags;
    };

    typedef CVar ConVar;
};