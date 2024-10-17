namespace TheNomad::SGame {
	class AfterImage {
		AfterImage() {
			@m_TorsoSpriteSheet = TheNomad::Engine::ResourceCache.GetSpriteSheet( "sprites/players/" +
				TheNomad::Engine::CvarVariableString( "skin" ) + "raio_torso", 512, 512, 32, 32 );
			if ( @m_TorsoSpriteSheet is null ) {
				GameError( "AfterImage::AfterImage: failed to load torso sprite sheet" );
			}
			for ( int i = 0; i < NUMFACING; i++ ) {
				@m_LegSpriteSheet[ i ] = TheNomad::Engine::ResourceCache.GetSpriteSheet( "sprites/players/"
					+ TheNomad::Engine::CvarVariableString( "skin" ) + "raio_legs_" + i, 512, 512, 32, 32 );
				if ( @m_LegSpriteSheet[ i ] is null ) {
					GameError( "AfterImage::AfterImage: failed to load leg sprite sheet" );
				}

				@m_ArmSpriteSheet[ i ] = TheNomad::Engine::ResourceCache.GetSpriteSheet( "sprites/players/"
					+ TheNomad::Engine::CvarVariableString( "skin" ) + "raio_arms_" + i, 512, 512, 32, 32 );
				if ( @m_ArmSpriteSheet[ i ] is null ) {
					GameError( "AfterImage::AfterImage: failed to load arm sprite sheet" );
				}
			}

			m_TorsoFacing = FACING_RIGHT;
			m_LegsFacing = FACING_RIGHT;
			m_ArmsFacing = FACING_RIGHT;

			@m_TorsoState = null;
			@m_LegState = null;
			@m_ArmState = null;
		}

		void Create( PlayrObject@ target ) {
			m_Origin = target.GetOrigin();

			m_TorsoFacing = target.GetFacing();
			@m_TorsoState = @target.GetState();
			
			m_LegsFacing = target.GetLegsFacing();
			@m_LegState = @target.GetLegState();

			m_ArmsFacing = target.GetArmsFacing();
			@m_ArmState = @target.GetArmState();

			m_bActive = true;
		}
		void Finish() {
			m_bActive = false;
		}

		uint GetSpriteId( SpriteSheet@ sheet, EntityState@ state ) const {
			const uint offset = state.GetSpriteOffset().y * sheet.GetSpriteCountX() + state.GetSpriteOffset().x;
			return offset + state.GetAnimation().GetFrame();
		}

		void Draw() const {
			if ( !m_bActive ) {
				return;
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = m_Origin;
			refEntity.scale = 2.0f;

			refEntity.sheetNum = m_LegSpriteSheet[ m_LegsFacing ].GetShader();
			refEntity.spriteId = GetSpriteId( @m_LegSpriteSheet[ m_LegsFacing ], @m_LegState );
			refEntity.Draw();

			refEntity.sheetNum = m_ArmSpriteSheet[ m_ArmsFacing ].GetShader();
			refEntity.spriteId = GetSpriteId( @m_ArmSpriteSheet[ m_ArmsFacing ], @m_ArmState );
			refEntity.Draw();

			refEntity.sheetNum = m_TorsoSpriteSheet.GetShader();
			refEntity.spriteId = GetSpriteId( @m_TorsoSpriteSheet, @m_TorsoState ) + m_TorsoFacing;
			refEntity.Draw();
		}

		private vec3 m_Origin = vec3( 0.0f );

		private SpriteSheet@[] m_ArmSpriteSheet( NUMFACING );
		private SpriteSheet@[] m_LegSpriteSheet( NUMFACING );
		private SpriteSheet@ m_TorsoSpriteSheet = null;

		private EntityState@ m_ArmState = null;
		private EntityState@ m_LegState = null;
		private EntityState@ m_TorsoState = null;
		
		private int m_ArmsFacing = 0;
		private int m_LegsFacing = 0;
		private int m_TorsoFacing = 0;

		private bool m_bActive = false;
	};
};