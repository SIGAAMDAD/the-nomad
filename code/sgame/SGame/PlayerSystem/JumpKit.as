namespace TheNomad::SGame {
	class JumpKit {
		JumpKit() {
		}

		void ResetDash() {
			m_nTimeSinceDash = TheNomad::GameSystem::GameTic;
			m_nTimeSinceLastUse = m_nTimeSinceDash;
			m_nJumpKitCharges--;
		}
		uint64 GetTimeSinceLastDash() const {
			return TheNomad::GameSystem::GameTic - m_nTimeSinceDash;
		}
		void SetDashing( bool bDashing ) {
			m_bDashing = bDashing;
		}
		bool IsDashing() const {
			return m_bDashing;
		}

		void RunTic() {
			if ( TheNomad::GameSystem::GameTic - m_nTimeSinceLastUse > 5000 && m_nJumpKitCharges < 6 ) {
				m_nTimeSinceLastUse = TheNomad::GameSystem::GameTic;
				m_nJumpKitCharges++;
			}
		}
		int NumCharges() const {
			return m_nJumpKitCharges;
		}

		private uint64 m_nTimeSinceDash = 0;
		private uint64 m_nTimeSinceLastUse = 0;

		private bool m_bDashing = false;

		private int m_nJumpKitCharges = 6;
	};
};