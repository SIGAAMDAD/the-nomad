#include "util/detail.as"
#include "convar.as"
#include "GameSystem/GameSystem.as"
#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/LevelSystem.as"
#include "Engine/CommandSystem/CommandManager.as"
#include "SGame/CheatCodes.as"
#include "Engine/Engine.as"

namespace TheNomad::SGame {
	//
	// misc
	//
	ConVar sgame_NoRespawningMobs;
	ConVar sgame_MaxEntities;
	ConVar sgame_HellbreakerActive;
	ConVar sgame_HellbreakerOn;
	ConVar sgame_GfxDetail;
	ConVar sgame_Difficulty;
	ConVar sgame_DebugMode;

	//
	// sound system
	//
	ConVar sgame_MusicChangeDelta;
	ConVar sgame_MaxSoundChannels;
	ConVar sgame_AdaptiveSoundtrack;

	//
	// cheats
	//
	ConVar sgame_cheat_BlindMobs;
	ConVar sgame_cheat_DeafMobs;
	ConVar sgame_cheat_InfiniteAmmo;
	ConVar sgame_cheat_InfiniteHealth;
	ConVar sgame_cheat_InfiniteRage;
	ConVar sgame_cheat_GodMode;
	ConVar sgame_cheats_enabled;

	//
	// cvars that are used by the engine
	//
	ConVar sgame_LevelIndex;
	ConVar sgame_MapName;
	ConVar sgame_MaxFps;
	ConVar sgame_SaveName;

	//
	// game specific
	//
	ConVar sgame_QuickShotMaxTargets;
	ConVar sgame_QuickShotTime;
	ConVar sgame_QuickShotMaxRange;
	ConVar sgame_PlayerHealBase;
	ConVar sgame_BaseSpeed;
	ConVar sgame_GroundFriction;
	ConVar sgame_WaterFriction;
	ConVar sgame_AirFriction;
	ConVar sgame_MaxSpeed;
	ConVar sgame_ToggleHUD;
	ConVar sgame_Friction;
	ConVar sgame_PlayerHeight;
	ConVar sgame_PlayerWidth;
	ConVar sgame_Gravity;
	ConVar sgame_SwimSpeed;
	ConVar sgame_AirSpeed;

	class KeyBinding {
		KeyBinding() {
		}
		KeyBinding( uint num, TheNomad::Engine::CommandSystem::CommandFunc@ downFunc, TheNomad::Engine::CommandSystem::CommandFunc@ upFunc ) {
			m_nKeyNum = num;
			@m_DownFunc = @downFunc;
			@m_UpFunc = @upFunc;
		}

		uint m_nKeyNum = 0;
		TheNomad::Engine::CommandSystem::CommandFunc@ m_DownFunc = null;
		TheNomad::Engine::CommandSystem::CommandFunc@ m_UpFunc = null;
	};
	array<KeyBinding> sgame_KeyBindTable;
};

namespace ImGui {
	ImGuiWindowFlags MakeWindowFlags( uint flags ) {
		return ImGuiWindowFlags( flags );
	}
};

void LoadLevelAssets() {
}

//
// InitResources: caches all important SGame resources
//
void InitResources() {

	ConsolePrint( "Initializing SGame Resources...\n" );

	TheNomad::Engine::Timer timer;
	string str = TheNomad::Engine::CvarVariableString( "skin" );

	timer.Start();

	TheNomad::Engine::Renderer::RegisterSpriteSheet( "sprites/players/" + str + "_torso", 512, 512, 32, 32 );
	TheNomad::Engine::Renderer::RegisterSpriteSheet( "sprites/players/" + str + "_legs", 512, 512, 32, 32 );
	TheNomad::Engine::Renderer::RegisterSpriteSheet( "sprites/players/" + str + "_arms", 512, 512, 32, 32 );

	TheNomad::SGame::InfoSystem::InfoManager.LoadMobInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadItemInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadAmmoInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadWeaponInfos();

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/die1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/die2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/die3.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/slide0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/slide1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/dash.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/misc/passCheckpoint.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/mobs/detect.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/mobs/detectMeme.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel3.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveWater0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveWater1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal3.ogg" );

	timer.Stop();
	ConsolePrint( "LoadLevelAssets: " + timer.ElapsedMilliseconds() + "ms\n" );

	//
	// register strings
	//
	{
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryEasy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Easy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_MEDIUM", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Normal] );
		TheNomad::GameSystem::GetString( "SP_DIFF_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Hard] );
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryHard] );

		TheNomad::GameSystem::GetString( "SP_RANK_S_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankS] );
		TheNomad::GameSystem::GetString( "SP_RANK_A_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankA] );
		TheNomad::GameSystem::GetString( "SP_RANK_B_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankB] );
		TheNomad::GameSystem::GetString( "SP_RANK_C_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankC] );
		TheNomad::GameSystem::GetString( "SP_RANK_D_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankD] );
		TheNomad::GameSystem::GetString( "SP_RANK_F_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankF] );
		TheNomad::GameSystem::GetString( "SP_RANK_U_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankWereUBotting] );

		TheNomad::GameSystem::GetString( "SP_RANK_S_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankS ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_A_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankA ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_B_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankB ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_C_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankC ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_D_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankD ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_F_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankF ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_U_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankWereUBotting ] = TheNomad::Util::StringToColor( str );
	}
}

void InitCvars() {
	ConsolePrint( "Registering SGame Cvars...\n" );

	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_MaxEntities, "sgame_MaxEntities", "500", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_NoRespawningMobs, "sgame_NoRespawningMobs", "0", CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_HellbreakerActive, "sgame_HellbreakerActive", "0", CVAR_TEMP, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_HellbreakerOn, "sgame_HellbreakerOn", "0", CVAR_TEMP, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_GfxDetail, "sgame_GfxDetail", "10", CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_Difficulty, "sgame_Difficulty", "2", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_DebugMode, "sgame_DebugMode", "1", CVAR_TEMP, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_MusicChangeDelta, "sgame_MusicChangeDelta", "500", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_MaxSoundChannels, "sgame_MaxSoundChannels", "256", CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_AdaptiveSoundtrack, "sgame_AdaptiveSoundtrack", "1", CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheat_BlindMobs, "sgame_cheat_BlindMobs", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheat_DeafMobs, "sgame_cheat_DeafMobs", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheat_InfiniteAmmo, "sgame_cheat_InfiniteAmmo", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheat_InfiniteHealth, "sgame_cheat_InfiniteHealth", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheat_InfiniteRage, "sgame_cheat_InfiniteRage", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheat_GodMode, "sgame_cheat_GodMode", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_cheats_enabled, "sgame_cheats_enabled", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_LevelIndex, "g_levelIndex", "0", CVAR_LATCH | CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_MapName, "mapname", "", CVAR_LATCH | CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_MaxFps, "com_maxfps", "", CVAR_LATCH | CVAR_SAVE, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_SaveName, "sgame_SaveName", "nomadsv", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_QuickShotMaxTargets, "sgame_QuickShotMaxTargets", "20", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_QuickShotTime, "sgame_QuickShotTime", "100", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_QuickShotMaxRange, "sgame_QuickShotMaxRange", "40", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_PlayerHealBase, "sgame_PlayerHealBase", "0.05", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_BaseSpeed, "sgame_BaseSpeed", "1.5", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_GroundFriction, "sgame_GroundFriction", "0.9", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_GroundFriction, "sgame_WaterFriction", "2.5", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_AirFriction, "sgame_AirFriction", "0.5", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_MaxSpeed, "sgame_MaxSpeed", "2.5", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_ToggleHUD, "sgame_ToggleHUD", "1", CVAR_SAVE, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_Friction, "sgame_Friction", "0.1", CVAR_TEMP, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_Gravity, "sgame_Gravity", "0.9", CVAR_TEMP, true );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_PlayerWidth, "sgame_PlayerWidth", "0.5", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_PlayerHeight, "sgame_PlayerHeight", "2.0", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_SwimSpeed, "sgame_SwimSpeed", "1.0", CVAR_TEMP, false );
	TheNomad::CvarManager.AddCvar( @TheNomad::SGame::sgame_AirSpeed, "sgame_AirSpeed", "2.5", CVAR_TEMP, false );
}

int ModuleOnInit() {
	ConsolePrint( "----- SG_Init -----\n" );
	ConsolePrint( "GameName: " + GAME_NAME + "\n" );
	ConsolePrint( "GameVersion: " + NOMAD_VERSION_STRING + "\n" );

	//
	// register cvars
	//

	@TheNomad::CvarManager = cast<TheNomad::CvarSystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::CvarSystem() ) );

	InitCvars();

	TheNomad::Util::GetModuleList( TheNomad::SGame::sgame_ModList );
	ConsolePrint( TheNomad::SGame::sgame_ModList.Count() + " total mods registered.\n" );

	@TheNomad::Engine::FileSystem::FileManager = TheNomad::Engine::FileSystem::FileSystemManager();
	@TheNomad::Engine::SoundSystem::SoundManager = TheNomad::Engine::SoundSystem::SoundFrameData();
	@TheNomad::GameSystem::GameManager = cast<TheNomad::GameSystem::CampaignManager@>( @TheNomad::GameSystem::AddSystem( TheNomad::GameSystem::CampaignManager() ) );
	@TheNomad::SGame::LevelManager = cast<TheNomad::SGame::LevelSystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::SGame::LevelSystem() ) );
	@TheNomad::SGame::InfoSystem::InfoManager = TheNomad::SGame::InfoSystem::InfoDataManager();
	@TheNomad::SGame::EntityManager = cast<TheNomad::SGame::EntitySystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::SGame::EntitySystem() ) );
	@TheNomad::SGame::StateManager = cast<TheNomad::SGame::EntityStateSystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::SGame::EntityStateSystem() ) );

	TheNomad::SGame::InitCheatCodes();
	TheNomad::SGame::ScreenData.Init();

	//
	// init resources
	//

	InitResources();

	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnInit();
	}

	ConsolePrint( "--------------------\n" );

	return 1;
}

int ModuleOnShutdown() {
	TheNomad::SGame::sgame_ModList.Clear();

	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnShutdown();
		@TheNomad::GameSystem::GameSystems[i] = null;
	}
	@TheNomad::CvarManager = null;
	@TheNomad::GameSystem::GameManager = null;
	@TheNomad::SGame::LevelManager = null;
	@TheNomad::SGame::EntityManager = null;
	@TheNomad::SGame::StateManager = null;
	@TheNomad::SGame::InfoSystem::InfoManager = null;
	@TheNomad::Engine::FileSystem::FileManager = null;
	TheNomad::GameSystem::GameSystems.Clear();

	return 1;
}

int ModuleOnConsoleCommand() {
	const string cmd = TheNomad::Engine::CmdArgv( 0 );

	if ( TheNomad::Engine::CommandSystem::CmdManager.CheckCommand( cmd ) ) {
		return 1;
	}

	return 0;
}

int ModuleOnSaveGame() {
	ConsolePrint( "Saving game, please do not close out of the app...\n" );
	
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnSave();
	}
	ConsolePrint( "Done.\n" );

	return 1;
}

int ModuleOnLoadGame() {
	LoadLevelAssets();

	TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::InLevel;
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnLoad();
	}

	return 1;
}

int ModuleOnLevelStart() {
	LoadLevelAssets();
	TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::InLevel;
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnLevelStart();
	}
	TheNomad::SGame::ScreenData.InitPlayers();
	return 1;
}

int ModuleOnLevelEnd() {
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnLevelEnd();
	}
	return 1;
}

int ModuleOnKeyEvent( uint key, uint down ) {
	return 0;
}

int ModuleOnMouseEvent( int dx, int dy ) {
	TheNomad::GameSystem::GameManager.SetMousePos( uvec2( dx, dy ) );
	return 0;
}

int ModuleOnRunTic( uint msec ) {
	if ( TheNomad::SGame::GlobalState == TheNomad::SGame::GameState::LevelFinish ) {
		// this will automatically call OnLevelEnd for all registered game objects
		TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::StatsMenu;
		return 1;
	} else if ( TheNomad::SGame::GlobalState == TheNomad::SGame::GameState::EndOfLevel ) {
		TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::Inactive;
		return 3;
	}

	TheNomad::GameSystem::GameManager.SetMsec( TheNomad::Engine::System::Milliseconds() );

	// if we're paused, then just draw the stuff, don't run anything else
	if ( TheNomad::Engine::CvarVariableInteger( "g_paused" ) == 0 ) {
		TheNomad::SGame::LevelManager.Resume();
		TheNomad::SGame::EntityManager.SetActivePlayer( @TheNomad::SGame::ScreenData.GetPlayerAt( 0 ) );
		for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
			TheNomad::GameSystem::GameSystems[i].OnRunTic();
		}
	} else {
		TheNomad::SGame::LevelManager.Pause();
	}

	TheNomad::SGame::ScreenData.Draw();
	
	return TheNomad::SGame::GlobalState == TheNomad::SGame::GameState::StatsMenu ? 2 : 0;
}