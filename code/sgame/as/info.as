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
	
	shared class AttackInfo {
		AttackInfo() {
			damage = 0;
			range = 0;
			cooldown = 0;
			duration = 0;
			method = AttackMethod::HitScan;
			canParry = true;
		}
		
		string name;
		string effect;
		string description;
		float damage;
		float range;
		int cooldown;
		int duration;
		AttackMethod method;
		bool canParry;
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
		vec3 speed;
		float soundTolerance;
		float smellTolerance;
		int sightRange;
		int smellRangeX;
		int smellRangeY;
		int soundRangeX;
		int soundRangeY;
		uint flags;
		array<AttackInfo> melee;
		array<AttackInfo> missile;
	};
	
	shared class InfoDataManager {
		InfoDataManager() {
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
		
		private array<MobInfo@> m_MobInfos;
		private array<ItemInfo@> m_ItemInfos;
		private array<WeaponInfo@> m_WeaponInfos;
	};
	
	InfoDataManager@ InfoManager;
	
	InfoDataManager@ GetInfoManager() {
		return InfoManager;
	}
	
	bool LoadJSonFile( const string& in fileName, TheNomad::Util::JsonObject& in json ) {
		array<int8> text;
		string path;
		
		path.resize( MAX_NPATH );
		path = "modules/" + MODULE_NAME + "/" + fileName;
		
		if ( TheNomad::Engine::FileSystem::LoadFile( path, text ) is false ) {
			return false;
		}
		
		return true;
	}
	
	bool LoadItemInfo( TheNomad::Util::JsonObject& in json ) {
		ItemInfo@ info;
		TheNomad::Util::JsonObject& data = json.GetObject();
		
		@info = ItemInfo();
		info.name = data.GetString( "name" );
		info.cost = data;
	}
	
	bool LoadMobInfo( TheNomad::Util::JsonObject& in json ) {
		MobInfo@ info;
		TheNomad::Util::JsonObject& data = json.GetObject();
		
		@info = MobInfo();
		info.name = data.GetString( "name" );
		info.health = data.GetInt( "health" );
		info.speed = data.GetVec3( "speed" );
		info.soundTolerance = data.GetFloat( "soundTolerance" );
		
		return true;
	}
	
	void LoadMobInfos() {
		TheNomad::Util::JsonObject json;
		array<TheNomad::Util::JsonObject> mobInfos;
		
		LoadJSonFile( "mobs.txt", json );
		parse.GetArray( mobInfos );
		
		for ( uint i = 0; i < mobInfos.size(); i++ ) {
			if ( !LoadMobInfo( mobInfos[i] ) ) {
				
			}
		}
	}
	
	void InitInfos() {
		string token;
		
		@InfoManager = InfoDataManager();
		
		ConsolePrint( "Loading module infos...\n" );
		
		token.resize( MAX_TOKEN_CHARS );
		LoadMobInfos( token );
		LoadItemInfos( token );
		LoadWeaponInfos( token );
	}
};