//#include "entity.as"
//#include "level.as"
#include "convar.as"

namespace TheNomad {
	namespace Engine {
//		const int InvalidHandle = FS_INVALID_HANDLE;
	};
	namespace GameSystem {
		shared class GameObject {
			void OnLoad() {
			}
			void OnSave() {
			}
			void OnRunTic() {
				GameError( "GameObject::OnRunTic: called" );
			}
			void OnLevelStart() {
				GameError( "GameObject::OnLevelStart: called" );
			}
			void OnLevelEnd() {
				GameError( "GameObject::OnLevelEnd: called" );
			}
			const string& GetName() {
				GameError( "GameObject::GetName: called" );
				return " ";
			}
		};
		
		shared GameObject@ AddSystem( GameObject@ SystemHandle ) {
			return SystemHandle;
		}
		
		shared class CampaignManager : GameObject {
			CampaignManager() {
			}
			
			void OnLoad() {
				int hSection;
				int numEntities;

//				hSection = FindArchiveSection( "GameData" );
				if ( hSection == FS_INVALID_HANDLE ) {
					return;
				}
			}
			void OnSave() const {
				BeginSaveSection( "GameData" );
				
//				SaveArray( "soundBits", m_SoundBits );
//				SaveInt( "difficulty", m_Difficulty );
				
				EndSaveSection();
			}
			void OnRunTic() {

			}
			void OnLevelStart() {

			}
			void OnLevelEnd() {

			}
			const string& GetName() const {
				return "CampaignManager";
			}
			
			const array<int>& GetSoundBits() const {
				return m_SoundBits;
			}
			
			private array<int> m_SoundBits;
//			private LevelData@ m_Level;
//			private MapData@ m_MapData;
		};
		
		CampaignManager@ Game;
		ConVar@ sg_difficulty;
		
		void Init() {
			@Game = cast<CampaignManager>( AddSystem( CampaignManager() ) );
			Game.OnLoad();
			
//			sg_difficulty = ConVar( "sg_difficulty", "2", , );
		}
	};
};