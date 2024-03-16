#include "entity.as"
#include "game.as"
#include "convar.as"

namespace TheNomad::SGame {
	shared enum ItemType {
		IT_Weapon = 0,
		IT_Powerup,
		IT_Mana, // rage meter refill
		IT_Stim, // healthpack

		NumItemTypes
	};

	// custom weapon types will be implemented in Valden in a later version
	shared enum WeaponType {
		WT_ShottyDB = 0, // double-barreled shotgun (Asturion DB)
		WT_ShottyFAB, // full-auto shotgun
		WT_ShottyTriBurst, // 3-burst shotgun
		WT_Murstar, // murstar revolver
		WT_Asturion8Shot, // Asturion 8-shot heavy shotty
		WT_PlasmaSMG,

		NumWeaponTypes
	};

	shared class WeaponObject : EntityObject {
		WeaponObject() {
		}

		WeaponType GetWeaponType() const {
			return m_nType;
		}

		void Think() {
		}
		void Spawn( uint id, const vec3& in origin, EntityObject@ base ) override {
			m_nType = WeaponType( id );
			@m_Base = @base;
		}

		private WeaponType m_nType;
	};
	
	shared class ItemObject : EntityObject {
		ItemObject() {
		}
		
		ItemType GetItemType() const {
			return m_nType;
		}

		void Think() {
		}
		void Spawn( uint id, const vec3& in origin, EntityObject@ base ) override {
			m_nType = ItemType( id );
			@m_Base = @base;
		}
		const EntityObject@ GetBase() const {
			return m_Base;
		}
		EntityObject@ GetBase() {
			return m_Base;
		}
		
		private ItemType m_nType;
		ItemObject@ m_Next;
		ItemObject@ m_Prev;
	};
	
	shared class ItemSystem : TheNomad::GameSystem::GameObject {
		ItemSystem() {
		}
		
		void RemoveItem( ItemObject@ item ) {
			@item.m_Prev.m_Next = @item.m_Next;
			@item.m_Next.m_Prev = @item.m_Prev;
		}
		ItemObject@ FindItemInBounds( const TheNomad::GameSystem::BBox& in bounds ) {
			ItemObject@ item;

			for ( @item = @m_ActiveList.m_Next; @item !is null; @item = @item.m_Next ) {
				if ( TheNomad::Util::BoundsIntersect( bounds, item.m_Link.m_Bounsd ) ) {
					return item;
				}
			}
			return null;
		}
		ItemObject@ AddItem( ItemType type, int id ) {
			ItemObject@ item = ItemObject( type, id );
			
			m_ItemList.push_back( item );
			@m_ActiveList.m_Prev.m_Next = @item;
			@item.m_Prev = @m_ActiveList.m_Prev;
			@item.m_Next = @m_ActiveList;
			@m_ActiveList.m_Prev = @item;
			
			return item;
		}
		
		const string& GetName() const {
			return "ItemSystem";
		}
		void OnConsoleCommand() {
		}
		void OnRunTic() {
		}
		void OnSave() const {
			TheNomad::GameSystem::SaveSection save( GetName() );
			
			save.SaveUInt( "numItems", m_ItemList.size() );
			for ( uint i = 0; i < m_ItemList.size(); i++ ) {
				
			}
		}
		void OnLoad() {
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		
		private array<ItemObject> m_ItemList;
		private ItemObject m_ActiveList;
	};
};