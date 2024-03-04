#include "game.as"
#include "config.as"
#include "level.as"

shared class ModuleInfo {
	ModuleInfo() {
	}

	string version;
	string name;

	int versionMajor;
	int versionUpdate;
	int versionPatch;
};

ModuleInfo moduleInfo;

void ModuleInfoInit() {
	moduleInfo.versionMajor = MODULE_VERSION_MAJOR;
	moduleInfo.versionUpdate = MODULE_VERSION_UPDATE;
	moduleInfo.versionPatch = MODULE_VERSION_PATCH;

	moduleInfo.name = MODULE_NAME;
	moduleInfo.version = formatUInt( moduleInfo.versionMajor ) + formatUInt( moduleInfo.versionUpdate )
		+ formatUInt( moduleInfo.versionPatch );
	
	ConsolePrint( "[Module Info]\n" );
	ConsolePrint( "name: " + moduleInfo.name + "\n" );
	ConsolePrint( "version: " + moduleInfo.version + "\n" );
}

int ModuleInit()
{
	ConsolePrint( "----- SG_Init -----\n" );

	ModuleInfoInit();

	//
	// register cvars
	//
	TheNomad::GameSystem::sg_difficulty = TheNomad::ConVar( "sg_difficulty", "2", CVAR_LATCH | CVAR_TEMP, false );


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

int ModuleOnKeyEvent( int key, int down ) {
	return 1;
}

int ModuleOnMouseEvent( int dx, int dy ) {
	return 1;
}

int ModuleOnLevelStart( int index ) {
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
	const string& savename = TheNomad::SGame::sgame_SaveName.GetString();
	const string& newsave = TheNomad::CvarVariableString( "sg_savename" );
	
	if ( savename == newsave ) {
		ConsolePrint( "Refusing to load the same save file.\n" );
		return 0;
	}
	
	TheNomad::SGame::sgame_SaveName.Update();
	ConsolePrint( "Loading save file \"" + TheNomad::SGame::sgame_SaveName.GetString() + "\"\n" );
	
	for ( int i = 0; i < TheNomad::GameSystem::GameSystems.size(); i++ ) {
		TheNomad::GameSystem::GameSystems[i].OnLoad();
	}
	
	return 1;
}

int ModuleOnRunTic()
{
	return 1;
}
