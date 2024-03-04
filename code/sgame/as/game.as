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

		shared class SaveSection {
			SaveSection( const string& in name ) {
				TheNomad::GameSystem::BeginSaveSection( name );
			}
			~SaveSection() {
				TheNomad::GameSystem::EndSaveSection();
			}

			void SaveIntArray( const string& in name, const array<int>& in value ) const {
				SaveArray( name, value );
			}
			void SaveStringArray( const string& in name, const array<string>& in value ) const {
				SaveArray( name, value );
			}

			void SaveString( const string& in name, const string& in value ) const {
				TheNomad::GameSystem::SaveString( name, value );
			}

			void SaveChar( const string& in name, int8 value ) const {
				TheNomad::GameSystem::SaveInt8( name, value );
			}
			void SaveShort( const string& in name, int16 value ) const {
				TheNomad::GameSystem::SaveInt16( name, value );
			}
			void SaveInt( const string& in name, int32 value ) const {
				TheNomad::GameSystem::SaveInt32( name, value );
			}
			void SaveLong( const string& in name, int64 value ) const {
				TheNomad::GameSystem::SaveInt64( name, value );
			}

			void SaveByte( const string& in name, uint8 value ) const {
				TheNomad::GameSystem::SaveInt8( name, value );
			}
			void SaveUShort( const string& in name, uint16 value ) const {
				TheNomad::GameSystem::SaveUInt16( name, value );
			}
			void SaveUInt( const string& in name, uint32 value ) const {
				TheNomad::GameSystem::SaveUInt32( name, value );
			}
			void SaveULong( const string& in name, uint64 value ) const {
				TheNomad::GameSystem::SaveUInt64( name, value );
			}

			void SaveInt8( const string& in name, int8 value ) const {
				TheNomad::GameSystem::SaveInt8( name, value );
			}
			void SaveInt16( const string& in name, int16 value ) const {
				TheNomad::GameSystem::SaveInt16( name, value );
			}
			void SaveInt32( const string& in name, int32 value ) const {
				TheNomad::GameSystem::SaveInt32( name, value );
			}
			void SaveInt64( const string& in name, int64 value ) const {
				TheNomad::GameSystem::SaveInt64( name, value );
			}

			void SaveUInt8( const string& in name, uint8 value ) const {
				TheNomad::GameSystem::SaveUInt8( name, value );
			}
			void SaveUInt16( const string& in name, uint16 value ) const {
				TheNomad::GameSystem::SaveUInt16( name, value );
			}
			void SaveUInt32( const string& in name, uint32 value ) const {
				TheNomad::GameSystem::SaveUInt32( name, value );
			}
			void SaveUInt64( const string& in name, uint64 value ) const {
				TheNomad::GameSystem::SaveUInt64( name, value );
			}
		};
		
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
		ConVar@ sg_savename;
		
		void Init() {
			@Game = cast<CampaignManager>( AddSystem( CampaignManager() ) );
			Game.OnLoad();
		}
	};
};