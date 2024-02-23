#include "module_public.h"

moduleImport_t moduleImport;

extern "C" void CModuleLib::LoadModule( const char *pModule )
{
    CModuleInfo *m;

    m = eastl::addressof( m_LoadList.emplace_back() );
}

extern "C" void CModuleLib::Init( const moduleImport_t *pImport )
{
    char **dirList;
    uint64_t nDirs;

    memcpy( &moduleImport, pImport, sizeof(*pImport) );

    moduleImport.Printf( PRINT_INFO, "CModuleLib::Init: initializing mod library...\n" );

    
}

extern "C" void CModuleLib::Shutdown( void )
{

}

extern "C" void CModuleLib::CallModuleFunc( nhandle_t hModule, uint32_t nFuncId )
{

}
