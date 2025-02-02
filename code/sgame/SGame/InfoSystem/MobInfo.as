#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/EntitySystem.as"

namespace TheNomad::SGame::InfoSystem {
    class MobInfo : InfoLoader {
		MobInfo() {
		}

		private bool LoadRenderDataBlock( json@ json ) {
			uvec2 sheetSize = uvec2( 0 );
			uvec2 spriteSize = uvec2( 0 );
			string npath;

			if ( !json.get( "RenderData.SheetWidth", sheetSize.x ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'RenderData.SheetWidth' in \"" + name + "\"\n" );
				return false;
			}
			sheetSize.x = uint( json[ "RenderData.SheetWidth" ] );

			if ( !json.get( "RenderData.SheetHeight", sheetSize.y ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'RenderData.SheetHeight' in \"" + name + "\"\n" );
				return false;
			}
			sheetSize.y = uint( json[ "RenderData.SheetHeight" ] );

			if ( !json.get( "RenderData.SpriteWidth", spriteSize.x ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'RenderData.SpriteWidth' in \"" + name + "\"\n" );
				return false;
			}
			spriteSize.x = uint( json[ "RenderData.SpriteWidth" ] );

			if ( !json.get( "RenderData.SpriteHeight", spriteSize.y ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'RenderData.SpriteHeight' in \"" + name + "\"\n" );
				return false;
			}
			spriteSize.y = uint( json[ "RenderData.SpriteHeight" ] );

			if ( !json.get( "RenderData.SpriteSheet", npath ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'RenderData.SpriteSheet' in \"" + name + "\"\n" );
				return false;
			}

			DebugPrint( "Allocating sprite sheet for mob \"" + name + "\", [ " + sheetSize.x + ", " + sheetSize.y + " ]:[ " + spriteSize.x + ", "
				+ spriteSize.y + " ]\n" );
			@spriteSheet = @TheNomad::Engine::ResourceCache.GetSpriteSheet( npath, sheetSize.x, sheetSize.y,
				spriteSize.x, spriteSize.y );
			
			return true;
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

			if ( !json.get( "Stats.PainTolerance", painTolerance ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.PainTolerance' in \"" + name + "\"\n" );
				return false;
			}
			painTolerance = uint( json[ "Stats.PainTolerance" ] );

			if ( !json.get( "Stats.ReactionTime", reactionTime ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Stats.ReactionTime' in \"" + name + "\"\n" );
				return false;
			}
			reactionTime = uint( json[ "Stats.ReactionTime" ] );

			return true;
		}

		private bool LoadDetectionBlock( json@ json ) {
			if ( !json.get( "Detection.SightRange", sightRange ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Detection.SightRange' in \"" + name + "\"\n" );
				return false;
			}
			sightRange = float( json[ "Detection.SightRange" ] );

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

		private bool LoadStatesBlock( json@ json ) {
			string state;
			bool hasState = false;

			if ( !json.get( "States.Idle", state ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.Idle' in \"" + name + "\"\n" );
				return false;
			}
			@idleState = @StateManager.GetStateById( state );
			if ( @idleState is null ) {
				ConsoleWarning( "invalid mob info, bad state \"" + state + "\"\n" );
				return false;
			}

			if ( !json.get( "States.HasMissile", hasState ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.HasMissile' in \"" + name + "\"\n" );
				return false;
			}
			hasState = bool( json[ "States.HasMissile" ] );
			if ( hasState ) {
				if ( !json.get( "States.Missile", state ) ) {
					ConsoleWarning( "invalid mob info, missing variable 'States.Missile' in \"" + name + "\"\n" );
					return false;
				}
				@missileState = @StateManager.GetStateById( state );
			} else {
				@missileState = null;
			}
			
			if ( !json.get( "States.HasMelee", hasState ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.HasMelee' in \"" + name + "\"\n" );
				return false;
			}
			hasState = bool( json[ "States.HasMelee" ] );
			if ( hasState ) {
				if ( !json.get( "States.Melee", state ) ) {
					ConsoleWarning( "invalid mob info, missing variable 'States.Melee' in \"" + name + "\"\n" );
					return false;
				}
				@meleeState = @StateManager.GetStateById( state );
			} else {
				@meleeState = null;
			}

			if ( !json.get( "States.Chase", state ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.Chase' in \"" + name + "\"\n" );
				return false;
			}
			@chaseState = @StateManager.GetStateById( state );
			if ( @chaseState is null ) {
				ConsoleWarning( "invalid mob info, bad state \"" + state + "\"\n" );
				return false;
			}

			if ( !json.get( "States.Search", state ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.Search' in \"" + name + "\"\n" );
				return false;
			}
			@searchState = @StateManager.GetStateById( state );
			if ( @searchState is null ) {
				ConsoleWarning( "invalid mob info, bad state \"" + state + "\"\n" );
				return false;
			}

			if ( !json.get( "States.DieHigh", state ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.DieHigh' in \"" + name + "\"\n" );
				return false;
			}
			@dieHighState = @StateManager.GetStateById( state );
			if ( @dieHighState is null ) {
				ConsoleWarning( "invalid mob info, bad state \"" + state + "\"\n" );
				return false;
			}

			if ( !json.get( "States.DieLow", state ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'States.DieLow' in \"" + name + "\"\n" );
				return false;
			}
			@dieLowState = @StateManager.GetStateById( state );
			if ( @dieLowState is null ) {
				ConsoleWarning( "invalid mob info, bad state \"" + state + "\"\n" );
				return false;
			}

			return true;
		}
		
		bool Load( json@ json ) {
			const EntityData@ entity = null;
			string str;
			
			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Id", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Id' in \"" + name + "\"\n" );
				return false;
			}
			if ( ( @entity = @InfoManager.GetMobType( str ) ) !is null ) {
				this.type = entity.GetID();
			} else {
				ConsoleWarning( "invalid mob info, Id \"" + str + "\" wasn't found\n" );
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

			if ( !LoadStatesBlock( @json ) ) {
				return false;
			}
			if ( !LoadStatsBlock( @json ) ) {
				return false;
			}
			if ( !LoadDetectionBlock( @json ) ) {
				return false;
			}
			if ( !LoadRenderDataBlock( @json ) ) {
				return false;
			}

			return true;
		}
		
		string name;
		string className;

		vec3 speed = vec3( 1.0f );
		vec2 size = vec2( 1.0f );
		
		SpriteSheet@ spriteSheet = null;

		EntityState@ idleState = null;
		EntityState@ chaseState = null;
		EntityState@ searchState = null;
		EntityState@ missileState = null;
		EntityState@ meleeState = null;
		EntityState@ dieHighState = null;
		EntityState@ dieLowState = null;

		float soundTolerance = 0.0f;
		float soundRange = 0.0f;

		float sightRange = 0.0f;
		float sightRadius = 0.0f;

		uint reactionTime = 0;
		uint painTolerance = 0;

		float health = 0.0f;
		uint type = 0;

		ArmorType armorType = ArmorType::None;
		MobFlags mobFlags = MobFlags::None;
	};
};