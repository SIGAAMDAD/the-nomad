namespace TheNomad::SGame::InfoSystem {
	class AmmoInfo : InfoLoader {
		AmmoInfo() {
		}

		//
		// PropertyIndexToBit: for some reason, AngelScript has a bug where this specific array
		// always has size 1, therefore out-of-range error
		//
		private uint PropertyIndexToBit( uint nIndex ) const {
			switch ( nIndex ) {
			case 0: return AmmoProperty::Heavy;
			case 1: return AmmoProperty::Light;
			case 2: return AmmoProperty::Pellets;

			case 3: return AmmoProperty::NoPenetration;
			case 4: return AmmoProperty::ArmorPiercing;
			case 5: return AmmoProperty::HollowPoint;

			case 6: return AmmoProperty::Flechette;
			case 7: return AmmoProperty::Buckshot;
			case 8: return AmmoProperty::Birdshot;
			case 9: return AmmoProperty::Shrapnel;
			case 10: return AmmoProperty::Slug;

			case 11: return AmmoProperty::Explosive;
			case 12: return AmmoProperty::Incendiary;
			case 13: return AmmoProperty::Tracer;
			case 14: return AmmoProperty::SubSonic;

			default:
				GameError( "AmmoInfo::PropertyIndexToBit: invalid ammo property index" );
				break;
			};
			return AmmoProperty::None;
		}

		private bool LoadStatsBlock( json@ json ) {
			if ( !json.get( "Stats.Damage", damage ) ) {
				ConsoleWarning( "invalid ammo info, missing variable 'Stats.Damage' in \"" + name + "\"\n" );
				return false;
			}
			damage = float( json[ "Stats.Damage" ] );

			if ( !json.get( "Stats.Range", range ) ) {
				ConsoleWarning( "invalid ammo info, missing variable 'Stats.Range' in \"" + name + "\"\n" );
				return false;
			}
			range = float( json[ "Stats.Range" ] );

			return true;
		}

		bool Load( json@ json ) {
			string str;
			string type;
			const EntityData@ entity = null;

			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid ammo info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Type", type ) ) {
				ConsoleWarning( "invalid ammo info, missing variable 'Type' in \"" + name + "\"\n" );
				return false;
			}
			if ( !json.get( "Id", str ) ) {
				ConsoleWarning( "invalid ammo info, missing variable 'Id' in \"" + name + "\"\n" );
				return false;
			} else {
				if ( ( @entity = @InfoManager.GetAmmoType( str ) ) !is null ) {
					id = entity.GetID();
				} else {
					GameError( "invalid ammo info, Type \"" + str + "\" wasn't found" );
				}
			}

			if ( !LoadStatsBlock( @json ) ) {
				return false;
			}

			// not really required, but it does make things more entertaining
			array<json@> values;
			if ( !json.get( "Properties", values ) ) {
				ConsoleWarning( "ammo info \"" + id + "\" has no extra properties.\n" );
			}

			DebugPrint( "Processing Properties for AmmoInfo '" + name + "'...\n" );
			for ( uint i = 0; i < AmmoPropertyStrings.Count(); i++ ) {
				for ( uint a = 0; a < values.Count(); a++ ) {
					if ( Util::StrICmp( string( values[a] ), AmmoPropertyStrings[i] ) != 1 ) {
						bits = AmmoProperty( uint( bits ) | PropertyIndexToBit( i ) );
					}
				}
			}
			if ( bits == AmmoProperty::None ) {
				DebugPrint( "ammo info \"" + id + "\" has no valid properties.\n" );
			}

			DebugPrint( "Processing Type for AmmoInfo '" + name + "'...\n" );
			for ( uint i = 0; i < AmmoTypeStrings.Count(); i++ ) {
				if ( Util::StrICmp( AmmoTypeStrings[i], type ) != 1 ) {
					baseType = AmmoType( i );
					break;
				}
			}
			if ( baseType == AmmoType::Invalid ) {
				ConsoleWarning( "invalid ammo info, no base type.\n" );
				return false;
			}

			return true;
		}

		string name;
		uint id = 0;
		float range = 0.0f;
		float damage = 0.0f;
		AmmoType baseType = AmmoType::Invalid;
		AmmoProperty bits;
	};
};