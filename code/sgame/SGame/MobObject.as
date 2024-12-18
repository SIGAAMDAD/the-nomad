#include "SGame/EntityObject.as"
#include "SGame/EntitySystem.as"
#include "moblib/MobScript.as"

namespace TheNomad::SGame {
	class MobObject : EntityObject {
		MobObject() {
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
		
		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
			m_nHealth = section.LoadFloat( "health" );
//			m_MFlags = InfoSystem::MobFlags( section.LoadUInt( "mobFlags" ) );
			if ( section.LoadBool( "hasTarget" ) ) {
				@m_Target = @EntityManager.GetEntityForNum( section.LoadUInt( "target" ) );
			}
			
			return true;
		}
		
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			section.SaveFloat( "health", m_nHealth );
//			section.SaveUInt( "mobFlags", uint( m_MFlags ) );
			section.SaveBool( "hasTarget", @m_Target !is null );
			if ( @m_Target !is null ) {
				section.SaveUInt( "target", m_Target.GetEntityNum() );
			}
		}

		InfoSystem::MobFlags GetMFlags() const {
			return m_MFlags;
		}
		
		void LinkScript( moblib::MobScript@ script ) {
			@m_ScriptData = @script;
		}

		void Spawn( uint id, const vec3& in origin ) {
			@m_Info = @InfoSystem::InfoManager.GetMobInfo( id );
			if ( @m_Info is null ) {
				GameError( "MobObject::Spawn: bad MobType " + id + "! Info was null!" );
			}
			
			m_Name = m_Info.name;
			m_nHealth = m_Info.health;
			m_MFlags = m_Info.mobFlags;
			@m_State = @StateManager.GetStateForNum( m_Info.type + StateNum::ST_MOB_IDLE );
		}
		
		void Damage( EntityObject@ source, float nAmount ) {
			m_nHealth -= nAmount;
			if ( m_nHealth < 0.0f ) {
				if ( m_nHealth <= -m_Info.health ) {
					// GIBS!
//					EntityManager.GibbEntity( @this );
				}
				m_ScriptData.OnDeath();
				EntityManager.KillEntity( @source, cast<EntityObject@>( @this ) );
				
				return;
			}
			
//			EmitSound( m_Info.painSfx, 1.0f, 0xff );
			
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

		void Draw() override {
			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = m_Link.m_Origin;
			refEntity.scale = m_Info.size;
//			refEntity.sheetNum = m_Info.spriteSheet.GetShader();

			refEntity.Draw();
		}
		void Think() override {
			switch ( m_State.GetID() - m_Info.type ) {
			case StateNum::ST_MOB_IDLE:
				m_ScriptData.IdleThink();
				break;
			case StateNum::ST_MOB_FIGHT_MELEE:
			case StateNum::ST_MOB_FIGHT_MISSILE:
			case StateNum::ST_MOB_FIGHT:
				break;
			case StateNum::ST_MOB_CHASE:
				break;
			default:
				GameError( "MobEntity::Think: invalid mob state " + formatUInt( m_State.GetID() ) + "\n" );
				break;
			};
		}

		bool CanParry() const {
			return m_bParry;
		}
		void SetParry( bool bParry ) {
			m_bParry = bParry;
		}
		
		void SetTarget( EntityObject@ newTarget ) {
			@m_Target = @newTarget;
			m_PhysicsObject.SetAngle( atan2( m_Link.m_Origin.x - m_Target.GetOrigin().x, m_Link.m_Origin.y - m_Target.GetOrigin().y ) );
			m_Direction = Util::Angle2Dir( m_PhysicsObject.GetAngle() );
			
			DebugPrint( "Set mob target to " + m_Target.GetEntityNum() + "\n" );
		}

		//
		// SetState: custom for mobs
		//
		void SetState( StateNum state ) {
			@m_State = @StateManager.GetStateForNum( m_Info.type + state );
			if ( @m_State is null ) {
				ConsoleWarning( "MobObject::SetState: bad state" );
			}
			m_State.Reset( m_nTicker );
		}
		void SetState( EntityState@ state ) {
			@m_State = @state;
			if ( @m_State is null ) {
				ConsoleWarning( "MobObject::SetState: bad state" );
			}
			m_State.Reset( m_nTicker );
		}

		protected EntityObject@ m_Target = null;
		protected InfoSystem::MobInfo@ m_Info = null;
		protected InfoSystem::MobFlags m_MFlags = InfoSystem::MobFlags( 0 );
		
		// stealth tracking
		protected uint m_nLastAlertTime = 0;
		protected uint m_nAlertLevel = 0;
		
		// attack data
		protected bool m_bIsAttacking = false;
		protected uint m_nAttackTime = 0;
		protected uint m_nLastAttackTime = 0;

		private bool m_bParry = false;

		private moblib::MobScript@ m_ScriptData = null;
	};
};