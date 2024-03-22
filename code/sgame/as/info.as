#include "item.as"
#include "util/csv.as"

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

	enum MobFlags {
		Deaf      = 0x0001,
		Blind     = 0x0002,
		Terrified = 0x0004,
		Boss      = 0x0008,
		Sentry    = 0x0010,
		PermaDead = 0x0020,
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
		bool Load( json@ json );
	};

	const array<uint> AttackTypeData = {
		uint( AttackType::Melee ),
		uint( AttackType::Missile )
	};

	const array<string> AttackTypeStrings = {
		"Melee",
		"Missile"
	};

	const array<uint> AttackMethodData = {
		uint( AttackMethod::Hitscan ),
		uint( AttackMethod::Projectile ),
		uint( AttackMethod::AreaOfEffect )
	};

	const array<string> AttackMethodStrings = {
		"Hitscan",
		"Projectile",
		"AreaOfEffect"
	};

	const array<string> ArmorTypeStrings = {
		"None",
		"Light",
		"Standard",
		"Heavy",
		"Invul"
	};

	const array<uint> MobFlagBits = {
		uint( MobFlags::Deaf ),
		uint( MobFlags::Blind ),
		uint( MobFlags::Terrified ),
		uint( MobFlags::Boss ),
		uint( MobFlags::Sentry )
	};

	const array<string> MobFlagStrings = {
		"Deaf",
		"Blind",
		"Terrified",
		"Boss",
		"Sentry"
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
		
		bool Load( json@ json ) {
			string ammo;
			string type;
			uint i;
			array<json@> props;
			
			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "MagSize", magSize ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'MagSize'\n" );
				return false;
			}
			if ( !json.get( "MagMaxStack", magMaxStack ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'MagMaxStack'\n" );
				return false;
			}
			if ( !json.get( "WeaponType", type ) ) {
				ConsoleWarning( "invalid weapon info, missing variable 'WeaponType'\n" );
				return false;
			}
			if ( !json.get( "WeaponProperties", props ) ) {
				return false;
			}
			ammo = string( json["AmmoType"] );
			magSize = uint( json["MagSize"] );
			magMaxStack = uint( json["MagMaxStack"] );
			damage = float( json["Damage"] );
			range = float( json["Range"] );
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
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;
		int hShader = FS_INVALID_HANDLE;
	};

	class ItemInfo : InfoLoader {
		ItemInfo() {
		}
		
		bool Load( json@ json ) {
			string str;

			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid item info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Effect", name ) ) {
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

			ConsolePrint( "Loaded item info for '" + name + "'\n" );

			return true;
		}

		string name;
		string description;
		string effect;
		int cost = 0;
		int hShader = FS_INVALID_HANDLE;
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;
		uint maxStackSize = 0;
		TheNomad::Engine::SoundSystem::SoundEffect pickupSfx;
		TheNomad::Engine::SoundSystem::SoundEffect useSfx;
	};
	
	class AttackInfo : InfoLoader {
		AttackInfo() {
		}
		
		bool Load( json@ json ) {
			string methodStr;
			string typeStr;
			string shader;
			uint i;

			if ( !json.get( "Id", id ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Id'\n" );
				return false;
			}
			json.get( "Effect", effect );
			if ( !json.get( "Damage", damage ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Damage'\n" );
				return false;
			}
			if ( !json.get( "Range", range ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Range'\n" );
				return false;
			}
			if ( !json.get( "Cooldown", cooldown ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Cooldown'\n" );
				return false;
			}
			if ( !json.get( "Duration", duration ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Duration'\n" );
				return false;
			}
			if ( !json.get( "Method", methodStr ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Method'\n" );
				return false;
			}
			if ( !json.get( "Type", typeStr ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Type'\n" );
				return false;
			}
			if ( !json.get( "CanParry", canParry ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'CanParry'\n" );
				return false;
			}
			if ( !json.get( "Shader", shader ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'Shader'\n" );
				return false;
			} else {
				hShader = TheNomad::Engine::Renderer::RegisterShader( shader );
			}
			if ( !json.get( "SpriteOffsetX", spriteOffsetX ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'SpriteOffsetX'\n" );
				return false;
			}
			if ( !json.get( "SpriteOffsetY", spriteOffsetY ) ) {
				ConsoleWarning( "invalid mob attack info, missing variable 'SpriteOffsetY'\n" );
				return false;
			}
			
			TheNomad::GameSystem::GetString( id + "_DESC", description );

			for ( i = 0; i < AttackMethodStrings.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( AttackMethodStrings[i], methodStr ) == 0 ) {
					attackMethod = AttackMethod( AttackMethodData[i] );
					break;
				}
			}
			if ( i == AttackMethodStrings.size() ) {
				ConsoleWarning( "invalid attack info, AttackMethod '" + methodStr + "' isn't recognized.\n" );
				return false;
			}

			for ( i = 0; i < AttackTypeStrings.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( AttackTypeStrings[i], typeStr ) == 0 ) {
					attackType = AttackType( AttackTypeData[i] );
					break;
				}
			}
			if ( i == AttackTypeStrings.size() ) {
				ConsoleWarning( "invalid attack info, AttackType '" + typeStr + "' isn't recognized.\n" );
				return false;
			}

			ConsolePrint( "Loaded mob attack info '" + id + "'\n" );
			
			valid = true;

			return true;
		}
		
		string id;
		string effect;
		string description;
		float damage = 0.0f;
		float range = 0.0f;
		uint cooldown = 0;
		uint duration = 0;
		AttackMethod attackMethod = AttackMethod::Hitscan;
		AttackType attackType = AttackType::Melee;
		bool canParry = true;
		bool valid = false;
		int hShader = FS_INVALID_HANDLE;
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;
		TheNomad::Engine::SoundSystem::SoundEffect sound;
	};
	
	class MobInfo : InfoLoader {
		MobInfo() {
		}
		
		bool Load( json@ json ) {
			string armor;
			string str;
			array<json@> values;
			uint i;
			
			if ( !json.get( "Name", name ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Name'\n" );
				return false;
			}
			if ( !json.get( "Health", health ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Health'\n" );
				return false;
			}
			if ( !json.get( "ArmorType", armor ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'ArmorType'\n" );
				return false;
			}
			if ( !json.get( "Width", width ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Width'\n" );
				return false;
			}
			if ( !json.get( "Height", height ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Height'\n" );
				return false;
			}
			if ( !json.get( "SoundTolerance", soundTolerance ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'SoundTolerance'\n" );
				return false;
			}
		//	if ( !json.get( "SmellTolerance", smellTolerance ) ) {
		//		ConsoleWarning( "invalid mob info, missing variable 'SmellTolerance'\n" );
		//		return false;
		//	}
			if ( !json.get( "SightRadius", sightRadius ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'SightRadius'\n" );
				return false;
			}
			if ( !json.get( "SightRange", sightRange ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'SightRange'\n" );
				return false;
			}
			if ( !json.get( "DetectionRangeX", detectionRangeX ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'DetectionRangeX'\n" );
				return false;
			}
			if ( !json.get( "DetectionRangeY", detectionRangeY ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'DetectionRangeY'\n" );
				return false;
			}
			if ( !json.get( "WaitTics", waitTics ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'WaitTics'\n" );
				return false;
			}
			if ( !json.get( "SpriteOffsetX", spriteOffsetX ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'SpriteOffsetX'\n" );
				return false;
			}
			if ( !json.get( "SpriteOffsetY", spriteOffsetY ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'SpriteOffsetY'\n" );
				return false;
			}
			if ( !json.get( "Shader", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Shader'\n" );
				return false;
			} else {
				hShader = TheNomad::Engine::Renderer::RegisterShader( str );
			}
			if ( !json.get( "WakeupSfx", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'WakeupSfx'\n" );
				return false;
			} else {
				wakeupSfx.Set( str );
			}
			if ( !json.get( "MoveSfx", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'MoveSfx'\n" );
				return false;
			} else {
				moveSfx.Set( str );
			}
			if ( !json.get( "PainSfx", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'PainSfx'\n" );
				return false;
			} else {
				painSfx.Set( str );
			}
			if ( !json.get( "DieSfx", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'DieSfx'\n" );
				return false;
			} else {
				dieSfx.Set( str );
			}

			if ( !json.get( "Flags", str ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Flags'\n" );
				return false;
			}
			
			array<string>@ flagValues = @ParseCSV( str );
			
			ConsolePrint( "Processing MobFlags for '" + name + "'...\n" );
			for ( i = 0; i < MobFlagStrings.size(); i++ ) {
				for ( uint a = 0; a < flagValues.size(); a++ ) {
					if ( TheNomad::Util::StrICmp( flagValues[a], MobFlagStrings[i] ) == 0 ) {
						flags = EntityFlags( uint( flags ) | MobFlagBits[i] );
					}
				}
			}
			
			if ( !json.get( "Speed", values ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'Speed'\n" );
				return false;
			}
			
			if ( values.size() != 3 ) {
				ConsoleWarning( "invalid mob info, Speed value array is not exactly 3 values.\n" );
				return false;
			}
			for ( i = 0; i < values.size(); i++ ) {
				json data = values[i];
			}
			
			if ( !json.get( "AttackData", values ) ) {
				ConsoleWarning( "invalid mob info, missing variable 'AttackData'\n" );
				return false;
			}
			if ( values.size() < 1 ) {
				ConsoleWarning( "mob info has no attack data.\n" );
				return false;
			}
			ConsolePrint( "Processing AttackData for MobInfo '" + name + "'...\n" );
			for ( i = 0; i < values.size(); i++ ) {
				AttackInfo@ atk = AttackInfo();
				if ( !atk.Load( @values[i] ) ) {
					ConsoleWarning( "failed to load attack info.\n" );
					return false;
				}
				attacks.push_back( @atk );
			}
			
			for ( i = 0; i < ArmorTypeStrings.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( armor, ArmorTypeStrings[i] ) == 0 ) {
					armorType = ArmorType( i );
					break;
				}
			}
			if ( i == ArmorTypeStrings.size() ) {
				ConsoleWarning( "invalid mob info, ArmorType value '" + armor + "' not recognized.\n" );
				return false;
			}

			return true;
		}
		
		string name;
		float health = 0.0f;
		ArmorType armorType = ArmorType::None;
		float width = 0.0f;
		float height = 0.0f;
		vec3 speed = vec3( 0.0f );
		float soundTolerance = 0.0f;
		float smellTolerance = 0.0f;
		float sightRadius = 0.0f;
		int sightRange = 0;
		int detectionRangeX = 0;
		int detectionRangeY = 0;
		uint waitTics = 0; // the duration until the mob has to rethink again
		EntityFlags flags = EntityFlags::None;
		uint spriteOffsetX = 0;
		uint spriteOffsetY = 0;
		int hShader = 0;
		array<AttackInfo@> attacks;
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
		
		private array<json@>@ LoadJSonFile( const string& in fileName, const string& in ArrayName ) {
			string path;
			string buffer;
			json@ data;
			array<json@> values;
			
			path.reserve( MAX_NPATH );
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
		
		private void LoadMobInfos() {
			array<json@>@ infos;
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
			array<json@>@ infos;
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
			array<json@>@ infos;
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