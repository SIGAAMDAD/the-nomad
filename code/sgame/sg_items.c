#include "sg_local.h"

static item_t *sg_itemList;

item_t *Item_FindInBounds( const bbox_t *bounds )
{
    item_t *it;

    for ( it = sg_itemList; it; it = it->next ) {
        if ( BoundsIntersect( bounds, &it->ent->link.bounds ) ) {
            return it;
        }
    }
    return NULL;
}

void Item_Remove( item_t *item )
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    sg.numEntities--;
    sg.numItems--;
}

item_t *SG_SpawnItem( itemtype_t type )
{
    item_t *item;

    if ( type >= NUMITEMS ) {
        trap_Error( "SG_SpawnItem: incompatible mod with sgame, type >= NUMITEMS" );
    }
    if ( sg.numItems == MAXITEMS ) {
        trap_Error( "SG_SpawnItem: MAXITEMS hit" );
    }

    item = &sg.items[sg.numItems];
    memset( item, 0, sizeof(*item) );
    item->type = type;

    sg.numItems++;

    if ( !sg_itemList ) {
        sg_itemList = item;
    }

    item->prev = sg_itemList;

    return item;
}

weapon_t *SG_SpawnWeapon( weapontype_t type )
{
    item_t *item;
    weapon_t *w;

    if ( type >= NUMWEAPONTYPES ) {
        trap_Error( "SG_SpawnWeapon: incompatible mod with sgame, type >= NUMWEAPONS" );
    }
    if ( sg.numWeapons == MAXWEAPONS ) {
        trap_Error( "SG_SpawnWeapon: MAXWEAPONS hit" );
    }

    w = &sg.weapons[sg.numWeapons];
    memset( w, 0, sizeof(*w) );
    sg.numWeapons++;

    item = SG_SpawnItem( I_WEAPON );
    w->base = item;

    return w;
}

