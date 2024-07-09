namespace TheNomad::RenownSystem {
    class WorldSystem : TheNomad::GameSystem::GameObject {
        WorldSystem() {
        }

        void OnInit() {
        }
        void OnShutdown() {
        }
        void OnLevelStart() {
        }
        void OnLevelEnd() {
        }

        private array<Biome@> m_Regions;
        private uint m_nSeason = 0;
    };

    WorldSystem@ WorldManager = null;
};