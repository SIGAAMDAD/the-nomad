#include "sg_local.h"

static item_t sg_items[MAXITEMS];

item_t *SG_AllocItem( itemtype_t type )
{
    item_t *item;

    if ( sg.numItems == MAXITEMS ) {
        trap_Error( "SG_AllocItem: MAXITEMS hit" );
    }

    item = &sg_items[sg.numItems];
    
    memset( item, 0, sizeof(*item) );
    memcpy( item, &iteminfo, sizeof(item_t) );

    item->type = type;
    item->ent = SG_AllocEntity( ET_ITEM );
    item->ent->entPtr = item;

    sg.numItems++;

    return item;
}

