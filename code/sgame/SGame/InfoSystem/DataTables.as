namespace TheNomad::SGame::InfoSystem {
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

	enum ItemFlags {
		NoOwner		= 0x0001,
	};

	enum MobFlags {
		Deaf		= 0x0001,
		Blind		= 0x0002,
		Terrified	= 0x0004,
		Boss		= 0x0008,
		Sentry		= 0x0010,
		PermaDead	= 0x0020,
		Leader		= 0x004,

		None      = 0x0000
	};

	enum AmmoProperty {
		Heavy				= 0x0001, // 5.56/7.62/.223 REM
		Light				= 0x0002, // bullet/9mm/45 ACP
		Pellets				= 0x0003, // 12 gauge/20 gauge/8 gauge
		TypeBits			= 0x000f,

		NoPenetration		= 0x0010, // very little penetration, mostly for pistol ammunition
		ArmorPiercing		= 0x0020, // armor piercing specialized ammo, can bypass light to medium armor
		HollowPoint			= 0x0030, // does extra damage against light armor but less damage against heavy armor
		PenetrationBits		= 0x00f0,

		Flechette			= 0x0100, // warcrime, applies bleed status effect
		Buckshot			= 0x0200, // effective against heavy armor enemies or densly packed enemies, but weak at croud-control
		Birdshot			= 0x0300, // extremely effective at light to medium armor croud-control, almost useless against heavy armor
		Shrapnel			= 0x0400, // warcrime, effective against light armor enemies but weak against anything heavier, cheap however
		Slug				= 0x0500, // extremely effective on medium armor enemies, but not good as a croud-control or spray-and-pray
		ShotgunBullshitBits	= 0x0f00,

		Explosive			= 0x1000, // EXPLOOOOOOSION!
		Incendiary			= 0x2000, // dragon's breath, warcrime
		Tracer				= 0x3000, // tracers
		SubSonic			= 0x4000, // extra stealth modifier, but less damage
		ExtBulletBits		= 0xf000,

		None				= 0x0000 // invalid ammo
	};

	enum WeaponFireMode {
		Single,
		Burst,
		Automatic,

		NumFireModes,
	};

	enum WeaponProperty {
		IsOneHanded			= 0b01000000,
		IsTwoHanded			= 0b00100000,
		IsBladed			= 0b00000001,
		IsBlunt				= 0b00000010,
		IsFirearm			= 0b00001000,

		OneHandedBlade		= uint( WeaponProperty::IsOneHanded ) | uint( WeaponProperty::IsBladed ),
		OneHandedBlunt		= uint( WeaponProperty::IsOneHanded ) | uint( WeaponProperty::IsBlunt ),
		OneHandedFirearm	= uint( WeaponProperty::IsOneHanded ) | uint( WeaponProperty::IsFirearm ),

		TwoHandedBlade		= uint( WeaponProperty::IsTwoHanded ) | uint( WeaponProperty::IsBladed ),
		TwoHandedBlunt		= uint( WeaponProperty::IsTwoHanded ) | uint( WeaponProperty::IsBlunt ),
		TwoHandedFirearm	= uint( WeaponProperty::IsTwoHanded ) | uint( WeaponProperty::IsFirearm ),

		SpawnsObject		= 0b10000000,

		None				= 0b00000000 // here simply for the hell of it
	};

	const string[] WeaponPropertyStrings = {
		"OneHandedBlade",
		"OneHandedBlunt",
		"OneHandedPolearm",
		"OneHandedSideFirearm",
		"OneHandedPrimFirearm",

		"TwoHandedBlade",
		"TwoHandedBlunt",
		"TwoHandedPolearm",
		"TwoHandedSideFirearm",
		"TwoHandedPrimFirearm",

		"IsOneHanded",
		"IsTwoHanded",
		"IsBladed",
		"IsBlunt",
		"IsPolearm",
		"IsFirearm",

		"SpawnsObject"
	};

	const string[] ItemTypeStrings = {
		"Powerup",
		"Consumable"
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
		"Birdshot",
		"Shrapnel",
		"Slug",

		"Explosive",
		"Incendiary",
		"Tracer",
		"SubSonic"
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
};