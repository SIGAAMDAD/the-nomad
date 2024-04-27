namespace TheNomad::SGame {
    interface PlayerEvent {
		void OnRunTic( PlayrObject@ ent );
		void Activate( PlayrObject@ ent );
		bool IsActive() const;
		bool Load( json@ json );
	};
};