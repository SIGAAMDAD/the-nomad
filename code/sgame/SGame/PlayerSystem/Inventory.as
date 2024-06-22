namespace TheNomad::SGame {
    class InventoryStack {
        InventoryStack() {
        }
        InventoryStack( InfoSystem::ItemInfo@ type ) {
            Clear();
            @m_Type = @type;
            m_StackList.Reserve( m_Type.maxStackSize );
        }

        void PushItem( ItemObject@ item ) {
            if ( @item.GetInfo() !is @m_Type ) {
                ConsolePrint( "InventoryStack::PushItem: item doesn't have the same type as this slot\n" );
                return;
            }
            if ( m_StackList.Count() == m_Type.maxStackSize ) {
                return;
            }
            m_StackList.Add( @item );
        }
        ItemObject@ PopItem() {
            ItemObject@ item = @m_StackList[ m_StackList.Count() - 1 ];
            m_StackList.RemoveLast();
            return @item;
        }
        array<ItemObject@>@ GetItemList( uint nItems ) {
            array<ItemObject@> items;

            items.Reserve( nItems );
            for ( uint i = 0; i < nItems; i++ ) {
                @items[ i ] = @m_StackList[i];
                m_StackList.RemoveAt( i );
            }

            return @items;
        }
        array<ItemObject@>@ GetItemListFrom( uint nFirstElem, uint nItems ) {
            array<ItemObject@> items;

            if ( nFirstElem + nItems >= m_StackList.Count() ) {
                DebugPrint( "InventoryStack::GetItemListFrom: out of range\n" );
            }

            items.Reserve( nFirstElem + nItems );
            for ( uint i = 0; i < nItems; i++ ) {
                if ( nFirstElem + i >= m_StackList.Count() ) {
                    break; // dont go over
                }
                items.Add( @m_StackList[ nFirstElem + i ] );
                m_StackList.RemoveAt( nFirstElem + i );
            }

            return @items;
        }

        InfoSystem::ItemInfo@ GetItemType() {
            return @m_Type;
        }
        const InfoSystem::ItemInfo@ GetItemType() const {
            return @m_Type;
        }

        void Clear() {
            @m_Type = null;
            m_StackList.Clear();
        }
        uint Count() const {
            return m_StackList.Count();
        }

        private array<ItemObject@> m_StackList;
        private InfoSystem::ItemInfo@ m_Type = null;
    };

    const uint MAX_INVENTORY_SLOTS = 50;

    class InventoryManager {
        InventoryManager() {
        }

        private InventoryStack@ GetFreeStackSlot( InfoSystem::ItemInfo@ info ) {
            for ( uint i = 0; i < m_SlotList.Count(); i++ ) {
                if ( @m_SlotList[i].GetItemType() is null ) {
                    // empty type means empty slot
                    m_SlotList[i] = InventoryStack( @info );
                    return @m_SlotList[i];
                }
            }
            return null;
        }

        void AddItem( ItemObject@ item ) {
            InventoryStack@ stack = null;
            InfoSystem::ItemInfo@ itemType = cast<InfoSystem::ItemInfo@>( @item.GetInfo() );

            DebugPrint( "Pushing item of type " + itemType.type + " to inventory stack\n" );

            for ( uint i = 0; i < m_SlotList.Count(); i++ ) {
                if ( @itemType is @m_SlotList[i].GetItemType() ) {
                    @stack = @m_SlotList[i];
                    break;
                }
            }
            if ( @stack is null ) {
                @stack = GetFreeStackSlot( @itemType );
                if ( @stack is null ) {
                    return; // no free slots
                }
            }

            stack.PushItem( @item );
        }

        private InventoryStack[] m_SlotList( MAX_INVENTORY_SLOTS );
    };
};