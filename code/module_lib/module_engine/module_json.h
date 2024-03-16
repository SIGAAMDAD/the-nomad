#ifndef __MODULE_JSON__
#define __MODULE_JSON__

#pragma once

#include "../module_public.h"

class CModuleJsonObject
{
public:
    using JsonObject = nlohmann::json;

    CModuleJsonObject( void );
    CModuleJsonObject( const string_t *fileName );
    CModuleJsonObject( const CModuleJsonObject& );
    CModuleJsonObject( JsonObject& );
    ~CModuleJsonObject() = default;

    CModuleJsonObject& operator=( const CModuleJsonObject& ) = default;

    void SetFloat( const string_t *name, float value );
    void SetInt( const string_t *name, int32_t value );
    void SetUInt( const string_t *name, uint32_t value );
    void SetString( const string_t *name, const string_t *value );
    void SetObject( const string_t *name, const CModuleJsonObject *value );
    void SetFloatArray( const string_t *name, const CScriptArray *value );
    void SetIntArray( const string_t *name, const CScriptArray *value );
    void SetUIntArray( const string_t *name, const CScriptArray *value );
    void SetStringArray( const string_t *name, const CScriptArray *value );
    void SetObjectArray( const string_t *name, const CScriptArray *value );

    bool GetObject( const string_t *name, CModuleJsonObject *data );
    bool GetFloat( const string_t *name, float *data );
    bool GetUInt( const string_t *name, uint32_t *data );
    bool GetInt( const string_t *name, int32_t *data );
    bool GetString( const string_t *name, string_t *data );
    bool GetBool( const string_t *name, bool *value );
    bool GetObjectArray( const string_t *name, CScriptArray *data );
    bool GetFloatArray( const string_t *name, CScriptArray *data );
    bool GetIntArray( const string_t *name, CScriptArray *data );
    bool GetUIntArray( const string_t *name, CScriptArray *data );
    bool GetStringArray( const string_t *name, CScriptArray *data );
    bool GetBoolArray( const string_t *name, CScriptArray *data );

    bool Parse( const string_t *fileName );

    CThreadAtomic<int32_t> m_nRefCount;
    qboolean m_bGCFlag;
    JsonObject handle;
};

CModuleJsonObject::CModuleJsonObject( void ) {
    m_nRefCount = 1;
}

CModuleJsonObject::CModuleJsonObject( JsonObject& json ) {
    handle = json;
    m_nRefCount = 1;
}

CModuleJsonObject::CModuleJsonObject( const string_t *fileName ) {
    Parse( fileName );
    m_nRefCount = 1;
}

CModuleJsonObject::CModuleJsonObject( const CModuleJsonObject& other ) {
    *this = other;
}

void CModuleJsonObject::SetFloat( const string_t *name, float value ) {
    handle[ name->c_str() ] = value;
}

void CModuleJsonObject::SetInt( const string_t *name, int32_t value ) {
    handle[ name->c_str() ] = value;
}

void CModuleJsonObject::SetUInt( const string_t *name, uint32_t value ) {
    handle[ name->c_str() ] = value;
}

void CModuleJsonObject::SetString( const string_t *name, const string_t *value ) {
    handle[ name->c_str() ] = value->c_str();
}

void CModuleJsonObject::SetObject( const string_t *name, const CModuleJsonObject *value ) {
    handle[ name->c_str() ] = value->handle;
}

void CModuleJsonObject::SetFloatArray( const string_t *name, const CScriptArray *value ) {
    JsonObject& v = handle[ name->c_str() ];
    for ( uint32_t i = 0; i < value->GetSize(); i++ ) {
        v.emplace_back( *(float *)value->At( i ) );
    }
}

void CModuleJsonObject::SetIntArray( const string_t *name, const CScriptArray *value ) {
    JsonObject& v = handle[ name->c_str() ];
    for ( uint32_t i = 0; i < value->GetSize(); i++ ) {
        v.emplace_back( *(int32_t *)value->At( i ) );
    }
}

void CModuleJsonObject::SetUIntArray( const string_t *name, const CScriptArray *value ) {
    JsonObject& v = handle[ name->c_str() ];
    for ( uint32_t i = 0; i < value->GetSize(); i++ ) {
        v.emplace_back( *(uint32_t *)value->At( i ) );
    }
}

void CModuleJsonObject::SetStringArray( const string_t *name, const CScriptArray *value ) {
    JsonObject& v = handle[ name->c_str() ];
    for ( uint32_t i = 0; i < value->GetSize(); i++ ) {
        v.emplace_back( ( (const string_t *)value->At( i ) )->c_str() );
    }
}

void CModuleJsonObject::SetObjectArray( const string_t *name, const CScriptArray *value ) {
    JsonObject& v = handle[ name->c_str() ];
    for ( uint32_t i = 0; i < value->GetSize(); i++ ) {
        v.emplace_back( ( (const CModuleJsonObject *)value->At( i ) )->handle );
    }
}

bool CModuleJsonObject::GetObject( const string_t *name, CModuleJsonObject *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    data->handle = handle.at( name->c_str() );
    return true;
}
bool CModuleJsonObject::GetFloat( const string_t *name, float *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    *data = handle.at( name->c_str() );
    return true;
}
bool CModuleJsonObject::GetUInt( const string_t *name, uint32_t *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    *data = handle.at( name->c_str() );
    return true;
}
bool CModuleJsonObject::GetInt( const string_t *name, int32_t *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    *data = handle.at( name->c_str() );
    return true;
}
bool CModuleJsonObject::GetString( const string_t *name, string_t *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    *data = (string_t &&)eastl::move( handle.at( name->c_str() ) );
    return true;
}
bool CModuleJsonObject::GetBool( const string_t *name, bool *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    *data = handle.at( name->c_str() );
    return true;
}
bool CModuleJsonObject::GetObjectArray( const string_t *name, CScriptArray *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    const JsonObject& json = handle.at( name->c_str() );
    data->Resize( json.size() );
    for ( uint32_t i = 0; i < json.size(); i++ ) {
        ( (CModuleJsonObject *)data->At( i ) )->handle = json.at( i );
    }
    return true;
}
bool CModuleJsonObject::GetFloatArray( const string_t *name, CScriptArray *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    const JsonObject& json = handle.at( name->c_str() );
    data->Resize( json.size() );
    for ( uint32_t i = 0; i < json.size(); i++ ) {
        *(float *)data->At( i ) = json.at( i );
    }
    return true;
}
bool CModuleJsonObject::GetIntArray( const string_t *name, CScriptArray *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    const JsonObject& json = handle.at( name->c_str() );
    data->Resize( json.size() );
    for ( uint32_t i = 0; i < json.size(); i++ ) {
        *(int32_t *)data->At( i ) = json.at( i );
    }
    return true;
}
bool CModuleJsonObject::GetUIntArray( const string_t *name, CScriptArray *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    const JsonObject& json = handle.at( name->c_str() );
    data->Resize( json.size() );
    for ( uint32_t i = 0; i < json.size(); i++ ) {
        *(uint32_t *)data->At( i ) = json.at( i );
    }
    return true;
}
bool CModuleJsonObject::GetStringArray( const string_t *name, CScriptArray *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    const JsonObject& json = handle.at( name->c_str() );
    data->Resize( json.size() );
    for ( uint32_t i = 0; i < json.size(); i++ ) {
        *(string_t *)data->At( i ) = (string_t &&)json.at( i );
    }
    return true;
}
bool CModuleJsonObject::GetBoolArray( const string_t *name, CScriptArray *data ) {
    if ( !handle.contains( name->c_str() ) ) {
        return false;
    }
    const JsonObject& json = handle.at( name->c_str() );
    data->Resize( json.size() );
    for ( uint32_t i = 0; i < json.size(); i++ ) {
        *(bool *)data->At( i ) = json.at( i );
    }
    return true;
}


bool CModuleJsonObject::Parse( const string_t *fileName ) {
    try {
        handle = JsonObject::parse( fileName->c_str() );
    } catch ( const JsonObject::exception& e ) {
        Con_Printf( COLOR_RED "ERROR: nlohmann::json exception occurred when parsing '%s' ->\n"
                    COLOR_RED "\tid: %i\n"
                    COLOR_RED "\tmessage: %s\n"
        , fileName->c_str(), e.id, e.what() );
        return false;
    }
    return true;
}

#endif