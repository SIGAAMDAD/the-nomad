#include "Engine/SoundSystem/SoundFrameData.as"
#include "SGame/KeyBind.as"
#include "SGame/PlayerSystem/QuickShot.as"
#include "SGame/PlayerSystem/PMoveData.as"
#include "SGame/PlayerSystem/PlayerHud.as"
#include "SGame/PlayerSystem/SplitScreen.as"
#include "SGame/PlayerSystem/StyleTracker.as"

namespace TheNomad::SGame {
	const uint[] sgame_WeaponModeList = {
		uint( InfoSystem::WeaponProperty::OneHandedBlade | InfoSystem::WeaponProperty::TwoHandedBlade ),
		uint( InfoSystem::WeaponProperty::OneHandedBlunt | InfoSystem::WeaponProperty::TwoHandedBlunt ),
		uint( InfoSystem::WeaponProperty::OneHandedPolearm | InfoSystem::WeaponProperty::TwoHandedPolearm ),
		uint( InfoSystem::WeaponProperty::OneHandedSideFirearm | InfoSystem::WeaponProperty::OneHandedPrimFirearm
			| InfoSystem::WeaponProperty::TwoHandedSideFirearm | InfoSystem::WeaponProperty::TwoHandedPrimFirearm ),
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

			dieSfx0 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/die0.wav" );
			dieSfx1 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/die1.wav" );
			dieSfx2 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/die2.wav" );

			painSfx0 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain0.wav" );
			painSfx1 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain1.wav" );
			painSfx2 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain2.wav" );

			slideSfx0 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/slide0.wav" );
			slideSfx1 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/slide1.wav" );

			dashSfx = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/dash.ogg" );

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
		
		void SwitchWeaponWielding( InfoSystem::WeaponProperty& in hand, InfoSystem::WeaponProperty& in otherHand,
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
		void SwitchWeaponMode( InfoSystem::WeaponProperty& in hand, const WeaponObject@ weapon ) {
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
		InfoSystem::WeaponProperty GetCurrentWeaponMode() {
			switch ( m_nHandsUsed ) {
			case 0:
				return m_LeftHandMode;
			case 1:
			case 2:
				return m_RightHandMode;
			};
			return InfoSystem::WeaponProperty::None;
		}
		
		void DrawEmoteWheel() {
//			if ( !m_bEmoteWheelActive ) {
//				DebugPrint( "DrawEmoteWheel: called when inactive.\n" );
//				return;
//			}
			
		}

		bool IsSliding() const {
			return m_State.GetID() == StateNum::ST_PLAYR_SLIDING;
		}
		bool IsCrouching() const {
			return m_State.GetID() == StateNum::ST_PLAYR_CROUCHING;
		}
		bool IsDoubleJumping() const {
			return m_State.GetID() == StateNum::ST_PLAYR_DOUBLEJUMP;
		}

		void SetLegsFacing( int facing ) {
			m_LegsFacing = facing;
		}

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) override {
			LoadBase( section );

			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			SaveBase( section );
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
		
		void Damage( EntityObject@ attacker, float nAmount ) {
			if ( m_bEmoting ) {
				return; // god has blessed thy soul...
			}
			
			m_nHealth -= nAmount;

			if ( m_nHealth < 1 ) {
				if ( m_nFrameDamage > 0 ) {
					return; // as long as you're hitting SOMETHING, you cannot die
				}
				switch ( Util::PRandom() & 3 ) {
				case 0:
					dieSfx0.Play();
					break;
				case 1:
					dieSfx1.Play();
					break;
				case 2:
					dieSfx2.Play();
					break;
				};
				EntityManager.KillEntity( @attacker, @this );
				
				Util::HapticRumble( ScreenData.GetPlayerIndex( @this ), 0.80f, 4000 );
			} else {
				switch ( Util::PRandom() & 3 ) {
				case 0:
					painSfx0.Play();
					break;
				case 1:
					painSfx1.Play();
					break;
				case 2:
					painSfx2.Play();
					break;
				};
				
				Util::HapticRumble( ScreenData.GetPlayerIndex( @this ), 0.50f, 300 );
			}
		}
		
		WeaponObject@ GetCurrentWeapon() {
			return @m_WeaponSlots[ m_CurrentWeapon ];
		}
		const WeaponObject@ GetCurrentWeapon() const {
			return @m_WeaponSlots[ m_CurrentWeapon ];
		}
		
		void Think() override {
			TheNomad::Engine::SoundSystem::SetWorldListener( m_Link.m_Origin );

			if ( m_State.GetID() == StateNum::ST_PLAYR_QUICKSHOT ) {
				m_QuickShot.Think();
			}

			if ( m_bUseWeapon ) {
				m_nFrameDamage += GetCurrentWeapon().Use( cast<EntityObject@>( @this ), GetCurrentWeaponMode() );
			} else if ( m_bAltUseWeapon ) {
				m_nFrameDamage += GetCurrentWeapon().UseAlt( cast<EntityObject@>( @this ), GetCurrentWeaponMode() );
			}
		
			
			// run a movement frame
			Pmove.RunTic();

			if ( m_nHealth <= 15.0f ) {
				// if there's another haptic going on, we don't want to annihilate their hands
				Util::HapticRumble( ScreenData.GetPlayerIndex( @this ), 0.50f, 1000 );
			}

			if ( m_nHealth < 100.0f ) {
				m_nHealth += m_nHealMult * sgame_PlayerHealBase.GetFloat();
				m_nHealMult -= m_nHealMultDecay;

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
					ent.SetDirection( InverseDirs[ ent.GetDirection() ] );
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
					EntityManager.DamageEntity( cast<EntityObject@>( @this ), @ent );
					parrySfx.Play();
				}
			}
			
			// add in the haptic
			Util::HapticRumble( ScreenData.GetPlayerIndex( @this ), 0.80f, 500 );
			
			// add the fireball
			GfxManager.AddExplosionGfx( vec3( m_Link.m_Origin.x + GetGfxDirection(), m_Link.m_Origin.y, m_Link.m_Origin.z ) );

			return true;
		}
		
		void MakeSound() {
			if ( m_State.GetID() == StateNum::ST_PLAYR_CROUCHING ) {
				return;
			}
			
			array<EntityObject@>@ entList = @EntityManager.GetEntities();
			for ( uint i = 0; i < entList.Count(); i++ ) {
				if ( entList[i].GetType() != TheNomad::GameSystem::EntityType::Mob ) {
					continue;
				}
				MobObject@ mob = cast<MobObject@>( @entList[i] );
				InfoSystem::MobInfo@ info = cast<InfoSystem::MobInfo@>( @mob.GetInfo() );
				const vec3 detection = vec3( float( info.detectionRangeX ), float( info.detectionRangeY ), 0.0f );
				const float dist = Util::Distance( mob.GetOrigin(), m_Link.m_Origin );

				if ( dist < Util::VectorLength( detection ) ) {
					// is there a wall there?
					TheNomad::GameSystem::RayCast ray;
					const vec3 origin = mob.GetOrigin();
					
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
		
		//
		// PlayrObject::ParryThink: calculates the bounds of the parry box
		//
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
			//
			// init all player data
			//
			m_Link.m_Origin = origin;
			m_nHealth = 100.0f;
			m_nRage = 100.0f;
			@m_LeftHandWeapon = @m_LeftArm;
			@m_RightHandWeapon = @m_RightArm;
			m_CurrentWeapon = 0;
			m_Link.m_nEntityType = TheNomad::GameSystem::EntityType::Playr;

			m_EmptyInfo.name = "Fist (Empty)";
			m_EmptyInfo.type = ENTITYNUM_INVALID - 2;
			m_EmptyInfo.damage = 100.0f; // this is the god of war
			m_EmptyInfo.range = 1.5f;
			m_EmptyInfo.magSize = 0;
			m_EmptyInfo.weaponProps = InfoSystem::WeaponProperty::OneHandedBlunt;
			m_EmptyInfo.weaponType = InfoSystem::WeaponType::LeftArm;

			InfoSystem::InfoManager.GetWeaponTypes()[ "weapon_fist" ] = ENTITYNUM_INVALID - 2;
			InfoSystem::InfoManager.AddWeaponInfo( @m_EmptyInfo );
			for ( uint i = 0; i < m_WeaponSlots.Count(); i++ ) {
				m_WeaponSlots[i].SetOwner( cast<EntityObject@>( @this ) );
				m_WeaponSlots[i].Spawn( ENTITYNUM_INVALID - 2, origin );
			}

			m_PhysicsObject.Init( cast<EntityObject@>( @this ) );
			m_PhysicsObject.SetAngle( Util::Dir2Angle( TheNomad::GameSystem::DirType::East ) );

			m_Direction = Util::Angle2Dir( m_PhysicsObject.GetAngle() );

			@m_SpriteSheet = TheNomad::Engine::ResourceCache.GetSpriteSheet( "sprites/players/raio_base", 32, 32, 512, 512 );
			if ( @m_SpriteSheet is null ) {
				GameError( "PlayrObject::Spawn: failed to initialize sprite sheet" );
			}
			@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE );
			if ( @m_State is null ) {
				GameError( "PlayrObject::Spawn: failed to initialize idle state" );
			}

			m_nHealMult = 0.0f;
			m_nHealMultDecay = LevelManager.GetDifficultyScale();
		}
		
		// custom draw because of adaptive weapons and leg sprites
		void Draw() override {
			int hLegSprite = FS_INVALID_HANDLE;
			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.origin = m_Link.m_Origin;
			refEntity.sheetNum = m_SpriteSheet.GetShader();
			refEntity.spriteId = 0 + m_Facing;
			refEntity.scale = 2.0f;
			refEntity.Draw();
//			TheNomad::Engine::Renderer::AddSpriteToScene( m_Link.m_Origin, refEntity.sheetNum, refEntity.spriteId );
//			TheNomad::Engine::Renderer::AddPolyToScene( m_SpriteSheet.GetShader(), verts );
			
			//
			// draw the legs
			//
			hLegSprite = m_LegsFacing;
			if ( m_PhysicsObject.GetVelocity() == Vec3Origin ) {
				// not moving at all, just draw idle legs
				if ( !Pmove.groundPlane ) {
					// static air legs
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_IDLE_AIR );
				} else {
					// idle ground legs
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_IDLE_GROUND );
				}
			}
			else if ( !Pmove.groundPlane ) {
				// we're in the air, modify the legs to show a bit of momentum control
				if ( m_PhysicsObject.GetVelocity().z < 0.0f ) {
					// falling down
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_FALL_AIR );
				}
				else if ( m_Debuff == AttackEffect::Stunned ) {
					// player is literally flying through the air
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_STUN_AIR );
				}
				else {
					// ascending
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_IDLE_AIR );
				}
			}
			else {
				// moving on the ground
				if ( m_Debuff == AttackEffect::Stunned ) {
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_STUN_GROUND );
				} else {
					@m_LegState = @StateManager.GetStateForNum( StateNum::ST_PLAYR_LEGS_MOVE_GROUND );
				}
			}

			refEntity.origin = m_Link.m_Origin;
			refEntity.sheetNum = m_SpriteSheet.GetShader();
			refEntity.spriteId = 29 + m_LegsFacing;
			refEntity.scale = 2.0f;
			refEntity.Draw();
//			TheNomad::Engine::Renderer::AddSpriteToScene( m_Link.m_Origin, refEntity.sheetNum, refEntity.spriteId );
//			TheNomad::Engine::Renderer::AddPolyToScene( m_SpriteSheet.GetShader(), verts );
			
//			hLegSprite = m_LegState.GetSpriteOffset().y * m_SpriteSheet.GetSpriteCountX() +
//				m_LegState.GetSpriteOffset().x + m_LegState.GetAnimation().GetFrame() + m_LegsFacing;
//			TheNomad::Engine::Renderer::AddSpriteToScene( m_Link.m_Origin, m_SpriteSheet.GetShader(), hLegSprite );
		}
		
		KeyBind key_MoveNorth, key_MoveSouth, key_MoveEast, key_MoveWest;
		KeyBind key_Jump, key_Melee;
		
		private TheNomad::GameSystem::BBox m_ParryBox;
		float m_nParryBoxWidth;
		
		// toggled with "sgame_SaveLastUsedWeaponModes"
		private InfoSystem::WeaponProperty[] m_WeaponModes( 9 );

		TheNomad::Engine::Renderer::PolyVert[] verts( 4 );
		
		// 9 weapons in total
		WeaponObject@[] m_WeaponSlots( 9 );
		WeaponObject m_HeavyPrimary;
		WeaponObject m_HeavySidearm;
		WeaponObject m_LightPrimary;
		WeaponObject m_LightSidearm;
		WeaponObject m_Melee1;
		WeaponObject m_Melee2;
		WeaponObject m_RightArm;
		WeaponObject m_LeftArm;
		WeaponObject m_Ordnance;
		InfoSystem::WeaponInfo m_EmptyInfo;
		int m_CurrentWeapon = 0;
		
		private QuickShot m_QuickShot;
		
		//
		// sound effects
		//
		private TheNomad::Engine::SoundSystem::SoundEffect parrySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect counterParrySfx;
		TheNomad::Engine::SoundSystem::SoundEffect beginQuickshotSfx;
		TheNomad::Engine::SoundSystem::SoundEffect endQuickshotSfx;
		TheNomad::Engine::SoundSystem::SoundEffect beginSlidingSfx;
		TheNomad::Engine::SoundSystem::SoundEffect crouchDownSfx;
		TheNomad::Engine::SoundSystem::SoundEffect crouchUpSfx;
		TheNomad::Engine::SoundSystem::SoundEffect dashSfx;
		TheNomad::Engine::SoundSystem::SoundEffect slideSfx0;
		TheNomad::Engine::SoundSystem::SoundEffect slideSfx1;
		TheNomad::Engine::SoundSystem::SoundEffect weaponFancySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect painSfx0;
		private TheNomad::Engine::SoundSystem::SoundEffect painSfx1;
		private TheNomad::Engine::SoundSystem::SoundEffect painSfx2;
		private TheNomad::Engine::SoundSystem::SoundEffect dieSfx0;
		private TheNomad::Engine::SoundSystem::SoundEffect dieSfx1;
		private TheNomad::Engine::SoundSystem::SoundEffect dieSfx2;

		// the amount of damage dealt in the frame
		private float m_nFrameDamage = 0.0f;
		
		// what does the player have in their hands?
		int m_nHandsUsed = 0; // 0 for left, 1 for right, 2 if two-handed
		WeaponObject@ m_LeftHandWeapon = null;
		WeaponObject@ m_RightHandWeapon = null;
		InfoSystem::WeaponProperty m_LeftHandMode = InfoSystem::WeaponProperty::None;
		InfoSystem::WeaponProperty m_RightHandMode = InfoSystem::WeaponProperty::None;
		
		bool m_bUseWeapon = false;
		bool m_bAltUseWeapon = false;

		// the lore goes: the harder and faster you hit The Nomad, the harder and faster they hit back
		private float m_nDamageMult = 0.0f;
		private float m_nRage = 0.0f;

		private float m_nHealMult = 0.0f;
		private float m_nHealMultDecay = 1.0f;
		
		private SpriteSheet@ m_LegSpriteSheet;
		private int m_LegsFacing = 0;

		int m_nTimeSinceDash = 1000;
		bool m_bDashing = false;

		bool m_bEmoting = false;
		
		private EntityState@ m_LegState;
		private PlayerDisplayUI m_HudData;
		PMoveData Pmove( @this );
	};
};
