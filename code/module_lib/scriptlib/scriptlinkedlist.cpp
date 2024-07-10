#include "scriptlinkedlist.h"


static void CleanupTypeInfoArrayCache(asITypeInfo *type)
{
	SArrayCache *cache = reinterpret_cast<SArrayCache *>( type->GetUserData( ARRAY_CACHE ) );
	if ( cache ) {
		cache->~SArrayCache();
		Mem_Free( (byte *)cache );
	}
}
