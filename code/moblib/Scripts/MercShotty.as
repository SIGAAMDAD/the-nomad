#include "moblib/MobScript.as"
#include "moblib/System/AISquad.as"

namespace moblib::Script {
	const uint MERC_AIM_TIME = 2500;
	float MERC_SHOTGUN_DAMAGE = 25.0f;
	float MERC_SHOTGUN_RANGE = 20.75f;
	const uint MERC_BARK_COOLDOWN = 300;
	const uint MERC_AGGRESSION_SCALE = 6;
	
	final class MercShotty : MobScript {
		MercShotty() {
		}
		
		private float CalcMissChance() const {
			const TheNomad::SGame::EntityObject@ target = @m_EntityData.GetTarget();
			const float missChance = TheNomad::Util::Distance( target.GetOrigin(), m_EntityData.GetOrigin() );
			
			return missChance * m_nAimScale;
		}

		private void ChangeState( int hBark, TheNomad::SGame::EntityState@ state ) {
			m_Squad.SquadBark( hBark );
			m_EntityData.SetState( @state );
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
				ChangeState( ResourceCache.ShottyTargetSpottedSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyTargetSpottedSfx.Count() - 1 ) ],
					@m_ChaseState );
				m_EntityData.FaceTarget();
			}
			else if ( m_Sensor.CheckSound() ) {
				ChangeState( ResourceCache.ShottyConfusionSfx[ TheNomad::Util::PRandom()
					& ( ResourceCache.ShottyConfusionSfx.Count() - 1 ) ], @m_ChaseState );
				m_EntityData.FaceTarget();
				m_nLastCheckTime = TheNomad::GameSystem::GameTic;
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

			// NOTE: maybe add something in here if the player parries them up close right before they shoot
			
			TheNomad::GameSystem::RayCast ray;
			
			const vec3 origin = m_EntityData.GetOrigin();

			ray.m_Start = origin;
			ray.m_nLength = MERC_SHOTGUN_RANGE;
			ray.m_nAngle = m_EntityData.GetAngle();
			ray.m_nOwner = m_EntityData.GetEntityNum();
			ray.Cast();

			if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				// TODO: add wall hit mark here
				const float velocity = ray.m_nLength - TheNomad::Util::Distance( ray.m_Origin, ray.m_Start );
				TheNomad::SGame::GfxManager.AddDebrisCloud( ray.m_Origin, velocity );
				TheNomad::SGame::GfxManager.AddBulletHole( ray.m_Origin );
				m_EntityData.EmitSound( ResourceCache.ShottyAttackSfx, 10.0f, 0xff );
				TheNomad::SGame::GfxManager.AddMuzzleFlash( origin );
				
				TheNomad::SGame::PlayrObject@ player = TheNomad::SGame::EntityManager.GetActivePlayer();
				
				if ( TheNomad::Util::Distance( player.GetOrigin(), ray.m_Origin ) <= 2.90f ) {
					// if we're close to the bullet, then simulate a near-hit
					player.EmitSound(
						TheNomad::Engine::SoundSystem::RegisterSfx(
							"event:/sfx/env/bullet_impact/ricochet_" + ( TheNomad::Util::PRandom() & 2 )
						),
						10.0f, 0xff
					);
					// TODO: shake screen?
				}
				return;
			} else if ( ray.m_nEntityNumber == ENTITYNUM_INVALID ) {
				TheNomad::SGame::PlayrObject@ player = TheNomad::SGame::EntityManager.GetActivePlayer();
				
				if ( TheNomad::Util::Distance( player.GetOrigin(), ray.m_Origin ) <= 2.90f ) {
					// if we're close to the bullet, then simulate a near-hit
					player.EmitSound(
						TheNomad::Engine::SoundSystem::RegisterSfx(
							"event:/sfx/env/bullet_impact/ricochet_" + ( TheNomad::Util::PRandom() & 2 )
						),
						10.0f, 0xff
					);
					// TODO: shake screen?
				}
				return;
			}
			
			TheNomad::SGame::EntityObject@ hit = TheNomad::SGame::EntityManager.GetEntityForNum( ray.m_nEntityNumber );
			if ( hit.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				m_EntityData.EmitSound( ResourceCache.ShottyOutOfTheWay[ TheNomad::Util::PRandom()
					& ( ResourceCache.ShottyOutOfTheWay.Count() - 1 ) ], 10.0f, 0xff );
				return; // don't shoot
			}
			m_EntityData.EmitSound( ResourceCache.ShottyAttackSfx, 10.0f, 0xff );
			TheNomad::SGame::GfxManager.AddMuzzleFlash( origin );

			TheNomad::SGame::EntityManager.DamageEntity( hit, m_EntityData, MERC_SHOTGUN_DAMAGE );
		}
		void FightMelee() override {
		}
		void SearchThink() override {
			const bool canSee = m_Sensor.CheckSight();
			if ( canSee ) {
				ChangeState( ResourceCache.ShottyTargetSpottedSfx[ TheNomad::Util::PRandom() &
					( ResourceCache.ShottyTargetSpottedSfx.Count() - 1 ) ], @m_ChaseState );
			}
			else if ( m_Sensor.CheckSound() ) {
				ChangeState( ResourceCache.ShottyAlertSfx[ TheNomad::Util::PRandom() &
					( ResourceCache.ShottyAlertSfx.Count() - 1 ) ], @m_ChaseState );
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
			if ( @m_SubState !is null && !m_SubState.Done( m_nSubTicker ) ) {
				return;
			}

			const bool canSee = m_Sensor.CheckSight();
			const vec3 origin = m_EntityData.GetOrigin();
			const vec3 target = m_EntityData.GetTarget().GetOrigin();
			if ( canSee ) {
				/*
				if ( TheNomad::Util::DotProduct( target, origin ) > TheNomad::Util::DotProduct( m_OldTargetPosition, origin ) ) {
					m_EntityData.EmitSound( ResourceCache.ShottyTargetRunningSfx[ TheNomad::Util::PRandom()
						& ( ResourceCache.ShottyTargetRunningSfx.Count() - 1 ) ], 10.0f, 0xff );
				}
				*/
				m_OldTargetPosition = target;
			}
			if ( TheNomad::Util::Distance( origin, target ) >= MERC_SHOTGUN_RANGE / 4 ) {
				m_EntityData.SetState( @m_EntityData.GetState().Run( m_EntityData.GetTicker() ) );
				m_EntityData.Chase();
				return;
			}
			if ( ( TheNomad::Util::PRandom() & MERC_AGGRESSION_SCALE ) >= m_nAggressionScale ) {
//				return;
			}
			if ( TheNomad::Util::Distance( origin, target ) <= MERC_SHOTGUN_RANGE ) {
				// only start aiming if we can see the target
				@m_SubState = @ResourceCache.ShottyAimState;
				m_SubState.Reset( m_nSubTicker );
				m_EntityData.FaceTarget();
				m_EntityData.SetState( @m_FightMissileState );
				m_EntityData.EmitSound( ResourceCache.ShottyAimSfx, 10.0f, 0xff );
			}
		}
		void OnDamage( TheNomad::SGame::EntityObject@ attacker ) override {
			if ( attacker.GetType() == TheNomad::GameSystem::EntityType::Mob
				&& cast<TheNomad::SGame::MobObject@>( attacker ).GetMobInfo().type == MOB_ID::MERC_SHOTGUNNER )
			{
				m_EntityData.EmitSound( ResourceCache.ShottyCeasfire[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyCeasfire.Count() - 1 ) ],
					10.0f, 0xff );
				return;
			}
			m_EntityData.EmitSound( ResourceCache.ShottyPain[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyPain.Count() - 1 ) ], 10.0f, 0xff );
			if ( ( TheNomad::Util::PRandom() & 50 ) >= 25 ) {
				m_EntityData.EmitSound( ResourceCache.ShottyCurse[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyCurse.Count() - 1 ) ], 10.0f, 0xff );
			}

			// causes an assertion crash
			if ( m_EntityData.GetState() is m_IdleState ) {
				m_EntityData.SetState( m_ChaseState );
			}
		}
		void OnSpawn() override {
			// canonically, the higher the difficulty, the more risky the mission.
			// the riskier the mission, the more elite the enemies will be
			//
			// also give the player a lot of leeway
			TheNomad::SGame::InfoSystem::MobInfo@ info = m_EntityData.GetMobInfo();
			switch ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) ) {
			case TheNomad::GameSystem::GameDifficulty::Easy:
				m_nAggressionScale = 1;
				m_nAimScale = 0.25f;

				// they have the least experience with firearms, they can't hit a far shot
				MERC_SHOTGUN_RANGE = 8.75f;

				// they aren't hitting vital spots
				MERC_SHOTGUN_DAMAGE = 8.5f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Normal:
				m_nAggressionScale = 2;
				m_nAimScale = 0.60f;

				MERC_SHOTGUN_RANGE = 10.0f;
				MERC_SHOTGUN_DAMAGE = 13.0f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Hard:
				m_nAggressionScale = 3;
				m_nAimScale = 1.0f;

				MERC_SHOTGUN_RANGE = 14.75f;
				MERC_SHOTGUN_DAMAGE = 35.0f;

				info.health = 90.0f;
				break;
			case TheNomad::GameSystem::GameDifficulty::VeryHard:
				m_nAggressionScale = 4;
				m_nAimScale = 1.25f;
				
				MERC_SHOTGUN_RANGE = 15.0f;
				MERC_SHOTGUN_DAMAGE = 50.0f;

				info.health = 120.0f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Insane:
				// the elite
				m_nAggressionScale = 6;
				m_nAimScale = 1.75f;

				MERC_SHOTGUN_RANGE = 16.0f;
				MERC_SHOTGUN_DAMAGE = 69.0f;

				info.health = 160.0f;
				break;
			};
			m_EntityData.SetHealth( info.health );

			@m_Squad = @GlobalSquad;
			GlobalSquad.AddSquadMember( @this );
		}
		void OnDeath() override {
			m_EntityData.EmitSound( ResourceCache.ShottyDieSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyDieSfx.Count() - 1 ) ],
				10.0f, 0xff );
			
			TheNomad::SGame::EntityState@ deathState = null;
			if ( ( TheNomad::Util::PRandom() & 100 ) <= 50 ) {
				@deathState = ResourceCache.ShottyDieLowState;
				m_EntityData.EmitSound( ResourceCache.ShottyDieLowSfx, 10.0f, 0xff );
			} else {
				@deathState = ResourceCache.ShottyDieHighState;
				m_EntityData.EmitSound( ResourceCache.ShottyDieHighSfx, 10.0f, 0xff );
			}

			m_EntityData.SetState( deathState );
		}
		
		/*
		private uvec2 m_CoverPosition;
		private TheNomad::GameSystem::DirType m_nCoverDirection;
		private bool m_bInCover = false;
		private bool m_bSearchForCover = false;
		*/

		private moblib::System::AISquad@ m_Squad = null;

		private vec3 m_OldTargetPosition = vec3( 0.0f );
		private uint m_nLastBarkTime = 0;
		private uint m_nLastCheckTime = 0;

		private uint m_nAimStartTic = 0;
		private uint m_nAggressionScale = 0;
		private float m_nAimScale = 0.0f;
		
		private float m_nFearAmount = 0.0f;
		
		private uint m_nSubTicker = 0;
		private TheNomad::SGame::EntityState@ m_SubState = null;
	};
};