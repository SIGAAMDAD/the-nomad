#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/EntitySystem.as"

namespace TheNomad::SGame::InfoSystem {
    class MobInfo : InfoLoader {
		MobInfo() {
		}

		private bool LoadStatsBlock( json@ json ) {
			if ( !json.get( "Stats.Health", health ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.Health' in \"" + name + "\"\n" );
				return false;
			}
			health = float( json[ "Stats.Health" ] );

			if ( !json.get( "Stats.Width", size.x ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.Width' in \"" + name + "\"\n" );
				return false;
			}
			size.x = float( json[ "Stats.Width" ] );

			if ( !json.get( "Stats.Height", size.y ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.Height' in \"" + name + "\"\n" );
				return false;
			}
			size.y = float( json[ "Stats.Height" ] );

			string armor;
			if ( !json.get( "Stats.ArmorType", armor ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.ArmorType' in \"" + name + "\"\n" );
				return false;
			}

			uint armorIndex = 0;
			for ( armorIndex = 0; armorIndex < ArmorTypeStrings.Count(); armorIndex++ ) {
				if ( Util::StrICmp( armor, ArmorTypeStrings[ armorIndex ] ) == 0 ) {
					armorType = ArmorType( armorIndex );
					break;
				}
			}
			if ( armorIndex == ArmorTypeStrings.Count() ) {
				ConsoleWarning( "invalid mob info, ArmorType value '" + armor + "' in \"" + name +  "\" not recognized.\n" );
				return false;
			}

			if ( !json.get( "Stats.Speed.x", speed.x ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.Speed.x' in \"" + name + "\"\n" );
				return false;
			}
			speed.x = float( json[ "Stats.Speed.x" ] );

			if ( !json.get( "Stats.Speed.y", speed.y ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.Speed.y' in \"" + name + "\"\n" );
				return false;
			}
			speed.y = float( json[ "Stats.Speed.y" ] );

			if ( !json.get( "Stats.Speed.z", speed.z ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.Speed.z' in \"" + name + "\"\n" );
				return false;
			}
			speed.z = float( json[ "Stats.Speed.z" ] );

			return true;
		}

		private bool LoadDetectionBlock( json@ json ) {
			if ( !json.get( "Detection.SightRange", sightRange ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Detection.SightRange' in \"" + name + "\"\n" );
				return false;
			}
			sightRadius = float( json[ "Detection.SightRange" ] );

			if ( !json.get( "Detection.SightRadius", sightRadius ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Detection.SightRadius' in \"" + name + "\"\n" );
				return false;
			}
			sightRadius = float( json[ "Detection.SightRadius" ] );

			if ( !json.get( "Detection.SoundRange", soundRange ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Detection.SoundRange' in \"" + name + "\"\n" );
				return false;
			}
			soundRange = float( json[ "Detection.SoundRange" ] );

			if ( !json.get( "Detection.SoundTolerance", soundTolerance ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Detection.SoundTolerance' in \"" + name + "\"\n" );
				return false;
			}
			soundTolerance = float( json[ "Detection.SoundTolerance" ] );

			return true;
		}
		
		bool Load( json@ json ) {
			const EntityData@ entity = null;
			string str;
			
			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Type", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Type'\n" );
				return false;
			}
			if ( ( @entity = @InfoManager.GetMobType( str ) ) !is null ) {
				this.type = entity.GetID();
			} else {
				ConsoleWarning( "invalid mob info, Type \"" + str + "\" wasn't found\n" );
				return false;
			}

			if ( !json.get( "ScriptName", className ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'ScriptName' in \"" + name + "\"\n" );
				return false;
			}
			
			array<json@> flagValues;
			if ( json.get( "Flags", flagValues ) ) {
				// not required, just special modifiers
				DebugPrint( "Processing MobFlags for '" + name + "'...\n" );
				for( uint i = 0; i < flagValues.Count(); ++i ) {
					const string flag = string( flagValues[ i ] );
					if ( flag == "Deaf" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::Deaf ) );
					} else if ( flag == "Blind" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::Blind ) );
					} else if ( flag == "Leader" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::Leader ) );
					} else if ( flag == "Boss" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::Boss ) );
					} else if ( flag == "PermaDead" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::PermaDead ) );
					} else if ( flag == "Sentry" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::Sentry ) );
					} else if ( flag == "Terrified" ) {
						mobFlags = MobFlags( uint( mobFlags ) | uint( MobFlags::Terrified ) );
					} else {
						ConsoleWarning( "unrecognized MobFlag \"" + flag + "\", ignoring...\n" );
					}
				}
			}

			if ( !LoadStatsBlock( @json ) ) {
				return false;
			}
			if ( !LoadDetectionBlock( @json ) ) {
				return false;
			}

			return true;
		}
		
		string name;
		string className;

		vec3 speed = vec3( 1.0f );
		vec2 size = vec2( 1.0f );

		float soundTolerance = 0.0f;
		float soundRange = 0.0f;

		float sightRange = 0.0f;
		float sightRadius = 0.0f;

		float health = 0.0f;
		uint type = 0;

		ArmorType armorType = ArmorType::None;
		MobFlags mobFlags = MobFlags::None;
	};
};