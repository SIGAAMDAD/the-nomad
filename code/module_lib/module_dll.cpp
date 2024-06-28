#include "module_dll.h"

CModuleDynamicLibrary::CModuleDynamicLibrary( void )
{
    memset( m_szName, 0, sizeof( m_szName ) );
    m_bValid = qfalse;
}

CModuleDynamicLibrary::~CModuleDynamicLibrary()
{
}

bool CModuleDynamicLibrary::Load( CModuleInfo *pModule )
{
    const char *path;
    asIScriptModule *section;
    union {
        void *v;
        char *b;
    } f;
    uint64_t nLength;

    path = FS_BuildOSPath( FS_GetHomePath(), NULL, pModule->m_pHandle->GetModulePath() );
    section = pModule->m_pHandle->GetModule();

    Con_Printf( "Loading dynamic library symbols from '%s'...\n", path );

    const nlohmann::json& sourceFiles = pModule->m_pHandle->GetSourceFiles();
    for ( const auto& it : sourceFiles ) {
        const string_t& name = it.get<string_t>().c_str();

        nLength = FS_LoadFile( name.c_str(), &f.v );
        if ( !f.v ) {
            Con_Printf( COLOR_RED "Error loading source file '%s'.\n", name.c_str() );
            continue;
        }
        section->AddScriptSection( name.c_str(), f.b, nLength );
        FS_FreeFile( f.v );
    }
}
