#include "entity.as"
#include "game.as"
#include "convar.as"

namespace TheNomad::SGame {
	shared class WeaponObject : EntityObject {
		WeaponObject() {
		}
	};
	
	shared class ItemObject : EntityObject {
		ItemObject( ItemType type, int id ) {
			m_nType = type;
			m_nId = id;
		}
		
		ItemType GetType() const {
			return m_nType;
		}
		
		private ItemType m_nType;
		private int m_nId;
	};
	
	shared class ItemSystem : TheNomad::GameSystem::GameObject {
		ItemSystem() {
			m_Name = "ItemSystem";
		}
		
		void RemoveItem( ItemObject@ item ) {
			
		}
		ItemObject@ FindItemInBounds( const BBox& in bounds ) {
			for ( uint i = 0; i < m_ItemList.size(); i++ ) {
				if ( BoundsIntersect( bounds, m_ItemList[i].GetLink().GetBounds() ) ) {
					return m_ItemList[i];
				}
			}
			return null;
		}
		ItemObject@ AddItem( ItemType type, int id ) {
			ItemObject@ item = ItemObject( type, id );
			
			m_ItemList.push_back( item );
			m_ActiveList.insert( item );
			
			return item;
		}
		
		void OnLoad() override {
			
		}
		void OnSave() const override {
			TheNomad::GameSystem::SaveSection save( "ItemSystem" );
			
			save.SaveUInt( "numItems", m_ItemList.size() );
			for ( uint i = 0; i < m_ItemList.size(); i++ ) {
				
			}
		}
		void OnRunTic() override {
		}
		void OnLevelStart( const MapData@ in mapData ) override {
			for ( uint i = 0; i < mapData.GetSpawns(); i++ ) {
				
			}
		}
		void OnLevelEnd() override {
		}
		void OnKeyEvent( int key, int down ) override {
		}
		void OnMouseEvent( int dx, int dy ) override {
		}
		void OnConsoleCommand( const TheNomad::Engine::CmdArgs& in args ) override {
		}
		
		private array<ItemObject> m_ItemList;
		private linked_list<ItemObject> m_ActiveList;
		private single_linked_list<ItemObject> m_FreeList;
	};
};