namespace TheNomad::SGame {
	class StatusEffect {
		StatusEffect() {
		}

		bool Active() const {
			return false;
		}
		void Draw() {

		}
		
		protected int m_hShader = FS_INVALID_HANDLE;
		protected bool m_bActive = false;
	};
};