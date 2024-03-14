#include "game.as"
#include "convar.as"

namespace TheNomad {
	namespace SGame {
		shared enum AttackEffect {
			Effect_Knockback = 0,
			Effect_Stun,
			Effect_Bleed,
			Effect_Blind
		};
		
		shared interface EntityData {
			void Think();
			void Spawn();
			void Damage( uint nAmount );
			EntityObject@ GetBase();
			const EntityObject@ GetBase() const;
		};
		
		shared class EntityObject {
			EntityObject() {
			}
			
			int GetHealth() const {
				return m_nHealth;
			}
			const string& GetName() const {
				return m_Name;
			}
			const EntityState@ GetState() const {
				return m_State;
			}
			EntityState@ GetState() {
				return m_State;
			}
			void SetState( StateNum statenum ) {
				
			}
			const TheNomad::GameSystem::LinkEntity& GetLink() const {
				return m_Link;
			}
			TheNomad::GameSystem::LinkEntity& GetLink() {
				return m_Link;
			}
			
			protected AttackEffect m_Debuff;
			protected TheNomad::GameSystem::LinkEntity m_Link;
			protected string m_Name;
			protected int m_nHealth;
			protected uint m_Flags;
			protected EntityState@ m_State;
		};
		
		shared class EntitySystem : TheNomad::GameSystem::GameObject {
			EntitySystem() {
			}
			
			void OnLoad() {
			}
			void OnSave() const {
			}
			const string& GetName() const {
				return "EntityManager";
			}
			
			private void SpawnMob( int nId, const vec3& in origin ) {
//				EntityObject@ ent = EntityObject( m_InfoData.GetInfo( "ET_MOB" ).GetInfo( nId ) );
				
//				m_EntityList.push_back( EntityObject() );
			}
			
			void Spawn( TheNomad::GameSystem::EntityType type, int id, const uvec3& in origin ) {
				EntityObject@ ent = EntityObject( type, id, origin );
				
				switch ( type ) {
				case TheNomad::GameSystem::EntityType::Playr:
				case TheNomad::GameSystem::EntityType::Mob:
				case TheNomad::GameSystem::EntityType::Bot:
				case TheNomad::GameSystem::EntityType::Item:
				case TheNomad::GameSystem::EntityType::Weapon:
				};
			}
			
			void SpawnAll( const TheNomad::SGame::MapData& in mapData )
			{
				ConsolePrint( "Spawning entities...\n" );
				
//				const TheNomad::Engine::InfoParser parse = TheNomad::Engine::InfoParser( "entity_info.txt" );
//				const array<TheNomad::GameSystem::MapSpawn>& spawns = mapData.GetSpawns();
				
//				for ( int i = 0; i < spawns.size(); i++ ) {
//					switch ( spawns[i].GetType() ) {
//					case ET_PLAYR:
//						SpawnPlayer( spawns[i].GetOrigin(), parse.GetValueForKey( IntToString(  ) ) );
//						break;
//					case ET_MOB:
//						SpawnMob( spawns[i].GetId(), spawns[i].GetOrigin() );
//						break;
//					case ET_BOT:
//						ConsolePrint( COLOR_RED + "WARNING: bot enitites not supported yet...\n" );
//						break;
//					case ET_ITEM:
//					case ET_WEAPON:
//					};
//				}
			}
			
			const array<EntityObject@>& GetEntities() const {
				return m_EntityList;
			}
			
			private array<EntityObject@> m_EntityList;
		};
		
		EntitySystem@ EntityManager;
		
		EntitySystem@ GetEntityManager() {
			return EntityManager;
		}
		
		void InitEntities() {
			@EntityManager = cast<EntitySystem>( TheNomad::GameSystem::AddSystem( EntitySystem() ) );
		}
		
		
		//
		// effects
		//
		
		void Effect_Knockback_f() {
			// sgame.effect_knockback <attacker_num>
			
			EntityObject@ target, attacker;
			const uint attackerNum = StringToUInt( TheNomad::Engine::CmdArgv( 1 ) );
			
			@attacker = EntityManager.GetEntityForNum( attackerNum );
			if ( attacker is null ) {
				GameError( "Effect_Knockback_f triggered on null attacker" );
			}
			
			@target = attacker.GetTarget();
			if ( target is null ) {
				ConsoleWarning( "Effect_Knockback_f triggered but no target\n" );
				return;
			}
			
			EntityManager.ApplyEntityEffect( attacker, target, AttackEffect::Effect_Knockback );
		}
		
		void Effect_Stun_f() {
			// sgame.effect_stun <attacker_num>
			
			EntityObject@ target, attacker;
			const uint attackerNum = StringToUInt( TheNomad::Engine::CmdArgv( 1 ) );
			
			@attacker = EntityManager.GetEntityForNum( attackerNum );
			if ( attacker is null ) {
				GameError( "Effect_Stun_f triggered on null attacker" );
			}
			
			@target = attacker.GetTarget();
			if ( target is null ) {
				ConsoleWarning( "Effect_Stun_f triggered but no target\n" );
				return;
			}
			
			EntityManager.ApplyEntityEffect( attacker, target, AttackEffect::Effect_Stun );
		}
	};
};