namespace TheNomad::SGame {
	class ScreenShake {
		ScreenShake() {
		}
		
		void Start( uint nDuration, float nStrengthX, float ) {
			/*
			m_nMagnitude = nStrength;
			m_nDuration = nDuration;
			m_nStartTic = TheNomad::GameSystem::GameTic;
			
			// save the old camera position to restore it after the screen shake
			m_CameraPos = Game_CameraPos;
			*/
		}

		//vec2 GetNoise() {
		//	const uint time = ( TheNomad::GameSystem::GameTic - m_nStartTic ) * TheNomad::GameSystem::DeltaTic;
		//}
		
		void OnRunTic() {
			/*
			if ( ( TheNomad::GameSystem::GameTic - m_nStartTic ) * TheNomad::GameSystem::DeltaTic > m_nDuration ) {
				return;
			}

			const float sinValue = sin( m_nSpeed * ( m_nSeed + m_nSeed * ( TheNomad::GameSystem::GameTic - m_nStartTic ) ) );
			vec2 direction = m_Direction * GetNoise();

			Game_CameraPos.x += offset.x;
			Game_CameraPos.y += offset.y;
			*/
		}
		
		/*
		private vec3 m_CameraPos;
		private vec2 m_Direction = vec2( 0.0f );
		private float m_nMagnitude = 0.0f;
		private uint m_nDuration = 0;
		private uint m_nStartTic = 0;
		*/
	};
};