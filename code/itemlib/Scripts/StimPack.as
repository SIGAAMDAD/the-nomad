#include "itemlib/ItemScript.as"

namespace itemlib::Script {
	class StimPack : ItemScript {
		StimPack() {
		}

		void OnUse( TheNomad::SGame::EntityObject@ user ) override {
			user.SetHealth( user.GetHealth() + 20.0f );
			user.EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/items/stimpack_activate" ), 10.0f, 0xff );

			// release
			TheNomad::SGame::EntityManager.RemoveEntity( @m_EntityData );
		}
		void OnEquip( TheNomad::SGame::EntityObject@ user ) override {
			OnUse( @user );
		}
		void OnSpawn() override {
		}
		void OnDrop() override {
		}
	};
};