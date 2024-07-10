#include "nomadmain/GameSystem/GameSystem.as"

namespace hellbreaker {
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
        void OnCheckpointPassed( uint ) {
        }
        void OnPlayerDeath( int ) {
        }
        void OnInit() {
            json@ data;
            @data = json();
            if ( !data.ParseFile( "Config/nomadmain/hellbreaker.json" ) ) {
                ConsoleWarning( "Couldn't load hellbreaker config, using default values.\n" );
                return;
            }
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

        int m_nBackgroundShader = FS_INVALID_HANDLE;
    };

    HellBreakerSystem@ HellBreaker = null;
};