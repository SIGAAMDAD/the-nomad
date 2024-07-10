#include "SGame/EntitySystem.as"

namespace TheNomad::SGame::BossLib {
    class Boss_Gursgo : SGame::EntityObject {
        Boss_Gursgo() {
        }

        private SGame::InfoSystem::BossInfo@ m_Info = null;
        private array<BossPhase@> m_Phases;
    };
};