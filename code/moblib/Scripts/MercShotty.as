#include "moblib/MobScript.as"

namespace moblib::Script {
	const uint MERC_AIM_TIME = 2500;
	const float MERC_SHOTGUN_DAMAGE = 25.0f;
	const float MERC_SHOTGUN_RANGE = 20.75f;
	const uint MERC_BARK_COOLDOWN = 300;
	
	final class MercShotty : MobScript {
		MercShotty() {
		}
		
		private float CalcMissChance() const {
			const TheNomad::SGame::EntityObject@ target = @m_EntityData.GetTarget();
			const float missChance = TheNomad::Util::Distance( target.GetOrigin(), m_EntityData.GetOrigin() );
			
			return missChance * m_nAimScale;
		}

		private void Bark( int hSfx ) {
			if ( ( TheNomad::GameSystem::GameTic - m_nLastBarkTime ) * TheNomad::GameSystem::DeltaTic < MERC_BARK_COOLDOWN ) {
				return;
			}
			m_nLastBarkTime = TheNomad::GameSystem::GameTic;
			m_EntityData.EmitSound( hSfx, 10.0f, 0xff );
		}
		
		/* TODO: implement
		private bool FindCover() {
			if ( m_bInCover ) {
				return true;
			}
			
			if ( TheNomad::Util::Angle2Dir( ray.m_nAngle ) == InverseDirs[  ] ) {
				
			}
			
			TheNomad::GameSystem::RayCast ray;
			ray.m_Start = m_EntityData.GetOrigin();
			ray.m_nLength = m_EntityData.GetInfo().sightRange;
			ray.m_nAngle = TheNomad::Util::Dir2Angle( m_EntityData.GetDirection() );
			
			ray.Cast();
			
			if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				m_CoverPosition = uvec2( uint( ray.m_Origin.x ), uint( ray.m_Origin.y ) );
				m_bInCover = true;
				m_bSearchForCover = false;
				return true;
			}
			
			return false;
		}
		*/
		
		void FleeThink() override {
			if ( !m_Sensor.CheckSight() ) {
				// they don't see their target, let them take a breather
				m_EntityData.EmitSound(
					TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/relief_" + ( TheNomad::Util::PRandom() & 2 ) ),
					1.0f,
					0xff
				);
			}
			else {
				m_EntityData.EmitSound(
					TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/get_away_" + ( TheNomad::Util::PRandom() & 2 ) ),
					1.0f,
					0xff
				);
			}
		}
		void IdleThink() override {
			if ( m_Sensor.CheckSight() ) {
				Bark( ResourceCache.ShottyTargetSpottedSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyTargetSpottedSfx.Count() - 1 ) ] );
				m_EntityData.SetState( @m_ChaseState );
			}
			else if ( m_Sensor.CheckSound() ) {
				Bark( ResourceCache.ShottyConfusionSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyConfusionSfx.Count() - 1 ) ] );
				m_EntityData.SetState( @m_SearchState );
			}
		}
		void DeadThink() override {
		}
		void FightMissile() override {
			if ( !m_SubState.Done( m_nSubTicker ) ) {
				// while the merc is aiming, allow a parry
				m_EntityData.SetParry( true );
				return;
			}

			// clear sub state
			m_SubState.Reset( m_nSubTicker );
			
			m_EntityData.SetParry( false );
			m_EntityData.EmitSound( ResourceCache.ShottyAttackSfx, 10.0f, 0xff );
			
			TheNomad::GameSystem::RayCast ray;
			
			ray.m_Start = m_EntityData.GetOrigin();
			ray.m_nLength = MERC_SHOTGUN_RANGE;
			ray.m_nAngle = m_EntityData.GetAngle();
			ray.m_nOwner = m_EntityData.GetEntityNum();
			ray.Cast();

			m_EntityData.SetState( @m_ChaseState );

			if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				// TODO: add wall hit mark here
				return;
			} else if ( ray.m_nEntityNumber == ENTITYNUM_INVALID ) {
				return;
			}
			
			TheNomad::SGame::EntityManager.DamageEntity( @TheNomad::SGame::EntityManager.GetEntityForNum( ray.m_nEntityNumber ),
				cast<TheNomad::SGame::EntityObject@>( @m_EntityData ) );
		}
		void FightMelee() override {
		}
		void SearchThink() override {
			const bool canSee = m_Sensor.CheckSight();
			if ( canSee ) {
				Bark( ResourceCache.ShottyTargetSpottedSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyTargetSpottedSfx.Count() - 1 ) ] );
				m_EntityData.SetState( @m_ChaseState );
			}
			m_EntityData.Chase();
		}

		private bool IsAllyNearby() const {
			TheNomad::GameSystem::RayCast ray;

			ray.m_Start = m_EntityData.GetOrigin();
			ray.m_nLength = MERC_SHOTGUN_RANGE;
			ray.m_nAngle = m_EntityData.GetAngle();
			ray.m_nOwner = m_EntityData.GetEntityNum();
			ray.Cast();

			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				return false;
			}

			return TheNomad::SGame::EntityManager.GetEntityForNum( ray.m_nEntityNumber ).GetType() == TheNomad::GameSystem::EntityType::Mob;
		}
		void ChaseThink() override {
			const vec3 origin = m_EntityData.GetOrigin();
			const vec3 target = m_EntityData.GetTarget().GetOrigin();
			const bool canSee = m_Sensor.CheckSight();
			if ( canSee ) {
				if ( TheNomad::Util::DotProduct( target, origin ) > TheNomad::Util::DotProduct( m_OldTargetPosition, origin ) ) {
					Bark( ResourceCache.ShottyTargetRunningSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyTargetRunningSfx.Count() - 1 ) ] );
				}
				m_OldTargetPosition = target;
			} else {
				Bark( ResourceCache.ShottyTargetEscapedSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyTargetEscapedSfx.Count() - 1 ) ] );
				m_EntityData.SetState( @m_SearchState );
			}
			m_EntityData.Chase();

			if ( @m_SubState !is @ResourceCache.ShottyAimState ) {
				if ( TheNomad::Util::Distance( origin, target ) <= MERC_SHOTGUN_RANGE && canSee ) {
					// only start aiming if we can see the target
					@m_SubState = @ResourceCache.ShottyAimState;
					m_SubState.Reset( m_nSubTicker );
					m_EntityData.EmitSound( ResourceCache.ShottyAimSfx, 10.0f, 0xff );
					m_EntityData.SetState( @m_FightMissileState );

					if ( IsAllyNearby() ) {
						Bark( ResourceCache.ShottyOutOfTheWay[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyOutOfTheWay.Count() - 1 ) ] );
					}
				}
			}
		}
		void OnSpawn() override {
			// canonically, the higher the difficulty, the more risky the mission.
			// the riskier the mission, the more elite the enemies will be
			//
			// also give the player a lot of leeway
			switch ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) ) {
			case TheNomad::GameSystem::GameDifficulty::Easy:
				m_nAggressionScale = 1;
				m_nAimScale = 0.25f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Normal:
				m_nAggressionScale = 2;
				m_nAimScale = 0.60f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Hard:
				m_nAggressionScale = 3;
				m_nAimScale = 1.0f;
				break;
			case TheNomad::GameSystem::GameDifficulty::VeryHard:
				m_nAimScale = 1.25f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Insane:
				// the elite
				m_nAimScale = 1.75f;
				break;
			};

			m_EntityData.SetDirection( TheNomad::GameSystem::DirType::South );
		}
		void OnDeath() override {
		}
		
		/*
		private uvec2 m_CoverPosition;
		private TheNomad::GameSystem::DirType m_nCoverDirection;
		private bool m_bInCover = false;
		private bool m_bSearchForCover = false;
		*/

		private vec3 m_OldTargetPosition = vec3( 0.0f );
		private uint m_nLastBarkTime = 0;
		private int m_hLastBark = FS_INVALID_HANDLE;

		private uint m_nAimStartTic = 0;
		private uint m_nAggressionScale = 0;
		private float m_nAimScale = 0.0f;
		
		private float m_nFearAmount = 0.0f;
		
		private uint m_nSubTicker = 0;
		private TheNomad::SGame::EntityState@ m_SubState = null;
	};
};