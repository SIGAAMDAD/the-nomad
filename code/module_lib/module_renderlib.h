#ifndef __MODULE_RENDERLIB__
#define __MODULE_RENDERLIB__

#pragma once

#include "../rendercommon/r_public.h"
#include "../rendercommon/r_types.h"
#include "module_public.h"

class CRenderSceneRef : public asIScriptObject
{
public:
    CRenderSceneRef( void );
    CRenderSceneRef( const CRenderSceneRef & );
    CRenderSceneRef( CRenderSceneRef && ) = delete;

    CRenderSceneRef& operator=( const CRenderSceneRef & );

	// Memory management
	virtual int                     AddRef( void ) const override;
	virtual int                     Release( void ) const override;
	virtual asILockableSharedBool  *GetWeakRefFlag( void ) const override;

	// Type info
	virtual int            GetTypeId( void ) const override;
	virtual asITypeInfo   *GetObjectType( void ) const override;

	// Class properties
	virtual asUINT      GetPropertyCount( void ) const override;
	virtual int         GetPropertyTypeId( asUINT prop ) const override;
	virtual const char *GetPropertyName( asUINT prop ) const override;
	virtual void       *GetAddressOfProperty( asUINT prop ) override;

	// Miscellaneous
	virtual asIScriptEngine *GetEngine( void ) const override;
	virtual int              CopyFrom( const asIScriptObject *other ) override;

    static void Register( void );
public:
    renderSceneRef_t refdef;
    int32_t m_nRefCount;
protected:
	virtual ~CRenderSceneRef() override = default;
};

#endif