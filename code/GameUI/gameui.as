#include "Menu/MenuFramework.as"

namespace gameui {

int ModuleOnInit() {
	return 1;
}

int ModuleOnShutdown() {
	return 1;
}

int ModuleOnConsoleCommand() {
	return 0;
}

int ModuleOnSaveGame() {
	return 1;
}

int ModuleOnLoadGame() {
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

int ModuleOnMouseEvent( int dx, int dy ) {
	return 0;
}

int ModuleOnRunTic( int msec ) {
	return 0;
}

};