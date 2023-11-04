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

static squall::VMStd sgameNutVM;

void VM_CallSquirrel( void )
{
    try {

        // define all the system calls
        sgameNutVM.defun("G_Error", [&](const std::string& str) -> void {
            N_Error(ERR_DROP, "%s", str.c_str());
        });

    } catch (const squall::squirrel_error& e) {
        N_Error(ERR_DROP, "[SQuall Error] %s", e.what());
    }
}

