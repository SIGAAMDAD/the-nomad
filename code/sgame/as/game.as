#include "entity.as"
#include "level.as"

namespace TheNomad {
	namespace GameSystem {
		array<GameObject@> GameSystems;
		
		interface GameObject {
			void OnLoad();
			void OnSave() const;
			const string& GetName() const;
		};
		
		shared GameObject@ AddGameSystem( GameObject@ SystemHandle ) {
			ConsolePrint( "Registered game system '" + SystemHandle.GetName() "'\n" );
			GameSystems.append( SystemHandle );
			return SystemHandle;
		}
		
		class CampaignManager : GameObject {
			CampaignManager() {
			}
			
			void OnLoad() {
				int hSection;
				int numEntities;
				
				hSection = FindArchiveSection( "GameData" );
				if ( hSection == TheNomad::Engine::InvalidHandle ) {
					return;
				}
				
				GetInt( "entityCount" );
			}
			
			void OnSave() const {
			}
			
			TheNomad::SGame::EntityObject@ CreateEntity( TheNomad::SGame::EntityType nEntityType ) {
				TheNomad::SGame::EntityObject@ entityRef = TheNomad::SGame::EntityObject();
				m_EntityList.emplace_back( entityRef );
				return entityRef;
			}
			
			private array<TheNomad::SGame::EntityObject@> m_EntityList;
		};
		
		void Init() {
			Game = cast<CampaignManager>( AddGameSystem( CampaignManager() ) );
			Game.OnLoad();
		}
	};
};