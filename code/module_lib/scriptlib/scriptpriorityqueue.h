#ifndef __SCRIPT_PRIORITY_QUEUE__
#define __SCRIPT_PRIORITY_QUEUE__

#pragma once

#include "scriptarray.h"
#include <EASTL/priority_queue.h>

class CScriptPriorityQueue
{
public:
    static CScriptPriorityQueue *Create( asITypeInfo *ot );
	static CScriptPriorityQueue *Create( asITypeInfo *ot, asUINT length );
	static CScriptPriorityQueue *Create( asITypeInfo *ot, asUINT length, void *defaultValue );
	static CScriptPriorityQueue *Create( asITypeInfo *ot, void *listBuffer );

    bool IsEmpty( void ) const;
    asUINT GetSize( void ) const;

    const void *Top( void ) const;
    void Push( const void *value );
    void Pop( void );
private:
    CScriptArray mBase;
};

#endif