#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class ItemInfo : InfoLoader {
		ItemInfo() {
		}
		
		bool Load( json@ json ) {
			string str;

			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Type", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Type'\n" );
				return false;
			} else {
				if ( !InfoManager.GetItemTypes().TryGetValue( str, this.type ) ) {
					GameError( "invalid item info, Type \"" + str + "\" wasn't found" );
				}
			}
			if ( !json.get( "Effect", effect ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Effect'\n" );
				return false;
			}
			if ( !json.get( "Cost", cost ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Cost'\n" );
				return false;
			}
			if ( !json.get( "Shader", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Shader'\n" );
				return false;
			} else {
				hShader = TheNomad::Engine::Renderer::RegisterShader( str );
			}
			if ( !json.get( "SpriteOffsetX", spriteOffsetX ) ) {
				ConsoleWarning( "invalid item info, missing variable 'SpriteOffsetX'\n" );
				return false;
			}
			if ( !json.get( "SpriteOffsetY", spriteOffsetY ) ) {
				ConsoleWarning( "invalid item info, missing variable 'SpriteOffsetY'\n" );
				return false;
			}
			if ( !json.get( "PickupSfx", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'PickupSfx'\n" );
				return false;
			}
			if ( !json.get( "UseSfx", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'UseSfx'\n" );
				return false;
			}

			TheNomad::GameSystem::GetString( name + "_DESC", description );

			ConsolePrint( "Loaded item info for '" + name + "'\n" );

			return true;
		}

		string name;
		string description;
		string effect;
		uint type = 0;
		int cost = 0;
		int hShader = FS_INVALID_HANDLE;
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;
		uint maxStackSize = 0;
		TheNomad::Engine::SoundSystem::SoundEffect pickupSfx;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx;
	};
};