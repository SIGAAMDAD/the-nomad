namespace itemlib::Script {
	class ShellsPickup : ItemScript {
		ShellsPickup() {
		}

		void OnEquip( TheNomad::SGame::EntityObject@ user ) override {
			TheNomad::SGame::PlayrObject@ player = cast<TheNomad::SGame::PlayrObject@>( @user );

			player.GetInventory().AddAmmo( @TheNomad::SGame::InfoSystem::InfoManager.GetAmmoInfo(
				TheNomad::SGame::InfoSystem::InfoManager.GetAmmoType( "ammo_shells" ).GetID() ) );
		}
		void OnUse( TheNomad::SGame::EntityObject@ user ) override {
		}
		void OnDrop() override {
		}
		void OnSpawn() override {
		}
	};
};