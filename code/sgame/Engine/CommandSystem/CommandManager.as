#include "Engine/Profiler.as"

namespace TheNomad::Engine::CommandSystem {
	funcdef void CommandFunc();

	class CommandManager {
		CommandManager() {
		}
		~CommandManager() {
		}

		void ClearCommands() {
			for ( uint i = 0; i < m_CommandList.Count(); ++i ) {
				TheNomad::Engine::CmdRemoveCommand( m_CommandList[i] );
			}
			m_CommandList.Clear();
			m_Commands.Clear();
			m_KeyBinds.Clear();
		}

		bool CheckCommand( const string& in name ) const {
		#if _NOMAD_DEBUG
			TheNomad::Engine::ProfileBlock block( "CheckCommand" );
		#endif

			if ( m_KeyBinds.Contains( name ) ) {
				cast<CommandFunc>( @m_KeyBinds[ name ] )();
				return true;
			}
			if ( m_Commands.Contains( name ) ) {
				cast<CommandFunc>( @m_Commands[ name ] )();
				return true;
			}

			return false;
		}

		void AddKeyBind( CommandFunc@ fn, const string& in name ) {
			m_KeyBinds.Add( name, @fn );
			m_CommandList.Add( name );
			TheNomad::Engine::CmdAddCommand( name );
		}

		void AddCommand( CommandFunc@ fn, const string& in name ) {
			m_Commands.Add( name, @fn );
			m_CommandList.Add( name );
			TheNomad::Engine::CmdAddCommand( name );
		}

		private dictionary m_Commands;
		private dictionary m_KeyBinds;
		private array<string> m_CommandList;
	};

	CommandManager CmdManager;
};