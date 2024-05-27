namespace TheNomad::Engine {
    class ResourceManager {
        ResourceManager() {
        }

        int GetShader( const string& in shader ) {
            int ret;
            if ( !m_ShaderCache.TryGetValue( shader, ret ) ) {
                ret = TheNomad::Engine::Renderer::RegisterShader( shader );
                m_ShaderCache.Add( shader, ret );
            }
            return ret;
        }
        int GetSfx( const string& in sfx ) {
            int ret;
            if ( !m_SfxCache.TryGetValue( sfx, ret ) ) {
                ret = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );
                m_SfxCache.Add( sfx, ret );
            }
            return ret;
        }
        int GetTrack( const string& in npath ) {
            int ret;
            if ( !m_MusicCache.TryGetValue( npath, ret ) ) {
                ret = TheNomad::Engine::SoundSystem::RegisterTrack( npath );
                m_MusicCache.Add( npath, ret );
            }
            return ret;
        }
        TheNomad::SGame::SpriteSheet@ GetSpriteSheet( const string& in shader, uint spriteWidth, uint spriteHeight,
            uint sheetWidth, uint sheetHeight )
        {
            TheNomad::SGame::SpriteSheet@ ret;
            if ( !m_SpriteSheetCache.TryGetValue( shader, @ret ) ) {
                @ret = TheNomad::SGame::SpriteSheet( shader, vec2( float( sheetWidth ), float( sheetHeight ) ),
                    vec2( float( spriteWidth ), float( spriteHeight ) ) );
            }
            return @ret;
        }

        private dictionary m_ShaderCache;
        private dictionary m_SfxCache;
        private dictionary m_MusicCache;
        private dictionary m_SpriteSheetCache;
    };

    ResourceManager ResourceCache;
};