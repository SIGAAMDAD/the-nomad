
//
// ModuleInit: initialize all your variables here
//
int ModuleInit()
{
    ConsolePrint( "SGameInit: initializing sgame...\n" );
    return 1;
}

int ModuleShutdown()
{
    ConsolePrint( "SGameShutdown: shutting down sgame...\n" );
    return 1;
}

int ModuleOnRunTic( uint frametime, uint msec )
{
    return 1;
}

int ModuleOnKeyEvent( uint key, uint down )
{
    return 0;
}

int ModuleOnSaveGame()
{
    return 0;
}

int ModuleOnLoadGame()
{
    return 0;
}

int ModuleOnMouseEvent( uint dx, uint dy )
{
    return 0;
}

int ModuleOnCommand( void )
{
    return 0;
}
