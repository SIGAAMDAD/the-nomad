#include "entity.as"

namespace TheNomad {
	namespace GameSystem {
		array<ref> m_GameSystems;
		
		ref@ AddGameSystem( ref@ SystemHandle ) {
			m_GameSystems.append( SystemHandle );
			return SystemHandle;
		}
		
		class CampaignManager : CampaignObject {
			CampaignManager() {
			}
			
			void OnLoad() {
			}
			void OnSave() {
			}
			
			TheNomad::SGame::EntityObject@ CreateEntity( TheNomad::SGame::EntityType nEntityType ) {
				TheNomad::SGame::EntityObject@ entityRef = TheNomad::SGame::EntityObject();
				
				return entityRef;
			}
			
			private array<TheNomad::SGame::EntityObject> m_EntityList;
		};
		
		CampaignManager@ Game;
		
		void InitGame() {
			Game = cast<CampaignManager>( AddGameSystem( CampaignManager() ) );
			Game.OnLoad();
		}
	};
};