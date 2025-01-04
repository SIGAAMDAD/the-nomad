#include "nomadmain/SGame/InfoSystem/InfoDataManager.as"
#include "moblib/MobScript.as"
#include "moblib/Scripts/ScriptData.as"

namespace moblib {
	void InitCvars() {
	}

	void AllocScript( TheNomad::SGame::MobObject@ mob ) {
		MobScript@ script = Script::AllocateScriptEntity( mob.GetMobInfo().type );
		mob.LinkScript( @script );
		script.Link( @mob );
	}

	int ModuleOnInit() {
		//
		// register cvars
		//
		ConsolePrint( "Initializing AISystem...\n" );

		InitCvars();

		@Script::ResourceCache = cast<moblib::Script::Resources@>( @TheNomad::GameSystem::AddSystem( moblib::Script::Resources() ) );

		return 1;
	}

	int ModuleOnShutdown() {
		@Script::ResourceCache = null;
		return 1;
	}

	int ModuleOnLevelStart() {
		return 1;
	}

	int ModuleOnLevelEnd() {
		return 1;
	}

	int ModuleOnKeyEvent( int key, int down ) {
		return 0;
	}

	int ModuleOnRunTic( int msec ) {
		return 0;
	}
};