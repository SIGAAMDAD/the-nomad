#include <jni.h>
#include "n_shared.h"

/* java shit */
void Mod_LoadJava(const char *path)
{
    JavaVM *vm;
    JNIEnv *env; 
    JavaVMInitArgs vmArgs;
    vmArgs.ignoreUnrecognized = 1;
    vmArgs.nOptions = 0;
    vmArgs.version = JNI_VERSION_1_2;

    // construct the global vm
    jint ret = JNI_CreateJavaVM(&vm, (void **)&env, &vmArgs);

    // fetch the main vm class
    jclass vmClass = env->FindClass("vm");

    // get the method id
    jmethodID vmMain = env->GetStaticMethodID(vmClass, "main", "(IIIIIIIIIIIII)Ivm/vmMain");

    if (!vmMain) {
        Con_Printf(ERROR, "Failed to load vmMain java function");
        return;
    }
}

int VM_Run(uint64_t index)
{
    jvalue args[13];

    args[0].i = vmCommand;
    for (uint32_t i = 1; i < arraylen(args); i++)
        args[i].i = vmArgs[i - 1];

    jint retn = javaVM[index]->env->CallStaticIntMethodA(javaVM[index]->vmClass, javaVM[index]->vmMain, args);
    return (int)retn;
}