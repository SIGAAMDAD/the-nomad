#include "moblib/MobScript.as"

namespace moblib {
	const uint GRUNT_BLOWUP_TIME = 25000;
	const uint GRUNT_MELEE_WINDUP_TIME = 1000;
	
	final class ZurgutGrunt : MobScript {
		ZurgutGrunt() {
		}
		
		private bool CheckRage() const {
			return m_bEnraged || m_EntityData.GetHealth() <= m_EntityData.GetInfo().health * 0.25f;
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
		}
		
		void OnSpawn() override {
			m_bEnraged = false;
		}
		void OnDeath() override {
		}
		void FightMissile() override {
			if ( m_nBombStartTic == 0 ) {
				m_nBombStartTic = TheNomad::GameSystem::GameManager.GetGameTic();
			} else if ( TheNomad::GameSystem::GameManager.GetGameTic() - m_nBombStartTic <= GRUNT_BLOWUP_TIME ) {
				return;
			}
			
			// suicide bomb
			m_EntityData.EmitSound( TheNomad::Engine::ResourceCache.GetSfx( "event:/sfx/mobs/grunt_explosion" ), 2.5f, 0xff );
			TheNomad::SGame::GfxManager.AddExplosionMark( m_EntityData.GetOrigin() );
			TheNomad::SGame::EntityManager.KillEntity( cast<TheNomad::SGame::EntityObject@>( @mob ), null );
		}
		void FightMelee() override {
			if ( m_nMeleeWindupStartTic == 0 ) {
				m_nMeleeWindupStartTic = TheNomad::GameSystem::GameManager.GetGameTic();
			}
			
			const uint lerpTime = m_State.GetAnimation().GetTicRate() * m_State.GetAnimation().NumFrames();
			if ( TheNomad::GameSystem::GameManager.GetGameTic() - m_nMeleeWindupStartTic >= lerpTime ) {
				TheNomad::SGame::EntityManager.ForEachEntity( function( TheNomad::SGame::EntityObject@ ent, ref@ thisData ){
					ZurgutGrunt@ this = cast<ZurgutGrunt@>( @thisData );
					
					if ( TheNomad::Util::Distance( ent.GetOrigin(), this.GetBase().GetOrigin() ) > this.GetBase().GetInfo().meleeRange ) ) {
						return;
					}
					
					TheNomad::SGame::EntityManager.DamageEntity( cast<TheNomad::SGame::EntityObject@>( @this.GetBase() ), @ent );
					
					// reset
					this.SetState( this.GetFightState() );
				}, @this );
			}
		}
		
		void FightThink() override {
			if ( CheckRage() ) {
				m_bEnraged = true;
				
				// force a no-move
				m_EntityData.SetDirection( TheNomad::GameSystem::DirType::Inside );
				if ( ( Util::PRandom() & 100 ) == 100 ) {
					m_EntityData.EmitSound( TheNomad::Engine::ResourceCache.GetSfx( "event:/sfx/secrets/scream" ), 1.25f, 0xff );
				} else {
					m_EntityData.EmitSound( TheNomad::Engine::ResourceCache.GetSfx( "event:/sfx/mobs/grunt_scream" ), 1.25f, 0xff );
				}
				m_EntityData.SetState( @m_MissileState );
			}
			if ( TheNomad::Util::Distance( m_EntityData.GetOrigin(),
				m_EntityData.GetTarget().GetOrigin() ) <= m_EntityData.GetInfo().meleeRange )
			{
				m_EntityData.SetState( @m_MeleeState );
			} else {
				m_EntityData.TryWalk();
			}
		}
		void FleeThink() override {
			// fight to the death
		}
		void IdleThink() override {
			if ( m_Sensor.SightCheck() ) {
				m_EntityData.SetState( @m_FightState );
			}
		}
		
		private uint m_nBombStartTic = 0;
		private uint m_nMeleeWindupStartTic = 0;
		private bool m_bEnraged = false;
	};
};