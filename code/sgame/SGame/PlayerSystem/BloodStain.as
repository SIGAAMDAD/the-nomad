namespace TheNomad::SGame {
    class BloodStain {
        BloodStain() {
        }

        void Draw() {
            TheNomad::Engine::Renderer::RenderEntity refEntity;
            
            refEntity.origin = m_Origin;
            refEntity.scale = 1.5f;
            refEntity.sheetNum = -1;
            refEntity.spriteId = TheNomad::Engine::ResourceCache.GetShader( "bloodMark" );
            refEntity.Draw();
        }

        vec3 m_Origin = vec3( 0.0f );
        BloodStain@ m_Next = null;
        BloodStain@ m_Prev = null;
    };
};