namespace moblib::System {
	enum SensorType {
		Sight,
		Sound
	};
	
	class AISensor {
		AISensor() {
		}

		void Init( TheNomad::SGame::MobObject@ mob ) {
			@m_EntityData = @mob;	
		}
		
		bool CheckSensor() {
			if ( m_Eyes.DoCheck( @m_EntityData ) ) {
				
			}
			if ( m_Ears.DoCheck( @m_EntityData ) ) {
				
			}
			return false;
		}
		bool CheckSight() {
			return m_Eyes.DoCheck( @m_EntityData );
		}
		bool CheckSound() {
			return m_Ears.DoCheck( @m_EntityData );
		}
		void Stimulate( SensorType nType ) {
		}
		
		private float m_nSuspicion = 0.0f;
		private AISensorSight m_Eyes;
		private AISensorSound m_Ears;
		private TheNomad::SGame::AfterImage m_TargetAfterImage;
		private TheNomad::SGame::MobObject@ m_EntityData = null;
	};
};