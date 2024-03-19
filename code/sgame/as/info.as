#include "item.as"

namespace TheNomad::SGame {
	enum AttackMethod {
		Hitscan = 0,
		Projectile = 1,
		AreaOfEffect = 2,
	};

	enum ArmorType {
		None = 0,
		Light,
		Standard,
		Heavy,
		Invul
	};

	enum AmmoType {
		Bullet = 0,
		Shell,
		Rocket,
		Grenade,
		Invalid
	};

	enum AttackType {
		Melee = 0,
		Missile,
	};
	
	enum WeaponType {
		Sidearm,
		HeavySidearm,
		Pimary,
		HeavyPrimary,
		Grenadier,
		Melee,
		LeftArm,
		RightArm,
		
		NumWeaponTypes
	};

	enum WeaponProperty {
		TwoHandedBlade       = 0x00000001,
		OneHandedBlade       = 0x00000002,
		TwoHandedBlunt       = 0x00000004,
		OneHandedBlunt       = 0x00000008,
		TwoHandedPolearm     = 0x00000010,
		OneHandedPolearm     = 0x00000020,
		OneHandedSideFirearm = 0x00000040,
		TwoHandedSideFirearm = 0x00000080,
		OneHandedPrimFirearm = 0x00000100,
		TwoHandedPrimFirearm = 0x00000200,
		SpawnsObject         = 0x10000000,

		None                 = 0x00000000
	};
	
	interface InfoLoader {
		bool Load( TheNomad::Util::JsonValue@ json );
	};
	
	const array<uint> WeaponPropertyBits = {
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
	
	const array<string> WeaponPropertyStrings = {
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
	
	const array<string> WeaponTypeStrings = {
		"Sidearm",
		"HeavySidearm",
		"Primary",
		"HeavyPrimary",
		"Grenadier",
		"Melee",
		"LeftArm",
		"RightArm"
	};
	
	const array<string> AmmoTypeStrings = {
		"Bullet",
		"Shell",
		"Rocket",
		"Grenade"
	};

	class WeaponInfo : InfoLoader {
		WeaponInfo() {
		}
		
		bool Load( TheNomad::Util::JsonValue@ json ) {
			string ammo;
			string type;
			uint i;
			const array<TheNomad::Util::JsonValue@>@ props;
			
			name = string( json["Name"] );
			magSize = uint( json["MagSize"] );
			magMaxStack = uint( json["MagMaxStack"] );
			type = string( json["WeaponType"] );
			@props = cast<array<TheNomad::Util::JsonValue@>@>( @json["WeaponProperties"] );
			ammo = string( json["AmmoType"] );
			magSize = uint( json["MagSize"] );
			magMaxStack = uint( json["MagMaxStack"] );
			damage = float( json["Damage"] );
			range = float( json["Range"] );
			spriteOffset = uint( json["SpriteOffset"] );
			hShader = TheNomad::Engine::Renderer::RegisterShader( string( json["Shader"] ) );
			
			for ( i = 0; i < WeaponTypeStrings.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( type, WeaponTypeStrings[i] ) != 1 ) {
					weaponType = WeaponType( i );
					break;
				}
			}
			if ( weaponType == WeaponType::NumWeaponTypes ) {
				ConsoleWarning( "invalid weapon info, WeaponType '" + type + "' not recognized\n" );
				return false;
			}
			
			for ( i = 0; i < WeaponPropertyStrings.size(); i++ ) {
				for ( uint a = 0; a < props.size(); a++ ){
					if ( TheNomad::Util::StrICmp( string( props[a] ), WeaponPropertyStrings[i] ) != 1 ) {
						weaponProps = WeaponProperty( uint( weaponProps ) | WeaponPropertyBits[i] );
					}
				}
			}
			if ( weaponProps == WeaponProperty::None ) {
				ConsoleWarning( "invalid weapon info, WeaponProperties are invalid (None, abide by physics pls)\n" );
				return false;
			}
			
			return true;
		}

		string name;
		int magSize = 0;
		int magMaxStack = 0;
		AmmoType ammoType = AmmoType::Invalid;
		float damage = 0.0f;
		float range = 0.0f;
		WeaponProperty weaponProps = WeaponProperty::None;
		WeaponType weaponType = WeaponType::NumWeaponTypes;
		uint spriteOffset = 0;
		int hShader = FS_INVALID_HANDLE;
	};

	class ItemInfo : InfoLoader {
		ItemInfo() {
		}
		
		bool Load( TheNomad::Util::JsonValue@ json ) {
			TheNomad::Util::JsonValue@ str;
			
			name = string( json["Name"] );
			TheNomad::GameSystem::GetString( name, description );
			@str = @json["Effect"];
			if ( @str !is null ) {
				effect = string( str );
			}
			
			cost = int( json["Cost"] );
			hShader = TheNomad::Engine::Renderer::RegisterShader( string( json["Shader"] ) );
			spriteOffset = uint( json["SpriteOffset"] );
			maxStackSize = uint( json["MaxStackSize"] );

			return true;
		}

		string name;
		string description;
		string effect;
		int cost = 0;
		int hShader = FS_INVALID_HANDLE;
		uint spriteOffset = 0;
		uint maxStackSize = 0;
	};
	
	class AttackInfo : InfoLoader {
		AttackInfo() {
		}
		
		bool Load( TheNomad::Util::JsonValue@ json ) {
			name = string( json["Name"] );

			return true;
		}
		
		string name;
		string effect;
		string description;
		float damage = 0.0f;
		float range = 0.0f;
		uint cooldown = 0;
		uint duration = 0;
		AttackMethod method = AttackMethod::Hitscan;
		AttackType type = AttackType::Melee;
		bool canParry = true;
		bool valid = false;
		uint spriteOffset = 0;
		TheNomad::Engine::SoundSystem::SoundEffect sound;
	};
	
	class MobInfo : InfoLoader {
		MobInfo() {
		}
		
		bool Load( TheNomad::Util::JsonValue@ json ) {
			return true;
		}
		
		string name;
		float health = 0.0f;
		ArmorType armor = ArmorType::None;
		float width = 0.0f;
		float height = 0.0f;
		vec3 speed = vec3( 0.0f );
		float soundTolerance = 0.0f;
		float smellTolerance = 0.0f;
		int sightRange = 0;
		int smellRangeX = 0;
		int smellRangeY = 0;
		int soundRangeX = 0;
		int soundRangeY = 0;
		uint waitTics = 0; // the duration until the mob has to rethink again
		uint flags = 0;
		uint spriteOffset = 0;
		int hShader = 0;
		array<AttackInfo> attacks;
		TheNomad::Engine::SoundSystem::SoundEffect wakeupSfx;
		TheNomad::Engine::SoundSystem::SoundEffect moveSfx;
		TheNomad::Engine::SoundSystem::SoundEffect painSfx;
		TheNomad::Engine::SoundSystem::SoundEffect dieSfx;
	};
	
	class InfoDataManager {
		InfoDataManager() {
			ConsolePrint( "Loading mod info files...\n" );

			LoadMobInfos();
			LoadItemInfos();
			LoadWeaponInfos();
		}
		
		private array<TheNomad::Util::JsonValue@>@ LoadJSonFile( const string& in fileName, const string& in ArrayName ) {
			string path;
			string buffer;
			array<TheNomad::Util::JsonValue@>@ values;
			TheNomad::Util::JsonValue@ data;
			TheNomad::Util::JsonObject json;
			
			path.reserve( MAX_NPATH );
			path = "modules/" + MODULE_NAME + "/scripts/" + fileName;
			if ( TheNomad::Engine::FileSystem::LoadFile( path, buffer ) == 0 ) {
				ConsolePrint( "no info file found for '" + fileName + "', skipping.\n" );
				return null;
			}
			
			@data = @json.Parse( buffer );
			@values = cast<array<TheNomad::Util::JsonValue@>@>( @data[ArrayName] );
			
			if ( @values is null ) {
				ConsoleWarning( "info file loaded, but no object named '" + ArrayName + "' found.\n" );
				return null;
			}
			
			return @values;
		}
		
		private void LoadMobInfos() {
			array<TheNomad::Util::JsonValue@>@ infos;
			MobInfo@ info;
			string msg;
			
			msg = "Loading mob infos from module \"";
			msg += MODULE_NAME;
			msg += "\"...\n";
			ConsolePrint( msg );
			
			@infos = @LoadJSonFile( "mobs.json", "MobInfo" );
			if ( @infos is null ) {
				return;
			}
			
			for ( uint i = 0; i < infos.size(); i++ ) {
				@info = MobInfo();
				if ( !info.Load( @infos[i] ) ) {
					ConsoleWarning( "failed to load mob info at " + i + ".\n" );
					continue;
				}
				
				AddMobInfo( @info );
			}
		}
		
		private void LoadItemInfos() {
			array<TheNomad::Util::JsonValue@>@ infos;
			ItemInfo@ info;
			string msg;
			
			msg = "Loading item infos from module \"";
			msg += MODULE_NAME;
			msg += "\"...\n";
			ConsolePrint( msg );
			
			@infos = @LoadJSonFile( "items.json", "ItemInfo" );
			if ( @infos is null ) {
				return;
			}
			
			for ( uint i = 0; i < infos.size(); i++ ) {
				@info = ItemInfo();
				if ( !info.Load( @infos[i] ) ) {
					ConsoleWarning( "failed to load item info at " + i + ".\n" );
					continue;
				}
				
				AddItemInfo( @info );
			}
		}
		
		private void LoadWeaponInfos() {
			array<TheNomad::Util::JsonValue@>@ infos;
			WeaponInfo@ info;
			string msg;
			
			msg = "Loading mob infos from module \"";
			msg += MODULE_NAME;
			msg += "\"...\n";
			ConsolePrint( msg );
			
			@infos = @LoadJSonFile( "weapon.json", "WeaponInfo" );
			if ( @infos is null ) {
				return;
			}
			
			for ( uint i = 0; i < infos.size(); i++ ) {
				@info = WeaponInfo();
				if ( !info.Load( @infos[i] ) ) {
					ConsoleWarning( "failed to load weapon info at " + i + ".\n" );
					continue;
				}
				
				AddWeaponInfo( @info );
			}
		}
		
		const WeaponInfo@ GetWeaponInfo( const string& in name ) const {
			for ( uint i = 0; i < m_WeaponInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_WeaponInfos[i].name, name ) == 0 ) {
					return m_WeaponInfos[i];
				}
			}
			return null;
		}
		
		const ItemInfo@ GetItemInfo( const string& in name ) const {
			for ( uint i = 0; i < m_ItemInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_ItemInfos[i].name, name ) == 0 ) {
					return m_ItemInfos[i];
				}
			}
			return null;
		}
		
		const MobInfo@ GetMobInfo( const string& in name ) const {
			for ( uint i = 0; i < m_MobInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_MobInfos[i].name, name ) == 0 ) {
					return m_MobInfos[i];
				}
			}
			return null;
		}
		
		bool WeaponInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_WeaponInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_WeaponInfos[i].name, name ) == 0 ) {
					return true;
				}
			}
			return false;
		}
		
		bool ItemInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_ItemInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_ItemInfos[i].name, name ) == 0 ) {
					return true;
				}
			}
			return false;
		}
		
		bool MobInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_MobInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_MobInfos[i].name, name ) == 0 ) {
					return true;
				}
			}
			return false;
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
		
		private array<MobInfo@> m_MobInfos;
		private array<ItemInfo@> m_ItemInfos;
		private array<WeaponInfo@> m_WeaponInfos;
	};

	InfoDataManager@ InfoManager;
};