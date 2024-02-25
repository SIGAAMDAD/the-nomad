#include "module_renderlib.h"

extern const renderExport_t *renderImport;

CRenderSceneRef::CRenderSceneRef( void )
    : asIScriptObject()
{
    memset( &refdef, 0, sizeof(refdef) );
}

CRenderSceneRef::CRenderSceneRef( const CRenderSceneRef& other ) {
    *this = other;
}

int CRenderSceneRef::AddRef( void ) const {
    return 0;
}

int CRenderSceneRef::Release( void ) const {
    return 0;
}

asILockableSharedBool *CRenderSceneRef::GetWeakRefFlag( void ) const {
    return NULL;
}

int CRenderSceneRef::GetTypeId( void ) const {
    return asTYPEID_APPOBJECT;
}

asITypeInfo *CRenderSceneRef::GetObjectType( void ) const {
    return NULL;
}

asUINT CRenderSceneRef::GetPropertyCount( void ) const {
    return 6;
}

int CRenderSceneRef::GetPropertyTypeId( asUINT prop ) const {
    return asTYPEID_UINT32;
}

const char *CRenderSceneRef::GetPropertyName( asUINT prop ) const {
    switch ( prop ) {
    case 0: return "viewX";
    case 1: return "viewY";
    case 2: return "viewWidth";
    case 3: return "viewHeight";
    case 4: return "flags";
    case 5: return "time";
    };
    return "<invalid>";
}

void *CRenderSceneRef::GetAddressOfProperty( asUINT prop ) {
    switch ( prop ) {
    case 0: return &refdef.x;
    case 1: return &refdef.y;
    case 2: return &refdef.width;
    case 3: return &refdef.height;
    case 4: return &refdef.flags;
    case 5: return &refdef.time;
    };
    return NULL;
}

asIScriptEngine *CRenderSceneRef::GetEngine( void ) const {
    return g_pModuleLib->GetScriptEngine();
}

int CRenderSceneRef::CopyFrom( const asIScriptObject *other )
{
    const asITypeInfo *pInfo = other->GetObjectType();

    if ( !N_stricmp( pInfo->GetName(), "RenderSceneReference" ) ) {
        *this = *dynamic_cast<const CRenderSceneRef *>( other );
        return 1;
    }
    return -1;
}

CRenderSceneRef& CRenderSceneRef::operator=( const CRenderSceneRef& other ) {
    memcpy( this, eastl::addressof( other ), sizeof(*this) );
    return *this;
}

static void RefDef_Init( CRenderSceneRef *pRefDef ) {
    memset( pRefDef, 0, sizeof(*pRefDef) );
}

static void RefDef_ClearScene( CRenderSceneRef * ) {
    re.ClearScene();
}

static void RefDef_RenderScene( const CRenderSceneRef *pRefDef ) {
    re.RenderScene( &pRefDef->refdef );
}

void CRenderSceneRef::Register( void )
{
    g_pModuleLib->GetScriptEngine()->RegisterObjectType( "RenderSceneReference", sizeof(renderSceneRef_t), asOBJ_APP_CLASS_CONSTRUCTOR );
    g_pModuleLib->GetScriptEngine()->RegisterObjectBehaviour( "RenderSceneReference", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION( RefDef_Init ), asCALL_CDECL_OBJLAST );
    g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( "RenderSceneReference", "uint32 viewX", offsetof( refdef_t, x ) );
    g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( "RenderSceneReference", "uint32 viewY", offsetof( refdef_t, y ) );
    g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( "RenderSceneReference", "uint32 viewWidth", offsetof( refdef_t, width ) );
    g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( "RenderSceneReference", "uint32 viewHeight", offsetof( refdef_t, height ) );
    g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( "RenderSceneReference", "uint32 flags", offsetof( refdef_t, flags ) );
    g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( "RenderSceneReference", "uint32 time", offsetof( refdef_t, time ) );

    g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "RenderSceneReference", "void DrawScene() const", asFUNCTION( RefDef_RenderScene ), asCALL_CDECL_OBJLAST );
    g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( "RenderSceneReference", "void ClearScene()", asFUNCTION( RefDef_ClearScene ), asCALL_CDECL_OBJLAST );
}