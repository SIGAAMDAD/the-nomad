#include "module_public.h"

extern "C" void Init( const moduleImport_t *pImport );
extern "C" void Shutdown( void );

extern "C" nhandle_t AddModule( const char *pModuleName, uint32_t nFunctionProcs, vm_t *pModule );
extern "C" nhandle_t GetModuleHandle( const char *pModuleName ) const;
extern "C" void CallModuleFunc( nhandle_t hModule );

extern "C" void AddModuleMemorySegment( nhandle_t hModule, uint32_t nBytes );
extern "C" void AddModuleMemoryLink( nhandle_t hModuleDst, nhandle_t hModuleSrc );

extern "C" void ModuleFreeMemory( nhandle_t hModule, void *pBuffer );
