namespace TheNomad::SGame {
    interface PlayerEvent {
		void OnRunTic( PlayrObject@ ent );
		void Activate( PlayrObject@ ent );
		bool Load( json@ json );
		bool IsActive() const {
			return m_bActive;
		}

		protected bool m_bActive = false;
	};
};