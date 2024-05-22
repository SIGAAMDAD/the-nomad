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
		}

		InfoSystem::AttackInfo@ GetCurrentAttack() {
			return @m_CurrentAttack;
		}
		const InfoSystem::AttackInfo@ GetCurrentAttack() const{
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
		
		private void DoAttack( InfoSystem::AttackInfo@ atk ) {
			TheNomad::GameSystem::RayCast@ rayData = null;
			
			switch ( atk.attackMethod ) {
			case InfoSystem::AttackMethod::Hitscan: {
				TheNomad::GameSystem::RayCast ray;
				
				@rayData = @ray; // force it out of scope
				
				ray.m_Origin = m_Link.m_Origin;
				ray.m_nLength = atk.range;
				ray.m_nAngle = m_PhysicsObject.GetAngle();
				ray.m_nEntityNumber = 0;

				TheNomad::GameSystem::CastRay( @ray );

				if ( ray.m_nEntityNumber == EntityManager.NumEntities() ) {
					return; // got nothing
				}
				break; }
			case InfoSystem::AttackMethod::Projectile:
				EntityManager.SpawnProjectile( m_Link.m_Origin, m_PhysicsObject.GetAngle(), @atk,
					vec2( m_CurrentAttack.projectileWidth, m_CurrentAttack.projectileHeight ) );
				return; // we'll let the entity manager deal with it now
			default:
				// should theoretically NEVER happen
				GameError( "MobObject::DoAttack: invalid attack method " + uint( atk.attackMethod ) );
			};
			
			if ( atk.effect.size() > 0 ) {
				// apply effects
				TheNomad::Engine::CmdExecuteCommand( atk.effect + " " + m_Link.m_nEntityNumber + "\n" );
			}
			
			m_CurrentAttack.sound.Play();
			
			EntityManager.DamageEntity( cast<EntityObject@>( @this ), @rayData );
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
				EntityManager.KillEntity( @source, cast<EntityObject@>( @this ) );
				
				// alert mobs within the vicinity
				EntityManager.GetPlayerObject().MakeSound();
				
				return;
			}
			
			TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( m_Info.painSfx );
			
			if ( @source !is @m_Target ) {
				SetTarget( @source );
			}
		}
		
		void FaceTarget() {
			if ( @m_Target is null ) {
				return;
			}
			// set them facing the target
			m_PhysicsObject.SetAngle( atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y ) );
			m_Direction = Util::Angle2Dir( m_PhysicsObject.GetAngle() );
		}
		
		void Think() {
			switch ( m_State.GetID() - m_Info.type ) {
			case StateNum::ST_MOB_IDLE:
				IdleThink();
				break;
			case StateNum::ST_MOB_FIGHT_MELEE:
			case StateNum::ST_MOB_FIGHT_MISSILE:
			case StateNum::ST_MOB_FIGHT:
				FightThink();
				break;
			case StateNum::ST_MOB_CHASE:
				ChaseThink();
				break;
			default:
				GameError( "MobEntity::Think: invalid mob state " + formatUInt( m_State.GetID() ) + "\n" );
				break;
			};
		}
		
		private void SetTarget( EntityObject@ newTarget ) {
			@m_Target = @newTarget;
			m_PhysicsObject.SetAngle( atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y ) );
			m_Direction = Util::Angle2Dir( m_PhysicsObject.GetAngle() );
			
			DebugPrint( "Set mob target to " + m_Target.GetEntityNum() + "\n" );
		}
		
		private void ShuffleDirection() {
			if ( m_Direction == TheNomad::GameSystem::DirType::North ) {
				m_Direction = TheNomad::GameSystem::DirType::Inside;
			} else {
				m_Direction--;
			}
			m_PhysicsObject.SetAngle( Util::Dir2Angle( m_Direction ) );
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
			
			ray.m_nLength = Util::VectorLength( m_Info.speed );
			ray.m_nAngle = m_PhysicsObject.GetAngle();
			ray.m_Origin = m_Link.m_Origin;
			
			TheNomad::GameSystem::CastRay( @ray );
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
					ShuffleDirection();
				}
				return false;
			} else if ( ray.m_nEntityNumber >= EntityManager.NumEntities() ) {
				GameError( "MobObject::CheckCollision: out of range entity number" );
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
				m_PhysicsObject.SetAngle( atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y ) );
			}
			
			m_PhysicsObject.OnRunTic();
		}
		
		//===========================================================
		//
		// Detection functions
		//
		//===========================================================
		
		void SetAlert( EntityObject@ ent, float dist ) {
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			const float scale = LevelManager.GetDifficultyScale();

			if ( m_nAlertLevel > uint( 6 / scale ) ) {
				SetState( StateNum::ST_MOB_FIGHT );
			}
			
			// wait roughly 6 seconds before another check
			if ( gameTic - m_nLastAlertTime > uint( 6000 / scale ) ) {
				m_nLastAlertTime = 0;
				m_nAlertLevel++;
			} else {
				m_nLastAlertTime += TheNomad::GameSystem::GameManager.GetGameTic();
			}
		}
		
		private bool SightCheck() {
			if ( ( m_Info.flags & InfoSystem::MobFlags::Blind ) != 0 ) {
				return false;
			}
			
			TheNomad::GameSystem::RayCast ray;
			vec3 delta, pos, p;
			EntityObject@ ent;
			
			delta.x = cos( m_PhysicsObject.GetAngle() );
			delta.y = sin( m_PhysicsObject.GetAngle() );
			delta.z = 0.0f;
			
			pos = EntityManager.GetPlayerObject().GetOrigin() - m_Link.m_Origin;
			
			p = Util::VectorNormalize( pos );
			
			if ( Util::DotProduct( pos, delta ) > cos( m_Info.sightRadius ) ) {
				return false;
			}
			
			//
			// make sure that the line of sight isn't obstructed
			//
			ray.m_nLength = m_Info.sightRange;
			ray.m_Origin = m_Link.m_Origin;
			ray.m_nEntityNumber = 0;
			ray.m_nAngle = m_PhysicsObject.GetAngle();
			TheNomad::GameSystem::CastRay( @ray );
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				return false;
			} else if ( ray.m_nEntityNumber >= EntityManager.NumEntities() ) {
				GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
			}

			@ent = @EntityManager.GetEntityForNum( ray.m_nEntityNumber );
			
			// TODO: make this more advanced
			if ( ent.GetType() != TheNomad::GameSystem::EntityType::Mob ) {
				if ( @m_Target is @ent ) {
					FaceTarget();
					return true; // might be infighting
				} else {
					return false;
				}
			}
			else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				// we see them, no need for wall checks
				SetAlert( @ent, Util::Distance( m_Link.m_Origin, ent.GetOrigin() ) );
			}
			
			@m_Target = @ent;
			DebugPrint( "Set mob target to " + ent.GetEntityNum() + "\n" );
			m_Info.wakeupSfx.Play();
			
			// we got something
			return true;
		}
		
		private bool SoundCheck() {
			if ( ( m_Info.flags & InfoSystem::MobFlags::Deaf ) != 0 ) {
				return false;
			}
			
			return true;
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
			const float dist = Util::Distance( m_Target.GetOrigin(), m_Link.m_Origin );
			
			// we need a more specific fighting state
			if ( m_State.GetID() - m_Info.type == StateNum::ST_MOB_FIGHT ) {
				@m_CurrentAttack = null;
				for ( uint i = 0; i < m_Info.attacks.Count(); i++ ) {
					if ( dist < m_Info.attacks[i].range && m_Info.attacks[i].valid ) {
						attackType = m_Info.attacks[i].attackType;
						SetState( StateNum::ST_MOB_FIGHT_MELEE );
						@m_CurrentAttack = @m_Info.attacks[i];
						break;
					}
				}
				if ( @m_CurrentAttack is null ) {
					// move closer
					SetState( StateNum::ST_MOB_CHASE );
					return; // cycle it
				}
			}
			
			if ( m_nAttackTime < m_nLastAttackTime && !m_bIsAttacking ) {
				// force a short delay before actually attacking to give
				// the player a little bit of leeway
				m_nAttackTime = TheNomad::GameSystem::GameManager.GetGameTic();
			}
			else if ( m_nAttackTime - m_nLastAttackTime > m_CurrentAttack.cooldown && !m_bIsAttacking ) {
				// initiate the attack
				m_bIsAttacking = true;
				
				if ( m_CurrentAttack.attackMethod == InfoSystem::AttackMethod::Projectile ) {
					// spawn an independent projectile entity
					m_bIsAttacking = false;
					// NOTE: spawn with an offset?
					DoAttack( @m_CurrentAttack );
				}
			}
			else if ( m_bIsAttacking ) {
				// we are attacking, take aim, and draw the parry indicator
				vec4 color;
				
				if ( m_CurrentAttack.canParry && m_nAttackTime <= ( m_CurrentAttack.duration / 2 ) ) {
					color = colorGreen;
				} else {
					color = colorRed;
				}
				TheNomad::Engine::Renderer::SetColor( color );
				switch ( m_CurrentAttack.attackMethod ) {
				case InfoSystem::AttackMethod::Hitscan:
					// boolets
					break;
				case InfoSystem::AttackMethod::RayCast:
					// melee without the AOE
					break;
				case InfoSystem::AttackMethod::AreaOfEffect:
					// this will draw a circle as a texture, then overlay it with color
					break;
				case InfoSystem::AttackMethod::Projectile:
					// shouldn't really be happening...
					DebugPrint( "MobObject::FightThink: attack method is projectile, but we're calculating a parry\n" );
					return;
				};
				
				if ( m_nAttackTime >= m_CurrentAttack.duration ) {
					// SHOOT!
					DoAttack( @m_CurrentAttack );
					m_nLastAttackTime = TheNomad::GameSystem::GameManager.GetGameTic();
					m_nAttackTime = 0;
					m_bIsAttacking = false;
					return;
				} else if ( m_nAttackTime <= ( m_CurrentAttack.duration / 2 ) ) {
					// parry it
					if ( EntityManager.GetPlayerObject().CheckParry( @this, @m_CurrentAttack ) ) {
						m_nLastAttackTime = TheNomad::GameSystem::GameManager.GetGameTic();
						m_nAttackTime = 0;
						m_bIsAttacking = false;
						return;
					}
				}
			}
			// increment the delta time
			m_nAttackTime += TheNomad::GameSystem::GameManager.GetGameTic();
		}
		
		private void IdleThink() {
			// move around a little
			TryMove();
			
			if ( @m_Target !is null ) {
				SetState( StateNum::ST_MOB_CHASE );
			}
			
			if ( m_State.GetTics() > m_Info.waitTics ) {
				if ( SoundCheck() || SightCheck() ) {
					SetState( StateNum::ST_MOB_FIGHT );
					return;
				}
			}
		}
		
		private InfoSystem::MobFlags m_MFlags = InfoSystem::MobFlags( 0 );
		private InfoSystem::AttackInfo@ m_CurrentAttack = null;
		private EntityObject@ m_Target = null;
		private InfoSystem::MobInfo@ m_Info = null;
		
		// stealth tracking
		private uint m_nLastAlertTime = 0;
		private uint m_nAlertLevel = 0;
		
		// attack data
		private bool m_bIsAttacking = false;
		private uint m_nAttackTime = 0;
		private uint m_nLastAttackTime = 0;
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