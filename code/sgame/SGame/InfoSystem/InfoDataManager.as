#include "SGame/InfoSystem/InfoLoader.as"
#include "SGame/InfoSystem/AttackInfo.as"
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
						if ( !values[a].get( "Name", name ) ) {
							ConsoleWarning( "invalid entity data info file, MobData object missing variable 'Name'\n" );
							continue;
						}
						if ( !values[a].get( "Id", id ) ) {
							ConsoleWarning( "invalid entity data info file, MobData object missing variable 'Id'\n" );
							continue;
						}
						m_MobTypes.Add( EntityData( name, id ) );
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
						if ( !values[a].get( "Name", name ) ) {
							ConsoleWarning( "invalid entity data info file, AmmoData object missing variable 'Name'\n" );
							continue;
						}
						if ( !values[a].get( "Id", id ) ) {
							ConsoleWarning( "invalid entity data info file, AmmoData object missing variable 'Id'\n" );
							continue;
						}
						m_AmmoTypes.Add( EntityData( name, id ) );
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
						if ( !values[a].get( "Name", name ) ) {
							ConsoleWarning( "invalid entity data info file, ItemData object missing variable 'Name'\n" );
							continue;
						}
						if ( !values[a].get( "Id", id ) ) {
							ConsoleWarning( "invalid entity data info file, ItemData object missing variable 'Id'\n" );
							continue;
						}
						m_ItemTypes.Add( EntityData( name, id ) );
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
						if ( !values[a].get( "Name", name ) ) {
							ConsoleWarning( "invalid entity data info file, WeaponData object missing variable 'Name'\n" );
							continue;
						}
						if ( !values[a].get( "Id", id ) ) {
							ConsoleWarning( "invalid entity data info file, WeaponData object missing variable 'Id'\n" );
							continue;
						}
						m_WeaponTypes.Add( EntityData( name, id ) );
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

					if ( m_MobInfos.Contains( id ) ) {
						@info = cast<MobInfo@>( @m_MobInfos[ id ] );
					} else {
						@info = MobInfo();
						m_MobInfos.Add( id, @info );
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load mob info " + id + "\n" );
						continue;
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
					
					if ( m_ItemInfos.Contains( id ) ) {
						@info = cast<ItemInfo@>( @m_ItemInfos[ id ] );
						if ( @info is null ) {
							GameError( "ItemInfo \"" + id + "\" is null!" );
						}
					} else {
						@info = ItemInfo();
						m_ItemInfos.Add( id, @info );
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load item info " + id + "\n" );
						continue;
					}
				}
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
					
					if ( m_AmmoInfos.Contains( id ) ) {
						@info = cast<AmmoInfo@>( @m_AmmoInfos[ id ] );
					} else {
						@info = AmmoInfo();
						m_AmmoInfos.Add( id, @info );
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load ammo info " + id + "\n" );
						continue;
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
					
					if ( m_WeaponInfos.Contains( id ) ) {
						@info = cast<WeaponInfo@>( @m_WeaponInfos[ id ] );
						if ( @info is null ) {
							GameError( "WeaponInfo \"" + id + "\" is null!" );
						}
					} else {
						@info = WeaponInfo();
						m_WeaponInfos.Add( id, @info );
					}
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load weapon info " + id + "\n" );
						continue;
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

	enum ItemType {
		Powerup,
		Consumable,

		NumItemTypes
	};

    enum AttackMethod {
		Hitscan,
		RayCast, // for melee without the AOE
		Projectile,
		AreaOfEffect,

		NumAttackMethods
	};

	enum ArmorType {
		None,
		Light,
		Standard,
		Heavy,
		Invul,

		NumArmorTypes
	};

	enum AmmoType {
		Bullet,
		Shell,
		Rocket,
		Grenade,
		Invalid,

		NumAmmoTypes,
	};

	enum AttackType {
		Melee,
		Missile,

		NumAttackTypes
	};
	
	enum WeaponType {
		Sidearm,
		HeavySidearm,
		Primary,
		HeavyPrimary,
		Grenadier,
		Melee,
		LeftArm,
		RightArm,
		
		NumWeaponTypes
	};

	enum MobFlags {
		Deaf      = 0x0001,
		Blind     = 0x0002,
		Terrified = 0x0004,
		Boss      = 0x0008,
		Sentry    = 0x0010,
		PermaDead = 0x0020,

		None      = 0x0000
	};

	enum AmmoProperty {
		Heavy               = 0x0001,
		Light               = 0x0002,
		Pellets             = 0x0003,
		TypeBits            = 0x000f,

		NoPenetration       = 0x0010,
		ArmorPiercing       = 0x0020,
		HollowPoint         = 0x0030,
		PenetrationBits     = 0x00f0,

		Flechette           = 0x0100,
		Buckshot            = 0x0200,
		Shrapnel            = 0x0300,
		Slug                = 0x0400,
		ShotgunBullshitBits = 0x0f00,

		Explosive           = 0x1000,
		Incendiary          = 0x2000,
		Tracer              = 0x3000,
		ExtBulletBits       = 0xf000,

		None                = 0x0000
	};

	enum WeaponProperty {
		OneHandedBlade       = 0x00000101,
		OneHandedBlunt       = 0x00001002,
		OneHandedPolearm     = 0x00010003,
		OneHandedSideFirearm = 0x00100004,
		OneHandedPrimFirearm = 0x00100005,

		TwoHandedBlade       = 0x00000110,
		TwoHandedBlunt       = 0x00001020,
		TwoHandedPolearm     = 0x00010030,
		TwoHandedSideFirearm = 0x00100040,
		TwoHandedPrimFirearm = 0x00100050,

		IsOneHanded          = 0x0000000f,
		IsTwoHanded          = 0x000000f0,
		IsBladed             = 0x00000f00,
		IsBlunt              = 0x0000f000,
		IsPolearm            = 0x000f0000,
		IsFirearm            = 0x00f00000,

		SpawnsObject         = 0x10000000,

		None                 = 0x00000000
	};

	const string[] AttackTypeStrings = {
		"Melee",
		"Missile"
	};

	const uint[] MobFlagBits = {
		uint( MobFlags::Deaf ),
		uint( MobFlags::Blind ),
		uint( MobFlags::Terrified ),
		uint( MobFlags::Boss ),
		uint( MobFlags::Sentry )
	};

	const string[] MobFlagStrings = {
		"Deaf",
		"Blind",
		"Terrified",
		"Boss",
		"Sentry"
	};

	const string[] ItemTypeStrings = {
		"Powerup",
		"Consumable"
	};
	
	const uint[] WeaponPropertyBits = {
		uint( WeaponProperty::TwoHandedBlade ),
		uint( WeaponProperty::OneHandedBlade ),
		uint( WeaponProperty::TwoHandedBlunt ),
		uint( WeaponProperty::OneHandedBlunt ),
		uint( WeaponProperty::TwoHandedPolearm ),
		uint( WeaponProperty::OneHandedPolearm ),
		uint( WeaponProperty::OneHandedSideFirearm ),
		uint( WeaponProperty::TwoHandedSideFirearm ),
		uint( WeaponProperty::OneHandedPrimFirearm ),
		uint( WeaponProperty::TwoHandedPrimFirearm ),
		uint( WeaponProperty::SpawnsObject )
	};
	
	const string[] WeaponPropertyStrings = {
		"TwoHandedBlade",
		"OneHandedBlade",
		"TwoHandedBlunt",
		"OneHandedBlunt",
		"TwoHandedPolearm",
		"OneHandedPolearm",
		"OneHandedSidearmFirearm",
		"TwoHandedSidearmFirearm",
		"OneHandedPrimaryFirearm",
		"TwoHandedPrimaryFirearm",
		"SpawnsObject"
	};

	const string[] AmmoPropertyStrings = {
		"Heavy",
		"Light",
		"Pellets",

		"NoPenetration",
		"ArmorPiercing",
		"HollowPoint",

		"Flechette",
		"Buckshot",
		"Shrapnel",
		"Slug",

		"Explosive",
		"Incendiary",
		"Tracer"
	};

	const string[] AmmoTypeStrings = {
		"Bullet",
		"Shell",
		"Grenade",
		"Rocket"
	};
	const string[] ArmorTypeStrings = {
		"None",
		"Light",
		"Standard",
		"Heavy",
		"Invul"
	};
	const string[] AttackMethodStrings = {
		"Hitscan",
		"RayCast",
		"Projectile",
		"AreaOfEffect"
	};
	const string[] WeaponTypeStrings = {
		"Sidearm",
		"HeavySidearm",
		"Primary",
		"HeavyPrimary",
		"Grenadier",
		"Melee",
		"LeftArm",
		"RightArm"
	};

    InfoDataManager@ InfoManager = null;
};