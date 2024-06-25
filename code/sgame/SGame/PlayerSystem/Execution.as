#include "SGame/PlayerSystem/PlayerEvent.as"

namespace TheNomad::SGame {
    class Execution : PlayerEvent {
        Execution() {
        }

        void OnRunTic( PlayrObject@ ent ) {
        }
		void Activate( PlayrObject@ ent ) {
        }
		bool Load( json@ json ) {
            return true;
        }
    };
};