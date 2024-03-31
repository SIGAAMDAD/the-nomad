#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class WeaponInfo : InfoLoader {
		WeaponInfo() {
		}
		
		bool Load( json@ json ) {
			string ammo;
			string type;
			string shader;
			uint i;
			array<json@> props;
			
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
			if ( !json.get( "MagMaxStack", magMaxStack ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'MagMaxStack'\n" );
				return false;
			}
			if ( !json.get( "WeaponType", type ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'WeaponType'\n" );
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
			if ( !json.get( "MagSize", magSize ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'MagSize'\n" );
				return false;
			}
			if ( !json.get( "MagMaxStack", magMaxStack ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'MagMaxStack'\n" );
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
			if ( !json.get( "Shader", shader ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Shader'\n" );
				return false;
			} else {
				hShader = TheNomad::Engine::Renderer::RegisterShader( shader );
			}

			for ( i = 0; i < WeaponTypeStrings.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( type, WeaponTypeStrings[i] ) != 1 ) {
					weaponType = WeaponType( i );
					break;
				}
			}
			if ( weaponType == WeaponType::NumWeaponTypes ) {
				ConsoleWarning( "invalid weapon info, WeaponType '" + type + "' not recognized\n" );
				return false;
			}
			
			for ( i = 0; i < WeaponPropertyStrings.Count(); i++ ) {
				for ( uint a = 0; a < props.Count(); a++ ){
					if ( TheNomad::Util::StrICmp( string( props[a] ), WeaponPropertyStrings[i] ) != 1 ) {
						weaponProps = WeaponProperty( uint( weaponProps ) | WeaponPropertyBits[i] );
					}
				}
			}
			if ( weaponProps == WeaponProperty::None ) {
				ConsoleWarning( "invalid weapon info, WeaponProperties are invalid (None, abide by physics pls)\n" );
				return false;
			}
			
			return true;
		}

		string name;
		uint type = 0;
		int magSize = 0;
		int magMaxStack = 0;
		AmmoType ammoType = AmmoType::Invalid;
		float damage = 0.0f;
		float range = 0.0f;
		WeaponProperty weaponProps = WeaponProperty::None;
		WeaponType weaponType = WeaponType::NumWeaponTypes;
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;
		int hShader = FS_INVALID_HANDLE;
	};
};