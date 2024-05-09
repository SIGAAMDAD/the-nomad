#include "SGame/Animation.as"

namespace TheNomad::SGame {
	class EntityStateSystem : TheNomad::GameSystem::GameObject {
		EntityStateSystem() {
		}
		
		private array<json@>@ LoadJSonFile( const string& in modName, const string& in fileName, const string& in name ) {
			string path;
			array<json@> values;
			json@ data;
			
			path = "modules/" + modName + "/scripts/" + fileName + ".json";
			
			@data = json();
			if ( !data.ParseFile( path ) ) {
				ConsoleWarning( "failed to load " + name + " info file '" + path + "', skipping.\n" );
				return null;
			}

			if ( !data.get( name, values ) ) {
				ConsoleWarning( fileName + "info file found, but no " + name + " infos found.\n" );
				return null;
			}
			
			return @values;
		}
		
		private void LoadAnimationsFromFile( const string& in modName ) {
			array<json@>@ animations;
			
			@animations = @LoadJSonFile( modName, "animations", "AnimationInfo" );
			if ( @animations is null ) {
				return;
			}
			
			ConsolePrint( "Got " + animations.Count() + " animation infos from \"" + modName + "\"\n" );
			for ( uint i = 0; i < animations.Count(); i++ ) {
				m_AnimationInfos.Add( @animations[i] );
			}
		}

		private void LoadStatesFromFile( const string& in modName ) {
			array<json@>@ states;

			@states = @LoadJSonFile( modName, "states", "StateInfo" );
			if ( @states is null ) {
				return;
			}

			ConsolePrint( "Got " + states.Count() + " state infos from \"" + modName + "\"\n" );
			for ( uint i = 0; i < states.Count(); i++ ) {
				m_StateInfos.Add( @states[i] );
			}
		}
		
		private bool LoadState( json@ json ) {
			return true;
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
				EntityState@ state = EntityState();

				m_States.Add( @state );
				m_StateCache.Add( state.GetName(), @state );
			}
			
			m_StateInfos.Clear();

			ConsolePrint( "Loading animation data...\n" );
			
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				LoadAnimationsFromFile( sgame_ModList[i] );
			}
			
			ConsolePrint( "Got " + m_AnimationInfos.Count() + " state infos.\n" );
			
			for ( uint i = 0; i < m_AnimationInfos.Count(); i++ ) {
				Animation@ anim = Animation();

				m_Animations.Add( @anim );
				m_AnimationCache.Add( anim.GetName(), @anim );
			}
			
			m_AnimationInfos.Clear();
		}
		void OnShutdown() {
			m_StateCache.Clear();
			m_AnimationCache.Clear();
			m_States.Clear();
			m_Animations.Clear();
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		void OnSave() const {
		}
		void OnLoad() {
		}
		void OnRenderScene() {
		}
		void OnRunTic() {
		}
		bool OnConsoleCommand( const string& in cmd ) {
			if ( TheNomad::Util::StrICmp( cmd, "sgame.state_info_list" ) == 0 ) {
				
			}
			
			return false;
		}
		
		EntityState@ GetStateById( const string& in name ) {
			return cast<EntityState@>( @m_StateCache[ name ] );
		}
		EntityState@ GetStateForNum( uint nIndex ) {
			return @m_States[ nIndex ];
		}
		
		private array<json@> m_StateInfos;
		private array<json@> m_AnimationInfos;
		private dictionary m_StateCache;
		private dictionary m_AnimationCache;
		private array<EntityState@> m_States;
		private array<Animation@> m_Animations;
	};

	EntityStateSystem@ StateManager;
};