#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class ItemInfo : InfoLoader {
		ItemInfo() {
		}

		bool LoadStatsBlock( json@ json ) {
			if ( !json.get( "Stats.Width", size.x ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Stats.Width' in \"" + name + "\"\n" );
				return false;
			}
			size.x = float( json[ "Stats.Width" ] );

			if ( !json.get( "Stats.Height", size.y ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Stats.Height' in \"" + name + "\"\n" );
				return false;
			}
			size.y = float( json[ "Stats.Height" ] );

			return true;
		}

		void LoadFlags( json@ json ) {
			array<json@> flagList;

			if ( !json.get( "Flags", flagList ) ) {
				return; // not required
			}
			for ( uint i = 0; i < flagList.Count(); ++i ) {
				const string data = string( flagList[i] );
				if ( data == "NoOwner" ) {
					flags |= uint( ItemFlags::NoOwner );
				}
			}
		}
		
		bool Load( json@ json ) {
			string str;
			EntityData@ entity = null;

			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Id", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Id' in \"" + name + "\"\n" );
				return false;
			} else {
				if ( ( @entity = @InfoManager.GetItemType( str ) ) !is null ) {
					type = entity.GetID();
				} else {
					GameError( "invalid item info, Type \"" + str + "\" wasn't found" );
				}
			}
			
			if ( !LoadStatsBlock( @json ) ) {
				return false;
			}

			if ( !json.get( "RenderData.Icon", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'RenderData.Icon'\n" );
				return false;
			} else {
				iconShader = TheNomad::Engine::Renderer::RegisterShader( string( json[ "RenderData.Icon" ] ) );
			}

			// load optional flags
			LoadFlags( @json );

			return true;
		}

		string name;
		vec2 size = vec2( 0.0f );
		uint flags = 0;
		int iconShader = FS_INVALID_HANDLE;
		uint type = 0;
		uint maxStackSize = 0;

		TheNomad::Engine::SoundSystem::SoundEffect pickupSfx;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx;
		TheNomad::Engine::SoundSystem::SoundEffect equipSfx;
	};
};