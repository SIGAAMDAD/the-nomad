#include "Engine/Profiler.as"

namespace TheNomad::Engine::CommandSystem {
	funcdef void CommandFunc();

	class CommandManager {
		CommandManager() {
		}
		~CommandManager() {
//			TheNomad::Engine::CmdRemoveCommand( m_CommandList[i].m_Name );
			m_CommandList.Clear();
		}

		bool CheckCommand( const string& in name ) const {
		#if _NOMAD_DEBUG
			TheNomad::Engine::ProfileBlock block( "CheckCommand" );
		#endif

			if ( m_KeyBinds.Contains( name ) ) {
				cast<CommandFunc>( @m_KeyBinds[ name ] )();
				return true;
			}
			if ( m_CommandList.Contains( name ) ) {
				cast<CommandFunc>( @m_CommandList[ name ] )();
				return true;
			}

/*			
			for ( uint i = 0; i < m_CommandList.Count(); i++ ) {
				if ( m_CommandList[i].m_Name == name ) {
					if ( @m_CommandList[i].m_Function is null ) {
						ConsoleWarning( "Command callback for \"" + name + "\" is null! (" + m_CommandList[i].m_Name + ")\n" );
						return false;
					} else if ( m_CommandList[i].m_bLevelCommand && TheNomad::SGame::GlobalState != TheNomad::SGame::GameState::InLevel ) {
						return false;
					}
					m_CommandList[i].m_Function();
					return true;
				}
			}
*/
			return false;
		}

		void AddKeyBind( CommandFunc@ fn, const string& in name ) {
			m_KeyBinds.Add( name, @fn );
			TheNomad::Engine::CmdAddCommand( name );
		}

		void AddCommand( CommandFunc@ fn, const string& in name ) {
			m_CommandList.Add( name, @fn );
			TheNomad::Engine::CmdAddCommand( name );
		}

		private dictionary m_CommandList;
		private dictionary m_KeyBinds;
//		private array<Command> m_CommandList;
	};

	CommandManager CmdManager;
};