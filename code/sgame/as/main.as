#include "util/detail.as"
#include "game.as"
#include "config.as"
#include "level.as"

string s_DebugMsg;

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

//
// InitConstants: initializes all variables that won't really change throughout execution
//
void InitConstants() {
	DebugPrint( "Initializing SGame Constants...\n" );

	string str;

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

		TheNomad::GameSystem::GetString( "SP_AMMO_BULLET", TheNomad::SGame::AmmoTypeStrings[ TheNomad::SGame::AmmoType::Bullet ] );
		TheNomad::GameSystem::GetString( "SP_AMMO_SHELL", TheNomad::SGame::AmmoTypeStrings[ TheNomad::SGame::AmmoType::Shell ] );
		TheNomad::GameSystem::GetString( "SP_AMMO_ROCKET", TheNomad::SGame::AmmoTypeStrings[ TheNomad::SGame::AmmoType::Rocket ] );
		TheNomad::GameSystem::GetString( "SP_AMMO_GRENADE", TheNomad::SGame::AmmoTypeStrings[ TheNomad::SGame::AmmoType::Grenade ] );

		TheNomad::GameSystem::GetString( "SP_ARMOR_NONE", TheNomad::SGame::ArmorTypeStrings[ TheNomad::SGame::ArmorType::None ] );
		TheNomad::GameSystem::GetString( "SP_ARMOR_LIGHT", TheNomad::SGame::ArmorTypeStrings[ TheNomad::SGame::ArmorType::Light ] );
		TheNomad::GameSystem::GetString( "SP_ARMOR_STANDARD", TheNomad::SGame::ArmorTypeStrings[ TheNomad::SGame::ArmorType::Standard ] );
		TheNomad::GameSystem::GetString( "SP_ARMOR_HEAVY", TheNomad::SGame::ArmorTypeStrings[ TheNomad::SGame::ArmorType::Heavy ] );
		TheNomad::GameSystem::GetString( "SP_ARMOR_INVUL", TheNomad::SGame::ArmorTypeStrings[ TheNomad::SGame::ArmorType::Invul ] );

		TheNomad::GameSystem::GetString( "SP_ATK_METHOD_HITSCAN", TheNomad::SGame::AttackMethodStrings[ TheNomad::SGame::AttackMethod::Hitscan ] );
		TheNomad::GameSystem::GetString( "SP_ATK_METHOD_PROJECTILE", TheNomad::SGame::AttackMethodStrings[ TheNomad::SGame::AttackMethod::Projectile ] );
		TheNomad::GameSystem::GetString( "SP_ATK_METHOD_AOE", TheNomad::SGame::AttackMethodStrings[ TheNomad::SGame::AttackMethod::AreaOfEffect ] );

		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_SIDE", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::Sidearm ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_HSIDE", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::HeavySidearm ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_PRIM", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::Primary ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_HPRIM", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::HeavyPrimary ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_GRENADIER", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::Grenadier ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_MELEE", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::Melee ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_LARM", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::LeftArm ] );
		TheNomad::GameSystem::GetString( "SP_WEAPON_TYPE_RARM", TheNomad::SGame::WeaponTypeStrings[ TheNomad::SGame::WeaponType::RightArm ] );
	}
}

void InitCvars() {
	ConsolePrint( "Registering SGame Cvars...\n" );

	@TheNomad::SGame::sgame_MaxEntities = TheNomad::CvarManager.AddCvar( "sgame_MaxEntities", "500", CVAR_LATCH | CVAR_SAVE, true );
	@TheNomad::SGame::sgame_NoRespawningMobs = TheNomad::CvarManager.AddCvar( "sgame_NoRespawningMobs", "0", CVAR_LATCH | CVAR_SAVE, false );
	@TheNomad::SGame::sgame_HellbreakerActive = TheNomad::CvarManager.AddCvar( "sgame_HellbreakerActive", "0", CVAR_TEMP, true );
	@TheNomad::SGame::sgame_HellbreakerOn = TheNomad::CvarManager.AddCvar( "sgame_HellbreakerOn", "0", CVAR_TEMP, true );
	@TheNomad::SGame::sgame_GfxDetail = TheNomad::CvarManager.AddCvar( "sgame_GfxDetail", "10", CVAR_SAVE, false );
	@TheNomad::SGame::sgame_Difficulty = TheNomad::CvarManager.AddCvar( "sgame_Difficulty", "2", CVAR_TEMP, false );
	@TheNomad::SGame::sgame_DebugMode = TheNomad::CvarManager.AddCvar( "sgame_DebugMode", "0", CVAR_TEMP, true );
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
}

int ModuleInit() {
	ConsolePrint( "----- SG_Init -----\n" );

//	InfoInit();

	@TheNomad::CvarManager = cast<TheNomad::CvarSystem>( TheNomad::GameSystem::AddSystem( TheNomad::CvarSystem() ) );

	//
	// register cvars
	//
	InitCvars();

	TheNomad::Util::GetModuleList( TheNomad::SGame::sgame_ModList );
	ConsolePrint( TheNomad::SGame::sgame_ModList.Count() + " total mods registered.\n" );

	//
	// init constants
	//
	InitConstants();

	@TheNomad::Engine::SoundSystem::SoundManager = TheNomad::Engine::SoundSystem::SoundScene();

	ModuleConfigInit();

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
	for ( uint i = 0; i < GameObjects.Count(); i++ ) {
		if ( GameObjects[i].OnConsoleCommand( TheNomad::Engine::CmdArgv( 0 ) ) ) {
			return 1;
		}
	}
	return 0;
}

int ModuleOnSaveGame() {
	ConsolePrint( "Saving game, please do not close out of app...\n" );

	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.Count(); i++ ) {
		GameObjects[i].OnSave();
	}

	ConsolePrint( "Done.\n" );

	return 1;
}

int ModuleOnLoadGame() {
	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.Count(); i++ ) {
		GameObjects[i].OnLoad();
	}
	return 1;
}

int ModuleOnLevelStart() {
	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.Count(); i++ ) {
		GameObjects[i].OnLevelStart();
	}

	return 1;
}

int ModuleOnLevelEnd() {
	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.Count(); i++ ) {
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
	TheNomad::GameSystem::GameManager.SetMousePos( ivec2( dx, dy ) );
	return 0;
}

int ModuleOnRunTic( uint msec )
{
	TheNomad::GameSystem::GameManager.SetMsec( msec );

	TheNomad::Engine::Renderer::ClearScene();

	const uint flags = RSF_ORTHO_TYPE_WORLD;

	array<TheNomad::GameSystem::GameObject@>@ GameObjects = @TheNomad::GameSystem::GameSystems;
	for ( uint i = 0; i < GameObjects.Count(); i++ ) {
		GameObjects[i].OnRunTic();
	}

	TheNomad::Engine::Renderer::RenderScene( 0, 0, TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
		TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight, flags, TheNomad::GameSystem::GameManager.GetGameMsec() );

	return 0;
}

void DebugPrint( const string& in msg ) {
	if ( TheNomad::SGame::sgame_DebugMode.GetInt() == 0 ) {
		return;
	}

	s_DebugMsg = COLOR_GREEN;
	s_DebugMsg += "DEBUG: ";
	s_DebugMsg += msg;
	ConsolePrint( s_DebugMsg );
}
