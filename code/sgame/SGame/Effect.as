namespace TheNomad::SGame {
	class Effect {
		Effect() {
		}

		const Animation& GetAnimation() const {
			return m_AnimationInfo;
		}

		Animation m_AnimationInfo;
	};
};