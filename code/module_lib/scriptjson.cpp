#include "module_public.h"
#include "scriptjson.h"
#include "scriptarray.h"

BEGIN_AS_NAMESPACE

CScriptJson *CScriptJson::Create(asIScriptEngine *engine)
{
    // Use the custom memory routine from AngelScript to allow application to better control how much memory is used
    CScriptJson *obj = (CScriptJson*)asAllocMem(sizeof(CScriptJson));
    new(obj) CScriptJson(engine);
    return obj;
}

CScriptJson *CScriptJson::Create(asIScriptEngine *engine, json js)
{
    // Use the custom memory routine from AngelScript to allow application to better control how much memory is used
    CScriptJson *obj = (CScriptJson*)asAllocMem(sizeof(CScriptJson));
    new(obj) CScriptJson(engine);
    *(obj->js_info) = js;
    return obj;
}

void CScriptJson::AddRef() const
{
    asAtomicInc(refCount);
}

void CScriptJson::Release() const
{
    if( asAtomicDec(refCount) == 0 )
    {
        this->~CScriptJson();
        asFreeMem(const_cast<CScriptJson*>(this));
    }
}

CScriptJson &CScriptJson::operator =(bool other)
{
    // Clear everything we had before
    js_info->clear();

    *js_info = other;

    return *this;
}

CScriptJson &CScriptJson::operator =(json::number_integer_t other)
{
    // Clear everything we had before
    js_info->clear();

    *js_info = other;

    return *this;
}

CScriptJson &CScriptJson::operator =(json::number_unsigned_t other)
{
    // Clear everything we had before
    js_info->clear();

    *js_info = other;

    return *this;
}

CScriptJson &CScriptJson::operator =(json::number_float_t other)
{
    // Clear everything we had before
    js_info->clear();

    *js_info = other;

    return *this;
}

CScriptJson &CScriptJson::operator =(const string_t &other)
{
    // Clear everything we had before
    js_info->clear();

    *js_info = other.c_str();

    return *this;
}

CScriptJson &CScriptJson::operator =(const CScriptArray &other)
{
    json js_temp = json::array({});
    for (asUINT i = 0; i < other.GetSize(); i++)
    {
        CScriptJson** node  = (CScriptJson**)other.At(i);
        if (node && *node)
        {
            js_temp += *(*node)->js_info;
        }
    }

    // Clear everything we had before
    js_info->clear();

    *js_info = js_temp;

    return *this;
}

CScriptJson &CScriptJson::operator =(const CScriptJson &other)
{
    // Clear everything we had before
    js_info->clear();

    *js_info = *other.js_info;

    return *this;
}

void CScriptJson::Set(const jsonKey_t &key, const bool &value)
{
    (*js_info)[key.c_str()] = value;
}

void CScriptJson::Set(const jsonKey_t &key, const json::number_integer_t &value)
{
    (*js_info)[key.c_str()] = value;
}

void CScriptJson::Set(const jsonKey_t &key, const json::number_unsigned_t &value)
{
    (*js_info)[key.c_str()] = value;
}

void CScriptJson::Set(const jsonKey_t &key, const json::number_float_t &value)
{
    (*js_info)[key.c_str()] = value;
}

void CScriptJson::Set(const jsonKey_t &key, const string_t &value)
{
    (*js_info)[key.c_str()] = value.c_str();
}

void CScriptJson::Set(const jsonKey_t &key, const CScriptArray &value)
{
    json js_temp = json::array({});
    for (asUINT i = 0; i < value.GetSize(); i++)
    {
        CScriptJson** node  = (CScriptJson**)value.At(i);
        if (node && *node)
        {
            js_temp += *(*node)->js_info;
        }
    }
    (*js_info)[key.c_str()] = js_temp;
}

bool CScriptJson::Get(const jsonKey_t &key, bool &value) const
{
    if(js_info->contains(key.c_str()))
    {
        if(js_info->is_boolean())
        {
            value = (*js_info)[key.c_str()];
            return true;
        }
    }
    return false;
}

bool CScriptJson::Get(const jsonKey_t &key, json::number_integer_t &value) const
{
    if(js_info->contains(key.c_str()))
    {
        if(js_info->is_number())
        {
            value = (*js_info)[key.c_str()];
            return true;
        }
    }
    return false;
}

bool CScriptJson::Get(const jsonKey_t &key, json::number_unsigned_t &value) const
{
    if(js_info->contains(key.c_str()))
    {
//        if(js_info->is_number())
//        {
            value = (*js_info)[key.c_str()];
            return true;
//        }
    }
    return false;
}

bool CScriptJson::Get(const jsonKey_t &key, json::number_float_t &value) const
{
    if(js_info->contains(key.c_str()))
    {
//        if(js_info->is_number())
//        {
            value = (*js_info)[key.c_str()];
            return true;
//        }
    }
    return false;
}

bool CScriptJson::Get(const jsonKey_t &key, string_t &value) const
{
    if(js_info->contains(key.c_str()))
    {
//        if(js_info->is_string())
//        {
            value = (*js_info)[key.c_str()].get<std::string>().c_str();
            return true;
//        }
    }
    return false;
}

bool CScriptJson::Get(const jsonKey_t &key, CScriptArray &value) const
{
    if(!js_info->contains(key.c_str()) || !(*js_info)[key.c_str()].is_array())
        return false;

    json& js_temp = (*js_info)[key.c_str()];
    if ( value.GetSize() < js_temp.size() ) {
        value.Resize(js_temp.size());
    }

    for (asUINT i = 0; i < js_temp.size(); ++i)
    {
        CScriptJson* childNode = Create(engine);
        *(childNode->js_info) = js_temp[i];
        value.SetValue(i, &childNode);
        childNode->Release();
    }
    return true;
}

bool CScriptJson::GetBool()
{
    return *js_info;
}

json::number_integer_t CScriptJson::GetNumber()
{
    return *js_info;
}

json::number_unsigned_t CScriptJson::GetUNumber()
{
    return *js_info;
}

json::number_float_t CScriptJson::GetReal()
{
    return *js_info;
}

string_t CScriptJson::GetString()
{
    return js_info->get<std::string>().c_str();
}

CScriptArray* CScriptJson::GetArray()
{
    CScriptArray* retVal = CScriptArray::Create(engine->GetTypeInfoByDecl("array<JsonValue@>"));
    for (json::iterator it = js_info->begin(); it != js_info->end(); ++it)
    {
        CScriptJson* childNode = CScriptJson::Create(engine, *it);

        retVal->InsertLast(childNode);
        childNode->Release();
    }
    return retVal;
}

CScriptJson *CScriptJson::operator[](const jsonKey_t &key)
{
    CScriptJson* retVal = Create(engine);
    retVal->js_info = &(*js_info)[key.c_str()];
    // Return the existing value if it exists, else insert an empty value
    return retVal;
}

const CScriptJson *CScriptJson::operator[](const jsonKey_t &key) const
{
    if(js_info->contains(key.c_str()))
    {
        CScriptJson* retVal = Create(engine);
        *(retVal->js_info) = (*js_info)[key.c_str()];
        return retVal;
    }

    // Else raise an exception
    asIScriptContext *ctx = asGetActiveContext();
    if( ctx )
        ctx->SetException("Invalid access to non-existing value");

    return 0;
}

bool CScriptJson::Exists(const jsonKey_t &key) const
{
    return js_info->contains(key.c_str());
}

bool CScriptJson::IsEmpty() const
{
    return js_info->empty();
}

asUINT CScriptJson::GetSize() const
{
    return js_info->size();
}

void CScriptJson::Clear()
{
    js_info->clear();
}

CScriptJsonType CScriptJson::Type()
{
    switch(js_info->type())
    {
    case json::value_t::object:
        return OBJECT_VALUE;
    case json::value_t::array:
        return ARRAY_VALUE;
    case json::value_t::string:
        return STRING_VALUE;
    case json::value_t::boolean:
        return BOOLEAN_VALUE;
    case json::value_t::number_integer:
    case json::value_t::number_unsigned:
        return NUMBER_VALUE;
    case json::value_t::number_float:
        return REAL_VALUE;
    default:
        return NULL_VALUE;
    }
}

int CScriptJson::GetRefCount()
{
    return refCount;
}

CScriptJson::CScriptJson(asIScriptEngine *e)
{
    js_info = new( asAllocMem( sizeof( *js_info ) ) ) json();
    // We start with one reference
    refCount = 1;

    engine = e;
}

CScriptJson::~CScriptJson()
{
    Clear();
    asFreeMem( js_info );
}

void ScriptJsonFactory_Generic(asIScriptGeneric *gen)
{
    *(CScriptJson **)gen->GetAddressOfReturnLocation() = CScriptJson::Create(gen->GetEngine());
}

void ScriptJsonAddRef_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    json->AddRef();
}

void ScriptJsonRelease_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    json->Release();
}

void ScriptJsonAssignBool_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	*json = (bool)gen->GetArgByte(0);
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = json;
}

void ScriptJsonAssignInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	*json = (json::number_integer_t)gen->GetArgQWord(0);
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = json;
}

void ScriptJsonAssignFlt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	*json = (json::number_integer_t)gen->GetArgDouble(0);
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = json;
}

void ScriptJsonAssignStr_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    string_t *other = *(string_t**)gen->GetAddressOfArg(0);
    *json = *other;
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = json;
}

void ScriptJsonAssignArr_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    CScriptArray *other = *(CScriptArray**)gen->GetAddressOfArg(0);
    *json = *other;
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = json;
}

void ScriptJsonAssign_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    CScriptJson *other = *(CScriptJson**)gen->GetAddressOfArg(0);
    *json = *other;
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = json;
}

void ScriptJsonSetBool_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    json->Set(*key, *(bool*)ref);
}

void ScriptJsonSetInt_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    json->Set(*key, *(json::number_integer_t*)ref);
}

void ScriptJsonSetUInt_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    json->Set(*key, *(json::number_unsigned_t*)ref);
}

void ScriptJsonSetFlt_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    json->Set(*key, *(json::number_float_t*)ref);
}

void ScriptJsonSetStr_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    json->Set(*key, *(string_t*)ref);
}

void ScriptJsonSetArr_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    json->Set(*key, *(CScriptArray*)ref);
}

void ScriptJsonGetBool_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(bool*)ref);
}

void ScriptJsonGetUInt_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(json::number_unsigned_t*)ref);
}

void ScriptJsonGetInt_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(json::number_integer_t*)ref);
}

void ScriptJsonGetFlt_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    void *ref = (void*)gen->GetAddressOfArg(1);
    *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(json::number_float_t*)ref);
}

void ScriptJsonGetStr_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(string_t*)gen->GetArgObject( 1 ));
}

void ScriptJsonGetArr_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(CScriptArray*)gen->GetArgObject( 1 ));
}

void ScriptJsonExists_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    bool ret = json->Exists(*key);
    *(bool*)gen->GetAddressOfReturnLocation() = ret;
}

void ScriptJsonIsEmpty_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    bool ret = json->IsEmpty();
    *(bool*)gen->GetAddressOfReturnLocation() = ret;
}

void ScriptJsonGetSize_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    asUINT ret = json->GetSize();
    *(asUINT*)gen->GetAddressOfReturnLocation() = ret;
}

void ScriptJsonClear_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    json->Clear();
}

void ScriptJsonGetType_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetObjectData();
    CScriptJsonType ret = json->Type();
    *(CScriptJsonType*)gen->GetAddressOfReturnLocation() = ret;
}

static void CScriptJson_opIndex_Generic(asIScriptGeneric *gen)
{
    CScriptJson *self = (CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    *(CScriptJson**)gen->GetAddressOfReturnLocation() = self->operator[](*key);
}

static void CScriptJson_opIndex_const_Generic(asIScriptGeneric *gen)
{
    const CScriptJson *self = (const CScriptJson*)gen->GetObjectData();
    jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
    *(const CScriptJson**)gen->GetAddressOfReturnLocation() = self->operator[](*key);
}

static void CScriptJson_ParseFile( asIScriptGeneric *gen ) {
    CScriptJson *newNode = (CScriptJson *)gen->GetObjectData();
    const string_t& fileName = *(const string_t *)gen->GetArgObject( 0 );
    union {
        void *v;
        char *b;
    } f;
    uint64_t nLength;

    Con_Printf( "Loading json file '%s' at vm request...\n", fileName.c_str() );

    nLength = FS_LoadFile( fileName.c_str(), &f.v );
    if ( !nLength || !f.v ) {
        Con_Printf( COLOR_RED "ERROR: failed to load json file '%s' at vm request\n", fileName.c_str() );
        *(bool *)gen->GetAddressOfReturnLocation() = false;
        return;
    }

    try {
        *(newNode->js_info) = json::parse( f.b, f.b + nLength );
    } catch ( const json::exception& e ) {
        FS_FreeFile( f.v );
        N_Error( ERR_DROP, "nlohmann::json exception thrown ->\n\tid: %i\n\tmessage: %s", e.id, e.what() );
        *(bool *)gen->GetAddressOfReturnLocation() = false;
        return;
    }

    FS_FreeFile( f.v );
    *(bool *)gen->GetAddressOfReturnLocation() = true;
}


// Json to text
static bool JsonWriteFile(const CScriptJson& node,const string_t& file)
{
    FILE* outputFile = NULL;
    if((outputFile = fopen(file.c_str(),"w")) == NULL)
    {
        return false;
    }
    char *data_str;
    string_t dump_str = eastl::move( node.js_info->dump(1, '\t').c_str() );
    data_str = (char *) malloc (dump_str.length() + 1);
    strcpy(data_str, dump_str.c_str());

    fwrite (data_str,strlen(data_str),1,outputFile);
    fclose(outputFile);
    free(data_str);
    return true;
}

static bool JsonWrite(const CScriptJson& node, string_t& content)
{
    content = eastl::move( node.js_info->dump(1, '\t').c_str() );
    return true;
}

static CScriptJson* JsonParseFile(const string_t& fileName)
{
    union {
        void *v;
        char *b;
    } f;
    uint64_t nLength;

    Con_Printf( "Loading json file '%s' at vm request...\n", fileName.c_str() );

    nLength = FS_LoadFile( fileName.c_str(), &f.v );
    if ( !nLength || !f.v ) {
        Con_Printf( COLOR_RED "ERROR: failed to load json file '%s' at vm request\n", fileName.c_str() );
        return NULL;
    }

    CScriptJson* newNode = CScriptJson::Create( g_pModuleLib->GetScriptEngine() );
    try {
        *(newNode->js_info) = json::parse( f.b, f.b + nLength );
    } catch ( const json::exception& e ) {
        FS_FreeFile( f.v );
        N_Error( ERR_DROP, "nlohmann::json exception thrown ->\n\tid: %i\n\tmessage: %s", e.id, e.what() );
        return NULL;
    }

    FS_FreeFile( f.v );
    return newNode;
}

static CScriptJson* JsonParse(const string_t& str)
{
    asIScriptContext * currentContext=asGetActiveContext();
    if (currentContext)
    {
        asIScriptEngine* engine=currentContext->GetEngine();
        if (engine)
        {
            CScriptJson* newNode = CScriptJson::Create(engine);
            *(newNode->js_info) = json::parse(str.c_str());
            return newNode;
        }
    }
    return NULL;
}

static void ScriptJson_ParseFile_Generic(asIScriptGeneric *gen)
{
    string_t *file = (string_t*)gen->GetArgObject(0);

    CScriptJson* ret = JsonParseFile(*file);
    gen->SetReturnAddress( ret );
}

static void ScriptJson_Parse_Generic(asIScriptGeneric *gen)
{
    string_t *file = (string_t*)gen->GetArgObject(0);

    CScriptJson* ret = JsonParse(*file);
    gen->SetReturnAddress( ret );
}

static void ScriptJson_WriteFile_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetArgObject(0);
    string_t *file = (string_t*)gen->GetArgObject(1);

    bool ret = JsonWriteFile(*json, *file);
    *(bool*)gen->GetAddressOfReturnLocation() = ret;
}

static void ScriptJson_Write_Generic(asIScriptGeneric *gen)
{
    CScriptJson *json = (CScriptJson*)gen->GetArgObject(0);
    string_t *content = (string_t*)gen->GetArgObject(1);

    bool ret = JsonWrite(*json, *content);
    *(bool*)gen->GetAddressOfReturnLocation() = ret;
}

//--------------------------------------------------------------------------
// Register the type

/*
void RegisterScriptJson_Native(asIScriptEngine *engine)
{
    int r;

    // The array<string> type must be available
    Assert( engine->GetTypeInfoByDecl("array<string>") );

    r = engine->RegisterEnum("jsonType");
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "OBJECT_VALUE", OBJECT_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "ARRAY_VALUE", ARRAY_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "BOOLEAN_VALUE", BOOLEAN_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "STRING_VALUE", STRING_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "NUMBER_VALUE", NUMBER_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "REAL_VALUE", REAL_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "NULL_VALUE", NULL_VALUE);
    Assert(r>=0);

    r = engine->RegisterObjectType("json", sizeof(CScriptJson), asOBJ_REF);
    Assert( r >= 0 );
    // Use the generic interface to construct the object since we need the engine pointer, we could also have retrieved the engine pointer from the active context
    r = engine->RegisterObjectBehaviour("json", asBEHAVE_FACTORY, "json@ f()", asFUNCTION(ScriptJsonFactory_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("json", asBEHAVE_ADDREF, "void f()", asMETHOD(CScriptJson,AddRef), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("json", asBEHAVE_RELEASE, "void f()", asMETHOD(CScriptJson,Release), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "json &opAssign(bool)", asMETHODPR(CScriptJson, operator=, (bool), CScriptJson&), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(int64)", asMETHODPR(CScriptJson, operator=, (asINT64), CScriptJson&), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(double)", asMETHODPR(CScriptJson, operator=, (double), CScriptJson&), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(const string& in)", asMETHODPR(CScriptJson, operator=, (const string_t&), CScriptJson&), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(const array<json@> &in)", asMETHODPR(CScriptJson, operator=, (const CScriptArray &), CScriptJson&), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(const json &in)", asMETHODPR(CScriptJson, operator=, (const CScriptJson &), CScriptJson&), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const bool&in)", asMETHODPR(CScriptJson,Set,(const string_t&,const bool&),void), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, bool &out) const", asMETHODPR(CScriptJson,Get,(const string_t&,bool&) const,bool), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const int64&in)", asMETHODPR(CScriptJson,Set,(const string_t&,const asINT64&),void), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, int64 &out) const", asMETHODPR(CScriptJson,Get,(const string_t&,asINT64&) const,bool), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const double&in)", asMETHODPR(CScriptJson,Set,(const string_t&,const double&),void), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, double &out) const", asMETHODPR(CScriptJson,Get,(const string_t&,double&) const,bool), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const string&in)", asMETHODPR(CScriptJson,Set,(const string_t&,const string_t&),void), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, string &out) const", asMETHODPR(CScriptJson,Get,(const string_t&,string_t&) const,bool), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const array<json@>&in)", asMETHODPR(CScriptJson,Set,(const string_t&,const CScriptArray&),void), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, array<json@> &out) const", asMETHODPR(CScriptJson,Get,(const string_t&,CScriptArray&) const,bool), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "bool exists(const string &in) const", asMETHOD(CScriptJson,Exists), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool isEmpty() const", asMETHOD(CScriptJson, IsEmpty), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "uint getSize() const", asMETHOD(CScriptJson, GetSize), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "void clear()", asMETHOD(CScriptJson,Clear), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "jsonType getType()", asMETHOD(CScriptJson,Type), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "json &opIndex(const string &in)", asMETHODPR(CScriptJson, operator[], (const jsonKey_t &), CScriptJson*), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "const json &opIndex(const string &in) const", asMETHODPR(CScriptJson, operator[], (const jsonKey_t &) const, const CScriptJson*), asCALL_THISCALL);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "bool opConv()", asMETHOD(CScriptJson, GetBool), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "string opConv()", asMETHOD(CScriptJson, GetString), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "int opConv()", asMETHOD(CScriptJson, GetNumber), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "double opConv()", asMETHOD(CScriptJson, GetReal), asCALL_THISCALL);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "array<json@>& opConv()", asMETHOD(CScriptJson, GetArray), asCALL_THISCALL);
    Assert( r >= 0 );

    // Json functions
    r = engine->RegisterGlobalFunction("json@ jsonParseFile(const string& in)", asFUNCTIONPR(JsonParseFile, (const string_t&), CScriptJson*), asCALL_CDECL);
    Assert( r >= 0 );
    r = engine->RegisterGlobalFunction("json@ jsonParse(const string& in)", asFUNCTIONPR(JsonParse, (const string_t&), CScriptJson*), asCALL_CDECL);
    Assert( r >= 0 );
    r = engine->RegisterGlobalFunction("bool jsonWriteFile(const json& in json,const string& in)", asFUNCTIONPR(JsonWriteFile, (const CScriptJson& node,const string_t&), bool), asCALL_CDECL);
    Assert( r >= 0 );
    r = engine->RegisterGlobalFunction("bool jsonWrite(const json& in json,string& out)", asFUNCTIONPR(JsonWrite, (const CScriptJson& node,string_t&), bool), asCALL_CDECL);
    Assert( r >= 0 );
}
*/

void RegisterScriptJson_Generic(asIScriptEngine *engine)
{
    int r;

    r = engine->RegisterEnum("jsonType");
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "OBJECT_VALUE", OBJECT_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "ARRAY_VALUE", ARRAY_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "BOOLEAN_VALUE", BOOLEAN_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "STRING_VALUE", STRING_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "NUMBER_VALUE", NUMBER_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "REAL_VALUE", REAL_VALUE);
    Assert(r>=0);
    r = engine->RegisterEnumValue("jsonType", "NULL_VALUE", NULL_VALUE);
    Assert(r>=0);

    r = engine->RegisterObjectType("json", sizeof(CScriptJson), asOBJ_REF);
    Assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("json", asBEHAVE_FACTORY, "json@ f()", asFUNCTION(ScriptJsonFactory_Generic), asCALL_GENERIC);
    Assert( r>= 0 );
    r = engine->RegisterObjectBehaviour("json", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptJsonAddRef_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectBehaviour("json", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptJsonRelease_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "json &opAssign(bool)", asFUNCTION(ScriptJsonAssignBool_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(int64)", asFUNCTION(ScriptJsonAssignInt_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(double)", asFUNCTION(ScriptJsonAssignFlt_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(const string &in)", asFUNCTION(ScriptJsonAssignStr_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(const array<json@> &in)", asFUNCTION(ScriptJsonAssignArr_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "json &opAssign(const json &in)", asFUNCTION(ScriptJsonAssign_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const bool&in)", asFUNCTION(ScriptJsonSetBool_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, bool&out) const", asFUNCTION(ScriptJsonGetBool_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const int64&in)", asFUNCTION(ScriptJsonSetInt_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, int64&out) const", asFUNCTION(ScriptJsonGetInt_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const double&in)", asFUNCTION(ScriptJsonSetFlt_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, double&out) const", asFUNCTION(ScriptJsonGetFlt_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const string&in)", asFUNCTION(ScriptJsonSetStr_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, string&out) const", asFUNCTION(ScriptJsonGetStr_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "void set(const string &in, const array<json@>&in)", asFUNCTION(ScriptJsonSetArr_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool get(const string &in, array<json@>&out) const", asFUNCTION(ScriptJsonGetArr_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "bool exists(const string &in) const", asFUNCTION(ScriptJsonExists_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "bool isEmpty() const", asFUNCTION(ScriptJsonIsEmpty_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "uint getSize() const", asFUNCTION(ScriptJsonGetSize_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "void clear()", asFUNCTION(ScriptJsonClear_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "jsonType getType()", asFUNCTION(ScriptJsonGetType_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    r = engine->RegisterObjectMethod("json", "json &opIndex(const string &in)", asFUNCTION(CScriptJson_opIndex_Generic), asCALL_GENERIC);
    Assert( r >= 0 );
    r = engine->RegisterObjectMethod("json", "const json &opIndex(const string &in) const", asFUNCTION(CScriptJson_opIndex_const_Generic), asCALL_GENERIC);
    Assert( r >= 0 );

    CheckASCall( engine->RegisterObjectMethod( "json", "bool ParseFile( const string& in )", asFUNCTION( CScriptJson_ParseFile ), asCALL_GENERIC ) );

    CheckASCall( engine->RegisterObjectMethod("json", "bool opConv() const", asFUNCTION(ScriptJsonGetBool_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod("json", "string opConv() const", asFUNCTION(ScriptJsonGetStr_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod("json", "int opConv() const", asFUNCTION(ScriptJsonGetInt_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod("json", "uint opConv() const", asFUNCTION(ScriptJsonGetUInt_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod("json", "float opConv() const", asFUNCTION(ScriptJsonGetFlt_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterObjectMethod("json", "array<json@>& opConv()", asFUNCTION(ScriptJsonGetArr_Generic), asCALL_GENERIC) );

//    CheckASCall( engine->RegisterObjectMethod("json", "const bool opImplConv() const", asFUNCTION(ScriptJsonGetBool_Generic), asCALL_GENERIC) );
//    CheckASCall( engine->RegisterObjectMethod("json", "const string& opImplConv() const", asFUNCTION(ScriptJsonGetStr_Generic), asCALL_GENERIC) );
//    CheckASCall( engine->RegisterObjectMethod("json", "const int opImplConv() const", asFUNCTION(ScriptJsonGetInt_Generic), asCALL_GENERIC) );
//    CheckASCall( engine->RegisterObjectMethod("json", "const uint opImplConv() const", asFUNCTION(ScriptJsonGetUInt_Generic), asCALL_GENERIC) );
//    CheckASCall( engine->RegisterObjectMethod("json", "const float opImplConv() const", asFUNCTION(ScriptJsonGetFlt_Generic), asCALL_GENERIC) );
//    CheckASCall( engine->RegisterObjectMethod("json", "const array<json@>& opImplConv() const", asFUNCTION(ScriptJsonGetArr_Generic), asCALL_GENERIC) );

    // Json functions
    CheckASCall( engine->RegisterGlobalFunction("json@ JsonParseFile(const string& in file)", asFUNCTION(ScriptJson_ParseFile_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterGlobalFunction("json@ JsonParse(const string& in str)", asFUNCTION(ScriptJson_Parse_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterGlobalFunction("bool JsonWriteFile(const json& in json,const string& in file)", asFUNCTION(ScriptJson_WriteFile_Generic), asCALL_GENERIC) );
    CheckASCall( engine->RegisterGlobalFunction("bool JsonWrite(const json& in json,string& out str)", asFUNCTION(ScriptJson_Write_Generic), asCALL_GENERIC) );
}

void RegisterScriptJson(asIScriptEngine *engine)
{
    if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
        RegisterScriptJson_Generic(engine);
//    else
//        RegisterScriptJson_Native(engine);
}

END_AS_NAMESPACE