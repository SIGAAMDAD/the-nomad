#include "SGame/EntityObject.as"
#include "SGame/InfoSystem/AttackInfo.as"
#include "SGame/InfoSystem/MobInfo.as"
#include "SGame/InfoSystem/ItemInfo.as"
#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/ItemObject.as"
#include "SGame/WeaponObject.as"
#include "SGame/MobObject.as"
#include "SGame/PlayrObject.as"

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

	EntitySystem@ EntityManager;
	PlayrObject@ GetPlayerObject() {
		return @EntityManager.GetPlayerObject();
	}

    class EntitySystem : TheNomad::GameSystem::GameObject {
		EntitySystem() {
		}
		
		void OnInit() {
			Engine::CmdAddCommand( "sgame.effect_entity_stun" );
			Engine::CmdAddCommand( "sgame.effect_entity_bleed" );
			Engine::CmdAddCommand( "sgame.effect_entity_knockback" );
			Engine::CmdAddCommand( "sgame.effect_entity_flameon" );
			Engine::CmdAddCommand( "sgame.add_player_health" );
			Engine::CmdAddCommand( "sgame.add_player_rage" );
			Engine::CmdAddCommand( "sgame.set_player_health" );
			Engine::CmdAddCommand( "sgame.set_player_rage" );
			Engine::CmdAddCommand( "sgame.print_player_state" );
		}
		void OnShutdown() {
			Engine::CmdRemoveCommand( "sgame.effect_entity_stun" );
			Engine::CmdRemoveCommand( "sgame.effect_entity_bleed" );
			Engine::CmdRemoveCommand( "sgame.effect_entity_knockback" );
			Engine::CmdRemoveCommand( "sgame.effect_entity_flameon" );
			Engine::CmdRemoveCommand( "sgame.add_player_health" );
			Engine::CmdRemoveCommand( "sgame.add_player_rage" );
			Engine::CmdRemoveCommand( "sgame.set_player_health" );
			Engine::CmdRemoveCommand( "sgame.set_player_rage" );
			Engine::CmdRemoveCommand( "sgame.print_player_state" );
		}

		void DrawEntity( const EntityObject@ ent ) {
		//	const SGame::SpriteSheet@ sheet;
			switch ( ent.GetType() ) {
			case TheNomad::GameSystem::EntityType::Playr: {
		//		cast<PlayrObject>( @ent ).DrawLegs();
				break; }
			case TheNomad::GameSystem::EntityType::Mob: {

				break; }
			case TheNomad::GameSystem::EntityType::Bot: {

				break; }
			case TheNomad::GameSystem::EntityType::Item: {

				break; }
			case TheNomad::GameSystem::EntityType::Weapon: {
		//		@sheet = ent.GetSpriteSheet();
		//		Engine::Renderer::AddSpriteToScene( ent.GetOrigin(), sheet.GetShader(),
		//			ent.GetState().SpriteOffset() );
				break; }
			case TheNomad::GameSystem::EntityType::Wall: {
				break; } // engine should handle this
			default:
				GameError( "DrawEntity: bad type" );
				break;
			};
		}
		
		const string& GetName() const {
			return "EntityManager";
		}
		void OnLoad() {
			EntityObject@ ent;
			TheNomad::GameSystem::SaveSystem::LoadSection section( GetName() );
			if ( !section.Found() ) {
				ConsoleWarning( "EntitySystem::OnLoad: no save section for entity data found\n" );
				return;
			}

			const uint numEntities = section.LoadUInt( "NumEntities" );

			DebugPrint( "Loading item data from save file...\n" );

			if ( numEntities == 0 ) {
				return;
			}

			m_EntityList.Clear();

			for ( uint i = 0; i < numEntities; i++ ) {
				TheNomad::GameSystem::SaveSystem::LoadSection data( "entity_" + i );
				if ( !data.Found() ) {
					GameError( "EntitySystem::OnLoad: save section \"entity_" + i + "\" not found" );
				}

				@ent = EntityObject();

				switch ( TheNomad::GameSystem::EntityType( data.LoadUInt( "type" ) ) ) {
				case TheNomad::GameSystem::EntityType::Playr: {
					if ( !cast<PlayrObject>( @ent ).Load( data ) ) {
						GameError( "EntitySystem::OnLoad: failed to load player data" );
					}
					break; }
				case TheNomad::GameSystem::EntityType::Mob: {
					if ( !cast<MobObject>( @ent ).Load( data ) ) {
						GameError( "EntitySystem::OnLoad: failed to load mob data" );
					}
					break; }
				case TheNomad::GameSystem::EntityType::Item: {
					if ( !cast<ItemObject>( @ent ).Load( data ) ) {
						GameError( "EntitySystem::OnLoad: failed to load item data" );
					}
					break; }
				default:
					GameError( "EntityObject::OnLoad: invalid entity type" );
					break;
				};
				m_EntityList.Add( @ent );
			}

			DebugPrint( "Loaded " + m_EntityList.Count() + " entities.\n" );
		}
		void OnSave() const {
			TheNomad::GameSystem::SaveSystem::SaveSection section( GetName() );

			DebugPrint( "Saving entity data...\n" );

			for ( uint i = 0; i < m_EntityList.Count(); i++ ) {
				section.SaveUInt( "type", m_EntityList[i].GetType() );
				switch ( m_EntityList[i].GetType() ) {
				case TheNomad::GameSystem::EntityType::Mob:
					section.SaveUInt( "id", cast<MobObject>( @m_EntityList[i] ).GetMobType() );
					break;
				case TheNomad::GameSystem::EntityType::Item:
					section.SaveUInt( "id", cast<ItemObject>( @m_EntityList[i] ).GetItemType() );
					break;
				case TheNomad::GameSystem::EntityType::Weapon:
					section.SaveUInt( "id", cast<WeaponObject>( @m_EntityList[i] ).GetWeaponID() );
					break;
				};
			}
		}
		void OnRunTic() {
			for ( uint i = 0; i < m_EntityList.Count(); i++ ) {
				if ( m_EntityList[i].CheckFlags( EntityFlags::Dead ) ) {
					if ( sgame_Difficulty.GetInt() > TheNomad::GameSystem::GameDifficulty::Hard ) {
						DeadThink( @m_EntityList[i] );
					} else {
						// remove it
						m_EntityList.RemoveAt( i );
					}
					continue;
				}

				if ( @m_EntityList[i].GetState() is null ) {
//					continue;
				} else {
//					m_EntityList[i].GetState().Run();
				}

				switch ( m_EntityList[i].GetType() ) {
				case TheNomad::GameSystem::EntityType::Playr: {
					cast<PlayrObject>( @m_EntityList[i] ).Think();
					break; }
				case TheNomad::GameSystem::EntityType::Mob: {
					cast<MobObject>( @m_EntityList[i] ).Think();
					break; }
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
				
//				if ( m_EntityList[i].GetState().Done() ) {
//					m_EntityList[i].SetState( m_EntityList[i].GetState().Cycle() );
//					continue;
//				}
			}
		}
		void OnLevelStart() {
			DebugPrint( "Spawning entities...\n" );

			@m_ActiveEntities.prev =
			@m_ActiveEntities.next =
				@m_ActiveEntities;

			const array<MapSpawn@>@ spawns = @LevelManager.GetMapData().GetSpawns();
			for ( uint i = 0; i < spawns.Count(); i++ ) {
				switch ( spawns[i].m_nEntityType ) {
				case TheNomad::GameSystem::EntityType::Mob:
					break;
				};
			}

			DebugPrint( "Found " + m_EntityList.Count() + " entity spawns.\n" );
		}
		void OnLevelEnd() {
			// clear all level locals
			m_EntityList.Clear();
			@m_PlayrObject = null;
		}
		bool OnConsoleCommand( const string& in cmd ) {
			if ( Util::StrICmp( cmd, "sgame.list_items" ) == 0 ) {
				ListActiveItems();
			}
			else if ( Util::StrICmp( cmd, "sgame.print_player_state" ) == 0 ) {
				PrintPlayerState();
			}

			return false;
		}
		
		private EntityObject@ AllocEntity( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin ) {
			EntityObject@ ent;

			switch ( type ) {
			case TheNomad::GameSystem::EntityType::Playr:
				@ent = PlayrObject();
				ent.Init( type, id, origin );
				cast<PlayrObject>( @ent ).Spawn( id, origin );
				break;
			case TheNomad::GameSystem::EntityType::Mob:
				cast<MobObject>( @ent ).Spawn( id, origin );
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
				GameError( "EntityManager::Spawn: invalid entity type " + id );
			};

			@m_ActiveEntities.prev.next = @ent;
			@ent.next = @m_ActiveEntities;
			@ent.prev = @m_ActiveEntities.prev;

			m_EntityList.Add( @ent );
			
			return @ent;
		}
		
		EntityObject@ Spawn( TheNomad::GameSystem::EntityType type, int id, const vec3& in origin ) {
			return @AllocEntity( type, id, origin );
		}
		
		void DeadThink( EntityObject@ ent ) {
			if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				if ( sgame_Difficulty.GetInt() < uint( TheNomad::GameSystem::GameDifficulty::VeryHard )
					|| sgame_NoRespawningMobs.GetInt() == 1 || ( cast<MobObject>( @ent ).GetMFlags() & InfoSystem::MobFlags::PermaDead ) != 0 )
				{
					return; // no respawning for this one
				} else {
					// TODO: add respawn code here
				}
			} else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				PlayrObject@ obj;
				
				@obj = cast<PlayrObject>( @ent );
				
				// is hellbreaker available?
				if ( sgame_HellbreakerOn.GetInt() != 0 && Util::IsModuleActive( "hellbreaker" )
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
		const array<EntityObject@>@ GetEntities() const {
			return @m_EntityList;
		}
		array<EntityObject@>@ GetEntities() {
			return @m_EntityList;
		}
		EntityObject@ GetActiveEnts() {
			return @m_ActiveEntities;
		}
		uint NumEntities() const {
			return m_EntityList.Count();
		}

		void SpawnProjectile( const vec3& in origin, float angle, const InfoSystem::AttackInfo@ info ) {

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
		void DamageEntity( MobObject@ attacker, TheNomad::GameSystem::RayCast@ rayCast, const InfoSystem::AttackInfo@ info ) {
			EntityObject@ target;

			if ( @rayCast is null ) {
				return; // not a hitscan
			}

			@target = @m_EntityList[ rayCast.m_nEntityNumber ];
			if ( target.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				// check for a parry
				PlayrObject@ p = cast<PlayrObject@>( @target );
				
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

		void RemoveItem( ItemObject@ item ) {
			@item.prev.next = @item.next;
			@item.next.prev = @item.prev;
		}
		ItemObject@ FindItemInBounds( const TheNomad::GameSystem::BBox& in bounds ) {
			ItemObject@ item;
			for ( @item = cast<ItemObject@>( @m_ActiveItems.next ); @item !is @m_ActiveItems; @item = cast<ItemObject>( @item.next ) ) {
				if ( Util::BoundsIntersect( bounds, item.GetBounds() ) ) {
					return @item;
				}
			}
			return null;
		}
		ItemObject@ AddItem( uint type, const vec3& in origin ) {
			ItemObject@ item;

			@item = cast<ItemObject@>( @Spawn( TheNomad::GameSystem::EntityType::Item, type, origin ) );
			@m_ActiveItems.prev.next = @item;
			@item.prev = @m_ActiveItems.prev;
			@item.next = @m_ActiveItems;

			return item;
		}

		private void PrintPlayerState() const {
			string msg;
			msg.reserve( MAX_STRING_CHARS );

			msg = "\nPlayer State:\n";
			msg += "Health: " + m_PlayrObject.GetHealth() + "\n";
			msg += "Rage: " + m_PlayrObject.GetRage() + "\n";

			ConsolePrint( msg );
		}
		private void ListActiveItems() {
			string msg;
			msg.reserve( MAX_STRING_CHARS );

//			ConsolePrint( "Active Game Items:\n" );
//			for ( uint i = 0; i < m_ItemList.Count(); i++ ) {
//				msg = "(";
//				msg += i;
//				msg += ") ";
//				msg += cast<const ItemInfo@>( @m_ItemList[i].GetInfo() ).name;
//				msg += " ";
//				msg += "[ ";
//				msg += m_ItemList[i].GetOrigin().x;
//				msg += ", ";
//				msg += m_ItemList[i].GetOrigin().y;
//				msg += ", ";
//				msg += m_ItemList[i].GetOrigin().z;
//				msg += " ]\n";
//				ConsolePrint( msg );
//			}
		}
		
		private array<EntityObject@> m_EntityList;
		private ItemObject m_ActiveItems;
		private EntityObject m_ActiveEntities;
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
			const uint attackerNum = Convert().ToUInt( Engine::CmdArgv( 1 ) );

			@attacker = GetEntityForNum( attackerNum );
			if ( attacker is null ) {
				GameError( "Effect_Knockback_f triggered on null attacker" );
			}

			@target = cast<MobObject@>( @attacker ).GetTarget();
			if ( target is null ) {
				ConsoleWarning( "Effect_Knockback_f triggered but no target\n" );
				return;
			}

			ApplyEntityEffect( attacker, target, AttackEffect::Effect_Knockback );
		}

		void Effect_Stun_f() {
			// sgame.effect_entity_stun <attacker_num>

			EntityObject@ target, attacker;
			const uint attackerNum = Convert().ToUInt( Engine::CmdArgv( 1 ) );

			@attacker = GetEntityForNum( attackerNum );
			if ( attacker is null ) {
				GameError( "Effect_Stun_f triggered on null attacker" );
			}

			@target = cast<MobObject@>(@ attacker ).GetTarget();
			if ( target is null ) {
				ConsoleWarning( "Effect_Stun_f triggered but no target\n" );
				return;
			}

			ApplyEntityEffect( attacker, target, AttackEffect::Effect_Stun );
		}
	};
};