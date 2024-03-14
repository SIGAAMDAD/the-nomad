#include "entity.as"
#include "level.as"

namespace TheNomad::SGame {
	enum SpecialMobAction {
		MA_GetBehind,
		MA_SneakAttack,
	};
	
	shared class MobEntity : EntityObject, EntityData {
		MobEntity() {
		}
		
		void Think() {
		}
		void Spawn() {
		}
		void Damage( uint nAmount ) {
			
		}
		
		EntityObject@ GetBase() {
			return @m_Base;
		}
		const EntityObject@ GetBase() const {
			return @m_Base;
		}
		
		const EntityObject@ GetTarget() const {
			return m_Target;
		}
		EntityObject@ GetTarget() {
			return m_Target;
		}
		
		
		private void ChaseThink() {
		}
		
		private void SpecialBehaviour() {
		}
		
		private void DoAttack( const AttackInfo@ in atk ) {
			RayCast@ rayData;
			
			switch ( atk.method ) {
			case AttackMethod::HitScan:
				RayCast ray;
				
				@rayData = @ray; // force it out of scope
				
				ray.origin = m_Origin;
				ray.length = atk.range;
				ray.angle = Dir2Angle( m_Direction );
				ray.link = null;
				
				if ( TheNomad::GameSystem::CastRay( ray ) is false ) {
					return; // got nothing
				}
				break;
			case AttackMethod::Projectile:
				EntityManager.SpawnProjectile( atk );
				return; // we'll let the entity manager deal with it now
			default:
				// should theoretically NEVER happen
				GameError( "MobObject::DoAttack: invalid attack method " + formatUInt( uint( atk.method ) ) );
			};
			
			if ( atk.effect.size() ) {
				// apply effects
				TheNomad::Engine::CmdExecute( atk.effect + " " + formatUInt( m_Link.GetNum() ) + "\n" );
			}
			
			EntityManager.DamageEntity( this, rayData );
		}
		private void FightThink() {
			if ( m_Target is null ) {
				// if there's no target, or if the target went out of sight,
				// then pursue
				SetState( StateNum::ST_MOB_CHASE );
				return;
			}
			
			AttackType attackType;
			const float dist = Distance( m_Target.GetOrigin, m_Origin );
			
			// we need a more specific fighting state
			if ( m_State.GetID() == StateNum::ST_MOB_FIGHT ) {
				if ( dist < m_Info.melee.range && m_Info.melee.valid ) {
					attackType = AttackType::Melee;
					@m_State = StateManager.GetStateForNum( m_Info.melee.stateId );
					@m_CurrentAttack = m_Info.melee;
				}
				else if ( dist < m_Info.missile.range && m_Info.missile.valid ) {
					attackType = AttackType::Missile;
					@m_State = StateManager.GetStateForNum( m_Info.missile.stateId );
					@m_CurrentAttack = m_Info.missile;
				}
				else {
					// move closer
					SetState( StateNum::ST_MOB_CHASE );
				}
				return; // cycle it
			}
			m_State.Run();
			
			// wait for it...
			if ( m_State.Tics() < m_CurrentAttack.cooldown ) {
				return;
			}
			
			DoAttack( m_CurrentAttack );
			
			m_State.Loop();
		}
		
		private void IdleThink() {
			// move around a little
			TryMove();
			
			if ( m_State.Tics() > m_Info.waitTics ) {
				if ( SoundCheck() || SightCheck() ) {
					SetState( StateNum::ST_MOB_FIGHT );
					return;
				}
			}
		}
		
		void Think() override {
			switch ( m_State.GetID() ) {
			case StateNum::ST_MOB_IDLE:
				IdleThink();
				break;
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
			case TheNomad::GameSystem::DirType::South:
				value = m_Info.speed.y;
				break;
			case TheNomad::GameSystem::DirType::NorthEast:
			case TheNomad::GameSystem::DirType::NorthWest:
			case TheNomad::GameSystem::DirType::SouthEast:
			case TheNomad::GameSystem::DirType::SouthWest:
			case TheNomad::GameSystem::DirType::East:
			case TheNomad::GameSystem::DirType::West:
				value = m_Info.speed.x;
				break;
			default:
				GameError( "MobObject::GetSpeed: invalid direction " + formatUInt( uint( m_Direction ) ) );
				return -1;
			};
			
			return value;
		}
		
		private void TryMove() {
			RayCast ray;
			
			ray.length = GetSpeed();
			ray.angle = Dir2Angle( m_Direction );
			ray.origin = m_Origin;
			
			if ( TheNomad::GameSystem::CastRay( ray ) is false ) {
				// move failed, attempt a new direction
				ShuffleDirection();
				return;
			}
			
			// NOTE: maybe get some adaptive velocity stuff here?
			
			// something in the way
			if ( ray.link != null ) {
				ShuffleDirection();
				return;
			}
			
			m_Origin = ray.origin;
		}
		
		private bool SightCheck() {
			if ( m_Info.flags & MobFlags::Blind ) {
				return false;
			}
			
			RayCast ray;
			
			ray.length = m_Info.sightRange;
			ray.origin = m_Origin;
			ray.angle = Dir2Angle( m_Direction );
			
			if ( TheNomad::GameSystem::CastRay( ray ) is false ) {
				return false;
			}
			
			// TODO: make this more advanced
			if ( ray.link.GetType() != TheNomad::GameSystem::EntityType::Playr ) {
				return false;
			}
			
			m_Target = EntityManager.GetEntityForNum( ray.link.GetNum() );
			if ( m_Target is null ) {
				// error?
				DebugPrint( "MobObject::SightCheck: got valid entity in raycast, but link number is invalid (" +
					formatUInt( ray.link.GetNum() ) + ")\n" );
				return false;
			}
			
			TheNomad::Engine::SoundSystem::PlaySfx( m_Info.wakeupSfx );
			
			// we got something
			return true;
		}
		private bool SoundCheck() {
			if ( m_Info.flags & MobFlags::Deaf ) {
				return false;
			}
			
			const float rangeX = m_Info.detectionRangeX / 2;
			const float rangeY = m_Info.detectionRangeY / 2;
			const vec3 start( m_Origin.x - rangeX, m_Origin.y - rangeY, m_Origin.z );
			const vec3 end( m_Origin.x + rangeX, m_Origin.y + rangeY, m_Origin.z );
			
			const array<float>& soundBits = data.GetSoundBits()[m_Origin.z].GetData();
			
			for ( uint y = uint( start.y ); y != uint( end.y ); y++ ) {
				for ( uint x = uint( start.x ); x != uint( end.x ); x++ ) {
					if ( soundBits[ y * data.GetWidth() + x ] - Distance( m_Origin, vec3( x, y, m_Origin.z ) ) >= m_Info.soundTolerance ) {
						return true;
					}
				}
			}
			
			return false;
		}
		
		EntityObject@ m_Base;
		private AttackInfo@ m_CurrentAttack;
		private EntityObject@ m_Target;
		private const MobInfo@ m_Info;
	};
};