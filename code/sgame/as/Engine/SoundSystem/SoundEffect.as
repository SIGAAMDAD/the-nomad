namespace TheNomad::Engine::SoundSystem {
    class SoundEffect {
        SoundEffect() {
        }

        void Set( const string& in fileName ) {
            m_hSfx = RegisterSfx( fileName );
            if ( m_hSfx == FS_INVALID_HANDLE ) {
                ConsoleWarning( "failed to load sfx file '" + fileName + "'\n" );
            }
        }

        void Play() const {
            PlaySfx( m_hSfx );
        }

        private int m_hSfx = FS_INVALID_HANDLE;
    };
};