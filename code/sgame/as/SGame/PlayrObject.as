#include "Engine/SoundSystem/SoundFrameData.as"
#include "SGame/KeyBind.as"
#include "SGame/QuickShot.as"
#include "SGame/PMoveData.as"
#include "SGame/PlayerHud.as"

namespace TheNomad::SGame {
	class WeaponMode {
		WeaponMode() {
		}
		WeaponMode( uint weaponBits ) {
			bits = weaponBits;
		}

		uint bits;
	};

	const WeaponMode[] sgame_WeaponModeList = {
		WeaponMode( InfoSystem::WeaponProperty::OneHandedBlade | InfoSystem::WeaponProperty::TwoHandedBlade ),
		WeaponMode( InfoSystem::WeaponProperty::OneHandedBlunt | InfoSystem::WeaponProperty::TwoHandedBlunt ),
		WeaponMode( InfoSystem::WeaponProperty::OneHandedPolearm | InfoSystem::WeaponProperty::TwoHandedPolearm ),
		WeaponMode( InfoSystem::WeaponProperty::OneHandedSideFirearm | InfoSystem::WeaponProperty::OneHandedPrimFirearm
			| InfoSystem::WeaponProperty::TwoHandedSideFirearm | InfoSystem::WeaponProperty::TwoHandedPrimFirearm ),
	};

	const uint PF_PARRY        = 0x00000001;
	const uint PF_DOUBLEJUMP   = 0x00000002;
	const uint PF_QUICKSHOT    = 0x00000004;
	const uint PF_DUELWIELDING = 0x00000008;
	
	class Emote {
		Emote() {
		}
		Emote( uint time, int hShader, const string& in spriteSheet, const vec2& in spriteSize, const vec2& in sheetSize ) {
			m_nDuration = time;
			m_hShader = hShader;
			@m_DrawData = SpriteSheet( spriteSheet, spriteSize, sheetSize );
		}
		
		void Activate() {
			m_nLifeTime = 0;
//			m_nEndTime = GameSystem::GameManager.GetGameTics() + m_nDuration;
		}
		void Draw() {
//			m_nLifeTime += GameSystem::GameManager.GetDeltaTics();
			if ( m_nLifeTime >= m_nEndTime ) {
				return;
			}
		}
		
		SpriteSheet@ m_DrawData = null;
		int m_hShader = FS_INVALID_HANDLE;
		uint m_nEndTime = 0;
		uint m_nDuration = 0;
		uint m_nLifeTime = 0;
	};
	
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

			m_nHealth = 100.0f;
			
			EntityManager.SetPlayerObject( @this );
			m_HudData.Init( @this );
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
				if ( ( uint( weapon.GetProperties() ) & sgame_WeaponModeList[i].bits ) != 0 ) {
					hand = InfoSystem::WeaponProperty( ( uint( weapon.GetProperties() ) & sgame_WeaponModeList[i].bits ) | handBits );
				}
			}
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
		void Quickshot_Down_f() { }
		void Quickshot_Up_f() { }
		void SwitchWeaponWielding_f() {
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
		void Melee_Down_f() {
			m_nParryBoxWidth = 0.0f;
			SetState( StateNum::ST_PLAYR_MELEE );
		}
		
		void NextWeapon_f() {
			m_CurrentWeapon++;
			if ( m_CurrentWeapon >= m_WeaponSlots.Count() ) {
				m_CurrentWeapon = 0;
			}
		}
		void PrevWeapon_f() {
			if ( m_CurrentWeapon == 0 ) {
				m_CurrentWeapon = m_WeaponSlots.Count();
			} else {
				m_CurrentWeapon--;
			}
		}
		void UseWeapon_f() {
			m_WeaponSlots[ m_CurrentWeapon ].Use( cast<EntityObject@>( @this ) );
		}
		void AltUseWeapon_f() {
			m_WeaponSlots[ m_CurrentWeapon ].UseAlt( cast<EntityObject@>( @this ) );
		}
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

		void SetLegFacing( int facing ) {
			m_LegsFacing = facing;
		}

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) override {
			LoadBase( section );

			m_PFlags = section.LoadUInt( "playrFlags" );
			m_Facing = section.LoadUInt( "torsoFacing" );

			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			SaveBase( section );

			section.SaveUInt( "playrFlags", m_PFlags );
			section.SaveUInt( "legsFacing", m_LegsFacing );
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

		WeaponObject@ GetCurrentWeapon() {
			return @m_WeaponSlots[m_CurrentWeapon];
		}
		const WeaponObject@ GetCurrentWeapon() const {
			return @m_WeaponSlots[m_CurrentWeapon];
		}
		
		void Think() override {
			if ( ( m_PFlags & PF_PARRY ) != 0 ) {
				ParryThink();
			} else if ( ( m_PFlags & PF_QUICKSHOT ) != 0 ) {
				m_QuickShot.Think();
			}
			
//			switch ( m_State.GetID() ) {
//			case StateNum::ST_PLAYR_IDLE:
//				IdleThink();
//				break; // NOTE: maybe let the player move in combat? (that would require more sprites for the dawgs)
//			case StateNum::ST_PLAYR_COMBAT:
//				CombatThink();
//				break;
//			};

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
			}
			else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob ) {
				// just a normal counter
				MobObject@ mob = cast<MobObject>( ent.GetData() );
				
				if ( !mob.CurrentAttack().canParry ) {
					// unblockable, deal damage
					EntityManager.DamageEntity( @ent, @this );
					return false;
				}
				else {
					// slap it back
					if ( mob.GetState().GetTics() <= ( mob.GetState().GetTics() / 4 ) ) {
						// counter parry, like in MGR, but more brutal, but for the future...
					}
					
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
			
			array<EntityObject@>@ entList = @EntityManager.GetEntities();
			for ( uint i = 0; i < entList.Count(); i++ ) {
				if ( entList[i].GetType() != TheNomad::GameSystem::EntityType::Mob ) {
					continue;
				}
				
				MobObject@ mob = cast<MobObject@>( @entList[i] );
				if ( TheNomad::Util::Distance( mob.GetOrigin(), m_Link.m_Origin ) < cast<InfoSystem::MobInfo>( mob.GetInfo() ).soundTolerance ) {
					// is there a wall there?
					
				}
			}
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
		
		KeyBind key_MoveNorth, key_MoveSouth, key_MoveEast, key_MoveWest;
		KeyBind key_Jump, key_Melee;
		
		private TheNomad::GameSystem::BBox m_ParryBox;
		private float m_nParryBoxWidth;
		
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
		
		// what does the player have in their hands?
		private int m_nHandsUsed = 0; // 0 for left, 1 for right, 2 if two-handed
		private WeaponObject@ m_LeftHandWeapon = null;
		private WeaponObject@ m_RightHandWeapon = null;
		private InfoSystem::WeaponProperty m_LeftHandMode = InfoSystem::WeaponProperty::None;
		private InfoSystem::WeaponProperty m_RightHandMode = InfoSystem::WeaponProperty::None;

		// the lore goes: the more and harder you hit The Nomad, the harder and faster they hit back
		private float m_nDamageMult = 0.0f;
		private float m_nRage = 0.0f;

		private float m_nHealMult = 0.0f;
		private float m_nHealMultDecay = 1.0f;

		private int m_LegsFacing = 0;

		private bool m_bEmoting = false;
		
		private PlayerDisplayUI m_HudData;
		private PMoveData Pmove;
	};
};