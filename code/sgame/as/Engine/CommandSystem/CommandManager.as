namespace TheNomad::Engine::CommandSystem {
    funcdef void CommandFunc();

    class GameObjectCommand {
        GameObjectCommand() {
        }
    };
    class Command {
        Command() {
        }
        Command( const string& in name, CommandFunc@ fn, bool isLevelCommand ) {
            m_Name = name;
            @m_Function = @fn;
            m_bLevelCommand = isLevelCommand;
        }

        string m_Name;
        CommandFunc@ m_Function = null;
        bool m_bLevelCommand = false;
    };

    class CommandManager {
        CommandManager() {
            m_CommandList.Reserve( 1024 );
        }
        ~CommandManager() {
            for ( uint i = 0; i < m_CommandList.Count(); i++ ) {
                @m_CommandList[i].m_Function = null;
                TheNomad::Engine::CmdRemoveCommand( m_CommandList[i].m_Name );
            }
            m_CommandList.Clear();
        }

        bool CheckCommand( const string& in name ) const {
            for ( uint i = 0; i < m_CommandList.Count(); i++ ) {
                if ( name == m_CommandList[i].m_Name ) {
                    if ( m_CommandList[i].m_bLevelCommand && TheNomad::SGame::GlobalState != TheNomad::SGame::GameState::InLevel ) {
                        continue;
                    }
                    m_CommandList[i].m_Function();
                    return true;
                }
            }
            return false;
        }

        void AddCommand( CommandFunc@ fn, const string& in name, bool levelCommand ) {
            m_CommandList.Add( Command( name, @fn, levelCommand ) );
            TheNomad::Engine::CmdAddCommand( name );
            DebugPrint( "Added SGame command \"" + name + "\".\n" );
        }

        private array<Command> m_CommandList;
    };

    CommandManager CmdManager;
};