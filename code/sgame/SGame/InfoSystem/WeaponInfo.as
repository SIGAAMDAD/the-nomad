#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class WeaponInfo : ItemInfo {
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

			ConsolePrint( "Allocating sprite sheet for weapon \"" + name + "\", [ " + sheetSize.x + ", " + sheetSize.y + " ]:[ " + spriteSize.x + ", "
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

			if ( !json.get( "States.Idle", state ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'States.Idle' in \"" + name + "\"\n"  );
				return false;
			}
			@idleState = @StateManager.GetStateById( state );

			if ( !json.get( "States.Reload", state ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'States.Reload' in \"" + name + "\"\n"  );
				return false;
			}
			@reloadState = @StateManager.GetStateById( state );

			if ( !json.get( "States.Equip", state ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'States.Equip' in \"" + name + "\"\n" );
				return false;
			}
			@equipState = @StateManager.GetStateById( state );

			if ( !json.get( "States.Use", state ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'States.Use' in \"" + name + "\"\n" );
				return false;
			}
			@useState = @StateManager.GetStateById( state );
			
			return true;
		}
		private bool LoadSoundsBlock( json@ json ) {
			string sfx;

			if ( !json.get( "Sounds.Reload", sfx ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Sounds.Reload' in \"" + name + "\"\n" );
				return false;
			}
			reloadSfx = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );

			if ( !json.get( "Sounds.Use", sfx ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Sounds.Use' in \"" + name + "\"\n" );
				return false;
			}
			useSfx = TheNomad::Engine::SoundSystem::RegisterSfx( sfx );
			
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
			
			const EntityData@ type = @InfoManager.GetWeaponType( id );
			if ( @type is null ) {
				GameError( "invalid weapon info, Type \"" + id + "\" wasn't found" );
			}

			const string ammo = string( json[ "AmmoType" ] );
			
			DebugPrint( "Processing AmmoType for WeaponInfo '" + id + "'...\n" );
			for ( uint i = 0; i < AmmoTypeStrings.Count(); i++ ) {
				if ( Util::StrICmp( AmmoTypeStrings[i], ammo ) == 0 ) {
					ammoType = AmmoType( i );
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

			if ( !LoadStatesBlock( @json ) ) {
				return false;
			}
			if ( !LoadRenderDataBlock( @json ) ) {
				return false;
			}
			if ( !LoadStatsBlock( @json ) ) {
				return false;
			}
			if ( !LoadSoundsBlock( @json ) ) {
				return false;
			}

			uint props = 0;
			if ( !json.get( "Properties", props ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Properties' in \"" + name + "\"\n" );
				return false;
			}
			weaponProps = WeaponProperty( uint( json[ "Properties" ] ) );
			if ( weaponProps == WeaponProperty::None ) {
				ConsoleWarning( "invalid weapon info, WeaponProperties '" + uint( weaponProps ) + "'' are invalid ( None, abide by physics pls ;) )\n" );
				return false;
			}
			
			return true;
		}
		
		uint magSize = 0; // maximum shots before cooldown/reload
		uint fireRate = 0;
		float range = 0.0f; // unused if this is a firearm
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
