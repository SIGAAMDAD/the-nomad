#include "game.as"
#include "config.as"
#include "level.as"

void ModuleInfoInit() {
	ConsolePrint( "[Module Info]\n" );
	ConsolePrint( "name: " + MODULE_NAME + "\n" );
	ConsolePrint( "version: v" + formatInt( MODULE_VERSION_MAJOR ) + "." + formatInt( MODULE_VERSION_UPDATE ) + "."
		+ formatInt( MODULE_VERSION_PATCH ) + "\n" );
}

int ModuleInit()
{
	ConsolePrint( "----- SG_Init -----\n" );

	ModuleInfoInit();

	//
	// register cvars
	//
	@TheNomad::SGame::sgame_Difficulty = TheNomad::CvarManager.AddCvar( "sgame_Difficulty", "2", CVAR_LATCH | CVAR_TEMP, false );
	@TheNomad::SGame::sgame_SaveName = TheNomad::CvarManager.AddCvar( "sgame_SaveName", "nomadsv.ngd", CVAR_LATCH | CVAR_TEMP | CVAR_PROTECTED, false );
	@TheNomad::SGame::sgame_DebugMode = TheNomad::CvarManager.AddCvar( "sgame_DebugMode", "0", CVAR_LATCH | CVAR_TEMP | CVAR_PROTECTED, true );

	if ( TheNomad::SGame::sgame_DebugMode.GetInt() ) {
		@TheNomad::SGame::sgame_LevelDebugPrint = TheNomad::CvarManager.AddCvar(
			"sgame_LevelDebugPrint", "1", CVAR_LATCH | CVAR_TEMP | CVAR_PROTECTED, true );
	} else {
		@TheNomad::SGame::sgame_LevelDebugPrint = TheNomad::CvarManager.AddCvar(
			"sgame_LevelDebugPrint", "0", CVAR_LATCH | CVAR_TEMP | CVAR_PROTECTED, true );
	}

	// init gamesystem
	TheNomad::GameSystem::Init();

	ConsolePrint( "--------------------\n" );
	return 1;
}

int ModuleShutdown()
{
	ConsolePrint( "----- SG_Shutdown -----\n" );

	ConsolePrint( "-----------------------\n" );

	return 1;
}

int ModuleOnKeyEvent( uint key, uint down ) {
	return 1;
}

int ModuleOnMouseEvent( uint dx, uint dy ) {
	return 1;
}

int ModuleOnLevelStart( uint index ) {
	ConsolePrint( "Starting level at index " + index + "\n" );

	return 1;
}

int ModuleOnConsoleCommand() {
	return 0;
}

int ModuleOnLevelEnd() { 
	return 1;
}

int ModuleOnSaveGame()
{

	ConsolePrint( "Saving Game, Please Do Close This App...\n" );
	
	
	return 1;
}

int ModuleOnLoadGame()
{
//	const string& savename = TheNomad::SGame::sgame_SaveName.GetString();
//	const string& newsave = TheNomad::CvarVariableString( "sg_savename" );
	
//	if ( savename == newsave ) {
//		ConsolePrint( "Refusing to load the same save file.\n" );
//		return 0;
//	}
//	
//	TheNomad::SGame::sgame_SaveName.Update();
//	ConsolePrint( "Loading save file \"" + TheNomad::SGame::sgame_SaveName.GetString() + "\"\n" );
//	
//	for ( int i = 0; i < TheNomad::GameSystem::GameSystems.size(); i++ ) {
//		TheNomad::GameSystem::GameSystems[i].OnLoad();
//	}
	
	return 1;
}

int ModuleOnRunTic( uint msec )
{
	

	return 1;
}
