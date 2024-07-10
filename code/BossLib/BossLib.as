namespace BossLib {
    int ModuleOnInit() {
        ConsolePrint( "BossLib_Init(): initializing mob bosses...\n" );

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
};