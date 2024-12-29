namespace TheNomad::SGame {
	class WallObject : EntityObject {
		WallObject() {
		}

		ref@ GetData() {
			return @m_Data;
		}
		void SetData( ref@ data ) {
			@m_Data = @data;
		}

		void Spawn( uint id, const vec3& in origin ) override {
			m_Link.m_Origin = origin;

			@m_State = @StateManager.GetNullState();
		}
		void Think() override {
			@m_State = @StateManager.GetNullState();

			m_Bounds.MakeBounds( m_Link.m_Origin );
		}
		void Draw() override {
			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = m_Link.m_Origin;
			switch ( m_Link.m_nEntityId ) {
			case WALL_CHECKPOINT_ID: {
				MapCheckpoint@ cp = cast<MapCheckpoint@>( @m_Data );
				
				refEntity.scale = vec2( 1.75f );
				if ( !cp.m_bPassed ) {
					cp.m_Animation.Run();
					refEntity.sheetNum = TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/checkpoint", 128, 32, 32, 32 ).GetShader();
					refEntity.spriteId = cp.m_Animation.GetFrame();
				} else {
					refEntity.sheetNum = -1;
					refEntity.spriteId = TheNomad::Engine::Renderer::RegisterShader( "gfx/completed_checkpoint" );
				}
				break; }
			};
			refEntity.Draw();
		}

		private ref@ m_Data = null;
	};
};