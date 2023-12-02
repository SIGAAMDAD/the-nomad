#include "n_shared.h"
#include <mono-2.0/mono/jit/jit.h>
#include <mono-2.0/mono/metadata/assembly.h>
#include <mono-2.0/mono/metadata/class.h>
#include <mono-2.0/mono/metadata/image.h>
#include <mono-2.0/mono/metadata/threads.h>
#include <mono-2.0/mono/utils/mono-logger.h>
#include <mono-2.0/mono/utils/mono-error.h>
#include <mono-2.0/mono/utils/mono-jemalloc.h>
#include <jni.h>
#include <squall/squall_vm.hpp>
#include <squall/squall_vmstd.hpp>
#include "vm_local.h"

static squall::VM sgameNutVM;

typedef struct MonoJIT {
    MonoDomain *m_pDomain;
    MonoAssembly *m_pAssembly;
} MonoJIT;

static MonoJIT sgameMonoVM;

void VM_CallSquirrel( int32_t command, int32_t args[MAX_VMMAIN_ARGS] )
{
    try {

        // define all the system calls
        sgameNutVM.defun( "G_Error", [&]( const std::string& str ) -> void {
            N_Error(ERR_DROP, "%s", str.c_str());
        } );
        sgameNutVM.defun( "G_Printf", [&]( const std::string& str ) -> void {
            Con_Printf( "%s", str.c_str() );
        } );

        sgameNutVM.call( "vmMain", command, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9],
                                            args[10], args[11] );

    } catch (const squall::squirrel_error& e) {
        N_Error(ERR_DROP, "[SQuall Error] %s", e.what());
    }
}


void VM_CallMono( int32_t command, int32_t args[MAX_VMMAIN_ARGS] )
{
    char argv[MAX_VMMAIN_ARGS][6];
    int32_t argc;

    argc = MAX_VMMAIN_ARGS;
    memset( argv, 0, sizeof(argv) );

    for (int32_t i = 0; i < MAX_VMMAIN_ARGS; i++) {
        snprintf( argv[i], sizeof(*argv), "%i", args[i] );
    }

    mono_jit_exec( sgameMonoVM.m_pDomain, sgameMonoVM.m_pAssembly, argc, argv );
}

