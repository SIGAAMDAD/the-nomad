#include "game.as"
#include "config.as"
#include "level.as"

namespace TheNomad::Util {
	bool IntToBool( int64 i ) {
		return i == 1 ? true : false;
	}

	bool UIntToBool( uint64 i ) {
		return i == 1 ? true : false;
	}
	
	bool IntToBool( int32 i ) {
		return i == 1 ? true : false;
	}

	bool UIntToBool( uint32 i ) {
		return i == 1 ? true : false;
	}

	bool IntToBool( int16 i ) {
		return i == 1 ? true : false;
	}

	bool UIntToBool( uint16 i ) {
		return i == 1 ? true : false;
	}

	bool StringToBool( const string& in str ) {
		return StrICmp( str, "true" ) == 0 ? true : false;
	}

	float DEG2RAD( float x ) {
		return ( ( x * M_PI ) / 180.0F );
	}

	float RAD2DEG( float x ) {
		return ( ( x * 180.0f ) / M_PI );
	}

	//
	// Dir2Angle: returns absolute degrees
	//
	float Dir2Angle( TheNomad::GameSystem::DirType dir ) {
		switch ( dir ) {
		case TheNomad::GameSystem::DirType::North:
			return 0.0f;
		case TheNomad::GameSystem::DirType::NorthEast:
			return 45.0f;
		case TheNomad::GameSystem::DirType::East:
			return 90.0f;
		case TheNomad::GameSystem::DirType::SouthEast:
			return 135.0f;
		case TheNomad::GameSystem::DirType::South:
			return 180.0f;
		case TheNomad::GameSystem::DirType::SouthWest:
			return 225.0f;
		case TheNomad::GameSystem::DirType::West:
			return 270.0f;
		case TheNomad::GameSystem::DirType::NorthWest:
			return 315.0f;
		default:
			GameError( "Dir2Angle: invalid dir " + formatUInt( dir ) );
		};
		return -1.0f;
	}

	//
	// Angle2Dir:
	//
	TheNomad::GameSystem::DirType Angle2Dir( float angle ) {
		if ( angle >= 337.5f && angle <= 22.5f ) {
			return TheNomad::GameSystem::DirType::North;
		} else if ( angle >= 22.5f && angle <= 67.5f ) {
			return TheNomad::GameSystem::DirType::NorthEast;
		} else if ( angle >= 67.5f && angle <= 112.5f ) {
			return TheNomad::GameSystem::DirType::East;
		} else if ( angle >= 112.5f && angle <= 157.5f ) {
			return TheNomad::GameSystem::DirType::SouthEast;
		} else if ( angle >= 157.5f && angle <= 202.5f ) {
			return TheNomad::GameSystem::DirType::South;
		} else if ( angle >= 202.5f && angle <= 247.5f ) {
			return TheNomad::GameSystem::DirType::SouthWest;
		} else if ( angle >= 247.5f && angle <= 292.5f ) {
			return TheNomad::GameSystem::DirType::West;
		} else if ( angle >= 292.5f && angle <= 337.5f ) {
			return TheNomad::GameSystem::DirType::NorthWest;
		} else {
			DebugPrint( "Angle2Dir: funny angle " + formatFloat( angle ) + "\n" );
		}
		return TheNomad::GameSystem::DirType::North;
	}
};

namespace ImGui {
	ImGuiWindowFlags MakeWindowFlags( uint flags ) {
		return ImGuiWindowFlags( flags );
	}
};

int ModuleInit() {
	ConsolePrint( "----- SG_Init -----\n" );

//	InfoInit();

	@TheNomad::CvarManager = cast<TheNomad::CvarSystem>( TheNomad::GameSystem::AddSystem( TheNomad::CvarSystem() ) );

	//
	// register cvars
	//
	@TheNomad::SGame::sgame_Difficulty = TheNomad::CvarManager.AddCvar( "sgame_Difficulty", "2", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_SaveName = TheNomad::CvarManager.AddCvar( "sgame_SaveName", "nomadsv.ngd", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_DebugMode = TheNomad::CvarManager.AddCvar( "sgame_DebugMode", "0", CVAR_LATCH | CVAR_TEMP, true );
	@TheNomad::SGame::sgame_GfxDetail = TheNomad::CvarManager.AddCvar( "sgame_GfxDetail", "4", CVAR_LATCH | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_PlayerHealBase = TheNomad::CvarManager.AddCvar( "sgame_PlayerHealBase", "0.25", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_AdaptiveSoundtrack = TheNomad::CvarManager.AddCvar( "sgame_AdaptiveSoundtrack", "1", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_MapName = TheNomad::CvarManager.AddCvar( "mapname", "", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_SoundDissonance = TheNomad::CvarManager.AddCvar( "sgame_SoundDissonance", "1.5", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_LevelInfoFile = TheNomad::CvarManager.AddCvar( "sgame_LevelInfoFile", "", CVAR_INIT | CVAR_ROM, false );

	if ( int( TheNomad::SGame::sgame_DebugMode.GetInt() ) == 1 ) {
		@TheNomad::SGame::sgame_LevelDebugPrint = TheNomad::CvarManager.AddCvar(
			"sgame_LevelDebugPrint", "1", CVAR_LATCH | CVAR_TEMP, true );
	} else {
		@TheNomad::SGame::sgame_LevelDebugPrint = TheNomad::CvarManager.AddCvar(
			"sgame_LevelDebugPrint", "0", CVAR_LATCH | CVAR_TEMP, true );
	}

	ModuleConfigInit();

	//
	// register strings
	//
	TheNomad::GameSystem::GetString( "SP_DIFF_VERY_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryEasy] );
	TheNomad::GameSystem::GetString( "SP_DIFF_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Easy] );
	TheNomad::GameSystem::GetString( "SP_DIFF_MEDIUM", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Normal] );
	TheNomad::GameSystem::GetString( "SP_DIFF_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Hard] );
	TheNomad::GameSystem::GetString( "SP_DIFF_VERY_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryHard] );

	TheNomad::SGame::selectedSfx.Set( "sfx/menu1.wav" );

	// init globals
	@TheNomad::GameSystem::GameManager = cast<TheNomad::GameSystem::CampaignManager>( TheNomad::GameSystem::AddSystem( TheNomad::GameSystem::CampaignManager() ) );
	@TheNomad::SGame::LevelManager = cast<TheNomad::SGame::LevelSystem>( TheNomad::GameSystem::AddSystem( TheNomad::SGame::LevelSystem() ) );
	@TheNomad::SGame::EntityManager = cast<TheNomad::SGame::EntitySystem>( TheNomad::GameSystem::AddSystem( TheNomad::SGame::EntitySystem() ) );
	@TheNomad::SGame::StateManager = cast<TheNomad::SGame::EntityStateSystem>( TheNomad::GameSystem::AddSystem( TheNomad::SGame::EntityStateSystem() ) );
	@TheNomad::SGame::InfoManager = TheNomad::SGame::InfoDataManager();
	
	ConsolePrint( "--------------------\n" );
	return 1;
}

int ModuleOnConsoleCommand() {
	return 1;
}

int ModuleOnSaveGame() {
	ConsolePrint( "Saving game, please do not close out of g_ModuleData app...\n" );

	ConsolePrint( "Done.\n" );
	return 1;
}

int ModuleOnLoadGame() {
	return 1;
}

int ModuleOnLevelStart() {
	return 1;
}

int ModuleOnLevelEnd() {
	return 1;
}

int ModuleShutdown() {
	ConsolePrint( "----- SG_Shutdown -----\n" );

	ConsolePrint( "-----------------------\n" );
	return 1;
}

int ModuleOnKeyEvent( uint key, uint down )
{
	return 0;
}

int ModuleOnMouseEvent( int dx, int dy )
{
	return 0;
}

int ModuleOnRunTic( uint msec )
{
	TheNomad::GameSystem::GameManager.SetMsec( msec );

	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		GameObjects[i].OnRunTic();
	}

	return 0;
}

shared void DebugPrint( const string& in msg ) {
	if ( TheNomad::Engine::CvarVariableInteger( "sgame_DebugMode" ) == 0 ) {
		return;
	}

	ConsolePrint( "DEBUG: " + msg );
}
