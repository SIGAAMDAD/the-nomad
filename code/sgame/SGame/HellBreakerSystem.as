namespace TheNomad::SGame {
    class HellBreakerSystem : TheNomad::GameSystem::GameObject {
        HellBreakerSystem() {
        }

        void OnLoad() {
            TheNomad::GameSystem::SaveSystem::LoadSection section( GetName() );
            if ( !section.Found() ) {
                return;
            }

            m_nWave = section.LoadUInt( "wave" );
        }
        void OnSave() const {
            TheNomad::GameSystem::SaveSystem::SaveSection section( GetName() );

            section.SaveUInt( "wave", m_nWave );
        }

        void OnLevelStart() {
        }
        void OnLevelEnd() {
        }
        void OnInit() {
        }
        void OnShutdown() {
        }
        void OnRunTic() {
        }
        void OnRenderScene() {
        }
        bool OnConsoleCommand( const string& in cmd ) {
            return true;
        }
        const string& GetName() const {
            return "Hellbreaker";
        }

        private uint m_nWave = 0;
    };

    HellBreakerSystem@ HellBreaker = null;
};