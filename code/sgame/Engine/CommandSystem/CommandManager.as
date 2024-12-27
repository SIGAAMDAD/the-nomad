#include "Engine/Profiler.as"

namespace TheNomad::Engine::CommandSystem {
	funcdef void CommandFunc();

	class Command {
		Command() {
		}
		Command( const string& in name, CommandFunc@ fn ) {
			Name = name;
			@Function = @fn;
		}

		string Name;
		CommandFunc@ Function = null;
	};

	class CommandManager {
		CommandManager() {
		}
		~CommandManager() {
		}

		void ClearCommands() {
			for ( uint i = 0; i < m_CommandList.Count(); ++i ) {
				TheNomad::Engine::CmdRemoveCommand( m_CommandList[i].Name );
			}
			m_CommandList.Clear();
		}

		bool CheckCommand( const string& in name ) const {
		#if _NOMAD_DEBUG
			TheNomad::Engine::ProfileBlock block( "CheckCommand" );
		#endif

			for ( uint i = 0; i < m_CommandList.Count(); ++i ) {
				if ( m_CommandList[i].Name == name ) {
					if ( @m_CommandList[i].Function is null ) {
						ConsoleWarning( "Command callback for \"" + name + "\" is null! (" + m_CommandList[i].Name + ")\n" );
						return false;
					}
					m_CommandList[i].Function();
					return true;
				}
			}

			return false;
		}

		void AddKeyBind( CommandFunc@ fn, const string& in name ) {
			m_CommandList.Add( Command( name, @fn ) );
			TheNomad::Engine::CmdAddCommand( name );
		}

		void AddCommand( CommandFunc@ fn, const string& in name ) {
			m_CommandList.Add( Command( name, @fn ) );
			TheNomad::Engine::CmdAddCommand( name );
		}

		private dictionary m_Commands;
		private dictionary m_KeyBinds;
		private array<Command> m_CommandList;
	};

	CommandManager CmdManager;
};