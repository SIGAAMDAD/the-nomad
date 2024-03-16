namespace TheNomad::Engine::SoundSystem {
    shared class SoundEffect {
        SoundEffect() {
            m_hSfx = FS_INVALID_HANDLE;
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

        private int m_hSfx;
    };
};