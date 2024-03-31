#include "module_public.h"
#include "module_handle.h"

CModuleHandle::CModuleHandle( const char *pName, const std::vector<std::string>& sourceFiles,
	int32_t moduleVersionMajor, int32_t moduleVersionUpdate, int32_t moduleVersionPatch )
{
    MonoDomain *domain;

    domain = mono_jit_init( pName );
    if ( !domain ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: mono_jit_init failed" );
    }

    
}
