#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class WeaponInfo : ItemInfo {
		WeaponInfo() {
		}
		
		private bool LoadRenderDataBlock( json@ json ) {
			uvec2 sheetSize = uvec2( 0 );
			uvec2 spriteSize = uvec2( 0 );
			
			hIconShader = TheNomad::Engine::Renderer::RegisterShader( string( json[ "Icon" ] ) );
			sheetSize.x = uint( json[ "SheetWidth" ] );
			sheetSize.y = uint( json[ "SheetHeight" ] );
			spriteSize.x = uint( json[ "SpriteWidth" ] );
			spriteSize.y = uint( json[ "SpriteHeight" ] );
			@spriteSheet = @TheNomad::Engine::ResourceCache.GetSpriteSheet( string( json[ "SpriteSheet" ] ), sheetSize.x, sheetSize.y,
				spriteSize.x, spriteSize.y );
			
			return true;
		}
		private bool LoadStatsBlock( json@ json ) {
			magSize = uint( json[ "MagSize" ] );
			fireRate = uint( json[ "FireRate" ] );
			width = float( json[ "Width" ] );
			height = float( json[ "Height" ] );
			
			return true;
		}
		private bool LoadStatesBlock( json@ json ) {
			@idleState = @StateManager.GetStateById( string( json[ "Idle" ] ) );
			@reloadState = @StateManager.GetStateById( string( json[ "Reload" ] ) );
			@equipState = @StateManager.GetStateById( string( json[ "Equip" ] ) );
			@useState = @StateManager.GetStateById( string( json[ "Use" ] ) );
			
			return true;
		}
		private bool LoadSoundsBlock( json@ json ) {
			reloadSfx = TheNomad::Engine::SoundSystem::RegisterSfx( string( json[ "Reload" ] ) );
			useSfx = TheNomad::Engine::SoundSystem::RegisterSfx( string( json[ "Use" ] ) );
			
			return true;
		}
		
		bool Load( json@ json ) {
			array<json@> props;
			
			name = string( json[ "Name" ] );
			const string id = string( json[ "Id" ] );
			
			const EntityData@ type = @InfoManager.GetWeaponType( id );
			if ( @type is null ) {
				GameError( "invalid weapon info, Type \"" + id + "\" wasn't found" );
			}

			LoadStatesBlock( @json[ "States" ] );
			LoadSoundsBlock( @json[ "Sounds" ] );
			LoadRenderDataBlock( @json[ "RenderData" ] );
			LoadStatsBlock( @json[ "Stats" ] );

			if ( !json.get( "WeaponProperties", props ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'WeaponProperties' in \"" + name + "\"\n" );
				return false;
			}
			const string ammo = string( json[ "AmmoType" ] );
			
			DebugPrint( "Processing AmmoType for WeaponInfo '" + name + "'...\n" );
			for ( uint i = 0; i < AmmoTypeStrings.Count(); i++ ) {
				if ( AmmoTypeStrings[i] == ammo ) {
					ammoType = AmmoType( i );
					break;
				}
			}
			if ( ammoType == AmmoType::Invalid ) {
				ConsoleWarning( "invalid weapon info, AmmoType isn't valid ( Invalid, abide by physics pls ;) )\n" );
			}
			
			DebugPrint( "Processing WeaponType for WeaponInfo '" + name + "'...\n" );
			for ( uint i = 0; i < WeaponTypeStrings.Count(); i++ ) {
				if ( type == WeaponTypeStrings[i] ) {
					weaponType = WeaponType( i );
					break;
				}
			}
			if ( weaponType == WeaponType::NumWeaponTypes ) {
				ConsoleWarning( "invalid weapon info, WeaponType '" + name + "' not recognized\n" );
				return false;
			}
			
			DebugPrint( "Processing WeaponProperties for WeaponInfo '" + name + "'...\n" );
			for ( uint i = 0; i < WeaponPropertyStrings.Count(); i++ ) {
				for ( uint a = 0; a < props.Count(); a++ ){
					if ( string( props[a] ) == WeaponPropertyStrings[i] ) {
						weaponProps = WeaponProperty( uint( weaponProps ) | WeaponPropertyBits[i] );
					}
				}
			}
			if ( weaponProps == WeaponProperty::None ) {
				ConsoleWarning( "invalid weapon info, WeaponProperties are invalid ( None, abide by physics pls ;) )\n" );
				return false;
			}
			
			return true;
		}
		
		uint magSize = 0; // maximum shots before cooldown/reload
		uint fireRate = 0;
		vec2 size = vec2( 0.0f );
		float range = 0.0f; // unused if this is a firearm
		float weight = 0.0f;
		float damage = 0.0f; // unused if this is a firearm
		TheNomad::Engine::SoundSystem::SoundEffect reloadSfx;
		EntityState@ reloadState = null;
		EntityState@ useState = null;
		EntityState@ idleState = null;
		EntityState@ equipState = null;
		SpriteSheet@ spriteSheet = null;
		int hIconShader = FS_INVALID_HANDLE;
		AmmoType ammoType = AmmoType::Invalid; // ammo types allowed
		WeaponProperty weaponProps = WeaponProperty::None;
		WeaponType weaponType = WeaponType::NumWeaponTypes;
	};
};
