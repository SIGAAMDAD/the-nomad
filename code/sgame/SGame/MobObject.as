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

		InfoSystem::MobInfo@ GetMobInfo() {
			return @m_Info;
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
		
		private bool Move() {
			if ( m_Direction == TheNomad::GameSystem::DirType::Inside ) {
				return false;
			}
			if ( uint( m_Direction ) >= TheNomad::GameSystem::DirType::Inside ) {
				GameError( "MobObject::Move: weird movedir!" );
			}

			vec3 accel;
			const float angle = m_PhysicsObject.GetAngle();

			accel.x = m_Info.speed.x * cos( angle );
			accel.y = m_Info.speed.y * sin( angle );

			m_PhysicsObject.SetAcceleration( accel );
			return m_PhysicsObject.OnRunTic();
		}

		bool TryWalk() {
			if ( !Move() ) {
				return false;
			}
			m_nMoveCounter = Util::PRandom() & 30;
			return true;
		}

		void NewChaseDir() {
			TheNomad::GameSystem::DirType olddir;
			TheNomad::GameSystem::DirType d0, d1, d2;
			TheNomad::GameSystem::DirType turnaround;
			TheNomad::GameSystem::DirType tdir;

			if ( @m_Target is null ) {
				GameError( "MobObject::NewChaseDir: called with no target" );
			}

			olddir = m_Direction;
			turnaround = Util::InverseDir( olddir );

			const vec3 target = m_Target.GetOrigin();
			const float deltaX = target.x - m_Link.m_Origin.x;
			const float deltaY = target.y - m_Link.m_Origin.y;

			if ( deltaX > 0.0f ) {
				d1 = TheNomad::GameSystem::DirType::East;
			} else if ( deltaX < 0.0f ) {
				d1 = TheNomad::GameSystem::DirType::West;
			} else {
				d1 = TheNomad::GameSystem::DirType::Inside;
			}

			if ( deltaY > 0.0f ) {
				d2 = TheNomad::GameSystem::DirType::South;
			} else if ( deltaY < 0.0f ) {
				d2 = TheNomad::GameSystem::DirType::North;
			} else {
				d2 = TheNomad::GameSystem::DirType::Inside;
			}

			// try direct route
			if ( d1 != TheNomad::GameSystem::DirType::Inside && d2 != TheNomad::GameSystem::DirType::Inside ) {
				m_Direction++;
				if ( m_Direction == uint( turnaround ) && TryWalk() ) {
					return;
				}
			}

			// try other directions
			if ( Util::PRandom() > 200 || abs( deltaY ) > abs( deltaX ) ) {
				tdir = d1;
				d1 = d2;
				d2 = tdir;
			}

			if ( d1 == turnaround ) {
				d1 = TheNomad::GameSystem::DirType::Inside;
			}
			if ( d2 == turnaround ) {
				d2 = TheNomad::GameSystem::DirType::Inside;
			}

			if ( d1 != TheNomad::GameSystem::DirType::Inside ) {
				m_Direction = d1;
				if ( TryWalk() ) {
					// either moved forward or attacked
					return;
				}
			}

			if ( d2 != TheNomad::GameSystem::DirType::Inside ) {
				m_Direction = d2;
				if ( TryWalk() ) {
					return;
				}
			}

			// there is no direct path to the player,
			// so pick another direction.
			if ( olddir != TheNomad::GameSystem::DirType::Inside ) {
				m_Direction = olddir;
				if ( TryWalk() ) {
					return;
				}
			}

			// randomly determine direction of search
			if ( ( Util::PRandom() & 1 ) == 1 ) {
				for ( tdir = TheNomad::GameSystem::DirType::East; tdir <= TheNomad::GameSystem::DirType::SouthEast; tdir++ ) {
					if ( tdir != turnaround ) {
						m_Direction = tdir;
						if ( TryWalk() ) {
							return;
						}
					}
				}
			}
			else {
				for ( tdir = TheNomad::GameSystem::DirType::SouthEast; tdir != ( TheNomad::GameSystem::DirType::North - 1 ); tdir-- ) {
					if ( tdir == turnaround ) {
						m_Direction = tdir;
						if ( TryWalk() ) {
							return;
						}
					}
				}
			}

			if ( turnaround != TheNomad::GameSystem::DirType::Inside ) {
				m_Direction = turnaround;
				if ( TryWalk() ) {
					return;
				}
			}

			m_Direction = TheNomad::GameSystem::DirType::Inside; // can not move
		}
		void Chase() {
			int delta;

			if ( m_nReactionTime > 0 ) {
				m_nReactionTime--;
			}

			FaceTarget();

			if ( @m_Target is null ) {
				SetState( @m_Info.idleState );
			}

			// chase towards player
			Move();
		}

		void LinkScript( moblib::MobScript@ script ) {
			@m_ScriptData = @script;
			DebugPrint( "Linked mob script for \"" + m_Info.name + "\" at entity '" + m_Link.m_nEntityNumber + "''\n" );
		}

		void Spawn( uint id, const vec3& in origin ) {
			@m_Info = @InfoSystem::InfoManager.GetMobInfo( id );
			if ( @m_Info is null ) {
				GameError( "MobObject::Spawn: bad MobType " + id + "! Info was null!" );
			}

			m_Link.m_Origin = origin;
			
			m_Bounds.m_nWidth = m_Info.size.x;
			m_Bounds.m_nHeight = m_Info.size.y;

			m_Name = m_Info.name;
			m_nHealth = m_Info.health;
			m_MFlags = m_Info.mobFlags;
			@m_State = @StateManager.GetStateForNum( m_Info.type + StateNum::ST_MOB_IDLE );
			moblib::AllocScript( @this );

			m_ScriptData.OnSpawn();
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
			const vec3 target = m_Target.GetOrigin();

			const float deltaY = m_Link.m_Origin.y - target.y;
			const float deltaX = target.x - m_Link.m_Origin.x;
			float angle = atan2( deltaY, deltaX );
			m_PhysicsObject.SetAngle( -angle );
				
			angle = Util::RAD2DEG( angle );
			if ( angle < 0.0f ) {
				angle += 360.0f;
				if ( angle < 0.0f ) {
					angle = 0.0f;
				}
			}
			m_Direction = Util::Angle2Dir( angle );

			switch ( m_Direction ) {
			case TheNomad::GameSystem::DirType::North:
			case TheNomad::GameSystem::DirType::NorthEast:
			case TheNomad::GameSystem::DirType::East:
			case TheNomad::GameSystem::DirType::SouthEast:
				m_Facing = FACING_RIGHT;
				break;
			case TheNomad::GameSystem::DirType::South:
			case TheNomad::GameSystem::DirType::SouthWest:
			case TheNomad::GameSystem::DirType::West:
			case TheNomad::GameSystem::DirType::NorthWest:
				m_Facing = FACING_LEFT;
				break;
			};
		}

		void Draw() override {
			TheNomad::Engine::Renderer::RenderEntity refEntity;

			if ( @m_State is @m_Info.searchState ) {
				m_AfterImage.Draw();
			}
			if ( m_bParry ) {
				// draw a large blue parry icon above the mob to signal for an action
				refEntity.origin = m_Link.m_Origin;
				refEntity.origin.x += m_Info.size.x * 0.25f;
				refEntity.origin.y -= m_Info.size.y;
				refEntity.scale = vec2( 1.5f );
				refEntity.sheetNum = -1;
				refEntity.spriteId = TheNomad::Engine::Renderer::RegisterShader( "gfx/mob_parry" );
				refEntity.Draw();
			}

			refEntity.origin = m_Link.m_Origin;
			refEntity.origin.x += m_Info.size.x * 0.5f;
			refEntity.origin.y -= m_Info.size.y * 0.25f;

			refEntity.scale = TheNomad::Engine::Renderer::GetFacing( m_Facing, m_Info.size );
			refEntity.sheetNum = m_Info.spriteSheet.GetShader();
			refEntity.spriteId = TheNomad::Engine::Renderer::GetSpriteId( @m_Info.spriteSheet, @m_State );

			refEntity.Draw();
		}
		void Think() override {
			m_Bounds.m_nWidth = m_Info.size.x;
			m_Bounds.m_nHeight = m_Info.size.y;
			m_Bounds.MakeBounds( m_Link.m_Origin );

			switch ( m_State.GetBaseNum() ) {
			case StateNum::ST_MOB_IDLE:
				m_ScriptData.IdleThink();
				break;
			case StateNum::ST_MOB_FIGHT_MELEE:
				m_ScriptData.FightMelee();
				break;
			case StateNum::ST_MOB_FIGHT_MISSILE:
				m_ScriptData.FightMissile();
				break;
			case StateNum::ST_MOB_CHASE:
				m_ScriptData.ChaseThink();
				break;
			case StateNum::ST_MOB_SEARCH:
				m_ScriptData.SearchThink();
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
				return;
			}
			m_State.Reset( m_nTicker );
		}
		void SetState( EntityState@ state ) {
			@m_State = @state;
			if ( @m_State is null ) {
				ConsoleWarning( "MobObject::SetState: bad state" );
				return;
			}
			m_State.Reset( m_nTicker );
		}

		uint GetMoveCounter() const {
			return m_nMoveCounter;
		}
		void SetMoveCounter( uint nMoveCounter ) {
			m_nMoveCounter = nMoveCounter;
		}
		uint GetReactionTime() const {
			return m_nReactionTime;
		}
		void SetReactionTime( uint nTime ) {
			m_nReactionTime = nTime;
		}
		void ResetReactionTime() {
			m_nReactionTime = m_Info.reactionTime;
		}

		private EntityObject@ m_Target = null;
		private InfoSystem::MobInfo@ m_Info = null;
		private InfoSystem::MobFlags m_MFlags = InfoSystem::MobFlags( 0 );
		private moblib::MobScript@ m_ScriptData = null;

		// stealth tracking
		private uint m_nLastAlertTime = 0;
		private float m_nAlertAmount = 0.0f;
		private uint m_nAlertLevel = 0;

		private uint m_nMoveCounter = 0;
		private uint m_nReactionTime = 0;

		private AfterImage m_AfterImage;
		
		// attack data
		private bool m_bIsAttacking = false;
		private uint m_nAttackTime = 0;
		private uint m_nLastAttackTime = 0;

		private bool m_bParry = false;
	};
};