namespace TheNomad::Engine::Renderer {
    class DrawCmd {
        DrawCmd() {
        }
    };

    class SceneData {
        SceneData() {
        }

        void OnInit() {
            m_PolyCache.Reserve( 1024 );
        }
        void OnShutdown() {
            m_PolyCache.Clear();
        }
        void OnRunTic() {
            for ( uint i = 0; m_PolyCache.Count(); i++ ) {
                
            }
        }

        void DrawLine( const vec3& in start, const vec3& in end, int hShader, const vec4& in color ) {
            PolyVert startVert;
            PolyVert endVert;

            startVert.xyz = start;
            startVert.color = Util::ColorAsUInt32( color );
            startVert.worldPos = start;
            startVert.uv = vec2( 0, 0 );

            endVert.xyz = end;
            endVert.color = Util::ColorAsUInt32( color );
            endVert.worldPos = end;
            endVert.uv = vec2( 1, 1 );

            
        }
        
        private array<PolyVert> m_PolyCache;
    };
};