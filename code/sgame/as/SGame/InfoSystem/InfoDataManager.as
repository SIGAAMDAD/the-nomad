#include "util/csv.as"
#include "SGame/InfoSystem/InfoLoader.as"
#include "SGame/InfoSystem/AttackInfo.as"
#include "SGame/InfoSystem/MobInfo.as"
#include "SGame/InfoSystem/ItemInfo.as"
#include "SGame/InfoSystem/WeaponInfo.as"

namespace TheNomad::SGame::InfoSystem {
    class InfoDataManager {
		InfoDataManager() {
			ConsolePrint( "Loading mod info files...\n" );

			LoadEntityIds();

			LoadMobInfos();
			LoadItemInfos();
			LoadWeaponInfos();
		}

		private array<json@>@ LoadJSonFile( string& in path, const string& in modName, const string& in fileName,
			const string& in ArrayName )
		{
			json@ data;
			array<json@> values;
			
			path = "modules/" + MODULE_NAME + "/scripts/" + fileName;

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
			string path;
			json@ data;
			json@ obj;
			array<json@> values;
			string name;
			uint id;

			path.reserve( MAX_NPATH );

			@data = json();
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				path = "modules/" + sgame_ModList[i] + "/scripts/entitydata.json";
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
					for ( uint a = 0; a < values.Count(); a++ ) {
						if ( !values[a].get( "Name", name ) ) {
							GameError( "invalid entity data info file, MobData object missing variable 'Name'" );
						}
						if ( !values[a].get( "Id", id ) ) {
							GameError( "invalid entity data info file, MobData object missing variable 'Id'" );
						}
						m_MobTypes[ name ] = id;
					}
				}

				//
				// load item infos
				//
				if ( !data.get( "ItemData", values ) ) {
					ConsoleWarning( "entity data info file for \"" + sgame_ModList[i] + "\" has no item data\n" );
				} else {
					for ( uint a = 0; a < values.Count(); a++ ) {
						if ( !values[a].get( "Name", name ) ) {
							GameError( "invalid entity data info file, ItemData object missing variable 'Name'" );
						}
						if ( !values[a].get( "Id", id ) ) {
							GameError( "invalid entity data info file, ItemData object missing variable 'Id'" );
						}
						m_ItemTypes[ name ] = id;
					}
				}

				//
				// load weapon infos
				//
				if ( !data.get( "WeaponData", values ) ) {
					ConsoleWarning( "entity data info file for \"" + sgame_ModList[i] + "\" has no weapon data\n" );
				} else {
					for ( uint a = 0; a < values.Count(); a++ ) {
						if ( !values[a].get( "Name", name ) ) {
							GameError( "invalid entity data info file, WeaponData object missing variable 'Name'" );
						}
						if ( !values[a].get( "Id", id ) ) {
							GameError( "invalid entity data info file, WeaponData object missing variable 'Id'" );
						}
						m_WeaponTypes[ name ] = id;
					}
				}
			}
		}
		
		private void LoadMobInfos() {
			array<json@>@ infos;
			MobInfo@ info;
			string path;

			path.reserve( MAX_NPATH );
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading mob infos from module \"" + sgame_ModList[i] + "\"...\n" );

				@infos = @LoadJSonFile( path, sgame_ModList[i], "mobs.json", "MobInfo" );
				if ( @infos is null ) {
					return;
				}
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					@info = MobInfo();
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load mob info at " + a + "\n" );
						continue;
					}

					m_MobInfos.Add( @info );
				}
			}
		}
		
		private void LoadItemInfos() {
			array<json@>@ infos;
			ItemInfo@ info;
			string path;

			path.reserve( MAX_NPATH );
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading item infos from module \"" + sgame_ModList[i] + "\"...\n" );

				@infos = @LoadJSonFile( path, sgame_ModList[i], "items.json", "ItemInfo" );
				if ( @infos is null ) {
					return;
				}
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					@info = ItemInfo();
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load item info at " + a + "\n" );
						continue;
					}
					
					m_ItemInfos.Add( @info );
				}
			}
		}
		
		private void LoadWeaponInfos() {
			array<json@>@ infos;
			WeaponInfo@ info;
			string path;

			path.reserve( MAX_NPATH );
			for ( uint i = 0; i < sgame_ModList.Count(); i++ ) {
				ConsolePrint( "Loading weapon infos from module \"" + sgame_ModList[i] + "\"...\n" );

				@infos = @LoadJSonFile( path, sgame_ModList[i], "weapons.json", "WeaponInfo" );
				if ( @infos is null ) {
					return;
				}
			
				for ( uint a = 0; a < infos.Count(); a++ ) {
					@info = WeaponInfo();
					if ( !info.Load( @infos[a] ) ) {
						ConsoleWarning( "failed to load weapon info at " + a + "\n" );
						continue;
					}

					m_WeaponInfos.Add( @info );
				}
			}
		}
		
		WeaponInfo@ GetWeaponInfo( const string& in name ) {
			for ( uint i = 0; i < m_WeaponInfos.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_WeaponInfos[i].name, name ) == 0 ) {
					return m_WeaponInfos[i];
				}
			}
			return null;
		}
		
		ItemInfo@ GetItemInfo( const string& in name ) {
			for ( uint i = 0; i < m_ItemInfos.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_ItemInfos[i].name, name ) == 0 ) {
					return m_ItemInfos[i];
				}
			}
			return null;
		}
		
		MobInfo@ GetMobInfo( const string& in name ) {
			for ( uint i = 0; i < m_MobInfos.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_MobInfos[i].name, name ) == 0 ) {
					return m_MobInfos[i];
				}
			}
			return null;
		}
		
		bool WeaponInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_WeaponInfos.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_WeaponInfos[i].name, name ) == 0 ) {
					return true;
				}
			}
			return false;
		}
		
		bool ItemInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_ItemInfos.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_ItemInfos[i].name, name ) == 0 ) {
					return true;
				}
			}
			return false;
		}
		
		bool MobInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_MobInfos.Count(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_MobInfos[i].name, name ) == 0 ) {
					return true;
				}
			}
			return false;
		}

		WeaponInfo@ GetWeaponInfo( uint id ) {
			for ( uint i = 0; i < m_WeaponInfos.Count(); i++ ) {
				if ( m_WeaponInfos[i].type == id ) {
					return @m_WeaponInfos[i];
				}
			}
			return null;
		}

		ItemInfo@ GetItemInfo( uint id ) {
			for ( uint i = 0; i < m_ItemInfos.Count(); i++ ) {
				if ( m_ItemInfos[i].type == id ) {
					return @m_ItemInfos[i];
				}
			}
			return null;
		}

		MobInfo@ GetMobInfo( uint id ) {
			for ( uint i = 0; i < m_MobInfos.Count(); i++ ) {
				if ( m_MobInfos[i].type == id ) {
					return @m_MobInfos[i];
				}
			}
			return null;
		}

		const dictionary@ GetItemTypes() const {
			return @m_ItemTypes;
		}
		const dictionary@ GetMobTypes() const {
			return @m_MobTypes;
		}
		const dictionary@ GetWeaponTypes() const {
			return @m_WeaponTypes;
		}
		
		void AddMobInfo( MobInfo@ info ) {
			m_MobInfos.push_back( info );
		}
		void AddItemInfo( ItemInfo@ info ) {
			m_ItemInfos.push_back( info );
		}
		void AddWeaponInfo( WeaponInfo@ info ) {
			m_WeaponInfos.push_back( info );
		}

		private dictionary@ m_ItemTypes;
		private dictionary@ m_MobTypes;
		private dictionary@ m_WeaponTypes;
		
		private array<MobInfo@> m_MobInfos;
		private array<ItemInfo@> m_ItemInfos;
		private array<WeaponInfo@> m_WeaponInfos;
	};

    enum AttackMethod {
		Hitscan,
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

	const uint NumMobFlags = 6;
	const uint NumWeaponProperties = 12;

	const uint[] AttackMethodData = {
		uint( AttackMethod::Hitscan ),
		uint( AttackMethod::Projectile ),
		uint( AttackMethod::AreaOfEffect )
	};

	const uint[] AttackTypeData = {
		uint( AttackType::Melee ),
		uint( AttackType::Missile )
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
		"OneHandedSideFirearm",
		"TwoHandedSideFirearm",
		"OneHandedPrimFirearm",
		"TwoHandedPrimFirearm",
		"SpawnsObject"
	};

	string[] AmmoTypeStrings( AmmoType::NumAmmoTypes );
	string[] ArmorTypeStrings( ArmorType::NumArmorTypes );
	string[] AttackMethodStrings( AttackMethod::NumAttackMethods );
	string[] WeaponTypeStrings( WeaponType::NumWeaponTypes );

    InfoDataManager@ InfoManager;
};