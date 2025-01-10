#include "moblib/MobScript.as"

namespace moblib::Script {
	float MERC_GATLING_RANGE = 10.0f;
	float MERC_GATLING_DAMAGE = 0.25f;
	uint MERC_GATLING_BULLETS_PER_VOLLEY = 8;
	
	class MercGatling : MobScript {
		MercGatling() {
		}
		
		void IdleThink() override {
			if ( m_Sensor.CheckSight() || m_Sensor.CheckSound() && !m_EntityData.GetTarget().CheckFlags( TheNomad::SGame::EntityFlags::Dead ) ) {
				m_EntityData.SetState( ResourceCache.MercGatlingFightMissileState );
				m_EntityData.EmitSound( ResourceCache.MercGatlingParrySfx, 10.0f, 0xff );
				@m_SubState = ResourceCache.MercGatlingParryState;
				m_SubState.Reset( m_nSubTicker );
				m_nLastTargetSightedTime = TheNomad::GameSystem::GameTic;

				m_EntityData.FaceTarget();
			}
			
			// gatling gunners have a mix tape playing in their helmets,
			// so they can barely hear the player
		}
		void FightMelee() override {
		}
		void FightMissile() override {
			if ( m_EntityData.GetTarget().CheckFlags( TheNomad::SGame::EntityFlags::Dead ) ) {
				m_EntityData.SetState( ResourceCache.MercGatlingIdleState );
				return;
			}
			
			const vec3 origin = m_EntityData.GetOrigin();
			
			if ( m_SubState is ResourceCache.MercGatlingParryState && !m_SubState.Done( m_nSubTicker ) ) {
				// only give the player a small window to parry the attack
				// before the gatling gunner starts just fucking shit up
				
				m_EntityData.SetParry( true );
				return;
			} else if ( m_SubState !is null && m_SubState.Done( m_nSubTicker ) ) {
				m_EntityData.EmitSound( ResourceCache.MercGatlingAttackSfx, 10.0f, 0xff );
				m_EntityData.SetParry( false );
				@m_SubState = null;
			}
			if ( m_EntityData.GetState().Done( m_EntityData.GetTicker() ) ) {
				m_EntityData.EmitSound( ResourceCache.MercGatlingAttackSfx, 10.0f, 0xff );
			}
			
			// BRRR!
			TheNomad::GameSystem::RayCast ray;
			
			ray.m_nLength = MERC_GATLING_RANGE;
			ray.m_Start = origin;

			if ( m_EntityData.GetTarget() is null ) {
				m_EntityData.SetState( ResourceCache.MercGatlingIdleState );
			}

			// chase towards player
			if ( m_EntityData.GetTarget() !is null && !m_EntityData.Move() ) {
				m_EntityData.NewChaseDir();
			}

			const float angle = m_EntityData.GetAngle();
			for ( uint i = 0; i < 12; i++ ) {
				ray.m_nAngle = angle + ( TheNomad::Util::PRandom() & 100 > 50 ? -0.5f : 0.5f );
				ray.m_nOwner = m_EntityData.GetEntityNum();
				ray.Cast();
				
				if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
					const float velocity = 2.0f;
					TheNomad::SGame::GfxManager.AddDebrisCloud( ray.m_Origin, velocity );
					TheNomad::SGame::GfxManager.AddBulletHole( ray.m_Origin );

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

				TheNomad::SGame::EntityManager.DamageEntity( hit, m_EntityData, MERC_GATLING_DAMAGE );
			}
			TheNomad::SGame::GfxManager.AddMuzzleFlash( origin, 4.0f, 0.0026f, 0.0026f, 0.0f, 0.03f, vec3( 0.0f ) );
		}
		void SearchThink() override {
		}
		void ChaseThink() override {
		}
		void OnDamage( TheNomad::SGame::EntityObject@ attacker ) override {
			// TODO: implement shooting oxygen tank
		}
		void OnSpawn() override {
			// canonically, the higher the difficulty, the more risky the mission.
			// the riskier the mission, the more elite the enemies will be
			//
			// also give the player a lot of leeway
			TheNomad::SGame::InfoSystem::MobInfo@ info = m_EntityData.GetMobInfo();
			switch ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) ) {
			case TheNomad::GameSystem::GameDifficulty::Easy:
				// they have the least experience with firearms, they can't hit a far shot
				MERC_GATLING_RANGE = 10.75f;

				// they aren't hitting vital spots
				MERC_GATLING_DAMAGE = 0.075f;

				MERC_GATLING_BULLETS_PER_VOLLEY = 24;

				// this is only here so that easy mode has a one-shot-kill gatling gunner
				info.health = 40.0f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Normal:
				break;
			case TheNomad::GameSystem::GameDifficulty::Hard:
				MERC_GATLING_RANGE = 14.75f;
				MERC_GATLING_DAMAGE = 5.0f;

				MERC_GATLING_BULLETS_PER_VOLLEY = 8;
				break;
			case TheNomad::GameSystem::GameDifficulty::VeryHard:
				MERC_GATLING_RANGE = 16.0f;
				MERC_GATLING_DAMAGE = 16.0f;

				MERC_GATLING_BULLETS_PER_VOLLEY = 6;
				break;
			case TheNomad::GameSystem::GameDifficulty::Insane:
				// the elite

				MERC_GATLING_RANGE = 20.0f;

				// let's be realistic, its probably 7.62
				MERC_GATLING_DAMAGE = 40.0f;

				MERC_GATLING_BULLETS_PER_VOLLEY = 3;
				break;
			};
			m_EntityData.SetHealth( info.health );
		}
		void OnDeath() override {
			m_EntityData.EmitSound( ResourceCache.ShottyDieSfx[ TheNomad::Util::PRandom() & ( ResourceCache.ShottyDieSfx.Count() - 1 ) ],
				10.0f, 0xff );
			
			TheNomad::SGame::EntityState@ deathState = null;
			if ( ( TheNomad::Util::PRandom() & 100 ) <= 50 ) {
				@deathState = ResourceCache.MercGatlingDieHighState;
				m_EntityData.EmitSound( ResourceCache.ShottyDieLowSfx, 10.0f, 0xff );
			} else {
				@deathState = ResourceCache.MercGatlingDieLowState;
				m_EntityData.EmitSound( ResourceCache.ShottyDieHighSfx, 10.0f, 0xff );
			}

			m_EntityData.SetState( deathState );
		}
		void DeadThink() override {
		}
		
		private TheNomad::SGame::EntityState@ m_SubState = null;
		
		private uint m_nSubTicker = 0;
		private uint m_nLastTargetSightedTime = 0;
	};
};