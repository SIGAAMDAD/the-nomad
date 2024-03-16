namespace TheNomad::SGame {
	shared enum AttackMethod {
		HitScan = 0,
		Projectile = 1,
		AreaOfEffect = 2,
	};

	shared enum ArmorType {
		None = 0,
		Light,
		Standard,
		Heavy,
		Invul
	};

	shared enum AmmoType {
		Bullet = 0,
		Shell,
		Rocket,
		Grenade,
		Invalid
	};

	shared enum AttackType {
		Melee = 0,
		Missile,
	};

	shared enum WeaponProperty {
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

	shared class WeaponInfo {
		WeaponInfo() {
			ammoType = AmmoType::Invalid;
			magSize = 0;
			magMaxStack = 0;
			damage = 0.0f;
			range = 0.0f;
			weaponType = WeaponType::None;
			hShader = FS_INVALID_HANDLE;
		}

		string name;
		int magSize;
		int magMaxStack;
		AmmoType ammoType;
		float damage;
		float range;
		WeaponProperty weaponProps;
		WeaponType weaponType;
		int hShader;
	};

	shared class ItemInfo {
		ItemInfo() {
			cost = 0;
			hShader = 0;
			spriteOffset = 0;
		}

		string name;
		string description;
		string effect;
		int cost;
		int hShader;
		uint spriteOffset;
	};
	
	shared class AttackInfo {
		AttackInfo() {
			damage = 0;
			range = 0;
			cooldown = 0;
			duration = 0;
			method = AttackMethod::HitScan;
			type = AttackType::Melee;
			canParry = true;
			valid = false;
		}
		
		string name;
		string effect;
		string description;
		float damage;
		float range;
		int cooldown;
		int duration;
		AttackMethod method;
		AttackType type;
		bool canParry;
		bool valid;
	};
	
	shared class MobInfo {
		MobInfo() {
			health = 0;
			armor = ArmorType::None;
			width = 0.0f;
			height = 0.0f;
			speed = vec3( 0.0f );
			soundTolerance = 0.0f;
			smellTolerance = 0.0f;
			sightRange = 0;
			smellRangeX = 0;
			smellRangeY = 0;
			soundRangeX = 0;
			soundRangeY = 0;
			flags = 0;
		}
		
		string name;
		int health;
		ArmorType armor;
		float width;
		float height;
		vec2 speed;
		float soundTolerance;
		float smellTolerance;
		int sightRange;
		int smellRangeX;
		int smellRangeY;
		int soundRangeX;
		int soundRangeY;
		int waitTics; // the duration until the mob has to rethink again
		uint flags;
		array<AttackInfo> attacks;
	};
	
	shared class InfoDataManager {
		InfoDataManager() {
			ConsolePrint( "Loading mod info files...\n" );

			LoadMobInfos();
		}
		
		const WeaponInfo@ GetWeaponInfo( const string& in name ) const {
			for ( uint i = 0; i < m_WeaponInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_WeaponInfos[i].name, name ) is 0 ) {
					return m_WeaponInfos[i];
				}
			}
			return null;
		}
		
		const ItemInfo@ GetItemInfo( const string& in name ) const {
			for ( uint i = 0; i < m_ItemInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_ItemInfos[i].name, name ) is 0 ) {
					return m_ItemInfos[i];
				}
			}
			return null;
		}
		
		const MobInfo@ GetMobInfo( const string& in name ) const {
			for ( uint i = 0; i < m_MobInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_MobInfos[i].name, name ) is 0 ) {
					return m_MobInfos[i];
				}
			}
			return null;
		}
		
		bool WeaponInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_WeaponInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_WeaponInfos[i].name, name ) is 0 ) {
					return true;
				}
			}
			return false;
		}
		
		bool ItemInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_ItemInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_ItemInfos[i].name, name ) is 0 ) {
					return true;
				}
			}
			return false;
		}
		
		bool MobInfoExists( const string& in name ) const {
			for ( uint i = 0; i < m_MobInfos.size(); i++ ) {
				if ( TheNomad::Util::StrICmp( m_MobInfos[i].name, name ) is 0 ) {
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

		bool LoadJSonFile( const string& in fileName, TheNomad::Util::JsonObject& in json ) {
			array<int8> text;
			string path;

			path.resize( MAX_NPATH );
			path = "modules/" + MODULE_NAME + "/" + fileName;

			if ( TheNomad::Engine::FileSystem::LoadFile( path, text ) == 0 ) {
				return false;
			}

			return true;
		}

		bool MissingVar( const string& in infoName, const string& in varName ) {
			ConsoleWarning( "invalid " + infoName + " info, missing variable '" + varName + "'\n" );
			return false;
		}

		bool LoadItemInfo( TheNomad::Util::JsonObject& in json ) {
			ItemInfo@ info;

			@info = ItemInfo();

			return true;
		}

		bool LoadMobInfo( TheNomad::Util::JsonObject& in json ) {
			MobInfo@ info;
			float[] speed( 2 );
			string armorType;

			@info = MobInfo();
			if ( !json.GetString( "Name", info.name ) ) {
				return MissingVar( "mob", "Name" );
			}
			if ( !json.GetInt( "Health", info.health ) ) {
				return MissingVar( "mob", "Health" );
			}
			if ( !json.GetVec2( "Speed", info.speed ) ) {
				return MissingVar( "mob", "Speed" );
			}
			if ( !json.GetString( "ArmorType", armorType ) ) {
				return MissingVar( "mob", "ArmorType" );
			}
			if ( !json.GetInt( "SoundRangeX", info.soundRangeX ) ) {
				return MissingVar( "mob", "SoundRangeX" );
			}
			if ( !json.GetInt( "SoundRangeY", info.soundRangeY ) ) {
				return MissingVar( "mob", "SoundRangeY" );
			}
			if ( !json.GetFloat( "SoundTolerance", info.soundTolerance ) ) {
				return MissingVar( "mob", "SoundTolerance" );
			}

			if ( TheNomad::Util::StrICmp( armorType, "none" ) == 0 ) {

			}

			return true;
		}

		void LoadMobInfos() {
			TheNomad::Util::JsonObject json;
			array<TheNomad::Util::JsonObject> mobInfos;
			string msg;

			msg.reserve( 256 );
			msg = "Loading mob infos in \"";
			msg += MODULE_NAME;
			msg += "\"...\n";
			ConsolePrint( msg );

			LoadJSonFile( "mobs.json", json );
			if ( !json.GetObjectArray( "MobInfo", mobInfos ) ) {
				ConsoleWarning( "no mob infos found in module, skipping.\n" );
				return;
			}

			for ( uint i = 0; i < mobInfos.size(); i++ ) {
				if ( !LoadMobInfo( mobInfos[i] ) ) {
					msg = "failed to load mob info ";
					msg += formatUInt( i );
					msg += "!\n";
					ConsoleWarning( msg );
				}
			}
		}
		
		private array<MobInfo@> m_MobInfos;
		private array<ItemInfo@> m_ItemInfos;
		private array<WeaponInfo@> m_WeaponInfos;
	};
};