#include "util/detail.as"
#include "Engine/ConVar.as"
#include "GameSystem/GameSystem.as"
#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/LevelSystem.as"
#include "Engine/CommandSystem/CommandManager.as"
#include "SGame/CheatCodes.as"
#include "Engine/Engine.as"
#include "SGame/Cvars.as"
#include "config.as"

namespace ImGui {
	ImGuiWindowFlags MakeWindowFlags( uint flags ) {
		return ImGuiWindowFlags( flags );
	}
};

namespace nomadmain {

void InitCvars() {
	ConsolePrint( "Registering SGame Cvars...\n" );

	TheNomad::SGame::sgame_HellbreakerOn.Register( "sgame_HellbreakerOn", "0", CVAR_TEMP, true );
	TheNomad::SGame::sgame_EnableParticles.Register( "sgame_EnableParticles", "1", CVAR_SAVE, true );
	TheNomad::SGame::sgame_DebugMode.Register( "sgame_DebugMode", "1", CVAR_TEMP, true );
	TheNomad::SGame::sgame_cheat_BlindMobs.Register( "sgame_cheat_BlindMobs", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheat_DeafMobs.Register( "sgame_cheat_DeafMobs", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheat_InfiniteAmmo.Register( "sgame_cheat_InfiniteAmmo", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheat_InfiniteHealth.Register( "sgame_cheat_InfiniteHealth", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheat_InfiniteRage.Register( "sgame_cheat_InfiniteRage", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheat_GodMode.Register( "sgame_cheat_GodMode", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheats_enabled.Register( "sgame_cheats_enabled", "0", CVAR_CHEAT | CVAR_SAVE, false );
	TheNomad::SGame::sgame_cheats_enabled.Register( "sgame_NoClip", "0", CVAR_CHEAT, true );
	/*
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_LockShotMaxTargets, "sgame_LockShotMaxTargets", "20", CVAR_TEMP, false );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_LockShotTime, "sgame_LockShotTime", "100", CVAR_TEMP, false );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_LockShotMaxRange, "sgame_LockShotMaxRange", "40", CVAR_TEMP, false );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_PlayerHealBase, "sgame_PlayerHealBase", "1.0", CVAR_SAVE, true );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_BaseSpeed, "sgame_BaseSpeed", "0.9", CVAR_SAVE, true );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_GroundFriction, "sgame_GroundFriction", "1.0", CVAR_SAVE, true );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_GroundFriction, "sgame_WaterFriction", "2.5", CVAR_SAVE, true );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_AirFriction, "sgame_AirFriction", "0.5", CVAR_SAVE, true );
	TheNomad::Engine::CvarManager.AddCvar( @TheNomad::SGame::sgame_MaxSpeed, "sgame_MaxSpeed", "0.2", CVAR_TEMP, false );
	*/
	TheNomad::SGame::sgame_ToggleHUD.Register( "sgame_ToggleHUD", "1", CVAR_SAVE, true );
	TheNomad::SGame::sgame_SaveLastUsedWeaponModes.Register( "sgame_SaveLastUsedWeaponModes", "0", CVAR_SAVE, true );
	TheNomad::SGame::sgame_Blood.Register( "sgame_Blood", "1", CVAR_SAVE, false );
	TheNomad::SGame::sgame_InputMode.Register( "in_mode", "0", CVAR_SAVE, false );

	TheNomad::SGame::sgame_MaxEntities.Register( "sgame_MaxEntities", "500", CVAR_TEMP, false );
	TheNomad::SGame::sgame_GfxDetail.Register( "sgame_GfxDetail", "1", CVAR_TEMP, false );
	TheNomad::SGame::sgame_PlayerWidth.Register( "sgame_PlayerWidth", "0.5", CVAR_TEMP, false );
	TheNomad::SGame::sgame_PlayerHeight.Register( "sgame_PlayerHeight", "2.0", CVAR_TEMP, false );
	TheNomad::SGame::sgame_PlayerWeight.Register( "sgame_PlayerWeight", "5.0", CVAR_TEMP, false );
	TheNomad::SGame::sgame_Difficulty.Register( "sgame_Difficulty", "2", CVAR_TEMP, false );
	TheNomad::SGame::sgame_Friction.Register( "sgame_Friction", "0.5", CVAR_TEMP, true );
	TheNomad::SGame::sgame_CameraZoom.Register( "sgame_CameraZoom", "68", CVAR_SAVE, true );
	TheNomad::SGame::sgame_Gravity.Register( "sgame_Gravity", "0.2", CVAR_TEMP, true );
	TheNomad::SGame::sgame_SaveSlot.Register( "sgame_SaveSlot", "0", CVAR_TEMP, false );
}

//
// InitResources: caches all important SGame resources
//
void InitResources() {
	string str;

	ConsolePrint( "Initializing SGame Resources...\n" );

	TheNomad::Engine::Timer timer;

	timer.Start();

	//
	// load sound effects
	//

	// player specific
	{
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/death1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/death2" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/death3" );
	
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/pain_scream_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/pain_scream_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/pain_scream_2" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slide_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slide_1" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/use_jumpkit_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/use_jumpkit_1" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/melee" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/weapon_change_hand" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/weapon_change_mode" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/cloth_foley_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/cloth_foley_1" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slowmo_on" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slowmo_off" );
	}

	// environmental sounds
	{
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/bonfire" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/land_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/land_2" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/land_3" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/land_4" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_2" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_3" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_2" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_3" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_water_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_water_1" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/water_jump" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/water_land_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/water_land_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/water_land_2" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_metal_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_metal_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_metal_2" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_metal_3" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_rock_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_rock_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_rock_2" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/hit_rock_3" );

		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/ricochet_0" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/ricochet_1" );
		TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/bullet_impact/ricochet_2" );
	}
	
	//
	// load materials
	//

	TheNomad::Engine::Renderer::RegisterShader( "skins/" + TheNomad::Engine::CvarVariableString( "skin" ) );

	//
	// load particle effects
	//

	TheNomad::Engine::Renderer::RegisterShader( "gfx/completed_checkpoint" );
	TheNomad::Engine::Renderer::RegisterShader( "gfx/checkpoint" );

	TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/checkpoint", 128, 32, 32, 32 );

	TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/dash_screen" );
	TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/parry_screen" );
	TheNomad::Engine::Renderer::RegisterShader( "gfx/hud/bullet_time_blur" );

	//
	// register strings
	//
	{
		TheNomad::GameSystem::GetString( "SP_DIFF_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Easy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_MEDIUM", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Normal] );
		TheNomad::GameSystem::GetString( "SP_DIFF_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Hard] );
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryHard] );
		TheNomad::GameSystem::GetString( "SP_DIFF_INSANE", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Insane] );

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

	timer.Stop();
	ConsolePrint( "InitResources: " + timer.ElapsedMilliseconds() + "ms\n" );
}

int ModuleOnInit() {
	ConsolePrint( "----- SG_Init -----\n" );
	ConsolePrint( "GameName: " + GAME_NAME + "\n" );
	ConsolePrint( "GameVersion: " + NOMAD_VERSION_STRING + "\n" );

	//
	// register cvars
	//

	InitCvars();

	ConfigInit();

	TheNomad::Util::GetModuleList( TheNomad::SGame::sgame_ModList );
	ConsolePrint( TheNomad::SGame::sgame_ModList.Count() + " total mods registered.\n" );

	@TheNomad::Engine::FileSystem::FileManager = TheNomad::Engine::FileSystem::FileSystemManager();

	@TheNomad::SGame::LevelManager = cast<TheNomad::SGame::LevelSystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::SGame::LevelSystem() ) );
	@TheNomad::SGame::GfxManager = cast<TheNomad::SGame::GfxSystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::SGame::GfxSystem() ) );
	@TheNomad::SGame::EntityManager = cast<TheNomad::SGame::EntitySystem@>( @TheNomad::GameSystem::AddSystem( TheNomad::SGame::EntitySystem() ) );

	TheNomad::GameSystem::Init();

	ConsolePrint( "--------------------\n" );

	return 1;
}

int ModuleOnShutdown() {
	TheNomad::SGame::sgame_ModList.Clear();

	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnShutdown();
		@TheNomad::GameSystem::GameSystems[i] = null;
	}

	TheNomad::SGame::InfoSystem::InfoManager.Clear();

	@TheNomad::SGame::LevelManager = null;
	@TheNomad::SGame::EntityManager = null;
	@TheNomad::Engine::FileSystem::FileManager = null;
	@TheNomad::SGame::GoreManager = null;
	@TheNomad::SGame::GfxManager = null;

	TheNomad::GameSystem::GameSystems.Clear();
	TheNomad::Engine::ResourceCache.ClearCache();

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

void StartupGameLevel() {
	TheNomad::SGame::InitCheatCodes();

	@TheNomad::SGame::InfoSystem::InfoManager = TheNomad::SGame::InfoSystem::InfoDataManager();
	@TheNomad::SGame::StateManager = TheNomad::SGame::EntityStateSystem();

	TheNomad::SGame::ScreenData.Init();

	TheNomad::SGame::StateManager.InitStateCache();

	// load infos
	TheNomad::SGame::InfoSystem::InfoManager.LoadMobInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadItemInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadAmmoInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadWeaponInfos();

	// load assets
	InitResources();
}

int ModuleOnLoadGame() {
	StartupGameLevel();

	TheNomad::GameSystem::IsLoadGameActive = true;
	TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::InLevel;

	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); ++i ) {
		TheNomad::GameSystem::GameSystems[i].OnLevelStart();
	}
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); ++i ) {
		TheNomad::GameSystem::GameSystems[i].OnLoad();
	}

	TheNomad::SGame::LevelManager.CheckNewGamePlus();

	TheNomad::SGame::ScreenData.InitPlayers();
	TheNomad::GameSystem::IsLoadGameActive = false;

	return 1;
}

int ModuleOnLevelStart() {
	StartupGameLevel();
	
	TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::InLevel;
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); ++i ) {
		TheNomad::GameSystem::GameSystems[i].OnLevelStart();
	}

	TheNomad::SGame::ScreenData.InitPlayers();

	return 1;
}

int ModuleOnLevelEnd() {
	for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); ++i ) {
		TheNomad::GameSystem::GameSystems[i].OnLevelEnd();
	}

	@TheNomad::SGame::InfoSystem::InfoManager = null;
	@TheNomad::SGame::StateManager = null;

	TheNomad::Engine::CommandSystem::CmdManager.ClearCommands();

	TheNomad::Engine::ResourceCache.ClearCache();

	return 1;
}

int ModuleOnJoystickEvent( int side, int forward, int up, int roll, int yaw, int pitch ) {
	return 0;
}

int ModuleOnKeyEvent( int key, int down ) {
	return 0;
}

int ModuleOnMouseEvent( int dx, int dy ) {
	return 0;
}
int ModuleOnRunTic( int msec ) {
	switch ( TheNomad::SGame::GlobalState ) {
	case TheNomad::SGame::GameState::InLevel:
	{
		// if we're paused, then just draw the stuff, don't run anything else
		if ( TheNomad::Engine::CvarVariableInteger( "g_paused" ) == 1  ) {
			TheNomad::SGame::LevelManager.Pause();
			return 0;
		}

		TheNomad::GameSystem::DeltaTic = ( msec - TheNomad::GameSystem::GameTic ) * TheNomad::GameSystem::TIMESTEP;
		TheNomad::GameSystem::GameTic = msec;
		TheNomad::GameSystem::GameDeltaTic = TheNomad::GameSystem::GameTic * TheNomad::GameSystem::DeltaTic;
		TheNomad::GameSystem::MousePosition = TheNomad::Engine::GetMousePosition();

		TheNomad::SGame::LevelManager.Resume();
		TheNomad::SGame::EntityManager.SetActivePlayer( @TheNomad::SGame::ScreenData.GetPlayer() );
		for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); ++i ) {
			TheNomad::GameSystem::GameSystems[i].OnRunTic();
		}

		TheNomad::SGame::ScreenData.Draw();
		
		return 0;
	}
	case TheNomad::SGame::GameState::StatsMenu:
		TheNomad::SGame::LevelManager.OnRenderScene();
		return 2;
	case TheNomad::SGame::GameState::LevelFinish:
		// this will automatically call OnLevelEnd for all registered game objects
		TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::StatsMenu;
		return 1;
	case TheNomad::SGame::GameState::EndOfLevel:
		TheNomad::SGame::GlobalState = TheNomad::SGame::GameState::Inactive;
		return 3;
	case TheNomad::SGame::GameState::Inactive:
	default:
		return 0;
	};
}

};