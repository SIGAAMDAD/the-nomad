namespace TheNomad::SGame {
	class EntityStateSystem : TheNomad::GameSystem::GameObject {
		EntityStateSystem() {
		}
		
		private array<json@>@ LoadJSonFile( const string& in modName ) {
			string path;
			array<json@> values;
			json@ data;
			
			path.reserve( MAX_NPATH );
			path = "modules/" + modName + "/scripts/states.json";
			
			@data = json();
			if ( !data.ParseFile( path ) ) {
				ConsoleWarning( "failed to load state info file '" + path + "', skipping.\n" );
				return null;
			}

			if ( !data.get( "StateInfo", values ) ) {
				ConsoleWarning( "state info file found, but no state infos found.\n" );
				return null;
			}
			
			return @values;
		}

		private void LoadStatesFromFile( const string& in modName ) {
			array<json@>@ states;

			@states = @LoadJSonFile( modName );
			if ( @states is null ) {
				return;
			}

			ConsolePrint( "Got " + states.Count() + " state infos from \"" + modName + "\"\n" );
			for ( uint i = 0; i < states.Count(); i++ ) {
				m_StateInfos.Add( @states[i] );
			}
		}


		
		const string& GetName() const {
			return "EntityStateSystem";
		}
		void OnInit() {
			ConsolePrint( "Loading state data...\n" );
			
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				LoadStatesFromFile( sgame_ModList[i] );
			}
			
			ConsolePrint( "Got " + m_StateInfos.Count() + " state infos.\n" );
			
			for ( uint i = 0; i < m_StateInfos.Count(); i++ ) {
				
			}
			
			m_StateInfos.Clear();
		}
		void OnShutdown() {
			m_StateCache.Clear();
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		void OnSave() const {
		}
		void OnLoad() {
		}
		void OnRunTic() {
		}
		bool OnConsoleCommand( const string& in cmd ) {
			if ( Util::StrICmp( cmd, "sgame.state_info_list" ) == 0 ) {
				
			}
			
			return false;
		}
			
		const EntityState@ GetStateForNum( uint nIndex ) const {
			return m_StateCache[ nIndex ];
		}
		EntityState@ GetStateForNum( uint nIndex ) {
			return m_StateCache[ nIndex ];
		}
		
		private array<json@> m_StateInfos;
		private array<EntityState@> m_StateCache;
	};

	EntityStateSystem@ StateManager;
};