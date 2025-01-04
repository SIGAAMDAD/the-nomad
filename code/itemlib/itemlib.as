#include "nomadmain/SGame/InfoSystem/InfoDataManager.as"
#include "itemlib/ItemScript.as"
#include "itemlib/Scripts/ItemScriptData.as"

namespace itemlib {
	void InitCvars() {
	}

	void InitResources() {
		Script::LoadTutorialData();
	}
	
	void AllocScript( TheNomad::SGame::ItemObject@ item ) {
		ItemScript@ script = itemlib::Script::AllocateScriptEntity( item.GetItemInfo().type );
		item.LinkScript( @script );
		script.Link( @item );
	}
	void AllocWeapon( TheNomad::SGame::WeaponObject@ weapon, uint id ) {
		ItemScript@ script = itemlib::Script::AllocateScriptEntity( id );
		weapon.LinkScript( @script );
		script.Link( cast<TheNomad::SGame::ItemObject@>( @weapon ) );
	}

	int ModuleOnInit() {
		//
		// register cvars
		//
		ConsolePrint( "Initializing ItemSystem...\n" );

		InitCvars();

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