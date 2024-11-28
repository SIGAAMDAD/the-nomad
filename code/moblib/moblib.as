#include "moblib/AIManager.as"

namespace moblib {
	void InitCvars() {
	}

	void InitResources() {
	}

	int ModuleOnInit() {
		//
		// register cvars
		//
		ConsolePrint( "Initializing AISystem...\n" );

		InitCvars();

		@AIManager = cast<AISystem@>( @TheNomad::GameSystem::AddSystem( AISystem() ) );

		//
		// load assets
		//
		InitResources();

		return 1;
	}

	int ModuleOnShutdown() {
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