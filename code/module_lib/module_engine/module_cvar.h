#ifndef __MODULE_CVAR__
#define __MODULE_CVAR__

#pragma once

#include "../module_public.h"

class ConVar
{
public:
    ConVar( asIScriptGeneric *pGeneric );
    ConVar( const ConVar& );

    void Set( const UtlString& value );
    void Update( void );
    ConVar& operator=( const ConVar& );

	virtual int                    AddRef( void ) const;
	virtual int                    Release( void ) const;
	virtual asILockableSharedBool *GetWeakRefFlag( void ) const;

	virtual int            GetTypeId( void ) const;
	virtual asITypeInfo   *GetObjectType( void ) const;

	virtual asUINT      GetPropertyCount( void ) const;
	virtual int         GetPropertyTypeId( asUINT prop ) const;
	virtual const char *GetPropertyName( asUINT prop ) const;
	virtual void       *GetAddressOfProperty( asUINT prop );

	virtual asIScriptEngine *GetEngine( void ) const;
	virtual int              CopyFrom( const asIScriptObject *other );

	virtual void *SetUserData( void *data, asPWORD type = 0 );
	virtual void *GetUserData( asPWORD type = 0 ) const;

    UtlString m_Name;
    UtlString m_Value;

    int64_t m_IntValue;
    float m_FloatValue;
    int32_t m_nRefCount;
    uint32_t m_bTrackChanges;
    cvarHandle_t m_hCvar;
    uint32_t m_nModifiedCount;
};

ConVar::ConVar( asIScriptGeneric *pGeneric ) {
    vmCvar_t vmCvar;

    m_Name = *(const UtlString *)pGeneric->GetArgObject( 1 );
    m_Value = *(const UtlString *)pGeneric->GetArgObject( 2 );
    m_bTrackChanges = pGeneric->GetArgDWord( 4 );

    memset( &vmCvar, 0, sizeof(vmCvar) );
    Cvar_Register( &vmCvar, m_Name.c_str(), m_Value.c_str(), pGeneric->GetArgDWord( 3 ), 0 );

    m_IntValue = vmCvar.i;
    m_FloatValue = vmCvar.f;
    m_nRefCount = 1;
    m_hCvar = vmCvar.handle;
    m_nModifiedCount = vmCvar.modificationCount;
}

ConVar::ConVar( const ConVar& other ) {
    m_nRefCount = 1;
    AddRef();
    *this = other;
}

ConVar& ConVar::operator=( const ConVar& other ) {
    m_Name = other.m_Name;
    m_Value = other.m_Value;
    m_IntValue = other.m_IntValue;
    m_FloatValue = other.m_FloatValue;
    m_bTrackChanges = other.m_bTrackChanges;
    m_nModifiedCount = other.m_nModifiedCount;
    m_hCvar = other.m_hCvar;
    return *this;
}

void ConVar::Set( const UtlString& value ) {
    Cvar_Set( m_Name.c_str(), value.c_str() );
}

void ConVar::Update( void ) {
    vmCvar_t vmCvar;

    memset( &vmCvar, 0, sizeof(vmCvar) );
    vmCvar.handle = m_hCvar;
    vmCvar.modificationCount = m_nModifiedCount;
    N_strncpyz( vmCvar.s, m_Value.c_str(), sizeof(vmCvar.s) );
    vmCvar.i = m_IntValue;
    vmCvar.f = m_FloatValue;
    Cvar_Update( &vmCvar, CVAR_PRIVATE );

    m_Value = vmCvar.s;
    m_IntValue = vmCvar.i;
    m_FloatValue = vmCvar.f;
    m_nModifiedCount = vmCvar.modificationCount;
    m_hCvar = vmCvar.handle;
}

int ConVar::AddRef( void ) const {
    const_cast<ConVar *>( this )->m_nRefCount++;
    return asSUCCESS;
}

int ConVar::Release( void ) const {
    return asSUCCESS;
}

asILockableSharedBool *ConVar::GetWeakRefFlag( void ) const {
    return NULL;
}

int ConVar::GetTypeId( void ) const {
    return asTYPEID_APPOBJECT;
}

asITypeInfo *ConVar::GetObjectType( void ) const {
    return NULL;
}

asUINT ConVar::GetPropertyCount( void ) const {
    return 5;
}

int ConVar::GetPropertyTypeId( asUINT prop ) const {
    switch ( prop ) {
    case 0: return asTYPEID_HANDLETOCONST;
    case 1: return asTYPEID_FLOAT;
    case 2: return asTYPEID_INT64;
    };
    return asINVALID_ARG;
}

const char *ConVar::GetPropertyName( asUINT prop ) const {
    switch ( prop ) {
    case 0: return "m_Value";
    case 1: return "m_FloatValue";
    case 2: return "m_IntValue";
    };

    return "ConVar::GetPropertyName: invalid prop index";
}

void *ConVar::GetAddressOfProperty( asUINT prop ) {
    switch ( prop ) {
    case 0: return &m_Value;
    case 1: return &m_FloatValue;
    case 2: return &m_IntValue;
    };

    Con_Printf( COLOR_RED "ConVar::GetAddressOfProperty: invalid prop index\n" );
    return NULL;
}

asIScriptEngine *ConVar::GetEngine( void ) const {
    return g_pModuleLib->GetScriptEngine();
}

int ConVar::CopyFrom( const asIScriptObject *other ) {
    int typeId = other->GetTypeId();

    if ( !( typeId & asTYPEID_APPOBJECT ) ) {
        Con_Printf( COLOR_RED "ConVar::CopyFrom: object isn't a asTYPEID_HANDLETOCONST\n" );
        return asERROR;
    }

    *this = *dynamic_cast<ConVar *>( const_cast<asIScriptObject *>( other ) );

    return asSUCCESS;
}

void *ConVar::SetUserData( void *data, asPWORD type ) {
    return NULL;
}

void *ConVar::GetUserData( asPWORD type ) const {
    return NULL;
}

#endif