namespace TheNomad::Engine::SoundSystem {
    class SoundEffect {
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

    class SoundScene {
        SoundScene() {
            m_Channels.Reserve( m_nUsedChannels );
            for ( uint i = 0; i < TheNomad::SGame::sgame_MaxSoundChannels.GetInt(); i++ ) {
                m_Channels.Add( null );
            }
        }
        
        void Flush() {
            for ( uint i = 0; i < m_nUsedChannels; i++ ) {
                m_Channels[i].Play();
            }
            m_nUsedChannels = 0;
        }

        void PushSfxToScene( SoundEffect& in sfx ) {
            if ( m_nUsedChannels == m_Channels.Count() ) {
                return;
            }

            @m_Channels[ m_nUsedChannels ] = @sfx;
            m_nUsedChannels++;
        }

        private array<SoundEffect@> m_Channels;
        private uint m_nUsedChannels = 0;
    };

    SoundScene@ SoundManager;
};