#include "SGame/EntityObject.as"
#include "SGame/EntitySystem.as"

namespace TheNomad::SGame {
	import void GetMobFuncIndexes( uint mobType, array<uint>@ indexes ) from "BotLib";
	import void Mob_IdleThink( uint entityNumber ) from "BotLib";
	import void Mob_FightThink( uint entityNumber ) from "BotLib";
	import void Mob_DeadThink( uint entityNumber ) from "BotLib";
	import void Mob_ChaseThink( uint entityNumber ) from "BotLib";
	
	bool sgame_BotLibLoaded = false;
	
	class MobObject : EntityObject {
		MobObject() {
			m_EffectString.resize( MAX_STRING_CHARS );
		}

		InfoSystem::AttackInfo@ CurrentAttack() {
			return @m_CurrentAttack;
		}
		const InfoSystem::AttackInfo@ CurrentAttack() const{
			return @m_CurrentAttack;
		}
		
		const EntityObject@ GetTarget() const {
			return m_Target;
		}
		EntityObject@ GetTarget() {
			return m_Target;
		}

		uint GetMobType() const {
			return m_Info.type;
		}
		
		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) override {
			LoadBase( section );
			m_MFlags = InfoSystem::MobFlags( section.LoadUInt( "mobFlags" ) );
			
			return true;
		}
		
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			SaveBase( section );
			section.SaveUInt( "mobFlags", uint( m_MFlags ) );
		}

		InfoSystem::MobFlags GetMFlags() const {
			return m_MFlags;
		}
		
		private void DoAttack( const InfoSystem::AttackInfo@ atk ) {
			TheNomad::GameSystem::RayCast@ rayData = null;
			
			switch ( atk.attackMethod ) {
			case InfoSystem::AttackMethod::Hitscan: {
				TheNomad::GameSystem::RayCast ray;
				
				@rayData = @ray; // force it out of scope
				
				ray.m_Origin = m_Link.m_Origin;
				ray.m_nLength = atk.range;
				ray.m_nAngle = TheNomad::Util::Dir2Angle( m_Direction );
				ray.m_nEntityNumber = 0;

				TheNomad::GameSystem::CastRay( @ray );

				if ( ray.m_nEntityNumber == EntityManager.NumEntities() ) {
					return; // got nothing
				}
				break; }
			case InfoSystem::AttackMethod::Projectile:
				EntityManager.SpawnProjectile( m_Link.m_Origin, m_nAngle, @atk );
				return; // we'll let the entity manager deal with it now
			default:
				// should theoretically NEVER happen
				GameError( "MobObject::DoAttack: invalid attack method " + uint( atk.attackMethod ) );
			};
			
			if ( atk.effect.size() > 0 ) {
				// apply effects
				m_EffectString = atk.effect;
				m_EffectString += " ";
				m_EffectString += m_Link.m_nEntityNumber;
				m_EffectString += "\n";
				TheNomad::Engine::CmdExecuteCommand( m_EffectString );
			}
			
			EntityManager.DamageEntity( @this, @rayData, @atk );
		}
		
		void Spawn( uint id, const vec3& in origin ) {
			@m_Info = @InfoSystem::InfoManager.GetMobInfo( id );

			if ( @m_Info is null ) { // should never happen (at least with a competent modder)
				GameError( "MobObject::Spawn: bad MobType " + id + "! Info was null!" );
			}
			
			m_Name = m_Info.name;
			m_nHealth = m_Info.health;
			m_Flags = m_Info.flags;
			m_MFlags = m_Info.mobFlags;
			m_hShader = m_Info.hShader;
//			m_hSpriteSheet = m_Info.hSpriteSheet;
			@m_State = @StateManager.GetStateForNum( m_Info.type + StateNum::ST_MOB_IDLE );
		}
		
		void Damage( EntityObject@ source, float nAmount ) {
			m_nHealth -= nAmount;
			if ( m_nHealth < 0.0f ) {
				if ( m_nHealth <= -m_Info.health ) {
					// GIBS!
//					EntityManager.GibbEntity( @this );
				}
				TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( m_Info.dieSfx );
				EntityManager.KillEntity( cast<EntityObject@>( @this ) );
				return;
			}
			
			TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( m_Info.painSfx );
			
			if ( @source !is @m_Target ) {
				SetTarget( @source );
			}
		}
		
		void Think() {
			switch ( m_State.GetID() - m_Info.type ) {
			case StateNum::ST_MOB_IDLE:
				Mob_IdleThink( m_Link.m_nEntityNumber );
				break;
			case StateNum::ST_MOB_FIGHT:
				Mob_FightThink( m_Link.m_nEntityNumber );
				break;
			case StateNum::ST_MOB_CHASE:
				Mob_ChaseThink( m_Link.m_nEntityNumber );
				break;
			default:
				GameError( "MobEntity::Think: invalid mob state " + formatUInt( m_State.GetID() ) + "\n" );
				break;
			};
		}
		
		private void SetTarget( EntityObject@ newTarget ) {
			@m_Target = @newTarget;
			m_nAngle = atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y );
			m_Direction = TheNomad::Util::Angle2Dir( m_nAngle );
			
			m_EffectString = "Set mob target to ";
			m_EffectString += m_Target.GetEntityNum();
			m_EffectString += "\n";
			DebugPrint( m_EffectString );
		}
		
		private void ShuffleDirection() {
			if ( m_Direction == TheNomad::GameSystem::DirType::North ) {
				m_Direction = TheNomad::GameSystem::DirType::Inside;
			} else {
				m_Direction--;
			}
		}
		
		private float GetSpeed() const {
			float value;
			
			switch ( m_Direction ) {
			case TheNomad::GameSystem::DirType::North:
				value = -m_Info.speed.y;
				break;
			case TheNomad::GameSystem::DirType::South:
				value = m_Info.speed.y;
				break;
			case TheNomad::GameSystem::DirType::NorthWest:
			case TheNomad::GameSystem::DirType::SouthWest:
			case TheNomad::GameSystem::DirType::West:
				value = -m_Info.speed.x;
				break;
			case TheNomad::GameSystem::DirType::NorthEast:
			case TheNomad::GameSystem::DirType::SouthEast:
			case TheNomad::GameSystem::DirType::East:
				value = m_Info.speed.x;
				break;
			default:
				GameError( "MobObject::GetSpeed: invalid direction " + formatUInt( uint( m_Direction ) ) );
				return -1;
			};
			
			return value;
		}
		
		private bool CheckCollision() const {
			TheNomad::GameSystem::RayCast ray;
			
			ray.m_nLength = TheNomad::Util::VectorLength( m_Info.speed );
			ray.m_nAngle = m_nAngle;
			ray.m_Origin = m_Link.m_Origin;
			
			TheNomad::GameSystem::CastRay( @ray );
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID ) {
				return false;
			} else if ( ray.m_nEntityNumber >= EntityManager.NumEntities() ) {
				GameError( "MobObject::CheckCollision: out of range entity number" );
			}

			// is there a wall in the way?
			if ( EntityManager.GetEntityForNum( ray.m_nEntityNumber ).GetType() == TheNomad::GameSystem::EntityType::Wall ) {
				ShuffleDirection();
				return true;
			}
			// we can move
			
			return true;
		}
		
		private void TryMove() {
			if ( CheckCollision() ) {
				return;
			}
			
			if ( @m_Target !is null ) {
				// get closer to target
				m_nAngle = atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y );
			}
			
			const uint flags = LevelManager.GetMapData().GetTiles()[ uint( m_Link.m_Origin.y ) * LevelManager.GetMapData().GetWidth() +
				uint( m_Link.m_Origin.x ) ][ GetMapLevel( m_Link.m_Origin.z ) ];
			
			if ( ( flags & SURFACEPARM_WOOD ) != 0 ) {
				
			} else if ( ( flags & SURFACEPARM_WATER ) != 0 ) {
				
			}
			
			m_Velocity.x -= sgame_Friction.GetFloat();
			m_Velocity.y -= sgame_Friction.GetFloat();
			
			for ( uint i = 0; i < 3; i++ ) {
				m_Velocity[i] = TheNomad::Util::Clamp( m_Velocity[i], 0.0f, m_Info.speed[i] );
			}
			
			// NOTE: maybe get some adaptive velocity stuff here?
			m_Link.m_Origin += m_Velocity;
		}
		
		//===========================================================
		//
		// Detection functions
		//
		//===========================================================
		
		private bool SightCheck() {
			if ( ( m_Info.flags & InfoSystem::MobFlags::Blind ) != 0 ) {
				return false;
			}
			
			TheNomad::GameSystem::RayCast ray;
			vec3 delta, pos, p;
			EntityObject@ ent;
			
			delta.x = cos( m_nAngle );
			delta.y = sin( m_nAngle );
			delta.z = 0.0f;
			
			pos = EntityManager.GetPlayerObject().GetOrigin() - m_Link.m_Origin;
			
			p = TheNomad::Util::NormalizeVector( pos );
			
			if ( TheNomad::Util::DotProduct( pos, delta ) > cos( m_Info.sightRadius ) ) {
				return false;
			}
			
			//
			// make sure that the line of sight isn't obstructed
			//
			ray.m_nLength = m_Info.sightRange;
			ray.m_Origin = m_Link.m_Origin;
			ray.m_nAngle = m_nAngle;
			TheNomad::GameSystem::CastRay( @ray );
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID ) {
				return false;
			} else if ( ray.m_nEntityNumber >= EntityManager.NumEntities() ) {
				GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
			}

			@ent = @EntityManager.GetEntityForNum( ray.m_nEntityNumber );
			
			// TODO: make this more advanced
			if ( ent.GetType() != TheNomad::GameSystem::EntityType::Playr ) {
				if ( @m_Target is @ent ) {
					// set them facing the target
					m_nAngle = atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y );
					m_Direction = TheNomad::Util::Angle2Dir( m_nAngle );
					return true; // might be infightinh
				} else {
					return false;
				}
			}
			
			@m_Target = @ent;
			m_EffectString = "Set mob target to ";
			m_EffectString += ent.GetEntityNum();
			m_EffectString += "\n";
			DebugPrint( m_EffectString );
			m_Info.wakeupSfx.Play();
			
			// we got something
			return true;
		}
		private bool SoundCheck() {
			if ( ( m_Info.flags & InfoSystem::MobFlags::Deaf ) != 0 ) {
				return false;
			}
			
			return false;
		}
		
		//===========================================================
		//
		// Thinker functions
		//
		//===========================================================
		
		private void FearThink() {
			
		}
		private void ChaseThink() {
			
		}
		private void FightThink() {
			if ( @m_Target is null ) {
				// if there's no target, or if the target went out of sight,
				// then pursue
				SetState( StateNum::ST_MOB_CHASE );
				return;
			}
			
			InfoSystem::AttackType attackType;
			const float dist = TheNomad::Util::Distance( m_Target.GetOrigin(), m_Link.m_Origin );
			
			// we need a more specific fighting state
			if ( m_State.GetID() == StateNum::ST_MOB_FIGHT ) {
				@m_CurrentAttack = null;
				for ( uint i = 0; i < m_Info.attacks.size(); i++ ) {
					if ( dist < m_Info.attacks[i].range && m_Info.attacks[i].valid ) {
						attackType = m_Info.attacks[i].attackType;
						@m_State = StateManager.GetStateForNum( StateNum::ST_MOB_FIGHT_MELEE );
						@m_CurrentAttack = @m_Info.attacks[i];
						break;
					}
				}
				if ( @m_CurrentAttack is null ) {
					// move closer
					SetState( StateNum::ST_MOB_CHASE );
				}
				return; // cycle it
			}
			m_State.Run();
			
			// wait for it...
			if ( m_State.GetTics() < m_CurrentAttack.cooldown ) {
				return;
			}
			
			DoAttack( m_CurrentAttack );
		}
		
		private void IdleThink() {
			// move around a little
			TryMove();
			
			if ( @m_Target !is null ) {
				SetState( m_Info.type + StateNum::ST_MOB_CHASE );
			}
			
			if ( m_State.GetTics() > m_Info.waitTics ) {
				if ( SoundCheck() || SightCheck() ) {
					SetState( StateNum::ST_MOB_FIGHT );
					return;
				}
			}
		}
		
		private InfoSystem::MobFlags m_MFlags;
		private string m_EffectString;
		private InfoSystem::AttackInfo@ m_CurrentAttack;
		private EntityObject@ m_Target;
		private InfoSystem::MobInfo@ m_Info;
	};
	
	//
	// BotLib interface
	//
	
	MobObject@ CheckValidMobIndex( uint entityNumber ) {
		MobObject@ mob;
		array<EntityObject@>@ entList = @EntityManager.GetEntities();
		if ( entityNumber >= entityNumber ) {
			GameError( "invalid mob index" );
		}
		
		@mob = cast<MobObject@>( @entList[ entityNumber ] );
		if ( mob.GetType() != TheNomad::GameSystem::EntityType::Mob ) {
			GameError( "entityNumber provided is not a mob" );
		}
		
		return @mob;
	}
	
	void Mob_SetOrigin( uint entityNumber, vec3 origin ) {
		array<EntityObject@>@ entList = @EntityManager.GetEntities();
		if ( entityNumber >= entList.Count() ) {
			GameError( "Mob_GetOrigin: invalid entityNumber" );
		}
		
		MobObject@ mob = cast<MobObject@>( @entList[ entityNumber ] );
		if ( mob.GetType() != TheNomad::GameSystem::EntityType::Mob ) {
			GameError( "Mob_GetOrigin: not a mob" );
		}
		
		mob.SetOrigin( origin );
	}
	
	void Mob_SetHealth( uint entityNumber, float health ) {
		array<EntityObject@>@ entList = @EntityManager.GetEntities();
		if ( entityNumber >= entList.Count() ) {
			GameError( "Mob_GetOrigin: invalid entityNumber" );
		}
		
		MobObject@ mob = cast<MobObject@>( @entList[ entityNumber ] );
		if ( mob.GetType() != TheNomad::GameSystem::EntityType::Mob ) {
			GameError( "Mob_GetOrigin: not a mob" );
		}
		
		mob.SetHealth( health );
	}
	
	uint Mob_GetStateId( uint entityNumber ) {
		return CheckValidMobIndex( entityNumber ).GetState().GetID();
	}
	
	vec3 Mob_GetOrigin( uint entityNumber ) {
		return CheckValidMobIndex( entityNumber ).GetOrigin();
	}
	
	float Mob_GetHealth( uint entityNumber ) {
		return CheckValidMobIndex( entityNumber ).GetHealth();
	}
};