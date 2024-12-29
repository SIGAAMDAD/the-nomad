#include "SGame/InfoSystem/InfoDataManager.as"

namespace TheNomad::SGame::InfoSystem {
    class ItemInfo : InfoLoader {
		ItemInfo() {
		}

		bool LoadSoundsBlock( json@ json ) {
			string str;

			if ( !json.get( "Sounds.PickupSfx", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Sounds.PickupSfx' in \"" + name + "\"\n" );
				return false;
			} else {
				pickupSfx = TheNomad::Engine::SoundSystem::RegisterSfx( string( json[ "Sounds.PickupSfx" ] ) );
			}
			if ( !json.get( "Sounds.UseSfx", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Sounds.UseSfx' in \"" + name + "\"\n" );
				return false;
			} else {
				useSfx = TheNomad::Engine::SoundSystem::RegisterSfx( string( json[ "Sounds.UseSfx" ] ) );
			}
			if ( !json.get( "Sounds.EquipSfx", str ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Sounds.EquipSfx' in \"" + name + "\"\n" );
				return false;
			} else {
				equipSfx = TheNomad::Engine::SoundSystem::RegisterSfx( string( json[ "Sounds.EquipSfx" ] ) );
			}

			return true;
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
			
			if ( !LoadSoundsBlock( @json ) ) {
				return false;
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

			return true;
		}

		string name;
		int iconShader = FS_INVALID_HANDLE;
		uint type = 0;
		uint maxStackSize = 0;
		vec2 size = vec2( 0.0f );

		TheNomad::Engine::SoundSystem::SoundEffect pickupSfx;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx;
		TheNomad::Engine::SoundSystem::SoundEffect equipSfx;
	};
};