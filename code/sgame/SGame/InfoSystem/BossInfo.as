namespace TheNomad::SGame::InfoSystem {
    class BossInfo : InfoLoader {
        BossInfo() {
        }

        bool Load( json@ json ) {
            return true;
        }

        array<AttackInfo@> attacks;
    };
};