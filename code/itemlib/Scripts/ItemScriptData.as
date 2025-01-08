#include "itemlib/ItemScript.as"

// include your own items/weapons here
#include "itemlib/Scripts/StimPack.as"
#include "itemlib/Scripts/Scenery.as"
#include "itemlib/Scripts/Tutorial.as"
#include "itemlib/Scripts/ShellsPickup.as"

namespace itemlib::Script {
	// modify these if you want to add your own items
	enum ITEM_ID {
		STIMPACK,
		AMMOPACK,
		SHELLPACK,
		GRENADEPACK,
		ROCKETPACK,
		
		POWERUP_A,
		POWERUP_B,

		SCENERY_TENT,
		SCENERY_OUTCROP,

		WEAPON_PICKUP,
		SHELLS_PICKUP,

		TUTORIAL_DASHING,
		TUTORIAL_JUMPING,
		TUTORIAL_SLIDING,
		TUTORIAL_CHECKPOINTS,
		TUTORIAL_ITEM_PICKUPS,
		TUTORIAL_PARRYING,
		TUTORIAL_RAGE,
		TUTORIAL_BULLET_TIME,
		TUTORIAL_FOR_TUTORIALS,
		TUTORIAL_FACING,
		TUTORIAL_GUNS
	};

	ItemScript@ AllocateScriptEntity( uint nItemID ) {
		ItemScript@ scriptObject = null;

		switch ( nItemID ) {
		case ITEM_ID::STIMPACK:
			@scriptObject = StimPack();
			break;
		case ITEM_ID::AMMOPACK:
//			@scriptObject = AmmoPack();
			break;
		case ITEM_ID::SHELLPACK:
		case ITEM_ID::GRENADEPACK:
		case ITEM_ID::ROCKETPACK:
			break;
		case ITEM_ID::POWERUP_A:
		case ITEM_ID::POWERUP_B:
			break;
		case ITEM_ID::SCENERY_TENT:
		case ITEM_ID::SCENERY_OUTCROP:
			@scriptObject = Scenery();
			break;
		
		case ITEM_ID::SHELLS_PICKUP:
			@scriptObject = ShellsPickup();
			break;

		// tutorial popups
		case ITEM_ID::TUTORIAL_DASHING:
		case ITEM_ID::TUTORIAL_JUMPING:
		case ITEM_ID::TUTORIAL_SLIDING:
		case ITEM_ID::TUTORIAL_CHECKPOINTS:
		case ITEM_ID::TUTORIAL_ITEM_PICKUPS:
		case ITEM_ID::TUTORIAL_PARRYING:
		case ITEM_ID::TUTORIAL_RAGE:
		case ITEM_ID::TUTORIAL_BULLET_TIME:
		case ITEM_ID::TUTORIAL_FOR_TUTORIALS:
		case ITEM_ID::TUTORIAL_FACING:
		case ITEM_ID::TUTORIAL_GUNS:
			@scriptObject = @Tutorial();
			break;

		default:
			GameError( "AllocateScriptEntity: invalid item id '" + nItemID + "'!" );
		};

		return @scriptObject;
	}
};