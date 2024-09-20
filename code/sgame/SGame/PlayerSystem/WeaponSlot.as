namespace TheNomad::SGame {
	class WeaponSlot {
		WeaponSlot() {
		}
		
		WeaponObject@ Base = null;
		uint Index = 0;
		InfoSystem::WeaponProperty Mode = InfoSystem::WeaponProperty::None;
	};
};