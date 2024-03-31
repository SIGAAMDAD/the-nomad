#include "Engine/SoundSystem/SoundFrameData.as"
#include "SGame/KeyBind.as"
#include "SGame/QuickShot.as"
#include "SGame/PMoveData.as"
#include "SGame/PlayerHud.as"

namespace TheNomad::SGame {
    class PlayrObject : EntityObject {
		PlayrObject() {
			for ( uint i = 0; i < uint( sgame_MaxPlayerWeapons.GetInt() ); i++ ) {
				m_WeaponSlots.Add( null );
			}
			
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

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) override {
			m_PFlags = section.LoadUInt( "playrFlags" );
			m_LegsFacing = section.LoadUInt( "legsFacing" );
			m_Facing = section.LoadUInt( "torsoFacing" );
			m_nAngle = section.LoadFloat( "angle" );
			m_Direction = TheNomad::GameSystem::DirType( section.LoadUInt( "direction" ) );
			m_Flags = EntityFlags( section.LoadUInt( "entityFlags" ) );
			m_bProjectile = Convert().ToBool( section.LoadUInt( "isProjectile" ) );

			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			
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
					ent.SetDirection( GameSystem::InverseDirs[ ent.GetDirection() ] );
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
				const uint flags = LevelManager.GetMapData().GetTiles()[ GetMapLevel( m_Link.m_Origin.z ) ][ origin.y * LevelManager.GetMapData().GetWidth() + origin.x ];
				
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

		void Spawn( uint id, const vec3& in origin ) override {
			m_Link.m_Origin = origin;
			m_nHealth = 100.0f;
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
		private uint m_CurrentWeapon = 0;
		private uint m_PFlags = 0;
		
		private TheNomad::Engine::SoundSystem::SoundEffect moveWoodSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect moveMetalSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect moveSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect jumpSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect parrySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect[] painSfx( 3 );
		private TheNomad::Engine::SoundSystem::SoundEffect[] dieSfx( 3 );

		// the amount of damage dealt in the frame
		private uint m_nFrameDamage = 0;

		private float m_nDamageMult = 0.0f;
		private float m_nHealMult = 0.0f;

		private int m_LegsFacing = 0;

		private bool m_bEmoting = false;

		private float m_nHealMultDecay = 1.0f;
		
		private PMoveData Pmove;
	};
};