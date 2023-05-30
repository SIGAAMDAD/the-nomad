#include "n_shared.h"
#include "m_renderer.h"

static char *com_buffer;
static int com_bufferLen;

/*
Com_Init: initializes all the engine's systems
*/
void Com_Init(void)
{
    Con_Printf("Com_Init: initializing systems");

    Memory_Init();
    R_Init();

    com_bufferLen = 0;
    com_buffer = (char *)Hunk_Alloc(MAX_BUFFER_SIZE, "combuffer", h_low);
    memset(com_buffer, 0, MAX_BUFFER_SIZE);
}

/*
Com_Printf: can be used by either the main engine, or the vm
a raw string should NEVER be passed as fmt, same reason as the quake3 engine.
*/
void GDR_DECL Com_Printf(const char *fmt, ...)
{
    int length;
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    length = stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (com_bufferLen + length >= MAX_BUFFER_SIZE) {
        Sys_Print(com_buffer);
        Sys_Print(GDR_NEWLINE);
        memset(com_buffer, 0, MAX_BUFFER_SIZE);
        com_bufferLen = 0;
    }
    memcpy(com_buffer+com_bufferLen, msg, length);
    com_bufferLen += length;
}

/*
Com_Error: the vm's version of N_Error
*/
void GDR_DECL Com_Error(vm_t* vm, const char *fmt,  ...)
{
    if (VM_GetIndex(vm) == INVALID_VM) {
        N_Error("Com_Error: INVALID VM!!!"); // THIS SHOULD NEVER HAPPEN
    }
    const uint64_t index = VM_GetIndex(vm);
    int length;
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    length = stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    N_Error("(VM Error) %s", msg);
}

/*
Com_Crash_f: force crash, only for devs
*/
void Com_Crash_f(void)
{
    *((int *)0) = 0x1234;
}

/*
Com_Shutdown_f: for testing exit/crashing processes
*/
void Com_Shutdown_f(void)
{
    N_Error("testing");
}

/*
Sys_Print: this is meant as a replacement for fprintf
*/
void GDR_DECL Sys_Print(const char* str)
{
    if (!str) {
        N_Error("Sys_Print: null string");
    }
    int length = strlen(str);

    // no buffering, that's already done by the calling functions
#ifdef _WIN32
    _write(STDOUT_FILENO, (const void *)str, length); // shitty win32 api
#else
    write(STDOUT_FILENO, (const void *)str, length);
#endif
}

void GDR_DECL Sys_Exit(int code)
{
    exit(code);
}

int Sys_stat(nstat_t* buffer, const char *filepath)
{
#ifdef _WIN32
    return __stat64(filepath, buffer);
#else
    return stat(filepath, buffer);
#endif
}

FILE* Sys_FOpen(const char *filepath, const char *mode)
{
    if (!filepath)
        N_Error("Sys_FOpen: null filepath");
    if (!mode)
        N_Error("Sys_FOpen: null mode");
    
    return fopen(filepath, mode);
}

void* Sys_LoadLibrary(const char *libname)
{
#ifdef _WIN32
    if (!GetModuleHandleA(libname))
        return (void *)NULL;
    return LoadLibraryA(libname);
#elif defined(__unix__)
    if (!*libname)
        return (void *)NULL;
    return dlopen(libname, RTLD_NOW);
#endif
}

void* Sys_LoadProc(void *handle, const char *name)
{
    if (!handle)
        N_Error();
    if ((!name) || !*name)
        N_Error();
    
    
}