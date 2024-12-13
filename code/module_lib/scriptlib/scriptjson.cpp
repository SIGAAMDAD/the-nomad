#include "../module_public.h"
#include "scriptjson.h"
#include "../module_funcdefs.hpp"

BEGIN_AS_NAMESPACE

#define CATCH_JSON_BLOCK( action, ... ) try { action; } catch ( const nlohmann::json::exception& e ) \
	{ __VA_ARGS__ throw ModuleException( va( "nlohmann::json::exception occurred: (%i) %s", e.id, e.what() ) ); }

CScriptJson *CScriptJson::Create(asIScriptEngine *engine)
{
	CScriptJson *obj = (CScriptJson *)Mem_ClearedAlloc( sizeof( *obj ) );
	new (obj) CScriptJson(engine);
	return obj;
}

CScriptJson *CScriptJson::Create(asIScriptEngine *engine, const json& js)
{
	CScriptJson *obj = (CScriptJson *)Mem_ClearedAlloc( sizeof( *obj ) );
	new (obj) CScriptJson(engine);
	(obj->js_info) = js;
	return obj;
}

void CScriptJson::AddRef( void ) const
{
	const_cast<CScriptJson *>( this )->refCount.fetch_add();
}

void CScriptJson::Release( void ) const
{
	if ( const_cast<CScriptJson *>( this )->refCount.fetch_sub() == 0 ) {
		this->~CScriptJson();
		Mem_Free( (byte *)const_cast<CScriptJson *>( this ) );
	}
}

CScriptJson& CScriptJson::operator=( bool other )
{
	js_info.clear();

	js_info = other;

	return *this;
}

CScriptJson& CScriptJson::operator=( asINT32 other )
{
	js_info.clear();

	js_info = other;

	return *this;
}

CScriptJson& CScriptJson::operator=( asDWORD other )
{
	js_info.clear();

	js_info = other;

	return *this;
}

CScriptJson& CScriptJson::operator=( float other )
{
	js_info.clear();

	js_info = other;

	return *this;
}

CScriptJson& CScriptJson::operator=( const string_t& other )
{
	js_info.clear();
	js_info = other;

	return *this;
}

CScriptJson& CScriptJson::operator=(const CScriptArray& other)
{
	// make sure we're not just being handed a random array
	if ( N_stricmp( g_pModuleLib->GetScriptEngine()->GetTypeInfoById( other.GetElementTypeId() )->GetName(), "json" ) != 0 ) {
		throw ModuleException( "json opIndex( const array<json@>& in ) called on invalid array" );
	}

	js_info.clear();

	for ( asUINT i = 0; i < other.GetSize(); i++ ) {
		CScriptJson **node  = (CScriptJson**)other.At(i);
		if (node && *node) {
			js_info.emplace_back( (*node)->js_info );
		}
	}

	return *this;
}

CScriptJson& CScriptJson::operator=( const CScriptJson& other )
{
	js_info.clear();
	js_info = other.js_info;

	return *this;
}

void CScriptJson::Set( const jsonKey_t& key, const bool *value)
{
	js_info[key] = *value;
}

void CScriptJson::Set( const jsonKey_t& key, const asDWORD *value )
{
	js_info[key] = *value;
}

void CScriptJson::Set( const jsonKey_t& key, const asQWORD *value )
{
	js_info[key] = *value;
}

void CScriptJson::Set( const jsonKey_t& key, const asINT32 *value )
{
	js_info[key] = *value;
}

void CScriptJson::Set( const jsonKey_t& key, const asINT64 *value )
{
	js_info[key] = *value;
}

void CScriptJson::Set( const jsonKey_t& key, const float *value )
{
	js_info[key] = *value;
}

void CScriptJson::Set( const jsonKey_t& key, const string_t& value )
{
	js_info[key] = value;
}

void CScriptJson::Set( const jsonKey_t& key, const CScriptArray& value )
{
	// make sure we're not just being handed a random array
	if ( N_stricmp( g_pModuleLib->GetScriptEngine()->GetTypeInfoById( value.GetElementTypeId() )->GetName(), "json" ) != 0 ) {
		asGetActiveContext()->SetException( "json opIndex( const array<json@>& in ) called on invalid array" );
	}

	js_info.clear();

	for ( asUINT i = 0; i < value.GetSize(); i++ ) {
		CScriptJson **node  = (CScriptJson **)value.At( i );
		if ( node && *node ) {
			js_info += (*node)->js_info;
		}
	}
}

void CScriptJson::Set(const jsonKey_t& key, const CScriptJson& value)
{
	(js_info)[key] = value.js_info;
}

bool CScriptJson::Get( const jsonKey_t& key, bool *value ) const
{
	try {
		*value = js_info[ key ];
	} catch ( const json::exception& e ) {
		return false;
	}
	return true;
}

bool CScriptJson::Get( const jsonKey_t& key, asINT32 *value ) const
{
	try {
		*value = js_info[ key ];
	} catch ( const json::exception& e ) {
		return false;
	}
	return true;
}

bool CScriptJson::Get( const jsonKey_t& key, asDWORD *value ) const
{
	try {
		*value = js_info[ key ];
	} catch ( const json::exception& e ) {
		return false;
	}
	return true;
}

bool CScriptJson::Get( const jsonKey_t& key, float *value) const
{
	try {
		*value = js_info[ key ];
	} catch ( const json::exception& e ) {
		return false;
	}
	return true;
}

bool CScriptJson::Get(const jsonKey_t& key, string_t& value) const
{
	if ( js_info.contains( key ) ) {
		value = js_info[ key ].get<string_t>();
		return true;
	}
	return false;
}

bool CScriptJson::Get( const jsonKey_t& key, CScriptArray& value ) const
{
	if ( !js_info.contains( key ) || !js_info.at( key ).is_array() ) {
		return false;
	}

	json& js_temp = *const_cast<json *>( &js_info[ key ] );
	value.Resize( js_temp.size() );

	for ( asUINT i = 0; i < js_temp.size(); ++i ) {
		CScriptJson *childNode = Create( engine );
		childNode->js_info = js_temp[i];
		value.SetValue( i, &childNode );
		childNode->Release();
	}
	return true;
}

bool CScriptJson::Get( const jsonKey_t& key, CScriptJson& value ) const
{
	if ( !js_info.contains( key ) ) {
		return false;
	}

	value.js_info = js_info.at( key );
	return true;
}

bool CScriptJson::GetBool( void )
{
	return js_info;
}

asINT32 CScriptJson::GetNumber( void )
{
	return js_info;
}

asDWORD CScriptJson::GetUNumber( void )
{
	return js_info;
}

float CScriptJson::GetReal( void )
{
	return js_info;
}

string_t CScriptJson::GetString()
{
	return js_info;
}

CScriptArray *CScriptJson::GetArray()
{
	CScriptArray *retVal = CScriptArray::Create( engine->GetTypeInfoByDecl( "array<json@>" ) );
	
	retVal->Reserve( js_info.size() );
	for ( json::iterator it = js_info.begin(); it != js_info.end(); ++it ) {
		CScriptJson* childNode = CScriptJson::Create(engine, *it);

		retVal->InsertLast( childNode );
		childNode->Release();
	}
	return retVal;
}

CScriptJson *CScriptJson::operator[]( const jsonKey_t& key )
{
	CScriptJson* retVal = Create( engine );
	retVal->js_info = (js_info)[key];
	// Return the existing value if it exists, else insert an empty value
	return retVal;
}

const CScriptJson *CScriptJson::operator[](const jsonKey_t& key) const
{
	CScriptJson* retVal = Create(engine);
	(retVal->js_info) = (js_info)[key];
	return retVal;
}

CScriptJson *CScriptJson::operator[](const uint32_t key)
{
	CScriptJson* retVal = Create(engine);
	retVal->js_info = (js_info)[key];
	// Return the existing value if it exists, else insert an empty value
	return retVal;
}

const CScriptJson *CScriptJson::operator[](const uint32_t key) const
{
	CScriptJson* retVal = Create(engine);
	(retVal->js_info) = (js_info)[key];
	return retVal;
}

bool CScriptJson::Exists(const jsonKey_t& key) const
{
	return js_info.contains(key);
}

bool CScriptJson::IsEmpty() const
{
	return js_info.empty();
}

asUINT CScriptJson::GetSize() const
{
	return js_info.size();
}

void CScriptJson::Clear()
{
	js_info.clear();
}

CScriptJsonType CScriptJson::Type()
{
	switch(js_info.type())
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
	// We start with one reference
	refCount = 1;

	engine = e;
}

CScriptJson::~CScriptJson()
{
	Clear();
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
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonAssignInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	*json = (asINT32)gen->GetArgDWord(0);
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonAssignUInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	*json = (asDWORD)gen->GetArgDWord(0);
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonAssignFlt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	*json = gen->GetArgFloat(0);
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonAssignStr_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	string_t *other = *(string_t**)gen->GetAddressOfArg(0);
	*json = *other;
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonAssignArr_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	CScriptArray *other = *(CScriptArray**)gen->GetAddressOfArg(0);
	*json = *other;
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonAssign_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	CScriptJson *other = *(CScriptJson**)gen->GetAddressOfArg(0);
	*json = *other;
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = json; );
}

void ScriptJsonSetBool_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, (bool *)ref); );
}

void ScriptJsonSetInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, (asINT32 *)ref); );
}

void ScriptJsonSetUInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, (asDWORD *)ref); );
}

void ScriptJsonSetFlt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, (float *)ref); );
}

void ScriptJsonSetStr_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, *(string_t*)ref); );
}

void ScriptJsonSetArr_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, *(CScriptArray*)ref); );
}

void ScriptJsonSetObj_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( json->Set(*key, *(CScriptJson*)ref) );
}

void ScriptJsonGetBool_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, (bool *)gen->GetAddressOfArg(1)) );
}

void ScriptJsonGetUInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, (asDWORD *)gen->GetAddressOfArg(1)) );
}

void ScriptJsonGetInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, (asINT32 *)gen->GetAddressOfArg(1)) );
}

void ScriptJsonGetFlt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	void *ref = (void*)gen->GetAddressOfArg(1);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, (float *)ref) );
}

void ScriptJsonGetStr_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(string_t *)gen->GetArgObject( 1 )) );
}

void ScriptJsonGetArr_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(CScriptArray *)gen->GetArgObject( 1 )) );
}

void ScriptJsonGetObj_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = json->Get(*key, *(CScriptJson *)gen->GetArgObject( 1 )) );
}

void ScriptJsonConvBool_Generic( asIScriptGeneric *gen )
{
	CScriptJson *json = (CScriptJson *)gen->GetObjectData();
	CATCH_JSON_BLOCK( *(bool *)gen->GetAddressOfReturnLocation() = json->GetBool() );
}

void ScriptJsonConvInt_Generic( asIScriptGeneric *gen )
{
	CScriptJson *json = (CScriptJson *)gen->GetObjectData();
	CATCH_JSON_BLOCK( gen->SetReturnDWord( json->GetNumber() ) );
}

void ScriptJsonConvUInt_Generic( asIScriptGeneric *gen )
{
	CScriptJson *json = (CScriptJson *)gen->GetObjectData();
	CATCH_JSON_BLOCK( gen->SetReturnDWord( json->GetUNumber() ) );
}

void ScriptJsonConvFlt_Generic( asIScriptGeneric *gen )
{
	CScriptJson *json = (CScriptJson *)gen->GetObjectData();
	CATCH_JSON_BLOCK( gen->SetReturnFloat( json->GetReal() ) );
}

void ScriptJsonConvString_Generic( asIScriptGeneric *gen )
{
	CScriptJson *json = (CScriptJson *)gen->GetObjectData();
	CATCH_JSON_BLOCK( *(json::string_t *)gen->GetAddressOfReturnLocation() = json->GetString() );
}

void ScriptJsonConvArray_Generic( asIScriptGeneric *gen )
{
	CScriptJson *json = (CScriptJson *)gen->GetObjectData();
	CATCH_JSON_BLOCK( gen->SetReturnAddress( json->GetArray() ) );
}

void ScriptJsonExists_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	bool ret = json->Exists(*key);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = ret; );
}

void ScriptJsonIsEmpty_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	bool ret = json->IsEmpty();
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = ret; );
}

void ScriptJsonGetSize_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetObjectData();
	asUINT ret = json->GetSize();
	CATCH_JSON_BLOCK( *(asUINT*)gen->GetAddressOfReturnLocation() = ret; );
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
	CATCH_JSON_BLOCK( *(CScriptJsonType*)gen->GetAddressOfReturnLocation() = ret; );
}

static void CScriptJson_opIndex_Generic(asIScriptGeneric *gen)
{
	CScriptJson *self = (CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = self->operator[](*key); );
}

static void CScriptJson_opIndex_const_Generic(asIScriptGeneric *gen)
{
	const CScriptJson *self = (const CScriptJson*)gen->GetObjectData();
	jsonKey_t *key = (jsonKey_t*)gen->GetArgObject(0);

	CATCH_JSON_BLOCK( *(const CScriptJson**)gen->GetAddressOfReturnLocation() = self->operator[](*key); );
}

static void CScriptJson_opIndexUInt_Generic(asIScriptGeneric *gen)
{
	CScriptJson *self = (CScriptJson*)gen->GetObjectData();
	uint32_t key = gen->GetArgDWord(0);
	CATCH_JSON_BLOCK( *(CScriptJson**)gen->GetAddressOfReturnLocation() = self->operator[]( key ); );
}

static void CScriptJson_opIndexUInt_const_Generic(asIScriptGeneric *gen)
{
	const CScriptJson *self = (const CScriptJson*)gen->GetObjectData();
	uint32_t key = gen->GetArgDWord(0);

	CATCH_JSON_BLOCK( *(const CScriptJson**)gen->GetAddressOfReturnLocation() = self->operator[](key); );
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
	CATCH_JSON_BLOCK( (newNode->js_info) = eastl::move( json::parse( f.b, f.b + nLength, NULL, true, true ) );, FS_FreeFile( f.v ); );

	FS_FreeFile( f.v );
	*(bool *)gen->GetAddressOfReturnLocation() = true;
}


// Json to text
static bool JsonWriteFile(const CScriptJson& node,const string_t& file)
{
	const nlohmann::json::string_t&& str = node.js_info.dump( 1, '\t' );

	// this will throw an error if it fails anyway
	FS_WriteFile( file.c_str(), str.data(), str.size() );

	return true;
}

static bool JsonWrite(const CScriptJson& node, string_t& content)
{
	content = eastl::move( node.js_info.dump( 1, '\t' ) );
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
	CATCH_JSON_BLOCK( newNode->js_info = eastl::move( json::parse( f.b, f.b + nLength, NULL, true, true ) );, FS_FreeFile( f.v ); );

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
			newNode->js_info = eastl::move( json::parse( str.c_str(), NULL, true, true ) );
			return newNode;
		}
	}
	return NULL;
}

static void ScriptJson_ParseFile_Generic(asIScriptGeneric *gen)
{
	string_t *file = (string_t*)gen->GetArgObject( 0 );

	CScriptJson* ret = JsonParseFile(*file);
	gen->SetReturnAddress( ret );
}

static void ScriptJson_Parse_Generic(asIScriptGeneric *gen)
{
	string_t *file = (string_t*)gen->GetArgObject( 0 );

	CScriptJson* ret = JsonParse(*file);
	gen->SetReturnAddress( ret );
}

static void ScriptJson_WriteFile_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetArgObject( 0 );
	string_t *file = (string_t*)gen->GetArgObject(1);

	bool ret = JsonWriteFile(*json, *file);
	*(bool*)gen->GetAddressOfReturnLocation() = ret;
}

static void ScriptJson_Write_Generic(asIScriptGeneric *gen)
{
	CScriptJson *json = (CScriptJson*)gen->GetArgObject( 0 );
	string_t *content = (string_t*)gen->GetArgObject(1);

	bool ret = JsonWrite(*json, *content);
	CATCH_JSON_BLOCK( *(bool*)gen->GetAddressOfReturnLocation() = ret );
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

void RegisterScriptJson_Native( asIScriptEngine *engine )
{

}

void RegisterScriptJson_Generic( asIScriptEngine *engine )
{
	CheckASCall( engine->RegisterEnum("jsonType") );
	CheckASCall( engine->RegisterEnumValue("jsonType", "OBJECT_VALUE", OBJECT_VALUE) );
	CheckASCall( engine->RegisterEnumValue("jsonType", "ARRAY_VALUE", ARRAY_VALUE) );
	CheckASCall( engine->RegisterEnumValue("jsonType", "BOOLEAN_VALUE", BOOLEAN_VALUE) );
	CheckASCall( engine->RegisterEnumValue("jsonType", "STRING_VALUE", STRING_VALUE) );
	CheckASCall( engine->RegisterEnumValue("jsonType", "NUMBER_VALUE", NUMBER_VALUE) );
	CheckASCall( engine->RegisterEnumValue("jsonType", "REAL_VALUE", REAL_VALUE) );
	CheckASCall( engine->RegisterEnumValue("jsonType", "NULL_VALUE", NULL_VALUE) );

	CheckASCall( engine->RegisterObjectType("json", sizeof(CScriptJson), asOBJ_REF) );
	CheckASCall( engine->RegisterObjectBehaviour("json", asBEHAVE_FACTORY, "json@ f()", asFUNCTION(ScriptJsonFactory_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("json", asBEHAVE_ADDREF, "void f()", asFUNCTION(ScriptJsonAddRef_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectBehaviour("json", asBEHAVE_RELEASE, "void f()", asFUNCTION(ScriptJsonRelease_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(bool)", asFUNCTION(ScriptJsonAssignBool_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(int)", asFUNCTION(ScriptJsonAssignInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(uint)", asFUNCTION(ScriptJsonAssignUInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(float)", asFUNCTION(ScriptJsonAssignFlt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(const string &in)", asFUNCTION(ScriptJsonAssignStr_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(const array<json@> &in)", asFUNCTION(ScriptJsonAssignArr_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "json& opAssign(const json &in)", asFUNCTION(ScriptJsonAssign_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const bool& in)", asFUNCTION(ScriptJsonSetBool_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, bool& out) const", asFUNCTION(ScriptJsonGetBool_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const int32& in)", asFUNCTION(ScriptJsonSetInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, int32& out) const", asFUNCTION(ScriptJsonGetInt_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const uint32& in)", asFUNCTION(ScriptJsonSetUInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, uint32& out) const", asFUNCTION(ScriptJsonGetUInt_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const float& in)", asFUNCTION(ScriptJsonSetFlt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, float& out) const", asFUNCTION(ScriptJsonGetFlt_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const string& in)", asFUNCTION(ScriptJsonSetStr_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, string& out) const", asFUNCTION(ScriptJsonGetStr_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const array<json@>& in)", asFUNCTION(ScriptJsonSetArr_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, array<json@>& out) const", asFUNCTION(ScriptJsonGetArr_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "void set(const string& in, const json@)", asFUNCTION(ScriptJsonSetObj_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool get(const string& in, json@) const", asFUNCTION(ScriptJsonGetObj_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "bool Exists(const string& in) const", asFUNCTION(ScriptJsonExists_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "bool IsEmpty() const", asFUNCTION(ScriptJsonIsEmpty_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "uint Count() const", asFUNCTION(ScriptJsonGetSize_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "void Clear()", asFUNCTION(ScriptJsonClear_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "jsonType GetType()", asFUNCTION(ScriptJsonGetType_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "json& opIndex(const string &in)", asFUNCTION(CScriptJson_opIndex_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const json& opIndex(const string &in) const", asFUNCTION(CScriptJson_opIndex_const_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "json& opIndex( uint )", asFUNCTION(CScriptJson_opIndexUInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const json& opIndex( uint ) const", asFUNCTION(CScriptJson_opIndexUInt_const_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod( "json", "bool ParseFile( const string& in )", asFUNCTION( CScriptJson_ParseFile ), asCALL_GENERIC ) );

	CheckASCall( engine->RegisterObjectMethod("json", "bool opConv() const", asFUNCTION(ScriptJsonConvBool_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "string opConv() const", asFUNCTION(ScriptJsonConvString_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "int opConv() const", asFUNCTION(ScriptJsonConvInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "uint opConv() const", asFUNCTION(ScriptJsonConvUInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "float opConv() const", asFUNCTION(ScriptJsonConvFlt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "array<json@>@ opConv()", asFUNCTION(ScriptJsonConvArray_Generic), asCALL_GENERIC) );

	CheckASCall( engine->RegisterObjectMethod("json", "const bool opImplConv() const", asFUNCTION(ScriptJsonGetBool_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const string& opImplConv() const", asFUNCTION(ScriptJsonGetStr_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const int opImplConv() const", asFUNCTION(ScriptJsonGetInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const uint opConv() const", asFUNCTION(ScriptJsonGetUInt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const float opConv() const", asFUNCTION(ScriptJsonGetFlt_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterObjectMethod("json", "const array<json@>& opConv() const", asFUNCTION(ScriptJsonGetArr_Generic), asCALL_GENERIC) );

	// Json functions
	CheckASCall( engine->RegisterGlobalFunction("json@ JsonParseFile(const string& in file)", asFUNCTION(ScriptJson_ParseFile_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterGlobalFunction("json@ JsonParse(const string& in str)", asFUNCTION(ScriptJson_Parse_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterGlobalFunction("bool JsonWriteFile(const json& in json,const string& in file)", asFUNCTION(ScriptJson_WriteFile_Generic), asCALL_GENERIC) );
	CheckASCall( engine->RegisterGlobalFunction("bool JsonWrite(const json& in json,string& out str)", asFUNCTION(ScriptJson_Write_Generic), asCALL_GENERIC) );
}

void RegisterScriptJson(asIScriptEngine *engine)
{
	RegisterScriptJson_Generic( engine );
}

END_AS_NAMESPACE