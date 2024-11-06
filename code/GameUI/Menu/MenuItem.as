#include "Menu/MenuFramework.as"

namespace TheNomad::UserInterface {
	funcdef void EventCallback( ref self, int id );
	funcdef void OwnerDraw( ref self );

	const uint QMF_BLINK				= 0x00000001;
	const uint QMF_SMALLFONT			= 0x00000002;
	const uint QMF_NUMBERSONLY			= 0x00000004; // edit field is only numbers
	const uint QMF_HIGHLIGHT			= 0x00000008;
	const uint QMF_HIGHLIGHT_IF_FOCUS	= 0x00000010; // steady focus
	const uint QMF_PULSEIFFOCUS			= 0x00000020; // pulse if focus
	const uint QMF_HASMOUSEFOCUS		= 0x00000040;
	const uint QMF_NOONOFFTEXT			= 0x00000080;
	const uint QMF_MOUSEONLY			= 0x00000100; // only mouse input allowed
	const uint QMF_HIDDEN				= 0x00000200; // skips drawing
	const uint QMF_GRAYED				= 0x00000400; // grays and disables
	const uint QMF_INACTIVE				= 0x00000800; // disables any input
	const uint QMF_OWNERDRAW			= 0x00001000;
	const uint QMF_PULSE				= 0x00002000;
	const uint QMF_SILENT				= 0x00004000; // don't make any sounds
	const uint QMF_CUSTOMFONT			= 0x00008000; // use a custom font
	const uint QMF_SAMELINE_NEXT		= 0x00010000;
	const uint QMF_SAMELINE_PREV		= 0x00020000;

	// callback notifications
	const uint EVENT_GOTFOCUS			= 1;
	const uint EVENT_LOSTFOCUS			= 2;
	const uint EVENT_ACTIVATED			= 3;

	class MenuItem {
		MenuItem() {
		}

		protected string m_Name;
		protected MenuFramework@ m_Parent = null;
	
		protected int m_nType = 0;
		protected int m_nID = 0;
		protected uint m_iFlags = 0;
		protected bool m_bFocused = false;
		protected int m_nFont = FS_INVALID_HANDLE;

		protected EventCallback@ m_EventCallback = null;
		protected OwnerDraw@ m_OwnerDraw = null;
	};
};