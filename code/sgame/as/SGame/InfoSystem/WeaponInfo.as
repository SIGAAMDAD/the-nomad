#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class WeaponInfo : InfoLoader {
		WeaponInfo() {
		}
		
		bool Load( json@ json ) {
			string type;
			string shader;
			uint i;
			array<json@> props;
			string ammo;
			
			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Id", type ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Id'\n" );
				return false;
			} else {
				if ( !InfoManager.GetWeaponTypes().TryGetValue( type, this.type ) ) {
					GameError( "invalid weapon info, Id \"" + type + "\" wasn't found" );
				}
			}
			if ( !json.get( "MagSize", magSize ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'MagSize'\n" );
				return false;
			}
			if ( !json.get( "Damage", damage ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Damage'\n" );
				return false;
			}
			if ( !json.get( "Range", range ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Range'\n" );
				return false;
			}
			if ( !json.get( "Type", type ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Type'\n" );
				return false;
			}
			if ( !json.get( "WeaponProperties", props ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'WeaponProperties'\n" );
				return false;
			}
			if ( !json.get( "AmmoType", ammo ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'AmmoType'\n" );
				return false;
			}
			if ( !json.get( "Icon", shader ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Icon'\n" );
				return false;
			} else {
				hIconShader = Engine::Renderer::RegisterShader( shader );
			}

			ConsolePrint( "Processing AmmoType for WeaponInfo '" + name + "'...\n" );
			for ( i = 0; i < AmmoTypeStrings.Count(); i++ ) {
				if ( Util::StrICmp( AmmoTypeStrings[i], ammo ) != 1 ) {
					ammoType = AmmoType( i );
					break;
				}
			}
			if ( ammoType == AmmoType::Invalid ) {
				ConsoleWarning( "invalid weapon info, AmmoType isn't valid ( Invalid, abide by physics pls ;) )\n" );
			}
			
			ConsolePrint( "Processing WeaponType for WeaponInfo '" + name + "'...\n" );
			for ( i = 0; i < WeaponTypeStrings.Count(); i++ ) {
				if ( Util::StrICmp( type, WeaponTypeStrings[i] ) != 1 ) {
					weaponType = WeaponType( i );
					break;
				}
			}
			if ( weaponType == WeaponType::NumWeaponTypes ) {
				ConsoleWarning( "invalid weapon info, WeaponType '" + type + "' not recognized\n" );
				return false;
			}
			
			ConsolePrint( "Processing WeaponProperties for WeaponInfo '" + name + "'...\n" );
			for ( i = 0; i < WeaponPropertyStrings.Count(); i++ ) {
				for ( uint a = 0; a < props.Count(); a++ ){
					if ( Util::StrICmp( string( props[a] ), WeaponPropertyStrings[i] ) != 1 ) {
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

		string name;
		uint type = 0;
		float damage = 0.0f;
		float range = 0.0f;
		uint magSize = 0; // maximum shots before cooldown
		AmmoType ammoType = AmmoType::Invalid; // ammo types allowed
		WeaponProperty weaponProps = WeaponProperty::None;
		WeaponType weaponType = WeaponType::NumWeaponTypes;
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;

		int hIconShader = FS_INVALID_HANDLE;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx;
		TheNomad::Engine::SoundSystem::SoundEffect pickupSfx;
	};
};