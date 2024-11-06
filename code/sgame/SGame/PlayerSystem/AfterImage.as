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

			m_nLeftArmStateFrame = target.GetLeftArmState().GetSpriteOffset().y * target.GetLeftArmSpriteSheet().GetSpriteCountX()
				+ target.GetLeftArmSpriteSheet().GetSpriteOffset().x;
			m_nRightArmStateFrame = target.GetRightArmState().GetSpriteOffset().y * target.GetRightArmSpriteSheet().GetSpriteCountX()
				+ target.GetRightArmSpriteSheet().GetSpriteOffset().x;

			m_nTorsoShader = target.GetSpriteSheet().GetShader();
			m_nLegsShader = target.GetLegsSpriteSheet().GetShader();
			m_nLeftArmShader = target.GetLeftArmSpriteSheet().GetShader();
			m_nRightArmShader = target.GetRightArmSpriteSheet().GetShader();

			m_nFacing = target.GetFacing();

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

			if ( m_nFacing == FACING_LEFT ) {
				refEntity.sheetNum = m_nLeftArmShader;
				refEntity.spriteId = m_nLeftArmStateFrame;
				refEntity.Draw();
			} else if ( m_nFacing == FACING_RIGHT ) {
				refEntity.sheetNum = m_nRightArmShader;
				refEntity.spriteId = m_nRightArmStateFrame;
				refEntity.Draw();
			}
		}

		private vec3 m_Origin = vec3( 0.0f );

		private uint m_nFacing = FACING_RIGHT;

		private int m_nLeftArmShader = FS_INVALID_HANDLE;
		private int m_nRightArmShader = FS_INVALID_HANDLE;
		private int m_nLegsShader = FS_INVALID_HANDLE;
		private int m_nTorsoShader = FS_INVALID_HANDLE;

		private uint m_nTorsoStateFrame = 0;
		private uint m_nLeftArmStateFrame = 0;
		private uint m_nRightArmStateFrame = 0;
		private uint m_nLegsStateFrame = 0;
		
		private bool m_bActive = false;
	};
};