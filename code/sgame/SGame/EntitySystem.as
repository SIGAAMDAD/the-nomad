#include "SGame/EntityObject.as"
#include "SGame/InfoSystem/MobInfo.as"
#include "SGame/InfoSystem/ItemInfo.as"
#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/ItemObject.as"
#include "SGame/WeaponObject.as"
#include "SGame/MobObject.as"
#include "SGame/PlayrObject.as"
#include "SGame/WallObject.as"

namespace TheNomad::SGame {
	funcdef void ForEachEntityIterator( EntityObject@ ent );
	funcdef void ForEachEntityIteratorThis( ref@ this, EntityObject@ ent );
	
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
		Knockback = 0,
		Stunned,
		Bleeding,
		Blinded,
		
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

    class EntitySystem : TheNomad::GameSystem::GameObject {
		EntitySystem() {
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SetPlayerPosition_f ), "sgame.set_player_position", true
			);
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.DamagePlayer_f ), "sgame.damage_player", true
			);
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.PrintPlayerState_f ), "sgame.player_state", true
			);
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.GivePlayerWeapon_f ), "sgame.give_player_weapon", true
			);
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.GivePlayerItem_f ), "sgame.give_player_item", true
			);
		}
		
		void OnInit() {
		}
		void OnShutdown() {
		}
		const string& GetName() const {
			return "EntityManager";
		}
		void OnRenderScene() {
			EntityObject@ ent;

			for ( @ent = @m_ActiveEnts.m_Next; @ent !is @m_ActiveEnts; @ent = @ent.m_Next ) {
				// draw
				if ( Util::Distance( m_ActivePlayer.GetOrigin(), ent.GetOrigin() ) >= 16.0f ) {
					continue;
				}
				ent.Draw();
			}
		}
		void OnRunTic() {
			EntityObject@ ent = null;

			if ( GlobalState == GameState::StatsMenu ) {
				return;
			}
			
			for ( @ent = @m_ActiveEnts.m_Next; @ent !is @m_ActiveEnts; @ent = @ent.m_Next ) {
				if ( ent.CheckFlags( EntityFlags::Dead ) ) {
					if ( ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) > TheNomad::GameSystem::GameDifficulty::Hard
						&& ent.GetType() == TheNomad::GameSystem::EntityType::Mob )
						|| ent.GetType() == TheNomad::GameSystem::EntityType::Playr )
					{
						DeadThink( @ent );
					}
					else {
						// remove it
						RemoveEntity( @ent );
					}
					continue;
				}

				if ( @ent.GetState() is null ) {
					GameError( "EntitySystem::OnRunTic(): entity " + ent.GetName() + " state is null" );
				}

				/*
				switch ( ent.GetType() ) {
				case TheNomad::GameSystem::EntityType::Playr:
				case TheNomad::GameSystem::EntityType::Mob:
				case TheNomad::GameSystem::EntityType::Bot:
				case TheNomad::GameSystem::EntityType::Item:
				case TheNomad::GameSystem::EntityType::Weapon:
				case TheNomad::GameSystem::EntityType::Wall:
					break;
				default:
					GameError( "EntityManager::OnRunTic: invalid entity type " + formatUInt( uint( ent.GetType() ) ) );
				};
				*/

				ent.Think();

				// update engine data
				ent.GetLink().Update();

				ent.SetSoundPosition();
			}
		}

		void OnLoad() {
			uint numEntities = 0;

			m_EntityList.Clear();
			{
				TheNomad::GameSystem::SaveSystem::LoadSection load( GetName() );
				if ( !load.Found() ) {
					GameError( "EntitySystem::OnLoad: save file corruption, section '" + GetName() + "' not found!" );
				}
				numEntities = load.LoadUInt( "NumEntities" );
				m_EntityList.Reserve( numEntities );
			}
			
			@m_ActiveEnts.m_Next =
			@m_ActiveEnts.m_Prev =
				@m_ActiveEnts;

			LevelManager.GetMapData().GetCheckpoints()[ LevelManager.GetCheckpointIndex() ].m_bPassed = true;
			
			for ( uint i = 0; i < numEntities; i++ ) {
				TheNomad::GameSystem::SaveSystem::LoadSection load( "EntityData_" + i );
				if ( !load.Found() ) {
					GameError( "EntitySystem::OnLoad: save file corruption, section 'EntityData_" + i + "' not found!" );
				}

				EntityObject@ ent = null;
				const TheNomad::GameSystem::EntityType type = TheNomad::GameSystem::EntityType( load.LoadUInt( "Type" ) );
				const uint id = load.LoadUInt( "Id" );

				switch ( type ) {
				case TheNomad::GameSystem::EntityType::Playr:
					@ent = PlayrObject();
					break;
				case TheNomad::GameSystem::EntityType::Mob:
					@ent = MobObject();
					break;
				case TheNomad::GameSystem::EntityType::Bot:
					break;
				case TheNomad::GameSystem::EntityType::Item:
					@ent = ItemObject();
					break;
				case TheNomad::GameSystem::EntityType::Weapon:
					@ent = WeaponObject();
					break;
				case TheNomad::GameSystem::EntityType::Wall:
					// static geometry
					@ent = WallObject();
					break;
				default:
					GameError( "EntityManager::Spawn: invalid entity type " + id );
				};

				ent.Init( type, id, vec3( 0.0f ), m_EntityList.Count() );
				ent.Spawn( id, vec3( 0.0f ) );

				DebugPrint( "Spawned entity " + m_EntityList.Count() + "\n" );
				m_EntityList.Add( @ent );

				@m_ActiveEnts.m_Prev.m_Next = @ent;
				@ent.m_Next = @m_ActiveEnts;
				@ent.m_Prev = @m_ActiveEnts.m_Prev;
				@m_ActiveEnts.m_Prev = @ent;

				if ( !ent.Load( load ) ) {
					GameError( "EntitySystem::OnLoad: save file corruption, section 'EntityData_" + i + "' failed to load" );
				}
			}
			DebugPrint( formatUInt( m_EntityList.Count() ) + " total entities.\n" );
		}
		void OnSave() const {
			{
				TheNomad::GameSystem::SaveSystem::SaveSection section( GetName() );
				section.SaveUInt( "NumEntities", m_EntityList.Count() );
			}

			for ( uint i = 0; i < m_EntityList.Count(); i++ ) {
				TheNomad::GameSystem::SaveSystem::SaveSection section( "EntityData_" + i );
				section.SaveUInt( "Type", uint( m_EntityList[i].GetType() ) );
				section.SaveUInt( "Id", m_EntityList[i].GetId() );
				m_EntityList[i].Save( section );
			}
		}
		void OnLevelStart() {
			if ( TheNomad::GameSystem::GameManager.IsLoadActive() ) {
				return;
			}

			DebugPrint( "Initializing entities...\n" );

			@m_ActiveEnts.m_Next =
			@m_ActiveEnts.m_Prev =
				@m_ActiveEnts;
			
			array<MapCheckpoint>@ checkpoints = @LevelManager.GetMapData().GetCheckpoints();
			array<MapSpawn@>@ spawns = @checkpoints[ LevelManager.GetCheckpointIndex() ].m_Spawns;

			for ( uint i = 0; i < spawns.Count(); i++ ) {
				EntityManager.Spawn( spawns[i].m_nEntityType, spawns[i].m_nEntityId,
					vec3( float( spawns[i].m_Origin.x ), float( spawns[i].m_Origin.y ), float( spawns[i].m_Origin.z ) ),
					vec2( 0.0f, 0.0f ) );
			}
			
			for ( uint i = 0; i < checkpoints.Count(); i++ ) {
				checkpoints[i].InitEntity();
			}

			DebugPrint( formatUInt( m_EntityList.Count() ) + " total entities.\n" );

			LevelManager.GetMapData().GetCheckpoints()[ LevelManager.GetCheckpointIndex() ].m_bPassed = true;
		}
		void OnLevelEnd() {
			// clear all level locals
			m_EntityList.Clear();
		}
		void OnPlayerDeath( int ) {
		}
		void OnCheckpointPassed( uint ) {
		}

		void ForEachEntity( ForEachEntityIterator@ fn ) {
			EntityObject@ ent;

			for ( @ent = @m_ActiveEnts.m_Next; @ent !is @m_ActiveEnts; @ent.m_Next ) {
				fn( @ent );
			}
		}
		void ForEachEntity( ForEachEntityIteratorThis@ fn, ref thisObject ) {
			EntityObject@ ent;

			for ( @ent = @m_ActiveEnts.m_Next; @ent !is @m_ActiveEnts; @ent.m_Next ) {
				fn( @thisObject, @ent );
			}
		}
		
		private void RemoveEntity( EntityObject@ ent ) {
			@ent.m_Prev.m_Next = @ent.m_Next;
			@ent.m_Next.m_Prev = @ent.m_Prev;

			@ent.m_Next = @m_FreeEnts;
			@m_FreeEnts = @ent;
		}
		private EntityObject@ AllocEntity( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin, const vec2& in size ) {
			EntityObject@ ent = null;

			switch ( type ) {
			case TheNomad::GameSystem::EntityType::Playr:
				@ent = PlayrObject();
				break;
			case TheNomad::GameSystem::EntityType::Mob:
				@ent = MobObject();
				break;
			case TheNomad::GameSystem::EntityType::Bot:
				break;
			case TheNomad::GameSystem::EntityType::Item:
				@ent = ItemObject();
				break;
			case TheNomad::GameSystem::EntityType::Weapon:
				@ent = WeaponObject();
				break;
			case TheNomad::GameSystem::EntityType::Wall:
				// static geometry
				@ent = WallObject();
				break;
			default:
				GameError( "EntityManager::Spawn: invalid entity type " + id );
			};
			
			ent.Init( type, id, vec3( 0.0f ), m_EntityList.Count() );
			ent.Spawn( id, vec3( 0.0f ) );

			ent.GetBounds().m_nWidth = size.x;
			ent.GetBounds().m_nHeight = size.y;
			ent.GetBounds().MakeBounds( origin );

			DebugPrint( "Spawned entity " + m_EntityList.Count() + "\n" );
			m_EntityList.Add( @ent );

			@m_ActiveEnts.m_Prev.m_Next = @ent;
			@ent.m_Next = @m_ActiveEnts;
			@ent.m_Prev = @m_ActiveEnts.m_Prev;
			@m_ActiveEnts.m_Prev = @ent;
			
			return @ent;
		}
		
		EntityObject@ Spawn( TheNomad::GameSystem::EntityType type, int id, const vec3& in origin, const vec2& in size ) {
			return @AllocEntity( type, id, origin, size );
		}
		
		void DeadThink( EntityObject@ ent ) {
			if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				if ( sgame_Difficulty.GetInt() < uint( TheNomad::GameSystem::GameDifficulty::VeryHard )
					|| sgame_NoRespawningMobs.GetInt() == 1 || ( cast<MobObject@>( @ent ).GetMFlags() & InfoSystem::MobFlags::PermaDead ) != 0 )
				{
					return; // no respawning for this one
				} else {
					// TODO: add respawn code here
				}
			} else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				PlayrObject@ obj = cast<PlayrObject@>( @ent );
				
				// is hellbreaker available?
				if ( sgame_HellbreakerOn.GetInt() == 1 && sgame_HellbreakerActive.GetInt() == 0 ) {
					// ensure there's no recursion
					TheNomad::Engine::CvarSet( "sgame_HellbreakerActive", "1" );
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
		uint NumEntities() const {
			return m_EntityList.Count();
		}
		EntityObject@ GetActiveEnts() {
			return @m_ActiveEnts;
		}
		const EntityObject@ GetActiveEnts() const {
			return @m_ActiveEnts;
		}

		private void GenObituary( EntityObject@ attacker, EntityObject@ target ) {
			if ( target.GetType() != TheNomad::GameSystem::EntityType::Playr ) {
				// unless it's the player, we don't care about the death
				return;
			}
			
			string message = cast<PlayrObject@>( @target ).GetName();
			
			if ( @attacker is null ) {
				if ( target.GetVelocity() > Vec3Origin || target.GetVelocity() > Vec3Origin ) {
					// killed by impact
					if ( target.GetVelocity().z != 0.0f ) {
						message += " thought they could fly";
					}  else {
						message += " was met by the reality of physics";
					}
				} else {
					// player was killed by unknown reasons
					message += " was killed by strange and mysterious forces beyond our control...";
				}
			}
			else {
				// actual death
				
			}
			
			ConsolePrint( message + "\n" );
		}
		
		void KillEntity( EntityObject@ target, EntityObject@ attacker ) {
			if ( attacker.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				// write an obituary for the player
				GenObituary( @attacker, @target );
			}
			// TODO: make the enemies nearby declare the infighting individual a traitor
			if ( target.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				MobObject@ mob = cast<MobObject@>( @target );
				
				// respawn mobs on VeryHard
				if ( uint( sgame_Difficulty.GetInt() ) >= uint( TheNomad::GameSystem::GameDifficulty::VeryHard )
					&& sgame_NoRespawningMobs.GetInt() != 1 && ( uint( mob.GetMFlags() ) & InfoSystem::MobFlags::PermaDead ) == 0 )
				{
					// ST_MOB_DEAD only used with respawning mobs
					target.SetState( StateNum::ST_MOB_DEAD );
				}
				else {
					RemoveEntity( @target );
				}
			}
			else if ( target.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				target.SetState( StateNum::ST_PLAYR_DEAD );
			}
		}
		
		//
		// DamageEntity: entity v entity
		//
		void DamageEntity( EntityObject@ target, EntityObject@ attacker, float damage = 1.0f ) {
			switch ( attacker.GetType() ) {
			case TheNomad::GameSystem::EntityType::Mob: {
				target.Damage( @attacker, damage );
				break; }
			case TheNomad::GameSystem::EntityType::Playr: {
				target.Damage( @attacker, damage );
				if ( target.GetType() == TheNomad::GameSystem::EntityType::Bot ) {
					// TODO: calculate collateral damage here
				}
				// any other entity is just an inanimate object
				// TODO: add explosive objects
				
				break; }
			default:
				GameError( "EntitySystem::Damage: invalid entity type " + uint( attacker.GetType() ) );
			};
		}

		void SetActivePlayer( PlayrObject@ player ) {
			@m_ActivePlayer = @player;
		}
		PlayrObject@ GetActivePlayer() {
			return @m_ActivePlayer;
		}
		const PlayrObject@ GetActivePlayer() const {
			return @m_ActivePlayer;
		}
		
		private array<EntityObject@> m_EntityList;
		private EntityObject m_ActiveEnts;
		private EntityObject@ m_FreeEnts = null;
		private PlayrObject@ m_ActivePlayer = null;

		//==============================================================================
		// Commands
		//

		//
		// status logs
		//

		void PrintPlayerState_f() {
			ConsolePrint( "\n" );
			ConsolePrint( "[PLAYER STATE]\n" );
			ConsolePrint( "Origin: [ " + m_ActivePlayer.GetOrigin().x + ", " + m_ActivePlayer.GetOrigin().y + " ]\n" );
			ConsolePrint( "LegState: " + m_ActivePlayer.GetLegState().GetName() + "\n" );
			if ( @m_ActivePlayer.GetLegState().GetAnimation() is null ) {
				ConsolePrint( "LegAnimation: didn't load\n" );
			}
		}
		private void ListActiveItems() {
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

		//
		// developer commands
		//

		void GivePlayerItem_f() {
			PlayrObject@ obj = @m_ActivePlayer;
		}
		void GivePlayerWeapon_f() {
			
		}
		void SetPlayerPosition_f() {
			const float x = Convert().ToFloat( TheNomad::Engine::CmdArgv( 1 ) );
			const float y = Convert().ToFloat( TheNomad::Engine::CmdArgv( 2 ) );
			const float z = Convert().ToFloat( TheNomad::Engine::CmdArgv( 3 ) );

			m_ActivePlayer.GetLink().m_Origin = vec3( x, y, z );
		}
		void DamagePlayer_f() {
			const float damage = Convert().ToFloat( TheNomad::Engine::CmdArgv( 1 ) );

			m_ActivePlayer.Damage( cast<EntityObject@>( @m_ActivePlayer ), damage );
		}
	};

	EntitySystem@ EntityManager = null;
};