#include "game.as"
#include "info.as"

namespace TheNomad {
	namespace SGame {
		shared class EntityObject
		{
			EntityObject( const TheNomad::Engine::InfoParser@ in info ) {
				m_EntityInfo = info;
				
				m_nHealth = info["health"];
				m_Name = info["name"];
			}
			
			//
			// Getters & Setters
			//
			int GetHealth() const {
				return m_nHealth;
			}
			const string& GetName() const {
				return m_Name;
			}
			const EntityState@ GetState() const {
				return m_State;
			}
			void SetState( uint statenum ) {
				TheNomad::GameSystem::GameManager.GetInfo( "EntityStates" ).GetInfoInt( statenum );
			}
			const LinkEntity& GetLink() const {
				return m_Link;
			}
			LinkEntity& GetLink() {
				return m_Link;
			}
			void SetOrigin( const vec3& in origin ) {
				m_Link.SetOrigin( origin );
			}
			
			//
			// Utility functions
			//
			void Damage( int nAmount ) {
				
			}
			
			private LinkEntity m_Link;
			private string m_Name;
			private EntityType m_nType;
			private int m_nId;
			private int m_nHealth;
			private uint m_Flags;
			
			private EntityState@ m_State;
		};
		
		shared class EntitySystem
		{
			EntitySystem() {
				m_InfoData = TheNomad::Engine::InfoParser();
				m_EntityList.clear();
				
				ParseInfos();
			}
			
			private void ParseMob( const TheNomad::Engine::InfoParser@ in info ) {
				m_MobInfos.emplace_back();
				MobInfo& info = m_MobInfos.back();
				
				info.health = info.GetInt( "health" );
			}
			
			private void ParseInfos() {
				const array<string>& infoFiles = TheNomad::Engine::ModuleManager.GetInfoFiles();
				
				ConsolePrint( "Parsing info files...\n" );
				for ( int i = 0; i < infoFiles.size(); i++ ) {
					ConsolePrint( "Added '" + infoFiles[i] + "' to info data cache.\n" );
					m_InfoData.AddInfoData( infoFiles[i] );
				}
			}
			
			private void SpawnMob( int nId, const vec3& in origin ) {
				EntityObject@ ent = EntityObject( m_InfoData.GetInfo( "ET_MOB" ).GetInfo( nId ) );
				
				m_EntityList.emplace_back( EntityObject(  ) );
			}
			
			void SpawnAll( const TheNomad::GameSystem::MapData& in mapData )
			{
				ConsolePrint( "Spawning entities...\n" );
				
				const TheNomad::Engine::InfoParser parse = TheNomad::Engine::InfoParser( "entity_info.txt" );
				const array<TheNomad::GameSystem::MapSpawn>& spawns = mapData.GetSpawns();
				
				for ( int i = 0; i < spawns.size(); i++ ) {
					switch ( spawns[i].GetType() ) {
					case ET_PLAYR:
						SpawnPlayer( spawns[i].GetOrigin(), parse.GetValueForKey( IntToString(  ) ) );
						break;
					case ET_MOB:
						SpawnMob( spawns[i].GetId(), spawns[i].GetOrigin() );
						break;
					case ET_BOT:
						ConsolePrint( COLOR_RED "WARNING: bot enitites not supported yet...\n" );
						break;
					case ET_ITEM:
					case ET_WEAPON:
					};
				}
			}
			
			const array<EntityObject>& GetEntities() const {
				return m_EntityList;
			}
			
			private TheNomad::Engine::InfoParser@ m_InfoData;
			private array<EntityObject@> m_EntityList;
		};
		
		EntitySystem@ EntityManager;
	};
};