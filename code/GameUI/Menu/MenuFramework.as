#include "Engine/SoundSystem/SoundEffect.as"
#include "Menu/MenuItem.as"

namespace TheNomad::UserInterface {
	class MenuFramework {
		MenuFramework() {
		}

		void SetItems( const array<MenuItem@>& in items ) {
			m_Items = items;
		}
		void AddItem( MenuItem@ item ) {
			m_Items.Add( item );
		}

		bool IsFullscreen() const {
			return m_bFullscreen;
		}
		uint NumItems() const {
			return m_Items.Count();
		}

		void Draw() {
		}

		protected string m_Name;
		protected float m_nTitleFontScale = 0.0f;
		protected float m_nTextFontScale = 0.0f;
		protected TheNomad::Engine::SoundSystem::SoundEffect track = FS_INVALID_HANDLE;

		protected int m_iFlags;
		protected ivec2 m_Position;
		protected ivec2 m_Size;

		protected array<MenuItem@> m_Items;

		protected bool m_bFullscreen = false;
	};
};