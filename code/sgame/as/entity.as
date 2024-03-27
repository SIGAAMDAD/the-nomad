#include "game.as"
#include "mobs.as"
#include "playr.as"
#include "info.as"
#include "convar.as"

namespace TheNomad::SGame {
	enum CauseOfDeath {
		Cod_Unknown,
		Cod_Bullet,
		Cod_Imploded,
		Cod_Exploded,
		Cod_Suicide,
		Cod_Telefrag,
		Cod_Punch,
		
		Cod_Falling
	};
	
	enum AttackEffect {
		Effect_Knockback = 0,
		Effect_Stun,
		Effect_Bleed,
		Effect_Blind,
		
		None
	};
	
	enum EntityFlags {
		// DUH.
		Dead      = 0x00000001,
		// can't take damage
		Invul     = 0x00000002,
		// doesn't get drawn
		Invis     = 0x00000004,
		// doesn't get respawned in Nomad or greater difficulties
		PermaDead = 0x00000008,
		// will it bleed?
		Killable  = 0x00000010,

		None      = 0x00000000
	};
	
	class EntityObject {
		EntityObject( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin ) {
			Init( type, id, origin );
		}
		EntityObject() {
		}
		
		void Init( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin ) {
			// just create a temporary bbox to link it in, we'll rebuild every frame anyway
			TheNomad::GameSystem::BBox bounds( 1.0f, 1.0f, origin );
			m_Link = TheNomad::GameSystem::LinkEntity( origin, bounds, id, uint( type ) );

			switch ( type ) {
			case TheNomad::GameSystem::EntityType::Playr:
				@m_Data = cast<ref>( PlayrObject() );
				cast<PlayrObject>( m_Data ).Spawn( id, origin );
				break;
			case TheNomad::GameSystem::EntityType::Mob:
				@m_Data = cast<ref>( MobObject() );
				cast<MobObject>( m_Data ).Spawn( id, origin );
				break;
			case TheNomad::GameSystem::EntityType::Bot:
				break;
			case TheNomad::GameSystem::EntityType::Item:
				@m_Data = cast<ref>( ItemManager.AddItem( ItemType( id ) ) );
				break;
			case TheNomad::GameSystem::EntityType::Weapon:
				@m_Data = cast<ref>( WeaponObject() );
				cast<WeaponObject>( m_Data ).Spawn( id, origin );
				break;
			default:
				GameError( "EntityObject::Spawn: invalid type " + formatUInt( uint( type ) ) );
			};
		}
		
		float GetHealth() const {
			return m_nHealth;
		}
		const string& GetName() const {
			return m_Name;
		}

		const EntityState@ GetState() const {
			return @m_State;
		}
		EntityState@ GetState() {
			return @m_State;
		}
		const ref@ GetData() const {
			return @m_Data;
		}
		ref@ GetData() {
			return @m_Data;
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
		void SetFlags( uint flags ) {
			m_Flags = EntityFlags( flags );
		}
		
		//
		// EntityObject::Load: should only return false if we're missing something
		// in the save data
		//
		bool Load() {
			return true;
		}
		void Save() const {
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
		void SetDirection( TheNomad::GameSystem::DirType dir ) {
			m_Direction = dir;
		}
		TheNomad::GameSystem::EntityType GetType() const {
			return TheNomad::GameSystem::EntityType( m_Link.m_nEntityType );
		}
		void Damage( float nAmount ) {
		}
		int GetFacing() const {
			return m_Facing;
		}
		uint GetId() const {
			return m_Link.m_nEntityId;
		}
		uint GetEntityNum() const {
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
		TheNomad::GameSystem::DirType GetDirection() const {
			return m_Direction;
		}
		void Think() {
			ConsoleWarning( "EntityObject::Think: called\n" );
		}
		void Spawn( uint, const vec3& in ) {
			ConsoleWarning( "EntityObject::Spawn: called\n" );
		}
		void SetFacing( int facing ) {
			m_Facing = facing;
		}
		
		// the entity's current state
		protected EntityState@ m_State = null;
		
		// can only be a reference to a class that inherits from EntityObject, otherwise, it'll crash
		protected ref@ m_Data = null;
		
		// engine data, for physics
		protected TheNomad::GameSystem::LinkEntity m_Link;
		
		// mostly meant for debugging
		protected string m_Name;
		
		// need for speed
		protected vec3 m_Velocity = vec3( 0.0f );
		
		// current effect the entity's suffereing from
		protected AttackEffect m_Debuff = AttackEffect::None;
		
		// DUH.
		protected float m_nHealth = 0.0f;
		
		// flags, some are specific
		protected EntityFlags m_Flags = EntityFlags::None;
		
		// angle's really only used for telling direction
		protected float m_nAngle = 0.0f;
		protected TheNomad::GameSystem::DirType m_Direction = TheNomad::GameSystem::DirType::North;
		
		// is it a projectile?
		protected bool m_bProjectile = false;
		
		// for direction based sprite drawing
		protected int m_Facing = 0;
		
		// cached info
		protected InfoLoader@ m_InfoData = null;
		
		//
		// renderer data
		//
		protected int m_hShader = FS_INVALID_HANDLE;
		protected int m_hSpriteSheet = FS_INVALID_HANDLE;
		
		// linked list stuff
		EntityObject@ next = null;
		EntityObject@ prev = null;
	};
	
	class EntitySystem : TheNomad::GameSystem::GameObject {
		EntitySystem() {
			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_stun" );
			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_bleed" );
			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_knockback" );
			TheNomad::Engine::CmdAddCommand( "sgame.effect_entity_flameon" );
			TheNomad::Engine::CmdAddCommand( "sgame.add_player_health" );
			TheNomad::Engine::CmdAddCommand( "sgame.add_player_rage" );
			TheNomad::Engine::CmdAddCommand( "sgame.set_player_health" );
			TheNomad::Engine::CmdAddCommand( "sgame.set_player_rage" );
			
			m_EntityList.Reserve( sgame_MaxEntities.GetInt() );
			Init();
		}
		
		private void Init() {
			for ( uint i = 0; i < uint( sgame_MaxEntities.GetInt() ); i++ ) {
				m_EntityList.Add( EntityObject() );
			}

			@m_ActiveEntities.next =
			@m_ActiveEntities.prev = @m_ActiveEntities;
			@m_FreeEntities = @m_EntityList[0];
			
			for ( uint i = 0; i < m_EntityList.size() - 1; i++ ) {
				@m_EntityList[i].next = @m_EntityList[i + 1];
			}
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
			EntityObject@ ent;
			uint numEntities;
			TheNomad::GameSystem::LoadSection section( GetName() );
			
			if ( !section.Found() ) {
				GameError( "EntityManager::OnLoad: failed to get entity save data section" );
			}
			
			numEntities = section.LoadUInt( "NumEntities" );
			
			for ( uint i = 0; i < numEntities; i++ ) {
				if ( !m_EntityList[i].Load() ) {
					break; // failed once, don't try again
				}
			}
			
		}
		void OnSave() const {
			EntityObject@ ent;
			TheNomad::GameSystem::SaveSection section( GetName() );

			for ( @ent = @m_ActiveEntities.next; @ent !is null; @ent = @ent.next ) {
				section.SaveUInt( "LinkNext", ent.next.GetEntityNum() );
				section.SaveUInt( "LinkPrev", ent.prev.GetEntityNum() );
				
				
			}
		}
		const string& GetName() const {
			return "EntityManager";
		}
		void OnRunTic() {
			for ( uint i = 0; i < m_EntityList.size(); i++ ) {
				if ( m_EntityList[i].CheckFlags( EntityFlags::Dead ) ) {
					if ( sgame_Difficulty.GetInt() > TheNomad::GameSystem::GameDifficulty::Hard ) {
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
		bool OnConsoleCommand( const string& in cmd ) {
			return false;
		}
		
		private void FreeEntity( EntityObject@ ent ) {
			if ( @ent.prev is null ) {
				GameError( "EntityManager::FreeEntity: not active" );
			}
			
			// remove from the doubly linked list
			@ent.prev.next = @ent.next;
			@ent.next.prev = @ent.prev;
			
			// the free list is only singly linked
			@ent.next = @m_FreeEntities;
			@m_FreeEntities = @ent;
		}
		
		private EntityObject@ AllocEntity( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin ) {
			EntityObject@ ent;
			
			if ( @m_FreeEntities is null ) {
				GameError( "EntityObject::AllocEntity: out of entity slots (limit: " + m_EntityList.size() + " entities)" );
			}
			
			@ent = @m_FreeEntities;
			@m_FreeEntities = @m_FreeEntities.next;
			
			// link into active list
			@ent = @m_ActiveEntities.next;
			@ent = @m_ActiveEntities;
			@m_ActiveEntities.next.prev = @ent;
			@m_ActiveEntities.next = @ent;
			
			ent.Init( type, id, origin );
			
			return @ent;
		}
		
		EntityObject@ Spawn( TheNomad::GameSystem::EntityType type, int id, const vec3& in origin ) {
			return @AllocEntity( type, id, origin );
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
			if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				if ( sgame_Difficulty.GetInt() < uint( TheNomad::GameSystem::GameDifficulty::VeryHard )
					|| sgame_NoRespawningMobs.GetInt() == 1 || ( cast<MobObject>( ent.GetData() ).GetMFlags() & MobFlags::PermaDead ) != 0 )
				{
					return; // no respawning for this one
				} else {
					// TODO: add respawn code here
				}
			} else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				PlayrObject@ obj;
				
				@obj = cast<PlayrObject>( ent.GetData() );
				
				// is hellbreaker available?
				if ( sgame_HellbreakerOn.GetInt() != 0 && TheNomad::Util::IsModuleActive( "hellbreaker" )
					&& sgame_HellbreakerActive.GetInt() == 0 /* ensure there's no recursion */ )
				{
					HellbreakerInit();
					return; // startup the hellbreak
				}
				
				GlobalState = GameState::DeathMenu;
				ent.SetState( StateNum::ST_PLAYR_DEAD ); // play the death animation
			}
		}
		
		const EntityObject@ GetEntityForNum( uint nIndex ) const {
			return @m_EntityList[ nIndex ];
		}
		EntityObject@ GetEntityForNum( uint nIndex ) {
			return @m_EntityList[ nIndex ];
		}
		const array<EntityObject>@ GetEntities() const {
			return @m_EntityList;
		}
		array<EntityObject>@ GetEntities() {
			return @m_EntityList;
		}
		uint NumEntities() const {
			return m_EntityList.Count();
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
					return; // don't deal damage
				}
			}

			target.Damage( info.damage );
		}

		void SetPlayerObject( PlayrObject@ obj ) {
			@m_PlayrObject = @obj;
		}

		PlayrObject@ GetPlayerObject() {
			return @m_PlayrObject;
		}
		
		private array<EntityObject> m_EntityList;
		private EntityObject m_ActiveEntities;
		private EntityObject@ m_FreeEntities;
		private PlayrObject@ m_PlayrObject;
		
		//
		// effects
		//
		
		void Effect_Bleed_f() {
			// sgame.effect_entity_bleed <attacker_num>
		}

		void Effect_Knockback_f() {
			// sgame.effect_entity_knockback <attacker_num>

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
			// sgame.effect_entity_stun <attacker_num>

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

	EntitySystem@ EntityManager;
	PlayrObject@ GetPlayerObject() {
		return @EntityManager.GetPlayerObject();
	}
};