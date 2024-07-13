#include "MobLib/Mobs/ZurgutGrunt.as"

namespace MobLib {
    int ModuleOnInit() {
        ConsolePrint( "MobLib_Init(): initializing mob data...\n" );

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

    int ModuleOnRunTic( int msec ) {
        return 1;
    }
};