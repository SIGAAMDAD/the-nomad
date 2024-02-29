#include "game.as"
#include "state.as"
#include "info.as"

namespace TheNomad {
	namespace SGame {
		shared class EntityObject
		{
			EntityObject() {
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
			const TheNomad::GameSystem::LinkEntity& GetLink() const {
				return m_Link;
			}
			TheNomad::GameSystem::LinkEntity& GetLink() {
				return m_Link;
			}
			void SetOrigin( const vec3& in origin ) {
				m_Link.SetOrigin( origin );
			}
			const EntityState@ GetState() const {
				return m_State;
			}
			
			//
			// Utility functions
			//
			void Damage( int nAmount ) {
				
			}
			
			private TheNomad::GameSystem::LinkEntity m_Link;
			private string m_Name;
			private EntityType m_nType;
			private int m_nId;
			private int m_nHealth;
			private uint m_Flags;
			private EntityState@ m_State;
		};
		
		shared class EntitySystem : TheNomad::GameSystem::GameObject
		{
			EntitySystem() {
				m_EntityList.clear();
			}
			
			void OnLoad() {
			}
			void OnSave() const {
			}
			const string& GetName() const {
				return "EntityManager";
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
			
			private array<EntityObject@> m_EntityList;
		};
		
		EntitySystem@ EntityManager;
		
		void InitEntities() {
			EntityManager = cast<EntitySystem>( TheNomad::GameSystem::AddSystem( EntitySystem() ) );
		}
	};
};