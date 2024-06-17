#ifndef __SCRIPTSTACK_H__
#define __SCRIPTSTACK_H__

#include "../module_public.h"

class CScriptStack : public CScriptArray
{
public:
	void Push( void *pData );
	void Pop( void );
};

void RegisterScriptStack( asIScriptEngine *engine );

#endif
