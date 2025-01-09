#include "moblib/MobScript.as"

// include your own mobs here
#include "moblib/Scripts/Resources.as"
#include "moblib/Scripts/MercShotty.as"
#include "moblib/Scripts/MercGatling.as"
#include "moblib/Scripts/ZurgutGrunt.as"

namespace moblib::Script {
	// modify these if you want to add your own mobs
	enum MOB_ID {
		MERC_GATLING,
		MERC_SHOTGUNNER,
		ZURGUT_GRUNT,
		ZURGUT_HULK,
	};

	MobScript@ AllocateScriptEntity( uint nMobID ) {
		MobScript@ scriptObject = null;

		switch ( nMobID ) {
		case MOB_ID::MERC_GATLING:
			@scriptObject = MercGatling();
			break;
		case MOB_ID::MERC_SHOTGUNNER:
			@scriptObject = MercShotty();
			break;
		case MOB_ID::ZURGUT_GRUNT:
			@scriptObject = ZurgutGrunt();
			break;
		case MOB_ID::ZURGUT_HULK:
			break;
		default:
			GameError( "AllocateScriptEntity: invalid mob id '" + nMobID + "'!" );
		};

		return @scriptObject;
	}
};