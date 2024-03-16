#include "game.as"
#include "config.as"
#include "level.as"

namespace TheNomad::Util {
	shared float DEG2RAD( float x ) {
		return ( ( x * M_PI ) / 180.0F );
	}

	shared float RAD2DEG( float x ) {
		return ( ( x * 180.0f ) / M_PI );
	}

	//
	// Dir2Angle: returns absolute degrees
	//
	shared float Dir2Angle( TheNomad::GameSystem::DirType dir ) {
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
	shared TheNomad::GameSystem::DirType Angle2Dir( float angle ) {
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
	shared ImGuiWindowFlags MakeWindowFlags( uint flags ) {
		return ImGuiWindowFlags( flags );
	}
};

shared class ModuleObject {
	ModuleObject() {
	}

	TheNomad::GameSystem::GameObject@ AddSystem( TheNomad::GameSystem::GameObject@ SystemHandle ) {
		m_GameSystems.push_back( SystemHandle );
		return SystemHandle;
	}

	void InfoInit() {
//		ConsolePrint( "[Module Info]\n" );
//		ConsolePrint( "name: " + MODULE_NAME + "\n" );
//		ConsolePrint( "version: v" + formatInt( MODULE_VERSION_MAJOR ) + "." + formatInt( MODULE_VERSION_UPDATE ) + "."
//			+ formatInt( MODULE_VERSION_PATCH ) + "\n" );
	}

	int Init() {
		ConsolePrint( "----- SG_Init -----\n" );

		ModuleInfoInit();

		@CvarManager = cast<TheNomad::CvarSystem>( AddSystem( TheNomad::CvarSystem() ) );

		//
		// register cvars
		//
		@sgame_Difficulty = CvarManager.AddCvar( "sgame_Difficulty", "2", CVAR_LATCH | CVAR_TEMP, false );
		@sgame_SaveName = CvarManager.AddCvar( "sgame_SaveName", "nomadsv.ngd", CVAR_LATCH | CVAR_TEMP, false );
		@sgame_DebugMode = CvarManager.AddCvar( "sgame_DebugMode", "0", CVAR_LATCH | CVAR_TEMP, true );
		@sgame_GfxDetail = CvarManager.AddCvar( "sgame_GfxDetail", "4", CVAR_LATCH | CVAR_SAVE, true );
		@sgame_PlayerHealBase = CvarManager.AddCvar( "sgame_PlayerHealBase", "0.25", CVAR_LATCH | CVAR_TEMP, false );
		@sgame_AdaptiveSoundtrack = CvarManager.AddCvar( "sgame_AdaptiveSoundtrack", "1", CVAR_LATCH | CVAR_SAVE, false );
		@sgame_MapName = CvarManager.AddCvar( "mapname", "", CVAR_LATCH | CVAR_TEMP );
		@sgame_SoundDissonance = CvarManager.AddCvar( "sgame_SoundDissonance", "1.5", CVAR_LATCH | CVAR_SAVE, false );
		@sgame_LevelInfoFile = CvarManager.AddCvar( "sgame_LevelInfoFile", "", CVAR_INIT | CVAR_ROM, false );

		if ( int( sgame_DebugMode.GetInt() ) == 1 ) {
			@sgame_LevelDebugPrint = CvarManager.AddCvar(
				"sgame_LevelDebugPrint", "1", CVAR_LATCH | CVAR_TEMP, true );
		} else {
			@sgame_LevelDebugPrint = CvarManager.AddCvar(
				"sgame_LevelDebugPrint", "0", CVAR_LATCH | CVAR_TEMP, true );
		}

		//
		// register strings
		//
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_EASY", SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryEasy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_EASY", SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Easy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_MEDIUM", SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Normal] );
		TheNomad::GameSystem::GetString( "SP_DIFF_HARD", SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Hard] );
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_HARD", SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryHard] );

		selectedSfx.Set( "sfx/menu1.wav" );

		// init globals
		@Config = ModuleConfig( @this );
		@GameManager = cast<TheNomad::GameSystem::CampaignManager>( AddSystem( TheNomad::GameSystem::CampaignManager( @this ) ) );
		@LevelManager = cast<TheNomad::SGame::LevelSystem>( AddSystem( TheNomad::SGame::LevelSystem( @this ) ) );
		@EntityManager = cast<TheNomad::SGame::EntityManager>( AddSystem( TheNomad::SGame::EntitySystem( @this ) ) );
		@StateManager = cast<TheNomad::SGame::EntityStateSystem>( AddSystem( TheNomad::SGame::EntityStateSystem() ) );
		@InfoManager = cast<TheNomad::SGame::InfoDataManager>( AddSystem( TheNomad::SGame::InfoDataManager() ) );

		ConsolePrint( "--------------------\n" );
		return 1;
	}

	int OnConsoleCommand() {
		return 1;
	}

	int OnSaveGame() {
		ConsolePrint( "Saving game, please do not close out of this app...\n" );
		ConsolePrint( "Done.\n" );

		return 1;
	}

	int OnLoadGame() {
		return 1;
	}

	int OnLevelStart() {
		return 1;
	}

	int OnLevelEnd() {
		return 1;
	}

	int Shutdown() {
		ConsolePrint( "----- SG_Shutdown -----\n" );

		ConsolePrint( "-----------------------\n" );
		return 1;
	}
	
	TheNomad::ConVar@ sgame_GfxDetail;
	TheNomad::ConVar@ sgame_QuickShotMaxTargets;
	TheNomad::ConVar@ sgame_QuickShotTime;
	TheNomad::ConVar@ sgame_QuickShotMaxRange;
	TheNomad::ConVar@ sgame_MaxPlayerWeapons;
	TheNomad::ConVar@ sgame_PlayerHealBase;
	TheNomad::ConVar@ sgame_Difficulty;
	TheNomad::ConVar@ sgame_SaveName;
	TheNomad::ConVar@ sgame_AdaptiveSoundtrack;
	TheNomad::ConVar@ sgame_DebugMode;
	TheNomad::ConVar@ sgame_cheat_BlindMobs;
	TheNomad::ConVar@ sgame_cheat_DeafMobs;
	TheNomad::ConVar@ sgame_cheat_InfiniteAmmo;
	TheNomad::ConVar@ sgame_LevelDebugPrint;
	TheNomad::ConVar@ sgame_LevelIndex;
	TheNomad::ConVar@ sgame_LevelInfoFile;
	TheNomad::ConVar@ sgame_SoundDissonance;
	TheNomad::ConVar@ sgame_MapName;

	TheNomad::SGame::ItemSystem@ ItemManager;
	TheNomad::GameSystem::CampaignManager@ GameManager;
	TheNomad::SGame::LevelSystem@ LevelManager;
	TheNomad::SGame::EntitySystem@ EntityManager;
	TheNomad::SGame::EntityStateSystem@ StateManager;
	TheNomad::CvarSystem@ CvarManager;
	TheNomad::SGame::InfoDataManager@ InfoManager;

	TheNomad::Engine::SoundSystem::SoundEffect selectedSfx;
	string[] SP_DIFF_STRINGS( TheNomad::GameSystem::GameDifficulty::NumDifficulties );
	string[] sgame_RankStrings( LevelRank::NumRanks );
	vec4[] sgame_RankStringColors( LevelRank::NumRanks );

	ModuleConfig@ Config;
};

shared void DebugPrint( const string& in msg ) {
	if ( TheNomad::Engine::CvarVariableInteger( "sgame_DebugMode" ) == 0 ) {
		return;
	}

	ConsolePrint( "DEBUG: " + msg );
}
