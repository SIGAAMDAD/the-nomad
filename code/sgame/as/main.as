
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
	ConsolePrint( "SGameInit: initializing sgame module...\n" );

	ModuleInfoInit();
	return 1;
}

int ModuleShutdown()
{
	return 1;
}

int ModuleOnKeyEvent( int key, int down )
{
	return 1;
}

int ModuleOnMouseEvent( int dx, int dy )
{
	return 1;
}

int ModuleOnLevelStart() {
	return 1;
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
//	const string& savename = TheNomad::SGame::sg_savename.GetString();
//	const string& newsave = TheNomad::Engine::CvarVariableString( "sg_savename" );
//	
//	if ( savename == newsave ) {
//		ConsolePrint( "Refusing to load the same save file.n" );
//		return 0;
//	}
//	
//	TheNomad::SGame::sg_savename.Update();
//	ConsolePrint( "Loading save file \"" + TheNomad::SGame::sg_savename.GetString() + "\"\n" );
//	
//	for ( int i = 0; i < TheNomad::GameSystem::GameSystems.size(); i++ ) {
//		TheNomad::GameSystem::GameSystems[i].OnLoad();
//	}
//	
	return 1;
}

int ModuleOnRunTic()
{
	return 1;
}
