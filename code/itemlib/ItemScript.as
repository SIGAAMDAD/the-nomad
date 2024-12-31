#include "nomadmain/SGame/ItemObject.as"

namespace itemlib {
	class ItemScript {
		ItemScript() {
		}
		~ItemScript() {
			@m_EntityData = null;
		}

		void Link( TheNomad::SGame::ItemObject@ item ) {
			TheNomad::SGame::InfoSystem::ItemInfo@ info = @item.GetItemInfo();

			@m_EntityData = @item;
		}

		void OnInteraction( TheNomad::SGame::EntityObject@ user ) {
			GameError( "ItemScript::OnInteraction: pure virtual function called" );
		}
		void OnEquip( TheNomad::SGame::EntityObject@ user ) {
			GameError( "ItemScript::OnEquip: pure virtual function called" );
		}
		void OnDrop() {
			GameError( "ItemScript::OnDrop: pure virtual function called" );
		}
		void OnUse( TheNomad::SGame::EntityObject@ user ) {
			GameError( "ItemScript::OnUse: pure virtual function called" );
		}
		void OnSpawn() {
			GameError( "ItemScript::OnSpawn: pure virtual function called" );
		}

		protected TheNomad::SGame::ItemObject@ m_EntityData = null;
	};
};