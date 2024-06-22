#include "Engine/UserInterface/ConfigSet.as"

namespace TheNomad::Engine::UserInterface {
    class ConfigManager {
        ConfigManager() {
        }

        void Draw() {
            for ( uint i = 0; i < m_ConfigSets.Count(); i++ ) {
                
            }
        }

        void AddModule( ConfigSet@ config ) {
            for ( uint i = 0; i < m_ConfigSets.Count(); i++ ) {
                if ( @m_ConfigSets[i] is @config ) {
                    ConsoleWarning( "ConfigManager::AddModule: config " + config.GetName() + " set already added!\n" );
                    return;
                }
            }
            m_ConfigSets.Add( @config );
        }

        private array<ConfigSet@> m_ConfigSets;
    };

    ConfigManager@ ConfigSystem;
};