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
            m_bActive = true;
        }
        void OnRunTic( PlayrObject@ ent ) {
            switch ( m_nType ) {
            case QuickTimeEventType::QTE_ButtonMash:
                RunButtonMash();
                break;
            case QuickTimeEventType::QTE_ButtonPress:
                RunButtonPress();
                break;
            case QuickTimeEventType::QTE_PrecisionButton:
                RunPrecisionButton();
                break;
            };
        }

        private void RunButtonMash( void ) {

        }

        private void RunButtonPress( void ) {

        }

        private void RunPrecisionButton() {
        }

        QuickTimeEventType m_nType = QuickTimeEventType::QTE_ButtonMash;
        bool m_bActive = false;
    };
};