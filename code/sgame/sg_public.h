#ifndef _SG_PUBLIC_
#define _SG_PUBLIC_

#pragma once

typedef enum
{
    VM_MEMCPY = 0,
    VM_MEMSET,
    VM_MEMMOVE,
    
    VM_COM_PRINTF,
    VM_COM_ERROR,

    VM_FS_FOPENWRITE,
    VM_FS_FOPENREAD,
    VM_FS_FCLOSE,
    VM_FS_WRITE,
    VM_FS_READ,
    VM_FS_FILESEEK,
    VM_FS_FILELENGTH,
    VM_FS_FILEEXISTS,

    VM_G_GETKEYBOARDSTATE,

    VM_CVAR_GET,

    NUM_SGAME_IMPORT
} sgameImport_t;

typedef enum
{
    SGAME_INIT,
    SGAME_SHUTDOWN,
    SGAME_RUNTIC,
    SGAME_STARTLEVEL,
    SGAME_ENDLEVEL,
} sgameExport_t;


#endif
