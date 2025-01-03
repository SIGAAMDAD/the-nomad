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

			m_Facing = ent.GetFacing();
			switch ( m_Facing ) {
			case FACING_LEFT:
				SetState( @m_WeaponInfo.idleState_LEFT );
				break;
			case FACING_RIGHT:
				SetState( @m_WeaponInfo.idleState_RIGHT );
				break;
			};
			@m_Owner = @ent;

			m_Bounds.Clear();

			DebugPrint( "Weapon " + m_Link.m_nEntityNumber + " now owned by " + ent.GetEntityNum() + ".\n" );
		}

		const InfoSystem::WeaponInfo@ GetWeaponInfo() const {
			return @m_WeaponInfo;
		}
		InfoSystem::WeaponInfo@ GetWeaponInfo() {
			return @m_WeaponInfo;
		}
		InfoSystem::ItemInfo@ GetItemInfo() {
			return @m_WeaponInfo;
		}
		const InfoSystem::ItemInfo@ GetItemInfo() const {
			return @m_WeaponInfo;
		}
		void SetAmmo( InfoSystem::AmmoInfo@ ammo ) {
			@m_AmmoInfo = @ammo;
		}
		
		InfoSystem::WeaponFireMode GetFireMode() const {
			return m_WeaponInfo.weaponFireMode;
		}
		bool IsOneHanded() const {
			return ( m_WeaponInfo.weaponProps & InfoSystem::WeaponProperty::IsOneHanded ) != 0;
		}
		bool IsTwoHanded() const {
			return ( m_WeaponInfo.weaponProps & InfoSystem::WeaponProperty::IsTwoHanded ) != 0;
		}
		bool IsBladed() const {
			return ( m_WeaponInfo.weaponProps & InfoSystem::WeaponProperty::IsBladed ) != 0;
		}
		bool IsPolearm() const {
			return ( m_WeaponInfo.weaponProps & InfoSystem::WeaponProperty::IsPolearm ) != 0;
		}
		bool IsFirearm() const {
			return ( m_WeaponInfo.weaponProps & InfoSystem::WeaponProperty::IsFirearm ) != 0;
		}
		bool IsBlunt() const {
			return ( m_WeaponInfo.weaponProps & InfoSystem::WeaponProperty::IsBlunt ) != 0;
		}
		
		//
		// AreaOfEffect: does exactly what you think it does
		//
		private float AreaOfEffect( float damage ) {
			TheNomad::Engine::Physics::Bounds bounds;
			EntityObject@ activeEnts = @EntityManager.GetActiveEnts();
			EntityObject@ ent = @activeEnts.m_Next;
			const float range = m_AmmoInfo.range / 2;
			
			// shit works like DOOM 1993 (box instead of circle)
			// this weapon is a projectile or ordnance with its own entity
			bounds.m_nWidth = range;
			bounds.m_nHeight = range;
			bounds.MakeBounds( m_Link.m_Origin );
			
			for ( ; @ent !is @activeEnts; @ent = @ent.m_Next ) {
				if ( bounds.IntersectsPoint( ent.GetOrigin() ) ) {
					const float dist = Util::Distance( m_Link.m_Origin, ent.GetOrigin() );
					if ( dist < 1.0f ) { // immediate impact range means death
						EntityManager.KillEntity( @ent, @m_Owner );
						damage += m_WeaponInfo.damage;
					} else if ( dist <= m_WeaponInfo.range ) {
						EntityManager.DamageEntity( @ent, @m_Owner, m_WeaponInfo.damage
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
			const float angle = m_Owner.GetAngle();
			TheNomad::GameSystem::BBox bounds;
			
			return damage;
		}
		private float UsePolearm( float damage, uint weaponMode ) {
			const vec3 origin = m_Owner.GetOrigin();
			const float angle = m_Owner.GetAngle();
			vec3 end = vec3( 0.0f );
			EntityObject@ active = @EntityManager.GetActiveEnts();
			EntityObject@ it = null;
			
			end.x = origin.x + ( m_WeaponInfo.range * cos( angle ) );
			end.y = origin.y + ( m_WeaponInfo.range * sin( angle ) );
//			end.z = m_WeaponInfo.range * sin( angle );
			
			for ( @it = @active.m_Next; @it.m_Next !is @active; @it = @it.m_Next ) {
				if ( it.GetBounds().LineIntersection( origin, end ) ) {
					EmitSound( m_WeaponInfo.useSfx, 10.0f, 0xff );
					// pike, bannerlord mode (you crouch to get a spear brace), high-risk, high-reward
					if ( cast<PlayrObject@>( @m_Owner ).IsCrouching() || cast<PlayrObject@>( @m_Owner ).IsSliding() ) {
						return 1000.0f; // it's an insta-kill for most mobs
					} else {
						return m_WeaponInfo.damage;
					}
				}
			}
			return 0.0f;
		}
		private float UseFireArm( float damage, uint weaponMode ) {
			if ( @m_AmmoInfo is null ) {
				EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/weapons/noammo" ), 10.0f, 0xff );
				return 0.0f;
			}

			EmitSound( m_WeaponInfo.useSfx, 10.0f, 0xff );

			const vec3 origin = m_Owner.GetOrigin();
			GfxManager.AddMuzzleFlash( origin );

			m_nBulletsUsed += m_WeaponInfo.fireRate;

			TheNomad::GameSystem::RayCast ray;
			ray.m_Start = origin;
			ray.m_nOwner = m_Owner.GetEntityNum();
			ray.m_nOwner2 = m_Link.m_nEntityNumber;
			ray.m_nLength = m_AmmoInfo.range;
			ray.m_nAngle = EntityManager.GetActivePlayer().GetArmAngle();
			ray.Cast();
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID ) {
				PlayrObject@ player = @EntityManager.GetActivePlayer();
				
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
				GfxManager.AddBulletHole( ray.m_Origin );
				return 0.0f;
			}
			
			EntityManager.DamageEntity( @EntityManager.GetEntityForNum( ray.m_nEntityNumber ), @m_Owner );
			
			// health mult doesn't matter on harder difficulties if the player is attacking with a firearm,
			// that is, unless, the player is very close to the enemy
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) < int( TheNomad::GameSystem::GameDifficulty::Hard ) ) {
				return damage;
			} else {
				if ( Util::Distance( ray.m_Origin, m_Owner.GetOrigin() ) <= 2.75f ) {
					// close enough to the blood
					return damage;
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
			
			switch ( m_Owner.GetFacing() ) {
			case FACING_LEFT:
				SetState( @m_WeaponInfo.useState_LEFT );
				break;
			case FACING_RIGHT:
				SetState( @m_WeaponInfo.useState_RIGHT );
				break;
			};
			float damage = m_WeaponInfo.damage;
			
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
			} else if ( ( weaponMode & uint( InfoSystem::WeaponProperty::IsPolearm ) ) != 0 ) {
				return UsePolearm( damage, weaponMode );
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

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
			if ( section.LoadBool( "hasOwner" ) ) {
				m_Flags = EntityFlags( section.LoadUInt( "flags" ) );
				@m_Owner = @EntityManager.GetEntityForNum( section.LoadUInt( "owner" ) );
				
				m_Link.m_Origin = vec3( 0.0f );
			} else {
				m_Link.m_Origin = section.LoadVec3( "origin" );
			}
			
			m_Link.m_nEntityId = section.LoadUInt( "id" );

			Spawn( m_Link.m_nEntityId, m_Link.m_Origin );

			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) {
			section.SaveBool( "hasOwner", @m_Owner !is null );
			section.SaveUInt( "id",  m_Link.m_nEntityId );
			if ( @m_Owner !is null ) {
				section.SaveUInt( "flags", uint( m_Flags ) );
				section.SaveUInt( "owner", m_Owner.GetEntityNum() );
			} else {
				section.SaveVec3( "origin", m_Link.m_Origin );
			}
		}
		void Think() override {
			if ( @m_Owner is null ) {
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
						@newState = @m_WeaponInfo.idleState_LEFT;
						break;
					case FACING_RIGHT:
						@newState = @m_WeaponInfo.idleState_RIGHT;
						break;
					};
					break;
				case StateNum::ST_WEAPON_USE:
					switch ( facing ) {
					case FACING_LEFT:
						@newState = @m_WeaponInfo.useState_LEFT;
						break;
					case FACING_RIGHT:
						@newState = @m_WeaponInfo.useState_RIGHT;
						break;
					};
					break;
				case StateNum::ST_WEAPON_RELOAD:
					switch ( facing ) {
					case FACING_LEFT:
						@newState = @m_WeaponInfo.reloadState_LEFT;
						break;
					case FACING_RIGHT:
						@newState = @m_WeaponInfo.reloadState_RIGHT;
						break;
					};
					break;
				};

				// don't mess with the ticker
				@m_State = @newState;
			}
			m_Facing = facing;
			if ( !m_State.Done( m_nTicker ) ) {
				return; // only this can stop the spam, the bullet hell. OH LORD!
			}
			if ( m_nBulletsUsed >= m_WeaponInfo.magSize ) {
				EmitSound( m_WeaponInfo.reloadSfx, 10.0f, 0xff );
				switch ( facing ) {
				case FACING_LEFT:
					SetState( @m_WeaponInfo.reloadState_LEFT );
					break;
				case FACING_RIGHT:
					SetState( @m_WeaponInfo.reloadState_RIGHT );
					break;
				};
				m_nBulletsUsed = 0;
				return;
			}
			@m_State = @m_State.Run( m_nTicker );
		}
		void Draw() override {
			for ( uint i = 0; i < hit.Count(); ++i ) {
				TheNomad::Engine::Renderer::RenderEntity refEntity;
				refEntity.origin = hit[i];
				refEntity.sheetNum = -1;
				refEntity.spriteId = TheNomad::Engine::Renderer::RegisterShader( "world/sprites/crate" );
				refEntity.Draw();
			}
			if ( @m_Owner !is null ) {
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

			@m_Info = @InfoSystem::InfoManager.GetItemInfo( InfoSystem::InfoManager.GetItemType( "item_weapon_pickup" ).GetID() );
			if ( @m_Info is null ) {
				GameError( "WeaponObject::Spawn: couldn't get item info for \"item_weapon_pickup\"" );
			}

			m_Link.m_Origin = origin;
			m_Bounds.m_nWidth = m_WeaponInfo.size.x;
			m_Bounds.m_nHeight = m_WeaponInfo.size.y;
			m_Bounds.MakeBounds( origin );

			@m_SpriteSheet = @m_WeaponInfo.spriteSheet;

			@m_State = @StateManager.GetNullState();

			itemlib::AllocScript( @this );
		}
		

		array<vec3> hit;
		private InfoSystem::AmmoInfo@ m_AmmoInfo = null;
		private InfoSystem::WeaponInfo@ m_WeaponInfo = null;

		private EntityState@ m_OppositeState = null;
		
		private uint m_nBulletsUsed = 0;
	};
};
