#include "entity.as"
#include "game.as"
#include "convar.as"
#include "info.as"

namespace TheNomad::SGame {
	enum ItemType {
		IT_Weapon = 0,
		IT_Powerup,
		IT_Mana, // rage meter refill
		IT_Stim, // healthpack

		NumItemTypes,

		None // invalid
	};

	class WeaponObject : EntityObject {
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
	
	class ItemObject : EntityObject {
		ItemObject() {
		}
		ItemObject( ItemType type ) {
			m_nType = type;
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
	
	class ItemSystem : TheNomad::GameSystem::GameObject {
		ItemSystem() {
		}
		
		void RemoveItem( ItemObject@ item ) {
			@item.m_Prev.m_Next = @item.m_Next;
			@item.m_Next.m_Prev = @item.m_Prev;
		}
		ItemObject@ FindItemInBounds( const TheNomad::GameSystem::BBox& in bounds ) {
			ItemObject@ item;

			for ( @item = @m_ActiveList.m_Next; @item !is null; @item = @item.m_Next ) {
				if ( TheNomad::Util::BoundsIntersect( bounds, item.GetBounds() ) ) {
					return item;
				}
			}
			return null;
		}
		ItemObject@ AddItem( ItemType type ) {
			ItemObject@ item = ItemObject( type );
			
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

	ItemSystem@ ItemManager;
};