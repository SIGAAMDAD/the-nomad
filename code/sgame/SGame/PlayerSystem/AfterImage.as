namespace TheNomad::SGame {
	class AfterImage {
		AfterImage() {
		}

		void Create( PlayrObject@ target ) {
			m_Origin = target.GetOrigin();

			m_nTorsoStateFrame = target.GetState().GetSpriteOffset().y * target.GetSpriteSheet().GetSpriteCountX()
				+ target.GetState().GetSpriteOffset().x;

			m_nLegsStateFrame = target.GetLegState().GetSpriteOffset().y * target.GetLegsSpriteSheet().GetSpriteCountX()
				+ target.GetLegState().GetSpriteOffset().x;

			m_nArmsStateFrame = target.GetArmState().GetSpriteOffset().y * target.GetArmSpriteSheet().GetSpriteCountX()
				+ target.GetArmState().GetSpriteOffset().x;

			m_nTorsoShader = target.GetSpriteSheet().GetShader();
			m_nLegsShader = target.GetLegsSpriteSheet().GetShader();
			m_nArmsShader = target.GetArmSpriteSheet().GetShader();

			m_bActive = true;
		}
		void Finish() {
			m_bActive = false;
		}

		void Draw() const {
			if ( !m_bActive ) {
				return;
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = m_Origin;
			refEntity.scale = 2.0f;

			refEntity.sheetNum = m_nTorsoShader;
			refEntity.spriteId = m_nTorsoStateFrame;
			refEntity.Draw();

			refEntity.sheetNum = m_nLegsShader;
			refEntity.spriteId = m_nLegsStateFrame;
			refEntity.Draw();

			refEntity.sheetNum = m_nArmsShader;
			refEntity.spriteId = m_nArmsStateFrame;
			refEntity.Draw();
		}

		private vec3 m_Origin = vec3( 0.0f );

		private int m_nArmsShader = FS_INVALID_HANDLE;
		private int m_nLegsShader = FS_INVALID_HANDLE;
		private int m_nTorsoShader = FS_INVALID_HANDLE;

		private uint m_nTorsoStateFrame = 0;
		private uint m_nArmsStateFrame = 0;
		private uint m_nLegsStateFrame = 0;
		
		private bool m_bActive = false;
	};
};