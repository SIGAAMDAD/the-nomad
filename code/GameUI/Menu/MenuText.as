#include "Menu/MenuItem.as"

namespace TheNomad::UserInterface {
	class MenuText : MenuItem {
		MenuText() {
			super();
		}
		MenuText( const string& in str, const vec4& in color, bool bIsKeyString = false ) {
			super();
			if ( bIsKeyString ) {
				TheNomad::GameSystem::GetString( str, m_Text );
			} else {
				m_Text = str;
			}
		}

		private string m_Text;
		private vec4 m_Color = vec4( 0.0f );
	};
};