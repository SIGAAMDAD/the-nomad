#include "game.as"
#include "mobs.as"
#include "playr.as"
#include "info.as"
#include "convar.as"

namespace TheNomad::SGame {
	shared enum AttackEffect {
		Effect_Knockback = 0,
		Effect_Stun,
		Effect_Bleed,
		Effect_Blind
	};
	
	shared enum EntityFlags {
		// DUH.
		Dead      = 0x00000001,
		// can't move, stationary mob
		Sentry    = 0x00000002,
		// slightly buffed mob, not yet implemented
		Leader    = 0x00000004,
		// can't take damage
		Invul     = 0x00000008,
		// doesn't get drawn
		Invis     = 0x00000010,
		// doesn't get respawned in Nomad or greater difficulties
		PermaDead = 0x00000020,
	};
	
	shared class EntityObject {
		EntityObject( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin, ModuleObject@ main ) {
			// just create a temporary bbox to link it in, we'll rebuild every frame anyway
			TheNomad::GameSystem::BBox bounds( 1.0f, 1.0f, origin );
			m_Link = TheNomad::GameSystem::LinkEntity( origin, bounds, id, uint( type ) );
			@ModObject = @main;

			switch ( type ) {
			case TheNomad::GameSystem::EntityType::Playr:
				@m_Data = cast<ref>( PlayrObject() );
				cast<PlayrObject>( m_Data ).Spawn( id, origin, @this );
				break;
			case TheNomad::GameSystem::EntityType::Mob:
				@m_Data = cast<ref>( MobObject() );
				cast<MobObject>( m_Data ).Spawn( id, origin, @this );
				break;
			case TheNomad::GameSystem::EntityType::Bot:
				break;
			case TheNomad::GameSystem::EntityType::Item:
				@m_Data = cast<ref>( ModObject.ItemManager.AddItem( ItemType( id ) ) );
				break;
			case TheNomad::GameSystem::EntityType::Weapon:
				@m_Data = cast<ref>( WeaponObject() );
				cast<WeaponObject>( m_Data ).Spawn( id, origin, @this );
				break;
			default:
				GameError( "EntityObject::Spawn: invalid type " + formatUInt( uint( type ) ) );
			};
		}
		EntityObject() {
		}
		
		float GetHealth() const {
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
		const ref@ GetData() const {
			return m_Data;
		}
		ref@ GetData() {
			return m_Data;
		}
		uint GetFlags() const {
			return m_Flags;
		}
		bool CheckFlags( uint flags ) const {
			return ( m_Flags & flags ) != 0;
		}
		void SetState( StateNum statenum ) {
			
		}
		void SetState( EntityState@ state ) {
			
		}

		bool IsProjectile() const {
			return m_bProjectile;
		}

		const vec3& GetOrigin() const {
			return m_Link.m_Origin;
		}
		const vec3& GetVelocity() const {
			return m_Velocity;
		}
		vec3& GetVelocity() {
			return m_Velocity;
		}
		void SetVelocity( const vec3& in vel ) {
			m_Velocity = vel;
		}
		TheNomad::GameSystem::EntityType GetType() const {
			return TheNomad::GameSystem::EntityType( m_Link.m_nEntityType );
		}
		void Damage( float nAmount ) {
		}
		uint GetId() const {
			return m_Link.m_nEntityId;
		}
		uint32 GetEntityNum() const {
			return m_Link.m_nEntityNumber;
		}
		const TheNomad::GameSystem::BBox& GetBounds() const {
			return m_Link.m_Bounds;
		}
		const TheNomad::GameSystem::LinkEntity& GetLink() const {
			return m_Link;
		}
		TheNomad::GameSystem::LinkEntity& GetLink() {
			return m_Link;
		}
		EntityObject@ GetBase() {
			return m_Base;
		}
		const EntityObject@ GetBase() const {
			return m_Base;
		}
		void Think() {
			ConsoleWarning( "EntityObject::Think: called\n" );
		}
		void Spawn( uint, const vec3& in, EntityObject@ ) {
			ConsoleWarning( "EntityObject::Spawn: called\n" );
		}
		protected EntityObject@ m_Base;
		protected EntityState@ m_State;
		protected ModuleObject@ ModObject;
		protected ref@ m_Data;
		protected TheNomad::GameSystem::LinkEntity m_Link;
		protected string m_Name;
		protected vec3 m_Velocity;
		protected AttackEffect m_Debuff;
		protected float m_nHealth;
		protected uint m_Flags;
		protected float m_nAngle;
		protected TheNomad::GameSystem::DirType m_Direction;
		protected bool m_bProjectile = false;
	};
	
	shared class EntitySystem : TheNomad::GameSystem::GameObject {
		EntitySystem( ModuleObject@ main ) {
			@ModObject = @main;

			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_stun" );
			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_bleed" );
			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_knockback" );
		}

		void DrawEntity( const EntityObject@ ent ) {
			//	const TheNomad::SGame::SpriteSheet@ sheet;
				switch ( ent.GetType() ) {
				case TheNomad::GameSystem::EntityType::Playr:
			//		cast<PlayrObject>( ent.GetData() ).DrawLegs();
				case TheNomad::GameSystem::EntityType::Mob:
				case TheNomad::GameSystem::EntityType::Bot:
				case TheNomad::GameSystem::EntityType::Item:
				case TheNomad::GameSystem::EntityType::Weapon:
			//		@sheet = ent.GetSpriteSheet();
			//		TheNomad::Engine::Renderer::AddSpriteToScene( ent.GetOrigin(), sheet.GetShader(),
			//			ent.GetState().SpriteOffset() );
					break;
				case TheNomad::GameSystem::EntityType::Wall:
					break; // engine should handle this
				default:
					GameError( "DrawEntity: bad type" );
					break;
				};
			}
		
		void OnLoad() {
		}
		void OnSave() const {
		}
		const string& GetName() const {
			return "EntityManager";
		}
		void OnRunTic() {
			for ( uint i = 0; i < m_EntityList.size(); i++ ) {
				if ( m_EntityList[i].CheckFlags( EntityFlags::Dead ) ) {
					if ( ModObject.sgame_Difficulty.GetInt() > TheNomad::GameSystem::GameDifficulty::Hard ) {
						DeadThink( @m_EntityList[i] );
					}
					continue;
				}
				
				m_EntityList[i].GetState().Run();
		
				switch ( m_EntityList[i].GetType() ) {
				case TheNomad::GameSystem::EntityType::Playr:
					cast<PlayrObject>( m_EntityList[i].GetData() ).Think();
					break;
				case TheNomad::GameSystem::EntityType::Mob:
					cast<MobObject>( m_EntityList[i].GetData() ).Think();
					break;
				case TheNomad::GameSystem::EntityType::Bot:
					break;
				case TheNomad::GameSystem::EntityType::Item:
				case TheNomad::GameSystem::EntityType::Weapon:
					break;
				case TheNomad::GameSystem::EntityType::Wall:
					GameError( "WALLS DON'T THINK, THEY ACT" );
					break;
				default:
					GameError( "EntityManager::OnRunTic: invalid entity type " + formatUInt( uint( m_EntityList[i].GetType() ) ) );
				};

				// update engine data
				m_EntityList[i].GetLink().Update();
				
				// draw entity
				DrawEntity( m_EntityList[i] );
				
				if ( m_EntityList[i].GetState().Done() ) {
					m_EntityList[i].SetState( m_EntityList[i].GetState().Cycle() );
					continue;
				}
			}
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		void OnConsoleCommand() {
		}
		
		EntityObject@ Spawn( TheNomad::GameSystem::EntityType type, int id, const vec3& in origin ) {
			EntityObject@ ent = EntityObject( type, id, origin, @ModObject );
			m_EntityList.push_back( ent );
			return @ent;
		}
		
		void SpawnAll( const TheNomad::SGame::MapData& in mapData )
		{
			ConsolePrint( "Spawning entities...\n" );
			
//			const TheNomad::Engine::InfoParser parse = TheNomad::Engine::InfoParser( "entity_info.txt" );
//			const array<TheNomad::GameSystem::MapSpawn>& spawns = mapData.GetSpawns();
			
//			for ( int i = 0; i < spawns.size(); i++ ) {
//				switch ( spawns[i].GetType() ) {
//				case ET_PLAYR:
//					SpawnPlayer( spawns[i].GetOrigin(), parse.GetValueForKey( IntToString(  ) ) );
//					break;
//				case ET_MOB:
//					SpawnMob( spawns[i].GetId(), spawns[i].GetOrigin() );
//					break;
//				case ET_BOT:
//					ConsolePrint( COLOR_RED + "WARNING: bot enitites not supported yet...\n" );
//					break;
//				case ET_ITEM:
//				case ET_WEAPON:
//				};
//			}
		}
		void DeadThink( EntityObject@ ent ) {
		}
		
		const EntityObject@ GetEntityForNum( uint nIndex ) const {
			return @m_EntityList[ nIndex ];
		}
		EntityObject@ GetEntityForNum( uint nIndex ) {
			return @m_EntityList[ nIndex ];
		}
		const array<EntityObject@>& GetEntities() const {
			return m_EntityList;
		}
		array<EntityObject@>& GetEntities() {
			return m_EntityList;
		}
		uint NumEntities() const {
			return m_EntityList.size();
		}

		void SpawnProjectile( const vec3& in origin, float angle, const AttackInfo@ info ) {

		}

		void KillEntity( EntityObject@ ent ) {
		}
		void ApplyEntityEffect( EntityObject@ attacker, EntityObject@ target, AttackEffect effect ) {
		}

		//
		// DamageEntity: entity v entity
		//
		void DamageEntity( EntityObject@ attacker, EntityObject@ target ) {

		}

		//
		// DamageEntity: mobs v targets
		//
		void DamageEntity( EntityObject@ attacker, TheNomad::GameSystem::RayCast& in rayCast, const AttackInfo@ info ) {
			EntityObject@ target;

			@target = m_EntityList[ rayCast.m_nEntityNumber ];
			if ( target.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				// check for a parry
				PlayrObject@ p = cast<PlayrObject>( target.GetData() );

				if ( p.CheckParry( @attacker ) ) {

				}
			}

			target.Damage( info.damage );
		}
		
		private array<EntityObject@> m_EntityList;
		private ModuleObject@ ModObject;

		//
		// effects
		//

		void Effect_Knockback_f() {
			// sgame.effect_knockback <attacker_num>

			EntityObject@ target, attacker;
			const uint attackerNum = TheNomad::Util::StringToUInt( TheNomad::Engine::CmdArgv( 1 ) );

			@attacker = GetEntityForNum( attackerNum );
			if ( attacker is null ) {
				GameError( "Effect_Knockback_f triggered on null attacker" );
			}

			@target = cast<MobObject>( attacker.GetData() ).GetTarget();
			if ( target is null ) {
				ConsoleWarning( "Effect_Knockback_f triggered but no target\n" );
				return;
			}

			ApplyEntityEffect( attacker, target, AttackEffect::Effect_Knockback );
		}

		void Effect_Stun_f() {
			// sgame.effect_stun <attacker_num>

			EntityObject@ target, attacker;
			const uint attackerNum = TheNomad::Util::StringToUInt( TheNomad::Engine::CmdArgv( 1 ) );

			@attacker = GetEntityForNum( attackerNum );
			if ( attacker is null ) {
				GameError( "Effect_Stun_f triggered on null attacker" );
			}

			@target = cast<MobObject>( attacker.GetData() ).GetTarget();
			if ( target is null ) {
				ConsoleWarning( "Effect_Stun_f triggered but no target\n" );
				return;
			}

			ApplyEntityEffect( attacker, target, AttackEffect::Effect_Stun );
		}
	};
};