#include "SGame/InfoSystem/InfoLoader.as"
#include "SGame/InfoSystem/MobInfo.as"
#include "SGame/InfoSystem/ItemInfo.as"
#include "SGame/InfoSystem/WeaponInfo.as"
#include "SGame/InfoSystem/AmmoInfo.as"

namespace TheNomad::SGame::InfoSystem {
	class EntityData {
		EntityData() {
		}
		EntityData( const string& in name, uint id ) {
			m_Name = name;
			m_ID = id;
		}
		
		bool opEquals( const string& in name ) const {
			return m_Name == name;
		}
		bool opEquals( uint id ) const {
			return m_ID == id;
		}
		const string& GetName() const {
			return m_Name;
		}
		uint GetID() const {
			return m_ID;
		}
		
		private string m_Name;
		private uint m_ID = 0;
	};
	
    class InfoDataManager {
		InfoDataManager() {
			ConsolePrint( "Loading mod info files...\n" );

			LoadEntityIds();
			
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.DumpCache_f ), "sgame.print_entity_cache", true
			);

			if ( sgame_DebugMode.GetInt() == 1 ) {
				DumpCache_f();
			}
		}

		private void DumpTypeList( const string& in name, const array<EntityData>& in typeList ) const {
			ConsolePrint( "\n[" + name + "]\n" );
			for ( uint i = 0; i < typeList.Count(); i++ ) {
				ConsolePrint( "(" + typeList[i].GetID() + ") " + typeList[i].GetName() + "\n" );
			}
		}
		void DumpCache_f() const {
			DumpTypeList( "MobTypes", m_MobTypes );
			DumpTypeList( "ItemTypes", m_ItemTypes );
			DumpTypeList( "WeaponTypes", m_WeaponTypes );
			DumpTypeList( "AmmoTypes", m_AmmoTypes );
		}

		void Clear() {
			m_ItemTypes.Clear();
			m_MobTypes.Clear();
			m_WeaponTypes.Clear();
			m_AmmoTypes.Clear();

			m_ItemInfos.Clear();
			m_MobInfos.Clear();
			m_WeaponInfos.Clear();
			m_AmmoInfos.Clear();
		}

		private array<json@>@ LoadJSonFile( string& in path, const string& in modName, const string& in fileName,
			const string& in ArrayName )
		{
			json@ data;
			array<json@> values;
			
			path = "modules/" + modName + "/DataScripts/" + fileName;

			@data = json();
			if ( !data.ParseFile( path ) ) {
				return null;
			}
			if ( !data.get( ArrayName, values ) ) {
				ConsoleWarning( "info file '" + fileName + "' found, but no infos defined, skipping...\n" );
				return null;
			}
			
			return @values;
		}
		
		private void LoadEntityIds() {
			json@ obj;
			uint a, id;
			string name;
			
			json@ data = json();
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				const string path = "modules/" + sgame_ModList[i] + "/DataScripts/entitydata.json";
				array<json@> values;
				
				if ( !data.ParseFile( path ) ) {
					ConsoleWarning( "failed to load entity data info file for \"" + sgame_ModList[i] + "\"\n" );
					return;
				}

				//
				// load the mob infos
				//
				if ( !data.get( "MobData", values ) ) {
					ConsoleWarning( "entity data info file for \"" + sgame_ModList[i] + "\" has no mob data\n" );
				} else {
					m_MobTypes.Reserve( values.Count() );
					for ( a = 0; a < values.Count(); a++ ) {
						m_MobTypes.Add( EntityData( string( values[a][ "Name" ] ), uint( values[a][ "Id" ] ) ) );
					}
				}
				DebugPrint( "Loaded " + m_MobTypes.Count() + " mob types.\n" );

				//
				// load the ammo infos
				//
				if ( !data.get( "AmmoData", values ) ) {
					ConsoleWarning( "entity data info file for \"" + sgame_ModList[i] + "\" has no ammo data\n" );
				} else {
					m_AmmoTypes.Reserve( values.Count() );
					for ( a = 0; a < values.Count(); a++ ) {
						m_AmmoTypes.Add( EntityData( string( values[a][ "Name" ] ), uint( values[a][ "Id" ] ) ) );
					}
				}
				DebugPrint( "Loaded " + m_AmmoTypes.Count() + " ammo types.\n" );

				//
				// load item infos
				//
				if ( !data.get( "ItemData", values ) ) {
					ConsoleWarning( "entity data info file for \"" + sgame_ModList[i] + "\" has no item data\n" );
				} else {
					m_ItemTypes.Reserve( values.Count() );
					for ( a = 0; a < values.Count(); a++ ) {
						m_ItemTypes.Add( EntityData( string( values[a][ "Name" ] ), uint( values[a][ "Id" ] ) ) );
					}
				}
				DebugPrint( "Loaded " + m_ItemTypes.Count() + " item types.\n" );

				//
				// load weapon infos
				//
				if ( !data.get( "WeaponData", values ) ) {
					ConsoleWarning( "entity data info file for \"" + sgame_ModList[i] + "\" has no weapon data\n" );
				} else {
					m_WeaponTypes.Reserve( values.Count() );
					for ( a = 0; a < values.Count(); a++ ) {
						m_WeaponTypes.Add( EntityData( string( values[a][ "Name" ] ), uint( values[a][ "Id" ] ) ) );
					}
				}
				DebugPrint( "Loaded " + m_WeaponTypes.Count() + " weapon datas.\n" );
			}
		}
		
		void LoadMobInfos() {
			string path;

			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading mob infos from module \"" + sgame_ModList[i] + "\"...\n" );

				array<json@>@ infos = @LoadJSonFile( path, sgame_ModList[i], "mobs.json", "MobInfo" );
				if ( @infos is null ) {
					continue;
				}
				ConsolePrint( "Got " + infos.Count() + " mob infos.\n" );
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					MobInfo@ info = null;
					const string id = string( infos[a][ "Id" ] );
					bool added = false;

					if ( m_MobInfos.Contains( id ) ) {
						@info = cast<MobInfo@>( @m_MobInfos[ id ] );
						if ( @info is null ) {
							GameError( "MobInfo \"" + id + "\"" );
						}
						added = true;
					} else {
						@info = MobInfo();
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load mob info " + id + "\n" );
						@info = null;
						continue;
					}
					if ( !added ) {
						m_MobInfos.Add( id, @info );
					}
				}
			}
		}
		
		void LoadItemInfos() {
			string path;

			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading item infos from module \"" + sgame_ModList[i] + "\"...\n" );

				array<json@>@ infos = @LoadJSonFile( path, sgame_ModList[i], "items.json", "ItemInfo" );
				if ( @infos is null ) {
					continue;
				}
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					ItemInfo@ info = null;
					const string id = string( infos[a][ "Id" ] );
					bool added = false;
					
					if ( m_ItemInfos.Contains( id ) ) {
						@info = cast<ItemInfo@>( @m_ItemInfos[ id ] );
						if ( @info is null ) {
							GameError( "ItemInfo \"" + id + "\" is null!" );
						}
						added = true;
					} else {
						@info = ItemInfo();
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load item info " + id + "\n" );
						@info = null;
						continue;
					}
					if ( !added ) {
						m_ItemInfos.Add( id, @info );
					}
				}
				@infos = null;
			}
		}

		void LoadAmmoInfos() {
			string path;

			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading ammo infos from module \"" + sgame_ModList[i] + "\"...\n" );

				array<json@>@ infos = @LoadJSonFile( path, sgame_ModList[i], "ammo.json", "AmmoInfo" );
				if ( @infos is null ) {
					continue;
				}
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					AmmoInfo@ info = null;
					const string id = string( infos[a][ "Id" ] );
					bool added = false;
					
					if ( m_AmmoInfos.Contains( id ) ) {
						@info = cast<AmmoInfo@>( @m_AmmoInfos[ id ] );
						if ( @info is null ) {
							GameError( "AmmoInfo \"" + id + "\" is null!" );
						}
						added = true;
					} else {
						@info = AmmoInfo();
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load ammo info " + id + "\n" );
						@info = null;
						continue;
					}
					if ( !added ) {
						m_AmmoInfos.Add( id, @info );
					}
				}
			}
		}
		
		void LoadWeaponInfos() {
			string path;

			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading weapon infos from module \"" + sgame_ModList[i] + "\"...\n" );

				array<json@>@ infos = @LoadJSonFile( path, sgame_ModList[i], "weapons.json", "WeaponInfo" );
				if ( @infos is null ) {
					continue;
				}
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					WeaponInfo@ info = null;
					const string id = string( infos[a][ "Id" ] );
					bool added = false;
					
					if ( m_WeaponInfos.Contains( id ) ) {
						@info = cast<WeaponInfo@>( @m_WeaponInfos[ id ] );
						if ( @info is null ) {
							GameError( "WeaponInfo \"" + id + "\" is null!" );
						}
						added = true;
					} else {
						@info = WeaponInfo();
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load weapon info " + id + "\n" );
						@info = null;
						continue;
					}
					if ( !added ) {
						m_WeaponInfos.Add( id, @info );
					}
				}
			}
		}

		AmmoInfo@ GetAmmoInfo( const string& in name ) {
			if ( !m_AmmoInfos.Contains( name ) ) {
				return null;
			}
			return cast<AmmoInfo@>( @m_AmmoInfos[ name ] );
		}
		
		WeaponInfo@ GetWeaponInfo( const string& in name ) {
			if ( !m_WeaponInfos.Contains( name ) ) {
				return null;
			}
			return cast<WeaponInfo@>( @m_WeaponInfos[ name ] );
		}
		
		ItemInfo@ GetItemInfo( const string& in name ) {
			if ( !m_ItemInfos.Contains( name ) ) {
				return null;
			}
			return cast<ItemInfo@>( @m_ItemInfos[ name ] );
		}
		
		MobInfo@ GetMobInfo( const string& in name ) {
			if ( !m_MobInfos.Contains( name ) ) {
				return null;
			}
			return cast<MobInfo@>( @m_MobInfos[ name ] );
		}
		
		bool WeaponInfoExists( const string& in name ) const {
			return m_WeaponInfos.Contains( name );
		}
		
		bool ItemInfoExists( const string& in name ) const {
			return m_ItemInfos.Contains( name );
		}
		
		bool MobInfoExists( const string& in name ) const {
			return m_MobInfos.Contains( name );
		}

		WeaponInfo@ GetWeaponInfo( uint id ) {
			EntityData@ entity = @GetWeaponType( id );
			if ( @entity is null ) {
				return null;
			}
			if ( !m_WeaponInfos.Contains( entity.GetName() ) ) {
				GameError( "InfoDataManager::GetWeaponInfo: weapon \"" + entity.GetName()
					+ "\" has a type definition but not a data entry" );
			}
			return cast<WeaponInfo@>( @m_WeaponInfos[ entity.GetName() ] );
		}
		
		AmmoInfo@ GetAmmoInfo( uint id ) {
			EntityData@ entity = @GetAmmoType( id );
			if ( @entity is null ) {
				return null;
			}
			if ( !m_AmmoInfos.Contains( entity.GetName() ) ) {
				GameError( "InfoDataManager::GetAmmoInfo: item \"" + entity.GetName()
					+ "\" has a type definition but not a data entry" );
			}
			return cast<AmmoInfo@>( @m_AmmoInfos[ entity.GetName() ] );
		}

		ItemInfo@ GetItemInfo( uint id ) {
			EntityData@ entity = @GetItemType( id );
			if ( @entity is null ) {
				return null;
			}
			if ( !m_ItemInfos.Contains( entity.GetName() ) ) {
				GameError( "InfoDataManager::GetItemInfo: item \"" + entity.GetName()
					+ "\" has a type definition but not a data entry" );
			}
			return cast<ItemInfo@>( @m_ItemInfos[ entity.GetName() ] );
		}

		MobInfo@ GetMobInfo( uint id ) {
			EntityData@ entity = @GetMobType( id );
			if ( @entity is null ) {
				return null;
			}
			if ( !m_MobInfos.Contains( entity.GetName() ) ) {
				GameError( "InfoDataManager::GetMobInfo: mob \"" + entity.GetName()
					+ "\" has a type definition but not a data entry" );
			}
			return cast<MobInfo@>( @m_MobInfos[ entity.GetName() ] );
		}

		EntityData@ GetItemType( const string& in name ) {
			for ( uint it = 0; it < m_ItemTypes.Count(); ++it ) {
				if ( m_ItemTypes[ it ] == name ) {
					return @m_ItemTypes[ it ];
				}
			}
			return null;
		}
		EntityData@ GetMobType( const string& in name ) {
			for ( uint it = 0; it < m_MobTypes.Count(); ++it ) {
				if ( m_MobTypes[ it ] == name ) {
					return @m_MobTypes[ it ];
				}
			}
			return null;
		}
		EntityData@ GetWeaponType( const string& in name ) {
			for ( uint it = 0; it < m_WeaponTypes.Count(); ++it ) {
				if ( m_WeaponTypes[ it ] == name ) {
					return @m_WeaponTypes[ it ];
				}
			}
			return null;
		}
		EntityData@ GetAmmoType( const string& in name ) {
			for ( uint it = 0; it < m_AmmoTypes.Count(); ++it ) {
				if ( m_AmmoTypes[ it ] == name ) {
					return @m_AmmoTypes[ it ];
				}
			}
			return null;
		}

		EntityData@ GetItemType( uint id ) {
			for ( uint it = 0; it < m_ItemTypes.Count(); ++it ) {
				if ( m_ItemTypes[ it ].GetID() == id ) {
					return @m_ItemTypes[ it ];
				}
			}
			return null;
		}
		EntityData@ GetMobType( uint id ) {
			for ( uint it = 0; it < m_MobTypes.Count(); ++it ) {
				if ( m_MobTypes[ it ].GetID() == id ) {
					return @m_MobTypes[ it ];
				}
			}
			return null;
		}
		EntityData@ GetWeaponType( uint id ) {
			for ( uint it = 0; it < m_WeaponTypes.Count(); ++it ) {
				if ( m_WeaponTypes[ it ].GetID() == id ) {
					return @m_WeaponTypes[ it ];
				}
			}
			return null;
		}
		EntityData@ GetAmmoType( uint id ) {
			for ( uint it = 0; it < m_AmmoTypes.Count(); ++it ) {
				if ( m_AmmoTypes[ it ].GetID() == id ) {
					return @m_AmmoTypes[ it ];
				}
			}
			return null;
		}

		array<EntityData>@ GetItemTypes() {
			return @m_ItemTypes;
		}
		array<EntityData>@ GetMobTypes() {
			return @m_MobTypes;
		}
		array<EntityData>@ GetWeaponTypes() {
			return @m_WeaponTypes;
		}
		array<EntityData>@ GetAmmoTypes() {
			return @m_AmmoTypes;
		}

		const array<EntityData>@ GetItemTypes() const {
			return @m_ItemTypes;
		}
		const array<EntityData>@ GetMobTypes() const {
			return @m_MobTypes;
		}
		const array<EntityData>@ GetWeaponTypes() const {
			return @m_WeaponTypes;
		}
		const array<EntityData>@ GetAmmoTypes() const {
			return @m_AmmoTypes;
		}
		
		dictionary@ GetItemInfos() {
			return @m_ItemInfos;
		}
		dictionary@ GetMobInfos() {
			return @m_MobInfos;
		}
		dictionary@ GetWeaponInfos() {
			return @m_WeaponInfos;
		}
		dictionary@ GetAmmoInfos() {
			return @m_AmmoInfos;
		}
		
		private array<EntityData> m_ItemTypes;
		private array<EntityData> m_MobTypes;
		private array<EntityData> m_WeaponTypes;
		private array<EntityData> m_AmmoTypes;
		
		private dictionary m_ItemInfos;
		private dictionary m_MobInfos;
		private dictionary m_WeaponInfos;
		private dictionary m_AmmoInfos;
	};

	// just paste the file here
	#include "SGame/InfoSystem/DataTables.as"

    InfoDataManager@ InfoManager = null;
};