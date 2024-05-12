namespace TheNomad::Engine::CommandSystem {
    funcdef void CommandFunc();

    class GameObjectCommand {
        GameObjectCommand() {
        }
    };
    class Command {
        Command() {
        }
        Command( const string& in name, CommandFunc@ fn ) {
            m_Name = name;
            @m_Function = @fn;
        }
        Command( const string& in name, TheNomad::GameSystem::GameObject@ obj ) {
            m_Name = name;
            @m_GameObject = @obj;
        }

        string m_Name;
        CommandFunc@ m_Function = null;
        TheNomad::GameSystem::GameObject@ m_GameObject = null;
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
                if ( Util::StrICmp( name, m_CommandList[i].m_Name ) == 0 ) {
                    m_CommandList[i].m_Function();
                    return true;
                }
            }
            return false;
        }
        bool CheckGameCommand( const string& in name ) const {
            for ( uint i = 0; i < m_CommandList.Count(); i++ ) {
                if ( @m_CommandList[i].m_GameObject !is null && m_CommandList[i].m_GameObject.OnConsoleCommand( name ) ) {
                    return true;
                }
            }
            return false;
        }

        void AddCommand( CommandFunc@ fn, const string& in name ) {
            m_CommandList.Add( Command( name, @fn ) );
            TheNomad::Engine::CmdAddCommand( name );
            DebugPrint( "Added SGame command \"" + name + "\".\n" );
        }
//        void AddCommand( TheNomad::GameSystem::GameObject@ obj, const string& in name ) {
//            m_CommandList.Add( Command( name, @obj ) );
//            TheNomad::Engine::CmdAddCommand( name );
//            DebugPrint( "Added GameObject command \"" + name + "\".\n" );
//        }

        private array<Command> m_CommandList;
    };

    CommandManager CmdManager;
};