#include "entity.as"
#include "level.as"
#include "item.as"
#include "game.as"

namespace TheNomad::SGame {
	class QuickShot {
		QuickShot() {
		}
		QuickShot( const vec3& in origin ) {
			m_Targets.resize( sgame_QuickShotMaxTargets.GetInt() );
		}
		
		void Think() {
			if ( m_nLastTargetTime < uint( sgame_QuickShotTime.GetInt() ) ) {
				m_nLastTargetTime++;
				return;
			}
			
			DebugPrint( "QuickShot thinking...\n" );
			m_nLastTargetTime = 0;
			
			array<EntityObject>@ EntList = @EntityManager.GetEntities();

			// NOTE: this might be a little bit slow depending on how many mobs are in the area
			for ( uint i = 0; i < EntList.size(); i++ ) {
				if ( m_Targets.find( @EntList[i] ) == -1 ) {
					if ( TheNomad::Util::Distance( EntList[i].GetOrigin(), m_Origin ) > sgame_QuickShotMaxRange.GetFloat() ) {
						continue; // too far away
					}
					// make sure we aren't adding a duplicate
					@m_Targets[m_nTargetsFound] = @EntList[i];
					DebugPrint( "QuickShot added entity " + formatUInt( i ) + "\n" );
				}
			}
		}
		
		void Clear() {
			DebugPrint( "QuickShot cleared.\n" );
			m_nLastTargetTime = 0;
			m_nTargetsFound = 0;
		}

		private vec3 m_Origin;
		private array<EntityObject@> m_Targets;
		private uint m_nTargetsFound;
		private uint m_nLastTargetTime;
	};
	
	class KeyBind {
		KeyBind() {
			down[0] = down[1] = 0;
			downtime = 0;
			msec = 0;
			active = false;
		}
		KeyBind() {
		}
		
		KeyBind& opAssign( const KeyBind& in other ) {
			down[0] = other.down[0];
			down[1] = other.down[1];
			downtime = other.downtime;
			msec = other.msec;
			active = other.active;
			return this;
		}

		void Down() {
			int8[] c( MAX_TOKEN_CHARS );
			int k;
			
			TheNomad::Engine::CmdArgvFixed( c, MAX_TOKEN_CHARS, 1 );
			if ( c[0] != 0 ) {
				k = TheNomad::Util::StringToInt( c );
			} else {
				return;
			}
			
			if ( down[0] == 0 ) {
				down[0] = k;
			} else if ( down[1] == 0 ) {
				down[1] = k;
			} else {
				ConsolePrint( "Three keys down for a button!\n" );
				return;
			}
			
			if ( active ) {
				return; // still down
			}
			
			// save the timestamp for partial frame summing
			TheNomad::Engine::CmdArgvFixed( c, MAX_TOKEN_CHARS, 2 );
			downtime = TheNomad::Util::StringToInt( c );
			
			active = true;
		}
		
		void Up() {
			int8[] c( MAX_TOKEN_CHARS );
			uint uptime;
			uint k;
			
			TheNomad::Engine::CmdArgvFixed( c, MAX_TOKEN_CHARS, 1 );
			if ( c[0] != 0 ) {
				k = TheNomad::Util::StringToInt( c );
			} else {
				return;
			}
			
			if ( down[0] == k ) {
				down[0] = 0;
			} else if ( down[1] == k ) {
				down[1] = 0;
			} else {
				return; // key up without corresponding down (menu pass through)
			}
			
			active = false;
			
			// save timestamp for partial frame summing
			TheNomad::Engine::CmdArgvFixed( c, MAX_TOKEN_CHARS, 2 );
			uptime = TheNomad::Util::StringToInt( c );
			if ( uptime > 0 ) {
				msec += uptime - downtime;
			} else {
				msec += TheNomad::GameSystem::GameManager.GetGameMsec() / 2;
			}
			
			active = false;
		}
		
		uint[] down( 2 );
		uint downtime;
		uint msec;
		bool active;
	};
	
	
	///
	/// PMoveData
	/// a class to buffer user input per frame
	///
	class PMoveData {
		PMoveData() {
		}
		
		private void WalkMove( PlayrObject@ ent ) {
			vec3 vel;
			
			if ( !groundPlane ) {
				return;
			}
			
			vel = ent.GetVelocity();
			if ( northmove > 0 ) {
				vel.y -= northmove / sgame_BaseSpeed.GetFloat();
			}
			if ( southmove > 0 ) {
				vel.y += southmove / sgame_BaseSpeed.GetFloat();
			}
			if ( westmove > 0 ) {
				vel.x -= westmove / sgame_BaseSpeed.GetFloat();
			}
			if ( eastmove > 0 ) {
				vel.x += eastmove / sgame_BaseSpeed.GetFloat();
			}
			
			// clamp that shit, then apply friction
			for ( uint i = 0; i < 2; i++ ) {
				vel[i] = TheNomad::Util::Clamp( vel[i], 0.0f, sgame_MaxSpeed.GetFloat() );
				vel[i] -= ( sgame_GroundFriction.GetFloat() * TheNomad::GameSystem::GameManager.GetDeltaMsec() );
			}
			
			ent.SetVelocity( vel );
		}
		
		private void AirMove() {
			
		}
		
		void RunTic() {
			PlayrObject@ obj;
			ivec2 mousePos;
			int screenWidth, screenHeight;
			float angle;
			TheNomad::GameSystem::DirType dir;
			
			@obj = @GetPlayerObject();
			mousePos = TheNomad::GameSystem::GameManager.GetMousePos();
			screenWidth = TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth;
			screenHeight = TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight;
			
			northmove = obj.key_MoveNorth.active ? obj.key_MoveNorth.msec / sgame_MaxFps.GetInt() : 0;
			southmove = obj.key_MoveSouth.active ? obj.key_MoveSouth.msec / sgame_MaxFps.GetInt() : 0;
			eastmove = obj.key_MoveEast.active ? obj.key_MoveEast.msec / sgame_MaxFps.GetInt() : 0;
			westmove = obj.key_MoveWest.active ? obj.key_MoveWest.msec / sgame_MaxFps.GetInt() : 0;
			upmove = obj.key_Jump.active ? obj.key_Jump.msec / sgame_MaxFps.GetInt() : 0;
			
			// set leg sprite
			if ( eastmove > westmove ) {
				obj.SetLegFacing( 0 );
			} else if ( westmove > eastmove ) {
				obj.SetLegFacing( 1 );
			}
			
			// set torso direction
			angle = atan2( ( screenWidth / 2 ) - mousePos.x, ( screenHeight / 2 ) - mousePos.y );
			dir = TheNomad::Util::Angle2Dir( angle );
			
			switch ( dir ) {
			case TheNomad::GameSystem::DirType::North:
			case TheNomad::GameSystem::DirType::South:
				break; // not implemented for now
			case TheNomad::GameSystem::DirType::NorthEast:
			case TheNomad::GameSystem::DirType::SouthEast:
			case TheNomad::GameSystem::DirType::East:
				obj.SetFacing( 0 );
				break;
			case TheNomad::GameSystem::DirType::NorthWest:
			case TheNomad::GameSystem::DirType::SouthWest:
			case TheNomad::GameSystem::DirType::West:
				obj.SetFacing( 1 );
				break;
			default:
				break;
			};
			
			if ( obj.key_Jump.active && obj.GetOrigin().z > 0.0f ) {
				obj.SetFlags( obj.GetFlags() | obj.PF_DOUBLEJUMP );
			}
			
			groundPlane = upmove == 0;
		}
		
		uint northmove = 0;
		uint southmove = 0;
		uint eastmove = 0;
		uint westmove = 0;
		uint upmove = 0;
		
		bool groundPlane = false;
	};
	
	class PlayrObject : EntityObject {
		PlayrObject() {
			m_WeaponSlots.resize( sgame_MaxPlayerWeapons.GetInt() );

			key_MoveNorth = KeyBind();
			key_MoveSouth = KeyBind();
			key_MoveEast = KeyBind();
			key_MoveWest = KeyBind();
			key_Melee = KeyBind();
			key_Jump = KeyBind();
			
			EntityManager.SetPlayerObject( @this );
		}
		
		//
		// controls
		//
		void MoveNorth_Up_f() { key_MoveNorth.Up(); }
		void MoveNorth_Down_f() { key_MoveNorth.Down(); }
		void MoveSouth_Up_f() { key_MoveSouth.Up(); }
		void MoveSouth_Down_f() { key_MoveSouth.Down(); }
		void Jump_Down_f() { key_Jump.Down(); }
		void Jump_Up_f() { key_Jump.Up(); }
		
		void Quickshot_Down_f() {
			return;
//			m_PFlags |= PF_QUICKSHOT;
//			m_QuickShot.Clear();
//			m_QuickShot = QuickShot( m_Link.m_Origin, @ModObject );
		}
		
		void Quickshot_Up_f() {
			return;
			// TODO: perhaps add a special animation for putting the guns away?
//			if ( m_QuickShot.Empty() ) {
//				return;
//			}
			
//			m_QuickShot.Activate();
//			m_PFlags &= ~PF_QUICKSHOT;
		}
		
		void Melee_Down_f() {
			m_nParryBoxWidth = 0.0f;
			SetState( StateNum::ST_PLAYR_MELEE );
		}
		
		void NextWeapon_f() {
			m_CurrentWeapon++;
			if ( m_CurrentWeapon >= m_WeaponSlots.size() ) {
				m_CurrentWeapon = 0;
			}
		}
		void PrevWeapon_f() {
			m_CurrentWeapon--;
			if ( m_CurrentWeapon <= 0 ) {
				m_CurrentWeapon = m_WeaponSlots.size() - 1;
			}
		}
		void SetWeapon_f() {
			
		}
		void Emote_f() {
			m_bEmoting = true;
		}

		void SetLegFacing( int facing ) {
			m_LegsFacing = facing;
		}
		
		void Damage( float nAmount ) {
			if ( m_bEmoting ) {
				return; // god has blessed thy soul...
			}
			
			m_nHealth -= nAmount;

			if ( m_nHealth < 1 ) {
				if ( m_nFrameDamage > 0 ) {
					return; // as long as you're hitting SOMETHING, you cannot die
				}
				TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( dieSfx[ TheNomad::Util::PRandom() & 3 ] );
				EntityManager.KillEntity( @this );
			} else {
				TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( painSfx[ TheNomad::Util::PRandom() & 3 ] );
			}
		}
		
		void SetPFlags( uint flags ) {
			m_PFlags = flags;
		}
		uint GetPFlags() const {
			return m_PFlags;
		}
		
		void Think() override {
			if ( ( m_PFlags & PF_PARRY ) != 0 ) {
				ParryThink();
			} else if ( ( m_PFlags & PF_QUICKSHOT ) != 0 ) {
				m_QuickShot.Think();
			}
			
			switch ( m_State.GetID() ) {
			case StateNum::ST_PLAYR_IDLE:
				IdleThink();
				break; // NOTE: maybe let the player move in combat? (that would require more sprites for the dawgs)
			case StateNum::ST_PLAYR_COMBAT:
				CombatThink();
				break;
			};

			m_nHealth += sgame_PlayerHealBase.GetFloat() * m_nHealMult;
			m_nHealMult -= m_nHealMultDecay * LevelManager.GetDifficultyScale();
		}
		
		private float GetGfxDirection() const {
			if ( m_Facing == 1 ) {
				return -( m_Link.m_Bounds.m_nWidth / 2 );
			}
			return ( m_Link.m_Bounds.m_nWidth / 2 );
		}
		
		bool CheckParry( EntityObject@ ent ) {
			if ( TheNomad::Util::BoundsIntersect( ent.GetBounds(), m_ParryBox ) ) {
				if ( ent.IsProjectile() ) {
					// simply invert the direction and double the speed
					const vec3 v = ent.GetVelocity();
					ent.SetVelocity( vec3( v.x * 2, v.y * 2, v.z * 2 ) );
					ent.SetDirection( InverseDirs[ ent.GetDirection() ] );
				} else {
					return false;
				}
			} else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				// just a normal counter
				MobObject@ mob = cast<MobObject>( ent.GetData() );
				
				if ( !mob.CurrentAttack().canParry ) {
					// unblockable, deal damage
					EntityManager.DamageEntity( @ent, @this );
					return false;
				} else {
					// slap it back
//					if ( mob.GetState().GetTics() <= ( mob.GetState().GetDuration() / 4 ) ) {
						// counter parry, like in MGR, but more brutal, but for the future...
//					}
					// TODO: make dead mobs with ANY velocity flying corpses
					EntityManager.DamageEntity( @this, @ent );
					TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( parrySfx );
				}
			}
			
			// add the fireball
			GfxManager.AddExplosionGfx( vec3( m_Link.m_Origin.x + GetGfxDirection(), m_Link.m_Origin.y, m_Link.m_Origin.z ) );

			return true;
		}
		
		private void MakeSound() {
			if ( m_State.GetID() == StateNum::ST_PLAYR_CROUCH ) {
				return;
			}

			SoundData data( m_Velocity.x + m_Velocity.y, vec2( m_Velocity.x, m_Velocity.y ), ivec2( 2, 2 ),
				ivec3( int( m_Link.m_Origin.x ), int( m_Link.m_Origin.y ), int( m_Link.m_Origin.z ) ), 1.0f );
		}
		
		private void IdleThink() {
			ivec2 origin;
			
			for ( uint i = 0; i < 2; i++ ) {
				origin[i] = int( floor( m_Link.m_Origin[i] ) );
			}
			
			if ( Pmove.groundPlane ) {
				const uint flags = LevelManager.GetMapData().GetTiles()[ origin.y * LevelManager.GetMapData().GetWidth() + origin.x ];
				
				if ( ( flags & SURFACEPARM_METAL ) != 0 ) {
					TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( moveMetalSfx );
				} else if ( ( flags & SURFACEPARM_WOOD ) != 0 ) {
					TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( moveWoodSfx );
				} else {
					// in this case, just use the generic walking sound
					TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( moveSfx );
				}
			} else if ( key_Jump.active ) {
				TheNomad::Engine::SoundSystem::SoundManager.PushSfxToScene( jumpSfx );
			}
		}
		private void CombatThink() {
			if ( key_Melee.active ) {
				// check for a parry
				m_PFlags |= PF_PARRY;
			}
		}
		private void ParryThink() {
			m_ParryBox.m_nWidth = 2.5f + m_nParryBoxWidth;
			m_ParryBox.m_nHeight = 1.0f;
			m_ParryBox.MakeBounds( vec3( m_Link.m_Origin.x + ( m_Link.m_Bounds.m_nWidth / 2.0f ),
				m_Link.m_Origin.y, m_Link.m_Origin.z ) );

			if ( m_nParryBoxWidth >= 1.5f ) {
				return; // maximum
			}
			
			m_nParryBoxWidth += 0.5f;
		}
		
		uint PF_PARRY      = 0x00000001;
		uint PF_DOUBLEJUMP = 0x00000002;
		uint PF_QUICKSHOT  = 0x00000004;
		
		KeyBind key_MoveNorth, key_MoveSouth, key_MoveEast, key_MoveWest;
		KeyBind key_Jump, key_Melee;
		
		private TheNomad::GameSystem::BBox m_ParryBox;
		private float m_nParryBoxWidth;
		
		private array<WeaponObject@> m_WeaponSlots;
		private QuickShot m_QuickShot;
		private uint m_CurrentWeapon;
		private uint m_PFlags;
		
		private TheNomad::Engine::SoundSystem::SoundEffect moveWoodSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect moveMetalSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect moveSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect jumpSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect parrySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect[] painSfx( 3 );
		private TheNomad::Engine::SoundSystem::SoundEffect[] dieSfx( 3 );

		// the amount of damage dealt in the frame
		private uint m_nFrameDamage;

		private float m_nDamageMult;
		private float m_nHealMult;

		private int m_LegsFacing;

		private bool m_bEmoting;

		private float m_nHealMultDecay = 1.0f;
		
		private PMoveData Pmove;
	};
};