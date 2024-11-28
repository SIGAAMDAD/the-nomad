#include "hellbreaker/HellBreakerSystem.as"
#include "nomadmain/Engine/ConVar.as"

namespace hellbreaker {
	TheNomad::Engine::ConVar sgame_HellbreakerActive;

	void InitCvars() {
	}

	void InitResources() {
	}

	int ModuleOnInit() {
		//
		// register cvars
		//
		ConsolePrint( "Initializing Hellbreaker...\n" );

		InitCvars();

		@HellBreaker = cast<HellBreakerSystem@>( @TheNomad::GameSystem::AddSystem( HellBreakerSystem() ) );

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