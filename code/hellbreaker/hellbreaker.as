namespace TheNomad::SGame {
    void HellbreakerInit() {
    }
    bool HellbreakerRunTic() {
        return false;
    }
};

int ModuleInit() {
    return 1;
}

int ModuleShutdown() {
    return 1;
}

int ModuleOnLevelStart() {
    return 1;
}

int ModuleOnLevelEnd() {
    return 1;
}

int ModuleOnKeyEvent( uint key, uint down ) {
	return 1;
}

int ModuleOnMouseEvent( int dx, int dy ) {
	return 1;
}

int ModuleOnRunTic( uint msec ) {
    return 1;
}

int ModuleOnConsoleCommand() {
    return 1;
}
