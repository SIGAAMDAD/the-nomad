#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class WeaponInfo : InfoLoader {
		WeaponInfo() {
		}
		
		private bool LoadRenderDataBlock( json@ json ) {
			uvec2 sheetSize = uvec2( 0 );
			uvec2 spriteSize = uvec2( 0 );
			string npath;

			if ( !json.get( "RenderData.Icon", npath ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'RenderData.Icon' in \"" + name + "\"\n" );
				return false;
			}
			hIconShader = TheNomad::Engine::Renderer::RegisterShader( npath );

			if ( !json.get( "RenderData.SheetWidth", sheetSize.x ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'RenderData.SheetWidth' in \"" + name + "\"\n" );
				return false;
			}
			sheetSize.x = uint( json[ "RenderData.SheetWidth" ] );

			if ( !json.get( "RenderData.SheetHeight", sheetSize.y ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'RenderData.SheetHeight' in \"" + name + "\"\n" );
				return false;
			}
			sheetSize.y = uint( json[ "RenderData.SheetHeight" ] );

			if ( !json.get( "RenderData.SpriteWidth", spriteSize.x ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'RenderData.SpriteWidth' in \"" + name + "\"\n" );
				return false;
			}
			spriteSize.x = uint( json[ "RenderData.SpriteWidth" ] );

			if ( !json.get( "RenderData.SpriteHeight", spriteSize.y ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'RenderData.SpriteHeight' in \"" + name + "\"\n" );
				return false;
			}
			spriteSize.y = uint( json[ "RenderData.SpriteHeight" ] );

			if ( !json.get( "RenderData.SpriteSheet", npath ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'RenderData.SpriteSheet' in \"" + name + "\"\n" );
				return false;
			}

			DebugPrint( "Allocating sprite sheet for weapon \"" + name + "\", [ " + sheetSize.x + ", " + sheetSize.y + " ]:[ " + spriteSize.x + ", "
				+ spriteSize.y + " ]\n" );
			@spriteSheet = @TheNomad::Engine::ResourceCache.GetSpriteSheet( npath, sheetSize.x, sheetSize.y,
				spriteSize.x, spriteSize.y );
			
			return true;
		}
		private bool LoadStatsBlock( json@ json ) {
			if ( !json.get( "Stats.MagSize", magSize ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Stats.MagSize' in \"" + name + "\"\n" );
				return false;
			}
			magSize = uint( json[ "Stats.MagSize" ] );

			if ( !json.get( "Stats.FireRate", fireRate ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Stats.FireRate' in \"" + name + "\"\n" );
				return false;
			}
			fireRate = uint( json[ "Stats.FireRate" ] );

			if ( !json.get( "Stats.Width", size.x ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Stats.Width' in \"" + name + "\"\n" );
				return false;
			}
			size.x = float( json[ "Stats.Width" ] );

			if ( !json.get( "Stats.Height", size.y ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Stats.Height' in \"" + name + "\"\n" );
				return false;
			}
			size.y = float( json[ "Stats.Height" ] );
			
			return true;
		}
		private bool LoadStatesBlock( json@ json ) {
			string state;

			if ( ( uint( weaponProps ) & WeaponProperty::IsFirearm ) != 0 ) {
				if ( !json.get( "States.Idle.FireArm.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Idle.FireArm.Left' in \"" + name + "\"\n"  );
					return false;
				}
				@idleState_FireArm_LEFT = @StateManager.GetStateById( state );
				if ( @idleState_FireArm_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, idle state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Idle.FireArm.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Idle.FireArm.Right' in \"" + name + "\"\n"  );
					return false;
				}
				@idleState_FireArm_RIGHT = @StateManager.GetStateById( state );
				if ( @idleState_FireArm_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, idle state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Use.FireArm.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Use.FireArm.Left' in \"" + name + "\"\n" );
					return false;
				}
				@useState_FireArm_LEFT = @StateManager.GetStateById( state );
				if ( @useState_FireArm_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, use state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Use.FireArm.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Use.FireArm.Right' in \"" + name + "\"\n" );
					return false;
				}
				@useState_FireArm_RIGHT = @StateManager.GetStateById( state );
				if ( @useState_FireArm_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, use state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Reload.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Reload.Left' in \"" + name + "\"\n"  );
					return false;
				}
				@reloadState_LEFT = @StateManager.GetStateById( state );
				if ( @reloadState_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, reload state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Reload.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Reload.Right' in \"" + name + "\"\n"  );
					return false;
				}
				@reloadState_RIGHT = @StateManager.GetStateById( state );
				if ( @reloadState_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, reload state '" + state + "' couldn't be found\n" );
					return false;
				}
			}
			if ( ( uint( weaponProps ) & WeaponProperty::IsBladed ) != 0 ) {
				if ( !json.get( "States.Idle.Bladed.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Idle.Bladed.Left' in \"" + name + "\"\n"  );
					return false;
				}
				@idleState_Bladed_LEFT = @StateManager.GetStateById( state );
				if ( @idleState_Bladed_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, idle state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Idle.Bladed.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Idle.Bladed.Right' in \"" + name + "\"\n"  );
					return false;
				}
				@idleState_Bladed_RIGHT = @StateManager.GetStateById( state );
				if ( @idleState_Bladed_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, idle state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Use.Bladed.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Use.Bladed.Left' in \"" + name + "\"\n" );
					return false;
				}
				@useState_Bladed_LEFT = @StateManager.GetStateById( state );
				if ( @useState_Bladed_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, use state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Use.Bladed.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Use.Bladed.Right' in \"" + name + "\"\n" );
					return false;
				}
				@useState_Bladed_RIGHT = @StateManager.GetStateById( state );
				if ( @useState_Bladed_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, use state '" + state + "' couldn't be found\n" );
					return false;
				}
			}
			if ( ( uint( weaponProps ) & WeaponProperty::IsBlunt ) != 0 ) {
				if ( !json.get( "States.Idle.Blunt.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Idle.Blunt.Left' in \"" + name + "\"\n"  );
					return false;
				}
				@idleState_Blunt_LEFT = @StateManager.GetStateById( state );
				if ( @idleState_Blunt_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, idle state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Idle.Blunt.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Idle.Blunt.Right' in \"" + name + "\"\n"  );
					return false;
				}
				@idleState_Blunt_RIGHT = @StateManager.GetStateById( state );
				if ( @idleState_Blunt_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, idle state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Use.Blunt.Left", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Use.Blunt.Left' in \"" + name + "\"\n" );
					return false;
				}
				@useState_Blunt_LEFT = @StateManager.GetStateById( state );
				if ( @useState_Blunt_LEFT is null ) {
					ConsoleWarning( "invalid weapon info, use state '" + state + "' couldn't be found\n" );
					return false;
				}

				if ( !json.get( "States.Use.Blunt.Right", state ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'States.Use.Blunt.Right' in \"" + name + "\"\n" );
					return false;
				}
				@useState_Blunt_RIGHT = @StateManager.GetStateById( state );
				if ( @useState_Blunt_RIGHT is null ) {
					ConsoleWarning( "invalid weapon info, use state '" + state + "' couldn't be found\n" );
					return false;
				}
			}

			if ( !json.get( "States.Equip.Left", state ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'States.Equip.Left' in \"" + name + "\"\n" );
				return false;
			}
			@equipState_LEFT = @StateManager.GetStateById( state );
			if ( @equipState_LEFT is null ) {
				ConsoleWarning( "invalid weapon info, equip state '" + state + "' couldn't be found\n" );
				return false;
			}

			if ( !json.get( "States.Equip.Right", state ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'States.Equip.Right' in \"" + name + "\"\n" );
				return false;
			}
			@equipState_RIGHT = @StateManager.GetStateById( state );
			if ( @equipState_RIGHT is null ) {
				ConsoleWarning( "invalid weapon info, equip state '" + state + "' couldn't be found\n" );
				return false;
			}

			return true;
		}
		private bool LoadSoundsBlock( json@ json ) {
			string sfx;

			if ( ( uint( weaponProps ) & WeaponProperty::IsFirearm ) != 0 ) {
				if ( !json.get( "Sounds.Reload", sfx ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'Sounds.Reload' in \"" + name + "\"\n" );
					return false;
				}
				reloadSfx = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );
			}

			if ( ( uint( weaponProps ) & WeaponProperty::IsFirearm ) != 0 ) {
				if ( !json.get( "Sounds.Use.FireArm", sfx ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'Sounds.Use.FireArm' in \"" + name + "\"\n" );
					return false;
				}
				useSfx_FireArm = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );
			}
			if ( ( uint( weaponProps ) & WeaponProperty::IsBlunt ) != 0 ) {
				DebugPrint( "Loading blunt usage sound effects...\n" );
				if ( !json.get( "Sounds.Use.Blunt", sfx ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'Sounds.Use.Blunt' in \"" + name + "\"\n" );
					return false;
				}
				useSfx_Blunt = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );
			}
			if ( ( uint( weaponProps ) & WeaponProperty::IsBladed ) != 0 ) {
				if ( !json.get( "Sounds.Use.Bladed", sfx ) ) {
					ConsoleWarning( "invalid weapon info, missing variable 'Sounds.Use.Bladed' in \"" + name + "\"\n" );
					return false;
				}
				useSfx_Bladed = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );
			}
			
			return true;
		}
		private bool LoadWeaponProperty( json@ json ) {
			uint props = 0;
			if ( !json.get( "Properties", props ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Properties' in \"" + name + "\"\n" );
				return false;
			}
			weaponProps = WeaponProperty( uint( json[ "Properties" ] ) );
			if ( weaponProps == WeaponProperty::None ) {
				ConsoleWarning( "invalid weapon info, WeaponProperty '" + uint( weaponProps ) + "' are invalid ( None, abide by physics pls ;) )\n" );
				return false;
			}

			if ( !json.get( "DefaultMode", props ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'DefaultMode' in \"" + name + "\"\n" );
				return false;
			}
			defaultMode = WeaponProperty( uint( json[ "DefaultMode" ] ) );
			if ( defaultMode == WeaponProperty::None ) {
				ConsoleWarning( "invalid weapon info, WeaponProperty '" + uint( defaultMode ) + "' are invalid ( None, abide by physics pls ;) )\n" );
				return false;
			}
			return true;
		}
		private bool LoadWeaponFireModes( json@ json ) {
			uint fireMode = 0;
			if ( !json.get( "FireMode", fireMode ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'FireMode' in \"" + name + "\"\n" );
				return false;
			}
			weaponFireMode = WeaponFireMode( uint( json[ "FireMode" ] ) );
			if ( weaponFireMode == WeaponFireMode::NumFireModes ) {
				ConsoleWarning( "invalid weapon info, WeaponFireMode '" + uint( weaponFireMode )
					+ "' are invalid ( None, please make a real weapon sir ;) \n" );
				return false;
			}
			return true;
		}
		
		bool Load( json@ json ) {
			string id;
			string typeStr;

			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Id", id ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Id' in \"" + name + "\"\n" );
				return false;
			}
			if ( !json.get( "Type", typeStr ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Type' in \"" + name + "\"\n" );
				return false;
			}
			
			const EntityData@ entityType = @InfoManager.GetWeaponType( id );
			if ( @entityType is null ) {
				GameError( "invalid weapon info, Type \"" + id + "\" wasn't found" );
			}
			type = entityType.GetID();

			const string ammo = string( json[ "AmmoType" ] );
			
			DebugPrint( "Processing AmmoType for WeaponInfo '" + id + "'...\n" );
			for ( uint i = 0; i < AmmoTypeStrings.Count(); i++ ) {
				if ( Util::StrICmp( AmmoTypeStrings[i], ammo ) == 0 ) {
					ammoType = AmmoType( i );
					DebugPrint( "weapon \"" + name + "\" using ammo baseType \"" + AmmoTypeStrings[i] + "\"\n" );
					break;
				}
			}
			if ( ammoType == AmmoType::Invalid ) {
				ConsoleWarning( "invalid weapon info, AmmoType isn't valid ( Invalid, abide by physics pls ;) )\n" );
			}
			
			DebugPrint( "Processing WeaponType for WeaponInfo '" + id + "'...\n" );
			for ( uint i = 0; i < WeaponTypeStrings.Count(); i++ ) {
				if ( Util::StrICmp( typeStr, WeaponTypeStrings[i] ) == 0 ) {
					weaponType = WeaponType( i );
					break;
				}
			}
			if ( weaponType == WeaponType::NumWeaponTypes ) {
				ConsoleWarning( "invalid weapon info, WeaponType '" + typeStr + "' not recognized\n" );
				return false;
			}

			if ( !LoadWeaponProperty( @json ) ) {
				return false;
			}
			if ( !LoadSoundsBlock( @json ) ) {
				return false;
			}
			if ( !LoadStatesBlock( @json ) ) {
				return false;
			}
			if ( !LoadRenderDataBlock( @json ) ) {
				return false;
			}
			if ( !LoadStatsBlock( @json ) ) {
				return false;
			}
			if ( !LoadWeaponFireModes( @json ) ) {
				return false;
			}
			return true;
		}

		EntityState@ reloadState_LEFT = null;
		EntityState@ reloadState_RIGHT = null;

		EntityState@ useState_FireArm_LEFT = null;
		EntityState@ useState_FireArm_RIGHT = null;

		EntityState@ idleState_FireArm_LEFT = null;
		EntityState@ idleState_FireArm_RIGHT = null;

		EntityState@ useState_Blunt_LEFT = null;
		EntityState@ useState_Blunt_RIGHT = null;

		EntityState@ idleState_Blunt_LEFT = null;
		EntityState@ idleState_Blunt_RIGHT = null;

		EntityState@ useState_Bladed_LEFT = null;
		EntityState@ useState_Bladed_RIGHT = null;

		EntityState@ idleState_Bladed_LEFT = null;
		EntityState@ idleState_Bladed_RIGHT = null;

		EntityState@ equipState_LEFT = null;
		EntityState@ equipState_RIGHT = null;

		SpriteSheet@ spriteSheet = null;

		string name;
		vec2 size = vec2( 0.0f );
		uint type = 0;
		
		uint magSize = 0; // maximum shots before cooldown/reload
		uint fireRate = 0;
		float range = 0.0f; // unused if this is a firearm
		float damage = 0.0f; // unused if this is a firearm

		TheNomad::Engine::SoundSystem::SoundEffect equipSfx;

		TheNomad::Engine::SoundSystem::SoundEffect useSfx_FireArm;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx_Blunt;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx_Bladed;

		TheNomad::Engine::SoundSystem::SoundEffect reloadSfx;

		int hIconShader = FS_INVALID_HANDLE;
		AmmoType ammoType = AmmoType::Invalid; // ammo types allowed
		WeaponProperty weaponProps = WeaponProperty::None;
		WeaponProperty defaultMode = WeaponProperty::None;
		WeaponType weaponType = WeaponType::NumWeaponTypes;
		WeaponFireMode weaponFireMode = WeaponFireMode::NumFireModes;
	};
};
