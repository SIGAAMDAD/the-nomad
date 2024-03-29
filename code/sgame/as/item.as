#include "entity.as"
#include "game.as"
#include "convar.as"
#include "info.as"

namespace TheNomad::SGame {
	class WeaponObject : EntityObject {
		WeaponObject() {
		}

		WeaponType GetWeaponType() const {
			return m_nType;
		}
		uint GetWeaponID() const {
			return m_nID;
		}

		void Think() {
		}
		void Spawn( uint id, const vec3& in origin ) override {
			m_nID = id;
		}

		private WeaponType m_nType;
		private uint m_nID;
	};
	
	class ItemObject : EntityObject {
		ItemObject() {
		}
		ItemObject( uint type ) {
			m_nType = type;
		}
		
		uint GetItemType() const {
			return m_nType;
		}

		void Think() {
		}
		void Spawn( uint id, const vec3& in origin ) override {
			@m_InfoData = cast<InfoLoader>( @InfoDataManager.GetItemInfo( id ) );
			m_nType = id;
		}

		private uint m_nType;
		private ItemInfo@ m_Info;
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
		ItemObject@ AddItem( uint type ) {
			ItemObject@ item = ItemObject( type );
			
			m_ItemList.Add( @item );
			@m_ActiveList.m_Prev.m_Next = @item;
			@item.m_Prev = @m_ActiveList.m_Prev;
			@item.m_Next = @m_ActiveList;
			@m_ActiveList.m_Prev = @item;
			
			return item;
		}
		
		const string& GetName() const {
			return "ItemSystem";
		}
		bool OnConsoleCommand( const string& in cmd ) {
			if ( TheNomad::Util::StrICmp( cmd, "sgame.list_items" ) ) {
				ListActiveItems();
			}

			return false;
		}
		void OnRunTic() {
		}
		void OnSave() const {
			TheNomad::GameSystem::SaveSection save( GetName() );
			
			save.SaveUInt( "NumItems", m_ItemList.Count() );
			for ( uint i = 0; i < m_ItemList.Count(); i++ ) {
				save.SaveUInt( "itemType", m_ItemList[i].GetItemType() );
				save.SaveVec3( "origin", m_ItemList[i].GetOrigin() );
			}
		}
		void OnLoad() {
			TheNomad::GameSystem::LoadSection section( GetName() );

			if ( !section.Found() ) {
				ConsoleWarning( "ItemSystem::OnLoad: no save section for item data found\n" );
				return;
			}
		}
		void OnInit() {
		}
		void OnShutdown() {
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}

		private void ListActiveItems() {
			string msg;
			msg.reserve( MAX_STRING_CHARS );

			ConsolePrint( "Active Game Items:\n" );
			for ( uint i = 0; i < m_ItemList.Count(); i++ ) {
				msg = "(";
				msg += i;
				msg += ") ";
				msg += m_ItemList[i].GetItemInfo().name;
				msg += " ";
				msg += "[ ";
				msg += m_ItemList[i].GetOrigin().x;
				msg += ", ";
				msg += m_ItemList[i].GetOrigin().y;
				msg += ", ";
				msg += m_ItemList[i].GetOrigin().z;
				msg += " ]\n";
				ConsolePrint( msg );
			}
		}
		
		private array<ItemObject@> m_ItemList;
		private ItemObject m_ActiveList;
	};

	ItemSystem@ ItemManager;
};