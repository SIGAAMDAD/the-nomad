#include "util/detail.as"
#include "game.as"
#include "config.as"
#include "level.as"

namespace TheNomad::SGame {
	//
	// misc
	//
	ConVar@ sgame_NoRespawningMobs;
	ConVar@ sgame_MaxEntities;
	ConVar@ sgame_HellbreakerActive;
	ConVar@ sgame_HellbreakerOn;
	ConVar@ sgame_GfxDetail;
	ConVar@ sgame_Difficulty;
	ConVar@ sgame_DebugMode;

	//
	// sound system
	//
	ConVar@ sgame_MusicChangeDelta;
	ConVar@ sgame_SoundDissonance;
	ConVar@ sgame_MaxSoundChannels;
	ConVar@ sgame_AdaptiveSoundtrack;

	//
	// cheats
	//
	ConVar@ sgame_cheat_BlindMobs;
	ConVar@ sgame_cheat_DeafMobs;
	ConVar@ sgame_cheat_InfiniteAmmo;
	ConVar@ sgame_cheat_InfiniteHealth;
	ConVar@ sgame_cheat_InfiniteRage;
	ConVar@ sgame_cheat_GodMode;
	ConVar@ sgame_cheats_enabled;

	//
	// cvars that are used by the engine
	//
	ConVar@ sgame_LevelIndex;
	ConVar@ sgame_MapName;
	ConVar@ sgame_MaxFps;
	ConVar@ sgame_SaveName;

	//
	// player specific
	//
	ConVar@ sgame_QuickShotMaxTargets;
	ConVar@ sgame_QuickShotTime;
	ConVar@ sgame_QuickShotMaxRange;
	ConVar@ sgame_MaxPlayerWeapons;
	ConVar@ sgame_PlayerHealBase;
	ConVar@ sgame_BaseSpeed;
	ConVar@ sgame_GroundFriction;
	ConVar@ sgame_AirFriction;
	ConVar@ sgame_MaxSpeed;
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

	TheNomad::Util::GetModuleList( TheNomad::SGame::sgame_ModList );
	ConsolePrint( TheNomad::SGame::sgame_ModList.size() + " total mods registered.\n" );

	//
	// register cvars
	//
	@TheNomad::SGame::sgame_MaxEntities = TheNomad::CvarManager.AddCvar( "sgame_MaxEntities", "500", CVAR_LATCH | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_NoRespawningMobs = TheNomad::CvarManager.AddCvar( "sgame_NoRespawningMobs", "0", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_HellbreakerActive = TheNomad::CvarManager.AddCvar( "sgame_HellbreakerActive", "0", CVAR_LATCH | CVAR_TEMP, true );
	@TheNomad::SGame::sgame_HellbreakerOn = TheNomad::CvarManager.AddCvar( "sgame_HellbreakerOn", "0", CVAR_LATCH | CVAR_TEMP, true );
	@TheNomad::SGame::sgame_GfxDetail = TheNomad::CvarManager.AddCvar( "sgame_GfxDetail", "10", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_Difficulty = TheNomad::CvarManager.AddCvar( "sgame_Difficulty", "2", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_DebugMode = TheNomad::CvarManager.AddCvar( "sgame_DebugMode", "0", CVAR_LATCH | CVAR_TEMP, true );
	@TheNomad::SGame::sgame_MusicChangeDelta = TheNomad::CvarManager.AddCvar( "sgame_MusicChangeDelta", "500", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_SoundDissonance = TheNomad::CvarManager.AddCvar( "sgame_SoundDissonance", "2.2", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_MaxSoundChannels = TheNomad::CvarManager.AddCvar( "sgame_MaxSoundChannels", "256", CVAR_INIT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_AdaptiveSoundtrack = TheNomad::CvarManager.AddCvar( "sgame_AdaptiveSoundtrack", "1", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheat_BlindMobs = TheNomad::CvarManager.AddCvar( "sgame_cheat_BlindMobs", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheat_DeafMobs = TheNomad::CvarManager.AddCvar( "sgame_cheat_DeafMobs", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheat_InfiniteAmmo = TheNomad::CvarManager.AddCvar( "sgame_cheat_InfiniteAmmo", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheat_InfiniteHealth = TheNomad::CvarManager.AddCvar( "sgame_cheat_InfiniteHealth", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheat_InfiniteRage = TheNomad::CvarManager.AddCvar( "sgame_cheat_InfiniteRage", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheat_GodMode = TheNomad::CvarManager.AddCvar( "sgame_cheat_GodMode", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_cheats_enabled = TheNomad::CvarManager.AddCvar( "sgame_cheats_enabled", "0", CVAR_CHEAT | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_LevelIndex = TheNomad::CvarManager.AddCvar( "g_levelIndex", "0", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_MapName = TheNomad::CvarManager.AddCvar( "mapname", "", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_MaxFps = TheNomad::CvarManager.AddCvar( "com_maxfps", "60", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_SaveName = TheNomad::CvarManager.AddCvar( "sgame_SaveName", "nomadsv.ngd", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_QuickShotMaxTargets = TheNomad::CvarManager.AddCvar( "sgame_QuickShotMaxTargets", "20", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_QuickShotTime = TheNomad::CvarManager.AddCvar( "sgame_QuickShotTime", "100", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_QuickShotMaxRange = TheNomad::CvarManager.AddCvar( "sgame_QuickShotMaxRange", "40", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_MaxPlayerWeapons = TheNomad::CvarManager.AddCvar( "sgame_MaxPlayerWeapons", "11", CVAR_ROM | CVAR_INIT, false );
	@TheNomad::SGame::sgame_PlayerHealBase = TheNomad::CvarManager.AddCvar( "sgame_PlayerHealBase", "0.05", CVAR_INIT | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_BaseSpeed = TheNomad::CvarManager.AddCvar( "sgame_BaseSpeed", "1.15", CVAR_INIT | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_GroundFriction = TheNomad::CvarManager.AddCvar( "sgame_GroundFriction", "0.9", CVAR_INIT | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_AirFriction = TheNomad::CvarManager.AddCvar( "sgame_AirFriction", "0.5", CVAR_INIT | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_MaxSpeed = TheNomad::CvarManager.AddCvar( "sgame_MaxSpeed", "20.5", CVAR_INIT | CVAR_ROM, false );

	@TheNomad::Engine::SoundSystem::SoundManager = TheNomad::Engine::SoundSystem::SoundScene();

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
//	const string command = TheNomad::Engine::CmdArgv( 0 );

	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		if ( GameObjects[i].OnConsoleCommand( TheNomad::Engine::CmdArgv( 0 ) ) ) {
			return 1;
		}
	}
	return 0;
}

int ModuleOnSaveGame() {
	ConsolePrint( "Saving game, please do not close out of app...\n" );

	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		GameObjects[i].OnSave();
	}

	ConsolePrint( "Done.\n" );

	return 1;
}

int ModuleOnLoadGame() {
	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		GameObjects[i].OnLoad();
	}
	return 1;
}

int ModuleOnLevelStart() {
	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		GameObjects[i].OnLevelStart();
	}

	return 1;
}

int ModuleOnLevelEnd() {
	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		GameObjects[i].OnLevelEnd();
	}

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

	TheNomad::Engine::Renderer::ClearScene();

	const uint flags = RSF_ORTHO_TYPE_WORLD;

	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.size(); i++ ) {
		GameObjects[i].OnRunTic();
	}

	TheNomad::Engine::Renderer::RenderScene( 0, 0, TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
		TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight, flags, TheNomad::GameSystem::GameManager.GetGameMsec() );

	return 0;
}

shared void DebugPrint( const string& in msg ) {
	if ( TheNomad::Engine::CvarVariableInteger( "sgame_DebugMode" ) == 0 ) {
		return;
	}

	ConsolePrint( "DEBUG: " + msg );
}
