#include "info.as"
#include "entity.as"
#include "level.as"

namespace TheNomad::SGame {
	enum SpecialMobAction {
		MA_GetBehind,
		MA_SneakAttack,
	};

	shared enum MobFlags {
		Deaf      = 0x0001,
		Blind     = 0x0002,
		Terrified = 0x0004,
		Boss      = 0x0008
	};
	
	class MobObject : EntityObject {
		MobObject() {
			m_EffectString.reserve( MAX_TOKEN_CHARS );
		}

		EntityObject@ GetBase() {
			return @m_Base;
		}
		const EntityObject@ GetBase() const {
			return @m_Base;
		}

		AttackInfo@ CurrentAttack() {
			return @m_CurrentAttack;
		}
		const AttackInfo@ CurrentAttack() const{
			return @m_CurrentAttack;
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
		
		private void DoAttack( const AttackInfo@ atk ) {
			TheNomad::GameSystem::RayCast@ rayData;
			
			switch ( atk.method ) {
			case AttackMethod::Hitscan: {
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
			case AttackMethod::Projectile:
				EntityManager.SpawnProjectile( m_Link.m_Origin, m_nAngle, @atk );
				return; // we'll let the entity manager deal with it now
			default:
				// should theoretically NEVER happen
				GameError( "MobObject::DoAttack: invalid attack method " + formatUInt( uint( atk.method ) ) );
			};
			
			if ( atk.effect.size() > 0 ) {
				// apply effects
				m_EffectString = atk.effect;
				m_EffectString += " ";
				m_EffectString += m_Link.m_nEntityNumber;
				m_EffectString += "\n";
				TheNomad::Engine::CmdExecuteCommand( m_EffectString );
			}
			
			EntityManager.DamageEntity( GetBase(), rayData, @atk );
		}
		private void FightThink() {
			if ( @m_Target is null ) {
				// if there's no target, or if the target went out of sight,
				// then pursue
				SetState( StateNum::ST_MOB_CHASE );
				return;
			}
			
			AttackType attackType;
			const float dist = TheNomad::Util::Distance( m_Target.GetOrigin(), m_Link.m_Origin );
			
			// we need a more specific fighting state
			if ( m_State.GetID() == StateNum::ST_MOB_FIGHT ) {
				@m_CurrentAttack = null;
				for ( uint i = 0; i < m_Info.attacks.size(); i++ ) {
					if ( dist < m_Info.attacks[i].range && m_Info.attacks[i].valid ) {
						attackType = m_Info.attacks[i].type;
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
			if ( m_State.Tics() < m_CurrentAttack.cooldown ) {
				return;
			}
			
			DoAttack( m_CurrentAttack );
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
		
		void Spawn( const vec3& in origin ) {

		}
		void Think() {
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
			if ( !TheNomad::GameSystem::CheckWallHit( m_Link.m_Origin, m_Direction ) ) {
				// move failed, attempt a new direction
				ShuffleDirection();
				return;
			}
			
			// NOTE: maybe get some adaptive velocity stuff here?
			m_Link.m_Origin += m_Velocity;
		}
		
		private bool SightCheck() {
			if ( ( m_Info.flags & MobFlags::Blind ) != 0 ) {
				return false;
			}
			
			TheNomad::GameSystem::RayCast ray;
			EntityObject@ ent;
			
			ray.m_nLength = m_Info.sightRange;
			ray.m_Origin = m_Link.m_Origin;
			ray.m_nAngle = TheNomad::Util::Dir2Angle( m_Direction );

			TheNomad::GameSystem::CastRay( @ray );
			
			if ( ray.m_nEntityNumber == EntityManager.NumEntities() ) {
				return false;
			}
			if ( ray.m_nEntityNumber > EntityManager.NumEntities() ) {
				GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
			}

			@ent = @EntityManager.GetEntityForNum( ray.m_nEntityNumber );
			
			// TODO: make this more advanced
			if ( ent.GetType() != TheNomad::GameSystem::EntityType::Playr ) {
				return false;
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
			if ( ( m_Info.flags & MobFlags::Deaf ) != 0 ) {
				return false;
			}

			const float rangeX = m_Info.soundRangeX / 2;
			const float rangeY = m_Info.soundRangeY / 2;
			const vec3 start( m_Link.m_Origin.x - rangeX, m_Link.m_Origin.y - rangeY, m_Link.m_Origin.z );
			const vec3 end( m_Link.m_Origin.x + rangeX, m_Link.m_Origin.y + rangeY, m_Link.m_Origin.z );
			
			const MapData@ data = @LevelManager.GetMapData();
			const float[]@ soundBits = data.GetSoundBits()[uint( floor( m_Link.m_Origin.z ) )].GetData();
			
			for ( uint y = uint( start.y ); y != uint( end.y ); y++ ) {
				for ( uint x = uint( start.x ); x != uint( end.x ); x++ ) {
					if ( soundBits[y * data.GetWidth() + x] - TheNomad::Util::Distance( m_Link.m_Origin, vec3( x, y, m_Link.m_Origin.z ) ) >= m_Info.soundTolerance ) {
						return true;
					}
				}
			}
			
			return false;
		}
		
		private string m_EffectString;
		private AttackInfo@ m_CurrentAttack;
		private EntityObject@ m_Target;
		private MobInfo@ m_Info;
	};
};