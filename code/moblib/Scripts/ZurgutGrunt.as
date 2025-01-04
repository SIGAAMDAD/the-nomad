namespace moblib::Script {
	const uint GRUNT_BLOWUP_TIME = 25000;
	const uint GRUNT_MELEE_WINDUP_TIME = 1000;
	const float GRUNT_MELEE_RANGE = 5.5f;
	const float GRUNT_EXPLOSION_RADIUS = 7.5f;
	const float GRUNT_HAMMER_RANGE = 3.0f;

	final class ZurgutGrunt : MobScript {
		ZurgutGrunt() {
		}
		
		private bool CheckRage() const {
			return m_bEnraged || m_EntityData.GetHealth() <= m_EntityData.GetMobInfo().health * 0.25f;
		}
		private bool IsBehindTarget() const {
			const TheNomad::GameSystem::DirType targetDir = m_EntityData.GetTarget().GetDirection();
			const TheNomad::GameSystem::DirType dir = m_EntityData.GetDirection();
			const vec3 targetPos = m_EntityData.GetTarget().GetOrigin();
			const vec3 pos = m_EntityData.GetOrigin();
			
			switch ( targetDir ) {
			case TheNomad::GameSystem::DirType::North: return targetPos.y < pos.y;
			case TheNomad::GameSystem::DirType::NorthEast: return targetPos.x > pos.x && targetPos.y < pos.y;
			case TheNomad::GameSystem::DirType::East: return targetPos.x > pos.x;
			case TheNomad::GameSystem::DirType::SouthEast: return targetPos.x > pos.x && targetPos.y > pos.y;
			case TheNomad::GameSystem::DirType::South: return targetPos.y > pos.y;
			case TheNomad::GameSystem::DirType::SouthWest: return targetPos.x < pos.x && targetPos.y > pos.y;
			case TheNomad::GameSystem::DirType::West: return targetPos.x < pos.y;
			case TheNomad::GameSystem::DirType::NorthWest: return targetPos.x < pos.x && targetPos.y < pos.y;
			default:
				break;
			};
			return false;
		}
		
		void OnSpawn() override {
			m_bEnraged = false;
		}
		void OnDeath() override {
		}
		void FightMissile() override {
			if ( m_nBombStartTic == 0 ) {
				m_nBombStartTic = TheNomad::GameSystem::GameTic;
			} else if ( TheNomad::GameSystem::GameTic - m_nBombStartTic <= GRUNT_BLOWUP_TIME ) {
				return;
			}
			
			// suicide bomb
			m_EntityData.EmitSound( m_hExplosionSfx, 10.5f, 0xff );
//			TheNomad::SGame::GfxManager.AddExplosionMark( m_EntityData.GetOrigin() );
			TheNomad::SGame::EntityObject@ activeEnts = @TheNomad::SGame::EntityManager.GetActiveEnts();
			TheNomad::SGame::EntityObject@ ent = @activeEnts.m_Next;

			TheNomad::Engine::Physics::Bounds bounds;
			bounds.m_nWidth = GRUNT_EXPLOSION_RADIUS;
			bounds.m_nHeight = GRUNT_EXPLOSION_RADIUS;
			bounds.MakeBounds( m_EntityData.GetOrigin() );

			for ( ; @ent !is @activeEnts; @ent = @ent.m_Next ) {
				if ( @ent !is @m_EntityData && bounds.IntersectsPoint( ent.GetOrigin() ) ) {
					TheNomad::SGame::EntityManager.DamageEntity( @ent, cast<TheNomad::SGame::EntityObject@>( @m_EntityData ) );
				}
			}
			TheNomad::SGame::EntityManager.KillEntity( cast<TheNomad::SGame::EntityObject@>( @m_EntityData ), null );
		}
		void FightMelee() override {
			if ( m_nMeleeWindupStartTic == 0 ) {
				m_nMeleeWindupStartTic = TheNomad::GameSystem::GameTic;
			}
			if ( TheNomad::GameSystem::GameTic - m_nMeleeWindupStartTic < m_EntityData.GetState().GetAnimation().GetLerpTime() ) {
				return;
			}

			TheNomad::Engine::Physics::Bounds bounds;
			bounds.m_nWidth = GRUNT_HAMMER_RANGE;
			bounds.m_nHeight = GRUNT_HAMMER_RANGE;
			bounds.MakeBounds( m_EntityData.GetOrigin() );

			TheNomad::SGame::EntityObject@ activeEnts = @TheNomad::SGame::EntityManager.GetActiveEnts();
			TheNomad::SGame::EntityObject@ ent = @activeEnts.m_Next;
			for ( ; @ent !is @activeEnts; @ent = @ent.m_Next ) {
				if ( @ent !is @m_EntityData && bounds.IntersectsPoint( ent.GetOrigin() ) ) {
					TheNomad::SGame::EntityManager.DamageEntity( @ent, cast<TheNomad::SGame::EntityObject@>( @m_EntityData ) );
				}
			}
		}
		
		void ChaseThink() override {
			if ( CheckRage() ) {
				m_bEnraged = true;
				
				// force a no-move
				m_EntityData.SetDirection( TheNomad::GameSystem::DirType::Inside );
				if ( ( TheNomad::Util::PRandom() & 100 ) == 100 ) {
					m_EntityData.EmitSound( ResourceCache.GruntScreamSecretSfx, 10.25f, 0xff );
				} else {
					m_EntityData.EmitSound( ResourceCache.GruntScreamSfx, 10.25f, 0xff );
				}
				m_EntityData.SetState( @m_FightMissileState );
			}
			if ( TheNomad::Util::Distance( m_EntityData.GetOrigin(), m_EntityData.GetTarget().GetOrigin() )
				<= GRUNT_MELEE_RANGE )
			{
				m_EntityData.SetState( @m_FightMeleeState );
			} else {
//				m_EntityData.TryWalk();
			}
		}
		void FleeThink() override {
			// fight to the death
		}
		void IdleThink() override {
			if ( m_Sensor.CheckSight() ) {
				m_EntityData.SetState( @m_FightMissileState );
			}
		}

		private int m_hScreamSecretSfx = FS_INVALID_HANDLE;
		private int m_hScreamSfx = FS_INVALID_HANDLE;
		private int m_hExplosionSfx = FS_INVALID_HANDLE;
		
		private uint m_nBombStartTic = 0;
		private uint m_nMeleeWindupStartTic = 0;
		private bool m_bEnraged = false;
	};
};