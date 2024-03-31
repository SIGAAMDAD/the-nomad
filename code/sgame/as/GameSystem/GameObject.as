namespace TheNomad::GameSystem {
    interface GameObject {
		void OnInit();
		void OnShutdown();
		void OnLoad();
		void OnSave() const;
		void OnRunTic();
		void OnLevelStart();
		void OnLevelEnd();
		bool OnConsoleCommand( const string& in );
		const string& GetName() const;
	};
};