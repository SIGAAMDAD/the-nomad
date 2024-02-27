int Module_Init()
{
    ConsolePrint( "SGameInit: initializing sgame...\n" );
    return 1;
}

int Module_Shutdown()
{
    ConsolePrint( "SGameShutdown: shutting down sgame...\n" );
    return 1;
}

int Module_RunTic( uint frametime, uint msec )
{
    return 1;
}

int Module_OnKeyEvent( uint key, uint down )
{
    return 0;
}

int Module_OnSaveGame()
{
    return 0;
}

int Module_LoadGame()
{
    return 0;
}

int Module_OnMouseEvent( uint dx, uint dy )
{
    return 0;
}

int Module_CommandLine( void )
{
    return 0;
}

void main()
{

}
