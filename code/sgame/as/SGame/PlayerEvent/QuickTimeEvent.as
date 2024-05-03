#include "SGame/PlayerEvent/PlayerEvent.as"

namespace TheNomad::SGame {
    // we've all experienced them (probably)
    enum QuickTimeEventType {
        QTE_ButtonMash,      // spam that button
        QTE_ButtonPress,     // a singular button press, a lot like the ones found in Metal Gear Rising: Revengeance
        QTE_PrecisionButton  // a lot like the last stand in Middle Earth: Shadow of Mordor
    };

    class QuickTimeEvent : PlayerEvent {
        QuickTimeEvent() {
        }

        bool IsActive() const {
            return m_bActive;
        }
        bool Load( json@ json ) {
            return true;
        }
        void Activate( PlayrObject@ ent ) {
            ConsoleWarning( "QuickTimeEvent::Activate: called\n" );
        }
        void OnRunTic( PlayrObject@ ent ) {
            ConsoleWarning( "QuickTimeEvent::OnRunTic: called\n" );
        }

        protected QuickTimeEventType m_nType = QuickTimeEventType::QTE_ButtonMash;
        protected bool m_bActive = false;
        protected uint m_nDuration = 0;
        protected uint m_nLifeTime = 0;
    };

    class ButtonMashEvent : QuickTimeEvent {
        ButtonMashEvent( TheNomad::Engine::KeyNum keyboardNum, TheNomad::Engine::KeyNum gamepadNum ) {
            super();

            m_KeyboardNum = keyboardNum;
            m_GamepadNum = gamepadNum;
        }

        private TheNomad::Engine::KeyNum m_KeyboardNum = TheNomad::Engine::KeyNum::X;
        private TheNomad::Engine::KeyNum m_GamepadNum = TheNomad::Engine::KeyNum::X;
        private uint m_nClicks;
    };
};