#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/InfoSystem/WeaponInfo.as"
#include "itemlib/ItemScript.as"

namespace TheNomad::SGame {
    class WeaponObject : ItemObject {
		WeaponObject() {
		}

		InfoSystem::WeaponProperty GetProperties() const {
			return m_WeaponInfo.weaponProps;
		}

		const string LogWeaponMode( uint weaponMode ) const {
			string msg = "WeaponMode is ";
			
			if ( ( weaponMode & InfoSystem::WeaponProperty::IsTwoHanded ) != 0 ) {
				msg += "[TwoHanded]";
			}
			if ( ( weaponMode & InfoSystem::WeaponProperty::IsOneHanded ) != 0 ) {
				msg += "[OneHanded]";
			}
			if ( ( weaponMode & InfoSystem::WeaponProperty::IsFirearm ) != 0 ) {
				msg += "[Firearm]";
			}
			if ( ( weaponMode & InfoSystem::WeaponProperty::IsBlunt ) != 0 ) {
				msg += "[Blunt]";
			}
			if ( ( weaponMode & InfoSystem::WeaponProperty::IsBladed ) != 0 ) {
				msg += "[Bladed]";
			}

			return msg;
		}

		void SetOwner( EntityObject@ ent ) override {
			if ( @ent is null ) {
				DebugPrint( "Clearing ownership of item '" + m_Link.m_nEntityNumber + "'\n" );
				@m_Owner = null;
				return;
			}

			// make sure nothing weird is going on
			switch ( ent.GetType() ) {
			case GameSystem::EntityType::Item:
			case GameSystem::EntityType::Weapon:
				GameError( "ItemObject::Pickup: invalid pickup entity (item/weapon)" );
			default:
				break;
			};

			@m_Owner = @ent;
			SetUseMode( m_WeaponInfo.defaultMode );

			m_Bounds.Clear();

			DebugPrint( "Weapon " + m_Link.m_nEntityNumber + " now owned by " + ent.GetEntityNum() + ".\n" );
		}

		const InfoSystem::WeaponInfo@ GetWeaponInfo() const {
			return @m_WeaponInfo;
		}
		InfoSystem::WeaponInfo@ GetWeaponInfo() {
			return @m_WeaponInfo;
		}
		void SetAmmo( InfoSystem::AmmoInfo@ ammo ) {
			@m_AmmoInfo = ammo;
		}
		
		InfoSystem::WeaponFireMode GetFireMode() const {
			return m_WeaponInfo.weaponFireMode;
		}
		bool IsOneHanded() const {
			return ( m_nLastUsedMode & InfoSystem::WeaponProperty::IsOneHanded ) != 0;
		}
		bool IsTwoHanded() const {
			return ( m_nLastUsedMode & InfoSystem::WeaponProperty::IsTwoHanded ) != 0;
		}
		bool IsBladed() const {
			return ( m_nLastUsedMode & InfoSystem::WeaponProperty::IsBladed ) != 0;
		}
		bool IsFirearm() const {
			return ( m_nLastUsedMode & InfoSystem::WeaponProperty::IsFirearm ) != 0;
		}
		bool IsBlunt() const {
			return ( m_nLastUsedMode & InfoSystem::WeaponProperty::IsBlunt ) != 0;
		}

		uint GetUseMode() const {
			return m_nLastUsedMode;
		}
		void SetUseMode( uint weaponMode ) {
			m_nLastUsedMode = weaponMode;
			
			if ( ( weaponMode & InfoSystem::WeaponProperty::IsFirearm ) != 0 ) {
				@m_UseState_LEFT = m_WeaponInfo.useState_FireArm_LEFT;
				@m_UseState_RIGHT = m_WeaponInfo.useState_FireArm_RIGHT;
				@m_IdleState_LEFT = m_WeaponInfo.idleState_FireArm_LEFT;
				@m_IdleState_RIGHT = m_WeaponInfo.idleState_FireArm_RIGHT;
			} else if ( ( weaponMode & InfoSystem::WeaponProperty::IsBladed ) != 0 ) {
				@m_UseState_LEFT = m_WeaponInfo.useState_Bladed_LEFT;
				@m_UseState_RIGHT = m_WeaponInfo.useState_Bladed_RIGHT;
				@m_IdleState_LEFT = m_WeaponInfo.idleState_Bladed_LEFT;
				@m_IdleState_RIGHT = m_WeaponInfo.idleState_Bladed_RIGHT;
			} else if ( ( weaponMode & InfoSystem::WeaponProperty::IsBlunt ) != 0 ) {
				@m_UseState_LEFT = m_WeaponInfo.useState_Blunt_LEFT;
				@m_UseState_RIGHT = m_WeaponInfo.useState_Blunt_RIGHT;
				@m_IdleState_LEFT = m_WeaponInfo.idleState_Blunt_LEFT;
				@m_IdleState_RIGHT = m_WeaponInfo.idleState_Blunt_RIGHT;
			}
			switch ( m_Facing ) {
			case FACING_LEFT:
				SetState( @m_IdleState_LEFT );
				break;
			case FACING_RIGHT:
				SetState( @m_IdleState_RIGHT );
				break;
			};
			if ( @m_UseState_LEFT is null || @m_UseState_RIGHT is null || @m_IdleState_LEFT is null || @m_IdleState_RIGHT is null ) {
				GameError( "WeaponObject::SetUseMode: bad weaponMode, states don't exist" );
			}
		}
		
		//
		// AreaOfEffect: does exactly what you think it does
		//
		private float AreaOfEffect( float damage ) {
			TheNomad::Engine::Physics::Bounds bounds;
			EntityObject@ activeEnts = EntityManager.GetActiveEnts();
			EntityObject@ ent = activeEnts.m_Next;
			const float range = m_AmmoInfo.range / 2;
			
			// shit works like DOOM 1993 (box instead of circle)
			// this weapon is a projectile or ordnance with its own entity
			bounds.m_nWidth = range;
			bounds.m_nHeight = range;
			bounds.MakeBounds( m_Link.m_Origin );
			
			for ( ; @ent !is activeEnts; @ent = ent.m_Next ) {
				if ( bounds.IntersectsPoint( ent.GetOrigin() ) ) {
					const float dist = Util::Distance( m_Link.m_Origin, ent.GetOrigin() );
					if ( dist < 1.0f ) { // immediate impact range means death
						EntityManager.KillEntity( ent, m_Owner );
						damage += m_WeaponInfo.damage;
					} else if ( dist <= m_WeaponInfo.range ) {
						EntityManager.DamageEntity( ent, m_Owner, m_WeaponInfo.damage
							+ ( m_AmmoInfo.range + ( dist * 0.1f ) ) );
						damage += m_WeaponInfo.damage;
					}
				}
			}
			// let's be real here, if the player is standing in the middle of an explosion,
			// they should taking damage as well
			return TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" )
				< int( TheNomad::GameSystem::GameDifficulty::Hard ) ? damage : 0.0f;
		}
		
		private float UseBlade( float damage, uint weaponMode ) {
			return 0.0f;
		}
		private float UseBlunt( float damage, uint weaponMode ) {
			const vec3 origin = m_Owner.GetOrigin();
			const float angle = cast<PlayrObject@>( @m_Owner ).GetArmAngle();
			EntityObject@ activeEnts = EntityManager.GetActiveEnts();
			EntityObject@ ent = @activeEnts.m_Next;

			EmitSound( m_WeaponInfo.useSfx_Blunt, 10.0f, 0xff );

			switch ( m_Facing ) {
			case FACING_LEFT:
				SetState( @m_UseState_LEFT );
				break;
			case FACING_RIGHT:
				SetState( @m_UseState_RIGHT );
				break;
			};

			// melee kills should only make a little bit of noise
			m_Owner.SetSoundLevel( m_Owner.GetSoundLevel() + 1.5f );

			vec3 end = origin;
			end.x += m_WeaponInfo.range * cos( angle );
			end.y += m_WeaponInfo.range * sin( angle );
			end.z += m_WeaponInfo.range * sin( angle );

			for ( ; ent !is activeEnts; @ent = ent.m_Next ) {
				if ( ent !is m_Owner && ent.GetBounds().LineIntersection( origin, end ) ) {
					EntityManager.DamageEntity( ent, m_Owner, damage );
				}
			}

			return damage;
		}
		private float UseFireArm( float damage, uint weaponMode ) {
			if ( m_AmmoInfo is null ) {
				EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/weapons/noammo" ), 10.0f, 0xff );
				return 0.0f;
			}

			EmitSound( m_WeaponInfo.useSfx_FireArm, 10.0f, 0xff );

			const vec3 origin = m_Owner.GetOrigin();
			GfxManager.AddMuzzleFlash( origin );

			m_Owner.SetSoundLevel( m_Owner.GetSoundLevel() + 25.0f );

			m_nBulletsUsed += m_WeaponInfo.fireRate;

			TheNomad::GameSystem::RayCast ray;
			ray.m_Start = origin;
			ray.m_nOwner = m_Owner.GetEntityNum();
			ray.m_nOwner2 = m_Link.m_nEntityNumber;
			ray.m_nLength = m_AmmoInfo.range;
			ray.m_nAngle = EntityManager.GetActivePlayer().GetArmAngle();
			ray.Cast();
			
			GfxManager.AddBulletHole( ray.m_Origin );
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID ) {
				PlayrObject@ player = EntityManager.GetActivePlayer();
				
				if ( Util::Distance( player.GetOrigin(), ray.m_Origin ) <= 2.90f ) {
					// if we're close to the bullet, then simulate a near-hit
					player.EmitSound(
						TheNomad::Engine::SoundSystem::RegisterSfx(
							"event:/sfx/env/bullet_impact/ricochet_" + ( Util::PRandom() & 2 )
						),
						10.0f, 0xff
					);
					// TODO: shake screen?
				}
				return 0.0f; // hit nothing
			} else if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				if ( ( ray.m_Origin.y < 0.0f || ray.m_Origin.y >= MapHeight ) || ( ray.m_Origin.x < 0.0f || ray.m_Origin.x >= MapWidth ) ) {
					return 0.0f; // out of bounds
				}
				const uint64 tile = MapTileData[0][ ray.m_Origin.y * MapWidth + ray.m_Origin.x ];
				if ( ( tile & SURFACEPARM_WATER ) != 0 ) {
//					SetSoundPosition( ray.m_Origin );
//					EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/water_land_" + ( Util::PRandom() & 2 ) ),
//						10.0f, 0xff );
					GfxManager.AddWaterWake( ray.m_Origin );
					// TODO: dynamically allocate a sound emitter for a bullet hitting a water surface
//					SetSoundPosition( origin );
				}
				if ( ( tile & SURFACEPARM_METAL ) != 0 ) {
					
				} else {
					const float velocity = ray.m_nLength - Util::Distance( ray.m_Origin, ray.m_Start );
					GfxManager.AddDebrisCloud( ray.m_Origin, velocity );
				}
				return 0.0f;
			}

			EntityManager.DamageEntity( EntityManager.GetEntityForNum( ray.m_nEntityNumber ), m_Owner, m_AmmoInfo.damage );
			
			// health mult doesn't matter on harder difficulties if the player is attacking with a firearm,
			// that is, unless, the player is very close to the enemy
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) < int( TheNomad::GameSystem::GameDifficulty::Hard ) ) {
				return m_AmmoInfo.damage;
			} else {
				if ( Util::Distance( ray.m_Origin, m_Owner.GetOrigin() ) <= 3.75f ) {
					// close enough to the blood
					return m_AmmoInfo.damage;
				}
				return 0.0f;
			}
		}
		
		//
		// WeaponObject::Use: returns the amount of damage dealt
		//
		float Use( const uint weaponMode ) {
			switch ( m_State.GetBaseNum() ) {
			case StateNum::ST_WEAPON_IDLE:
				break;
			case StateNum::ST_WEAPON_USE:
			case StateNum::ST_WEAPON_RELOAD:
			case StateNum::ST_WEAPON_EQUIP:
				return 0.0f;
			};

			SetUseMode( weaponMode );
			float damage = m_WeaponInfo.damage;

			if ( m_Owner.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
				damage += cast<PlayrObject@>( m_Owner ).GetDamageMult();
			}
			
			// TODO: adaptive weapon animation & cooldowns
			
			if ( ( weaponMode & uint( InfoSystem::WeaponProperty::IsOneHanded ) ) != 0
				&& ( weaponMode & uint( InfoSystem::WeaponProperty::IsFirearm ) == 0 ) )
			{
				// half the damage, we're not swinging with full force.
				// And since the weaponMode is just state and not actually the real
				// weapon mode, we can still use a musket like a baseball bat
				
				// bullets hit just as hard even if we're dual-wielding
				damage /= 2;
			}
			
			if ( ( weaponMode & uint( InfoSystem::WeaponProperty::IsFirearm ) ) != 0 ) {
				return UseFireArm( damage, weaponMode );
			} else if ( ( weaponMode & uint( InfoSystem::WeaponProperty::IsBladed ) ) != 0 ) {
				return UseBlade( damage, weaponMode );
			} else if ( ( weaponMode & uint( InfoSystem::WeaponProperty::IsBlunt ) ) != 0 ) {
				return UseBlunt( damage, weaponMode );
			} else {
				// not really an error because it could just be an empty slot, but warn anyway
				DebugPrint( "WARNING: bad weapon state " + weaponMode + ".\n" );
			}
			
			return 0.0f;
		}
		float UseAlt( uint weaponMode ) {
			float damage = 0.0f;
			
			return damage;
		}

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) override {
			if ( section.LoadBool( "hasOwner" ) ) {
				m_Flags = EntityFlags( section.LoadUInt( "flags" ) );
				@m_Owner = EntityManager.GetEntityForNum( section.LoadUInt( "owner" ) );
				
				m_Link.m_Origin = vec3( 0.0f );
			} else {
				m_Link.m_Origin = section.LoadVec3( "origin" );
			}
			
			m_Link.m_nEntityId = section.LoadUInt( "id" );

			Spawn( m_Link.m_nEntityId, m_Link.m_Origin );

			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) {
			section.SaveBool( "hasOwner", m_Owner !is null );
			section.SaveUInt( "id",  m_Link.m_nEntityId );
			if ( m_Owner !is null ) {
				section.SaveUInt( "flags", uint( m_Flags ) );
				section.SaveUInt( "owner", m_Owner.GetEntityNum() );
			} else {
				section.SaveVec3( "origin", m_Link.m_Origin );
			}
		}
		void Think() override {
			if ( m_Owner is null ) {
				m_Bounds.m_nWidth = m_WeaponInfo.size.x;
				m_Bounds.m_nHeight = m_WeaponInfo.size.y;
				m_Bounds.MakeBounds( m_Link.m_Origin );
				return;
			}

			//
			// set the correct sprite direction
			//
			const int facing = m_Owner.GetFacing();
			if ( m_Facing != facing ) {
				EntityState@ newState = null;
				switch ( m_State.GetBaseNum() ) {
				case StateNum::ST_WEAPON_IDLE:
					switch ( facing ) {
					case FACING_LEFT:
						@newState = m_IdleState_LEFT;
						break;
					case FACING_RIGHT:
						@newState = m_IdleState_RIGHT;
						break;
					};
					break;
				case StateNum::ST_WEAPON_USE:
					switch ( facing ) {
					case FACING_LEFT:
						@newState = m_UseState_LEFT;
						break;
					case FACING_RIGHT:
						@newState = m_UseState_RIGHT;
						break;
					};
					break;
				case StateNum::ST_WEAPON_RELOAD:
					switch ( facing ) {
					case FACING_LEFT:
						@newState = m_WeaponInfo.reloadState_LEFT;
						break;
					case FACING_RIGHT:
						@newState = m_WeaponInfo.reloadState_RIGHT;
						break;
					};
					break;
				};

				// don't mess with the ticker
				@m_State = newState;
				m_Facing = facing;
			}
			if ( !m_State.Done( m_nTicker ) ) {
				return; // only this can stop the spam, the bullet hell. OH LORD!
			}
			if ( ( m_nLastUsedMode & InfoSystem::WeaponProperty::IsFirearm ) != 0 ) {
				if ( m_nBulletsUsed >= m_WeaponInfo.magSize ) {
					EmitSound( m_WeaponInfo.reloadSfx, 10.0f, 0xff );
					switch ( facing ) {
					case FACING_LEFT:
						SetState( m_WeaponInfo.reloadState_LEFT );
						break;
					case FACING_RIGHT:
						SetState( m_WeaponInfo.reloadState_RIGHT );
						break;
					};
					m_nBulletsUsed = 0;
					return;
				}
			}
			@m_State = m_State.Run( m_nTicker );
		}
		void Draw() override {
			if ( m_Owner !is null ) {
				return;
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;
			
			refEntity.sheetNum = -1;
			refEntity.spriteId = m_WeaponInfo.hIconShader;
			refEntity.origin = m_Link.m_Origin;
			refEntity.scale = m_WeaponInfo.size;
			refEntity.Draw();
		}
		void Spawn( uint id, const vec3& in origin ) override {
			@m_WeaponInfo = @InfoSystem::InfoManager.GetWeaponInfo( id );
			if ( @m_WeaponInfo is null ) {
				GameError( "WeaponObject::Spawn: invalid weapon id " + id );
			}

			m_Link.m_Origin = origin;
			m_Bounds.m_nWidth = m_WeaponInfo.size.x;
			m_Bounds.m_nHeight = m_WeaponInfo.size.y;
			m_Bounds.MakeBounds( origin );

			@m_SpriteSheet = @m_WeaponInfo.spriteSheet;

			@m_State = @StateManager.GetNullState();
		}
		
		private InfoSystem::AmmoInfo@ m_AmmoInfo = null;
		private InfoSystem::WeaponInfo@ m_WeaponInfo = null;

		// these will change whenever the usage mode changes
		private EntityState@ m_UseState_LEFT = null;
		private EntityState@ m_UseState_RIGHT = null;
		private EntityState@ m_IdleState_LEFT = null;
		private EntityState@ m_IdleState_RIGHT = null;
		
		private uint m_nBulletsUsed = 0;
		private uint m_nLastUsedMode = 0;
	};
};
