namespace TheNomad::SGame {
	class AfterImage {
		AfterImage() {
		}

		void Create( PlayrObject@ target ) {
			m_Origin = target.GetOrigin();

			m_nTorsoStateFrame = target.GetState().GetSpriteOffset().y * target.GetSpriteSheet().GetSpriteCountX()
				+ target.GetState().GetSpriteOffset().x;

			m_nLegsStateFrame = target.GetLegState().GetSpriteOffset().y * target.GetSpriteSheet().GetSpriteCountX()
				+ target.GetLegState().GetSpriteOffset().x;

			m_nLeftArmStateFrame = target.GetLeftArmState().GetSpriteOffset().y * target.GetLeftArmSpriteSheet().GetSpriteCountX()
				+ target.GetLeftArmState().GetSpriteOffset().x;
			m_nRightArmStateFrame = target.GetRightArmState().GetSpriteOffset().y * target.GetRightArmSpriteSheet().GetSpriteCountX()
				+ target.GetRightArmState().GetSpriteOffset().x;

			m_hShader = target.GetSpriteSheet().GetShader();

			m_nFacing = target.GetFacing();
			m_nLegsFacing = target.GetLegsFacing();
			m_nLeftArmFacing = target.GetLeftArmFacing();
			m_nRightArmFacing = target.GetRightArmFacing();

			m_bActive = true;
		}
		void Finish() {
			m_bActive = false;
		}

		void Draw() const {
			if ( !m_bActive || Util::Distance( EntityManager.GetActivePlayer().GetOrigin(), m_Origin ) > 16.0f ) {
				return;
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = m_Origin;
			refEntity.sheetNum = m_hShader;
			refEntity.rotation = 0.0f;

			refEntity.scale = TheNomad::Engine::Renderer::GetFacing( m_nFacing );
			refEntity.spriteId = m_nTorsoStateFrame;
			refEntity.Draw();

			refEntity.scale = TheNomad::Engine::Renderer::GetFacing( m_nLegsFacing );
			refEntity.spriteId = m_nLegsStateFrame;
			refEntity.Draw();

			refEntity.scale = TheNomad::Engine::Renderer::GetFacing( m_nLeftArmFacing );
			refEntity.spriteId = m_nLeftArmStateFrame;
			refEntity.Draw();

			refEntity.scale = TheNomad::Engine::Renderer::GetFacing( m_nRightArmFacing );
			refEntity.spriteId = m_nRightArmStateFrame;
			refEntity.Draw();
		}

		private vec3 m_Origin = vec3( 0.0f );

		private uint m_nFacing = FACING_RIGHT;
		private uint m_nLegsFacing = FACING_RIGHT;
		private uint m_nRightArmFacing = FACING_RIGHT;
		private uint m_nLeftArmFacing = FACING_LEFT;

		private float m_nArmsAngle = 0.0f;

		private int m_hShader = FS_INVALID_HANDLE;

		private uint m_nTorsoStateFrame = 0;
		private uint m_nLeftArmStateFrame = 0;
		private uint m_nRightArmStateFrame = 0;
		private uint m_nLegsStateFrame = 0;
		
		private bool m_bActive = false;
	};
};