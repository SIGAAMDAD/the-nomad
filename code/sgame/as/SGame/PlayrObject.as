#include "Engine/SoundSystem/SoundFrameData.as"
#include "SGame/KeyBind.as"
#include "SGame/QuickShot.as"
#include "SGame/PMoveData.as"
#include "SGame/PlayerHud.as"

namespace TheNomad::SGame {
	const uint[] sgame_WeaponModeList = {
		uint( InfoSystem::WeaponProperty::OneHandedBlade | InfoSystem::WeaponProperty::TwoHandedBlade ),
		uint( InfoSystem::WeaponProperty::OneHandedBlunt | InfoSystem::WeaponProperty::TwoHandedBlunt ),
		uint( InfoSystem::WeaponProperty::OneHandedPolearm | InfoSystem::WeaponProperty::TwoHandedPolearm ),
		uint( InfoSystem::WeaponProperty::OneHandedSideFirearm | InfoSystem::WeaponProperty::OneHandedPrimFirearm
			| InfoSystem::WeaponProperty::TwoHandedSideFirearm | InfoSystem::WeaponProperty::TwoHandedPrimFirearm ),
	};
	
	
	#define PF_CROUCHING     0x00000001
	#define PF_DOUBLEJUMP    0x00000002
	#define PF_PARRY         0x00000004
	#define PF_QUICKSHOT     0x00000008
	#define PF_DUELWIELDING  0x00000010
	#define PF_SLIDING       0x00000020
	
    class PlayrObject : EntityObject {
		PlayrObject() {
			@m_WeaponSlots[0] = @m_HeavyPrimary;
			@m_WeaponSlots[1] = @m_HeavySidearm;
			@m_WeaponSlots[2] = @m_LightPrimary;
			@m_WeaponSlots[3] = @m_LightSidearm;
			@m_WeaponSlots[4] = @m_Melee1;
			@m_WeaponSlots[5] = @m_Melee2;
			@m_WeaponSlots[6] = @m_RightArm;
			@m_WeaponSlots[7] = @m_LeftArm;
			@m_WeaponSlots[8] = @m_Ordnance;
			
			EntityManager.SetPlayerObject( @this );
			m_HudData.Init( @this );
			
			//
			// add keybind commands
			//
			
			// these specific movement commands MUST NOT CHANGE as they are hardcoded into the engine
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveNorth_Down_f, "+north" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveNorth_Up_f, "-north" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveSouth_Down_f, "+south" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveSouth_Up_f, "-south" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveWest_Down_f, "+west" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveWest_Up_f, "-west" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveEast_Down_f, "+east" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.MoveEast_Up_f, "-east" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.Jump_Down_f, "+up" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.Jump_Up_f, "-up" );
			
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.Crouch_Down_f, "+crouch" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.Crouch_Up_f, "-scrouch" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.Dash_Down_f, "+dash" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.Dash_Up_f, "-dash" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.UseWeapon_Down_f, "+useweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.UseWeapon_Up_f, "-useweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.AltUseWeapon_Down_f, "+altuseweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand( @this.AltUseWeapon_Up_f, "-altuseweap" );
		}
		
		/*
		private bool TryOneHanded( HandMode hand, const WeaponObject@ handWeapon, const WeaponObject@ otherWeapon ) {
			if ( hand != HandMode::Empty ) {
				if ( ( handWeapon.IsTwoHanded() && handWeapon.IsOneHanded() ) && @otherWeapon is @handWeapon ) {
					// being used by both arms, switch to one-handed
					return true;
				}
			}
			return false;
		}
		
		private WeaponObject@ GetEmptyHand() {
			if ( TryOneHanded( m_LeftHandMode, @m_LeftHandWeapon, @m_RightHandWeapon ) ) {
				m_LeftHandMode = HandMode::OneHanded;
				m_RightHandMode = HandMode::Empty;
				@m_RightHandWeapon = @m_RightArm;
				return @m_RightHandWeapon;
			}
			else if ( TryOneHanded( m_RightHandMode, @m_RightHandWeapon, @m_LeftHandWeapon ) ) {
				m_RightHandMode = HandMode::OneHanded;
				m_LeftHandMode = HandMode::Empty;
				@m_LeftHandWeapon = @m_LeftArm;
				return @m_LeftHandWeapon;
			}
		}
		*/
		
		private void SwitchWeaponWielding( InfoSystem::WeaponProperty& in hand, InfoSystem::WeaponProperty& in otherHand,
			WeaponObject@ weapon, WeaponObject@ other )
		{
			switch ( InfoSystem::WeaponProperty( uint( hand ) & InfoSystem::WeaponProperty::IsOneHanded ) ) {
			case InfoSystem::WeaponProperty::OneHandedPolearm:
			case InfoSystem::WeaponProperty::OneHandedBlunt:
			case InfoSystem::WeaponProperty::OneHandedBlade:
			case InfoSystem::WeaponProperty::OneHandedSideFirearm:
			case InfoSystem::WeaponProperty::OneHandedPrimFirearm:
				if ( weapon.IsTwoHanded() ) {
					// set to two-handed
					hand = InfoSystem::WeaponProperty( uint( hand ) & ~uint( InfoSystem::WeaponProperty::IsOneHanded ) );
					otherHand = hand;
					@other = @weapon;
					return;
				}
				break;
			case InfoSystem::WeaponProperty::None:
			default:
				// nothing in that hand
				break;
			};
			
			switch ( InfoSystem::WeaponProperty( uint( hand ) & InfoSystem::WeaponProperty::IsTwoHanded ) ) {
			case InfoSystem::WeaponProperty::TwoHandedPolearm:
			case InfoSystem::WeaponProperty::TwoHandedBlunt:
			case InfoSystem::WeaponProperty::TwoHandedBlade:
			case InfoSystem::WeaponProperty::TwoHandedSideFirearm:
			case InfoSystem::WeaponProperty::TwoHandedPrimFirearm:
				if ( weapon.IsOneHanded() ) {
					// set to one-handed
					hand = InfoSystem::WeaponProperty( uint( hand ) & ~InfoSystem::WeaponProperty::IsTwoHanded );
					@other = null;
					return;
				}
				break;
			case InfoSystem::WeaponProperty::None:
			default:
				// nothing in that hand
				break;
			};
			
			// if we're here, its most likely that the player is using a two-handed weapon that
			// can't be wielded one-handed or they're duel wielding something
		}
		
		private void SwitchWeaponMode( InfoSystem::WeaponProperty& in hand, const WeaponObject@ weapon ) {
			// weapon mode order (default)
			// blade -> blunt -> polearm -> firearm
			
			// the weapon mode list doesn't contain hand bits
			const uint handBits = ( uint( hand ) & uint( InfoSystem::WeaponProperty::IsTwoHanded ) ) |
				( uint( hand ) & uint( InfoSystem::WeaponProperty::IsOneHanded ) );
			
			// find the next most suitable mode
			for ( uint i = 0; i < sgame_WeaponModeList.Count(); i++ ) {
				if ( ( uint( weapon.GetProperties() ) & sgame_WeaponModeList[i] ) != 0 ) {
					hand = InfoSystem::WeaponProperty( ( uint( weapon.GetProperties() ) & sgame_WeaponModeList[i] ) | handBits );
				}
			}
		}
		private uint GetCurrentWeaponMode() const {
			switch ( m_nHandsUsed ) {
			case 0:
				return uint( m_LeftHandMode );
			case 1:
			case 2:
				return uint( m_RightHandMode );
			};
			return 0;
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
			m_PFlags |= PF_QUICKSHOT;
			beginQuickshotSfx.Play();
		}
		void Quickshot_Up_f() {
			m_PFlags &= ~PF_QUICKSHOT;
			endQuickshotSfx.Play();
		}
		void SwitchWeaponWielding_f() {
			weaponFancySfx.Play();
			switch ( m_nHandsUsed ) {
			case 0:
				SwitchWeaponWielding( m_LeftHandMode, m_RightHandMode, @m_LeftHandWeapon, @m_RightHandWeapon );
				break;
			case 1:
			case 2:
				SwitchWeaponWielding( m_RightHandMode, m_LeftHandMode, @m_RightHandWeapon, @m_LeftHandWeapon );
				break;
			};
		}
		void SwitchWeaponMode_f() {
			weaponFancySfx.Play();
			switch ( m_nHandsUsed ) {
			case 0:
				SwitchWeaponMode( m_LeftHandMode, @m_LeftHandWeapon );
				break;
			case 1:
				SwitchWeaponMode( m_RightHandMode, @m_RightHandWeapon );
				break;
			case 2: {
				const uint bits = uint( m_LeftHandMode );
				SwitchWeaponMode( m_LeftHandMode, @m_LeftHandWeapon );
				if ( bits == uint( m_LeftHandMode ) ) {
					SwitchWeaponMode( m_RightHandMode, @m_RightHandWeapon );
				}
				break; }
			default:
				break;
			};
		}
		void SwitchHand_f() {
			switch ( m_nHandsUsed ) {
			case 0:
				m_nHandsUsed = 1;
				break;
			case 1:
				m_nHandsUsed = 0;
				break;
			case 2:
			default:
				break; // can't switch if we're using both hands for one weapon
			};
		}
		void Crouch_Down_f() {
			if ( key_MoveNorth.active || key_MoveSouth.active || key_MoveWest.active || key_MoveEast.active ) {
				m_PFlags |= PF_SLIDING;
				beginSlidingSfx.Play();
				@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_SLIDING );
			} else {
				m_PFlags |= PF_CROUCH;
				crouchSfxDown.Play();
				@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_CROUCH );
			}
		}
		void Crouch_Up_f() {
			if ( ( m_PFlags & PF_CROUCH ) != 0 ) {
				crouchSfxUp.Play();
			}
			m_PFlags &= ~( PF_CROUCH | PF_SLIDING );
		}
		
		void Melee_Down_f() {
			m_nParryBoxWidth = 0.0f;
			@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_MELEE );
		}
		void Melee_Up_f() {
			@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE );
		}
		
		void NextWeapon_f() {
			m_CurrentWeapon++;
			if ( m_CurrentWeapon >= m_WeaponSlots.Count() ) {
				m_CurrentWeapon = 0;
			}
			m_WeaponSlots[ m_CurrentWeapon ].GetInfo().equipSfx.Play();
		}
		void PrevWeapon_f() {
			m_CurrentWeapon--;
			if ( m_CurrentWeapon < 0 ) {
				m_CurrentWeapon = m_WeaponSlots.Count();
			}
			m_WeaponSlots[ m_CurrentWeapon ].GetInfo().equipSfx.Play();
		}
		void UseWeapon_Down_f() { m_bUseWeapon = true; }
		void UseWeapon_Up_f() { m_bUseWeapon = false; }
		void AltUseWeapon_Down_f() { m_bAltUseWeapon = true; }
		void AltUseWeapon_Up_f() { m_bAltUseWeapon = false; }
		void UseItem_f() {
		}
		void PickupItem_f() {
		}
		void Emote_f() {
			m_bEmoting = true;
//			m_nEmoteLifeTime = 0;
//			m_nEmoteEndTime = GameSystem::GameManager.GetGameMsec() + m_SelectedEmote.m_nDuration;
		}
		void EmoteWheel_Down_f() {
//			m_bEmoteWheelActive = true;
		}
		void EmoteWheel_Up_f() {
//			m_bEmoteWheelActive = false;
		}
		
		void DrawEmoteWheel() {
//			if ( !m_bEmoteWheelActive ) {
//				DebugPrint( "DrawEmoteWheel: called when inactive.\n" );
//				return;
//			}
			
		}

		bool IsSliding() const {
			return ( m_PFlags & PF_SLIDING ) != 0;
		}
		bool IsCrouching() const {
			return ( m_PFlags & PF_CROUCHING ) != 0;
		}

		void SetLegFacing( int facing ) {
			m_LegsFacing = facing;
		}

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) override {
			LoadBase( section );

			m_PFlags = section.LoadUInt( "playrFlags" );

			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			SaveBase( section );

			section.SaveUInt( "playrFlags", m_PFlags );
		}
		
		float GetHealthMult() const {
			return m_nHealMult;
		}
		float GetDamageMult() const {
			return m_nDamageMult;
		}
		float GetRage() const {
			return m_nRage;
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
				dieSfx[ TheNomad::Util::PRandom() & dieSfx.Count() ].Play();
				EntityManager.KillEntity( @this );
				
				TheNomad::Util::HapticRumble( 0.	80f, 4000 );
			} else {
				painSfx[ TheNomad::Util::PRandom() & painSfx.Count() ].Play();
				
				TheNomad::Util::HapticRumble( 0.50f, 300 );
			}
		}
		
		void SetPFlags( uint flags ) {
			m_PFlags = flags;
		}
		uint GetPFlags() const {
			return m_PFlags;
		}

		WeaponObject@ GetCurrentWeapon() {
			return @m_WeaponSlots[m_CurrentWeapon];
		}
		const WeaponObject@ GetCurrentWeapon() const {
			return @m_WeaponSlots[m_CurrentWeapon];
		}
		
		void Think() override {
			ivec3 origin;
			
			if ( ( m_PFlags & PF_PARRY ) != 0 ) {
				ParryThink();
			} else if ( ( m_PFlags & PF_QUICKSHOT ) != 0 ) {
				m_QuickShot.Think();
			}
			
			if ( m_bUseWeapon ) {
				m_CurrentWeapon.Use( cast<EntityObject@>( @this ), GetCurrentWeaponMode() );
			} else if ( m_bUseAltWeapon ) {
				m_CurrentWeapon.UseAlt( cast<EntityObject@>( @this ), GetCurrentWeaponMode() );
			}
			
			for ( uint i = 0; i < 3; i++ ) {
				origin[i] = int( floor( m_Link.m_Origin[i] ) );
			}
			
			// run a movement frame
			Pmove.RunTic();

			if ( m_nHealth <= 15.0f ) {
				// if there's another haptic going on, we don't want to annihilate their hands
				TheNomad::Util::HapticRumble( 0.	50f, 1000 );
			}

			if ( m_nHealth < 100.0f ) {
				m_nHealth += sgame_PlayerHealBase.GetFloat() * m_nHealMult;
				m_nHealMult -= m_nHealMultDecay * LevelManager.GetDifficultyScale();

				if ( m_nHealth > 100.0f ) {
					m_nHealth = 100.0f;
				}
				if ( m_nHealMult < 0.0f ) {
					m_nHealMult = 0.0f;
				}
			}

			m_HudData.Draw();
		}
		
		private float GetGfxDirection() const {
			if ( m_Facing == 1 ) {
				return -( m_Link.m_Bounds.m_nWidth / 2 );
			}
			return ( m_Link.m_Bounds.m_nWidth / 2 );
		}
		
		//
		// PlayrObject::CheckParry: called from DamageEntity mob v player
		//
		bool CheckParry( EntityObject@ ent, const InfoSystem::AttackInfo@ info ) {
			if ( Util::BoundsIntersect( ent.GetBounds(), m_ParryBox ) ) {
				if ( ent.IsProjectile() ) {
					// simply invert the direction and double the speed
					const vec3 v = ent.GetVelocity();
					ent.SetVelocity( vec3( v.x * 2, v.y * 2, v.z * 2 ) );
					ent.SetAngle( ent.GetAngle() * 2.0f / M_PI / 360.0f );
					ent.SetDirection( TheNomad::GameSystem::InverseDirs[ ent.GetDirection() ] );
				}
			}
			if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob && !ent.IsProjectile() ) {
				// just a normal counter
				MobObject@ mob = cast<MobObject@>( @ent );
				
				if ( info.canParry ) {
					// unblockable, deal damage
					return false;
				}
				else {
					// slap it back
					if ( mob.GetState().GetTics() <= ( mob.GetState().GetTics() / 4 ) ) {
						// counter parry, like in MGR, but more brutal, but for the future...
					}
					
					// TODO: make dead mobs with ANY velocity flying corpses
					EntityManager.DamageEntity( @this, @ent );
					parrySfx.Play();
				}
			}
			
			// add in the haptic
			TheNomad::Engine::CmdExecuteCommand( "in_haptic_rumble 0.8 500" );
			
			// add the fireball
			GfxManager.AddExplosionGfx( vec3( m_Link.m_Origin.x + GetGfxDirection(), m_Link.m_Origin.y, m_Link.m_Origin.z ) );

			return true;
		}
		
		void MakeSound() {
			if ( m_State.GetID() == StateNum::ST_PLAYR_CROUCH ) {
				return;
			}
			
			array<EntityObject@>@ entList = @EntityManager.GetEntities();
			for ( uint i = 0; i < entList.Count(); i++ ) {
				if ( entList[i].GetType() != TheNomad::GameSystem::EntityType::Mob ) {
					continue;
				}
				
				const vec2 detection = vec2( mob.GetInfo().detectionRangeX, mob.GetInfo().detectionRangeY );
				
				MobObject@ mob = cast<MobObject@>( @entList[i] );
				const float dist = Util::Distance( mob.GetOrigin(), m_Link.m_Origin );
				if ( dist < detection.length() ) {
					// is there a wall there?
					TheNomad::GameSystem::RayCast ray;
					const vec3& origin = mob.GetOrigin();
					
					ray.m_nLength = dist;
					ray.m_nAngle = atan2( ( origin.x - m_Link.m_Origin.x ), ( origin.y - m_Link.m_Origin.y ) );
					ray.m_Origin = m_Link.m_Origin;
					
					TheNomad::GameSystem::CastRay( @ray );
					if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
						continue; // hit a wall or dead air, no detection
					}
					
					mob.SetAlert( @this, dist );
				}
			}
		}
		
		///
		/// PlayrObject::ParryThink: calculates the bounds of the parry box
		///
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
			m_nRage = 100.0f;
		}
		
		// custom draw because of adaptive weapons and leg sprites
		void Draw() override {
			int hLegSprite = FS_INVALID_HANDLE;
			
			TheNomad::Engine::Renderer::AddSpriteToScene( m_Link.m_Origin, m_SpriteSheet.GetHandle(),
				m_SpriteSheet[ m_State.GetSpriteOffset().y * m_State.GetSpriteOffset().x + m_State.GetAnimation().GetFrame() ] );
			
			//
			// draw the legs
			//
			hLegSprite = m_LegsFacing;
			if ( m_Velocity == vec3( 0.0f ) ) {
				// not moving at all, just draw idle legs
				if ( !Pmove.groundPlane ) {
					// static air legs
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_IDLE_AIR + m_LegsFacing );
				} else {
					// idle ground legs
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_IDLE_GROUND + m_LegsFacing );
				}
			}
			else if ( !Pmove.groundPlane ) {
				// we're in the air, modify the legs to show a bit of momentum control
				if ( m_Velocity.z < 0.0f ) {
					// falling down
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_FALL_AIR + m_LegsFacing );
				}
				else if ( m_Debuff == AttackEffect::Stunned ) {
					// player is literally flying through the air
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_STUN_AIR + m_LegsFacing );
				}
				else {
					// ascending
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_ASCENDING + m_LegsFacing );
				}
			}
			else {
				// moving on the ground
				if ( m_Debuff == AttackEffect::Stunned ) {
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_STUN_GROUND + m_LegsFacing );
				} else {
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_MOVE_GROUND + m_LegsFacing );
				}
			}
			
			hLegSprite = m_LegState.GetSpriteOffset().y * m_LegSpriteSheet.GetSpriteCount().x +
				m_LegState.GetSpriteOffset().x + m_LegState.GetAnimation().GetFrame();
			TheNomad::Engine::Renderer::AddSpriteToScene( m_Link.m_Origin, m_SpriteSheet.GetHandle(),
				m_SpriteSheet[ hLegSprite ] );
		}
		
		KeyBind key_MoveNorth, key_MoveSouth, key_MoveEast, key_MoveWest;
		KeyBind key_Jump, key_Melee;
		
		private TheNomad::GameSystem::BBox m_ParryBox;
		private float m_nParryBoxWidth;
		
		// toggled with "sgame_SaveLastUsedWeaponModes"
		private InfoSystem::WeaponProperty[] m_WeaponModes( 9 );
		
		// 9 weapons in total
		private WeaponObject@[] m_WeaponSlots( 9 );
		private WeaponObject m_HeavyPrimary;
		private WeaponObject m_HeavySidearm;
		private WeaponObject m_LightPrimary;
		private WeaponObject m_LightSidearm;
		private WeaponObject m_Melee1;
		private WeaponObject m_Melee2;
		private WeaponObject m_RightArm;
		private WeaponObject m_LeftArm;
		private WeaponObject m_Ordnance;
		private int m_CurrentWeapon = 0;
		
		private QuickShot m_QuickShot;
		private uint m_PFlags = 0;
		
		// sound effects
		private TheNomad::Engine::SoundSystem::SoundEffect parrySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect counterParrySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect beginQuickshotSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect endQuickshotSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect beginSlideSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect crouchDownSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect crouchUpSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect[] painSfx( 3 );
		private TheNomad::Engine::SoundSystem::SoundEffect[] dieSfx( 3 );

		// the amount of damage dealt in the frame
		private float m_nFrameDamage = 0.0f;
		
		// what does the player have in their hands?
		private int m_nHandsUsed = 0; // 0 for left, 1 for right, 2 if two-handed
		private WeaponObject@ m_LeftHandWeapon = null;
		private WeaponObject@ m_RightHandWeapon = null;
		private InfoSystem::WeaponProperty m_LeftHandMode = InfoSystem::WeaponProperty::None;
		private InfoSystem::WeaponProperty m_RightHandMode = InfoSystem::WeaponProperty::None;
		
		private bool m_bUseWeapon = false;
		private bool m_bAltUseWeapon = false;

		// the lore goes: the harder and faster you hit The Nomad, the harder and faster they hit back
		private float m_nDamageMult = 0.0f;
		private float m_nRage = 0.0f;

		private float m_nHealMult = 0.0f;
		private float m_nHealMultDecay = 1.0f;
		
		private SpriteSheet@ m_LegSpriteSheet;
		private int m_LegsFacing = 0;

		private bool m_bEmoting = false;
		
		private EntityState@ m_LegState;
		private PlayerDisplayUI m_HudData;
		private PMoveData Pmove;
	};
};
