#ifndef _VM_SYSCALLS_
#define _VM_SYSCALLS_

#pragma once

typedef intptr_t (*syscallfn_t)(vm_t *, intptr_t *);
typedef struct
{
    intptr_t id;
    syscallfn_t call;
} vmSystemCall_t;

// vm is 32-bit
typedef uint32_t qvm_size_t;
typedef int32_t qvm_ssize_t;
typedef int qvm_int_t;

#define SYSCALL( name ) intptr_t  name(vm_t *vm, intptr_t *args)
#define CALL_ENTRY( id, func ) { (intptr_t)-(id), func }
#define OUT_OF_BOUNDS(addr,len) if (VM_MemoryRangeValid(addr,len,vm) != 0) VM_Error(vm, "%s: out-of-bounds segmentation violation",__func__)
#define NOMADLIB_FUNCS \
    CALL_ENTRY(VM_MEMCPY,         VM_memcpy), \
    CALL_ENTRY(VM_MEMSET,         VM_memset), \
    CALL_ENTRY(VM_MEMMOVE,        VM_memmove), \
    CALL_ENTRY(VM_COM_PRINTF,     VM_Com_Printf), \
    CALL_ENTRY(VM_COM_ERROR,      VM_Com_Error), \
    CALL_ENTRY(VM_FS_FOPENWRITE,  VM_FS_FOpenWrite), \
    CALL_ENTRY(VM_FS_FOPENREAD,   VM_FS_FOpenRead), \
    CALL_ENTRY(VM_FS_FCLOSE,      VM_FS_FClose), \
    CALL_ENTRY(VM_FS_WRITE,       VM_FS_Write), \
    CALL_ENTRY(VM_FS_READ,        VM_FS_Read), \
    CALL_ENTRY(VM_FS_FILESEEK,    VM_FS_FileSeek), \
    CALL_ENTRY(VM_FS_FILELENGTH,  VM_FS_FileLength), \
    CALL_ENTRY(VM_FS_FILEEXISTS,  VM_FS_FileExists) \

SYSCALL(VM_memcpy)
{
    OUT_OF_BOUNDS(args[1], args[3]);
    OUT_OF_BOUNDS(args[2], args[3]);
    return (intptr_t)memcpy(VMA(1), VMA(2), *(qvm_size_t *)VMA(3));
}

SYSCALL(VM_memset)
{
    OUT_OF_BOUNDS(args[1], args[3]);
    return (intptr_t)memset(VMA(1), *(qvm_int_t *)VMA(2), *(qvm_size_t *)VMA(3));
}

SYSCALL(VM_memmove)
{
    OUT_OF_BOUNDS(args[1], args[3]);
    OUT_OF_BOUNDS(args[2], args[3]);
    return (intptr_t)memmove(VMA(1), VMA(2), *(qvm_size_t *)VMA(3));
}

SYSCALL(VM_Com_Printf)
{
    Com_Printf("%s", (const char *)VMA(1));
    return 0;
}

SYSCALL(VM_Com_Error)
{
    Com_Error(vm, *(int *)VMA(1), "%s", (const char *)VMA(2));
    return 0;
}

SYSCALL(VM_FS_FOpenWrite)
{
    OUT_OF_BOUNDS(args[2], sizeof(file_t));
    FS_VM_FOpenWrite((const char *)VMA(1), (file_t *)VMA(2));
    return 0;
}

SYSCALL(VM_FS_FOpenRead)
{
    OUT_OF_BOUNDS(args[2], sizeof(file_t));
    FS_VM_FOpenRead((const char *)VMA(1), (file_t *)VMA(2));
    return 0;
}

SYSCALL(VM_FS_FClose)
{
    OUT_OF_BOUNDS(args[1], sizeof(file_t));
    FS_VM_FClose((file_t *)VMA(1));
    return 0;
}

SYSCALL(VM_FS_Write)
{
    OUT_OF_BOUNDS(args[1], args[2]);
    OUT_OF_BOUNDS(args[3], sizeof(file_t));
    return (intptr_t)FS_Write(VMA(1), *(qvm_size_t *)VMA(2), *(file_t *)VMA(3));
}

SYSCALL(VM_FS_Read)
{
    OUT_OF_BOUNDS(args[1], args[2]);
    OUT_OF_BOUNDS(args[3], sizeof(file_t));
    return (intptr_t)FS_Read(VMA(1), *(qvm_size_t *)VMA(2), *(file_t *)VMA(3));
}

SYSCALL(VM_FS_FileLength)
{
    OUT_OF_BOUNDS(args[1], sizeof(file_t));
    return (intptr_t)FS_FileLength(*(file_t *)VMA(1));
}

SYSCALL(VM_FS_FileExists)
{
    return (intptr_t)FS_FileExists((const char *)VMA(1));
}

SYSCALL(VM_FS_FileSeek)
{
    OUT_OF_BOUNDS(args[1], sizeof(file_t));
    return (intptr_t)FS_FileSeek(*(file_t *)VMA(1), *(fileOffset_t *)VMA(2), *(qvm_size_t *)VMA(3));
}

#endif