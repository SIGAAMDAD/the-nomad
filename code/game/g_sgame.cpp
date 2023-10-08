#include "../engine/n_shared.h"
#include "g_game.h"
#include "../rendergl/rgl_public.h"
#include "../common/vm_syscalls.h"
#include "../common/vm.h"
#include "../sgame/sg_public.h"
#include "../engine/n_sound.h"

static void G_AddSGameCommand(const char *name)
{
    Cmd_AddCommand(name, NULL);
}

static int FloatToInt(float f)
{
    floatint_t fi;
    fi.f = f;
    return fi.i;
}

static float IntToFloat(int32_t i)
{
    floatint_t fi;
    fi.i = i;
    return fi.f;
}

static float UIntToFloat(uint32_t u)
{
    floatint_t fi;
    fi.u = u;
    return fi.f;
}

static void *VM_ArgPtr(intptr_t addr)
{
    if (!addr || sgvm == NULL)
        return NULL;
    
    if (sgvm->entryPoint)
        return (void *)(addr);
    else
        return (void *)(sgvm->dataBase + (addr & sgvm->dataMask));
}

static intptr_t G_SGameSystemCalls(intptr_t *args)
{
    switch (args[0]) {
    case SG_PRINT:
        Com_Printf("%s", (const char *)VMA(1));
        return 0;
    case SG_ERROR:
        N_Error(ERR_DROP, "%s", (const char *)VMA(1));
        return 0;
    case SG_RE_ADDENTITY:
        return 0;
    case SG_SND_REGISTERSFX:
        return 0;
    case SG_SND_PLAYSFX:
        return 0;
    case SG_KEY_SETCATCHER:
        Key_SetCatcher(Key_GetCatcher() & args[1]);
        return 0;
    case SG_KEY_GETKEY:
        return Key_GetKey((const char *)VMA(1));
    case SG_KEY_ISDOWN:
        return (intptr_t)Key_IsDown(args[1]);
    case SG_RE_SETCOLOR:
        return 0;
    case SG_RE_DRAWRECT:
        return 0;
    case SG_ADDCOMMAND:
        Cmd_AddCommand((const char *)VMA(1), NULL);
        return 0;
    case SG_REMOVECOMMAND:
        Cmd_RemoveCommand((const char *)VMA(1));
        return 0;
    case SG_ARGC:
        return Cmd_Argc();
    case SG_ARGV:
        OUT_OF_BOUNDS(sgvm, args[2], args[3]);
        Cmd_ArgvBuffer(args[1], (char *)VMA(2), args[3]);
        return 0;
    case SG_ARGS:
        OUT_OF_BOUNDS(sgvm, args[1], args[2]);
        Cmd_ArgsBuffer((char *)VMA(1), args[2]);
        break;
    case SG_CVAR_UPDATE:
        Cvar_Update((vmCvar_t *)VMA(1), sgvm->privateFlag);
        return 0;
    case SG_CVAR_SET:
        Cvar_SetSafe((const char *)VMA(1), (const char *)VMA(2));
        return 0;
    case SG_CVAR_REGISTER:
        Cvar_Register((vmCvar_t *)VMA(1), (const char *)VMA(2), (const char *)VMA(2), args[3], sgvm->privateFlag);
    case SG_FS_FOPENREAD:
        return FS_VM_FOpenRead((const char *)VMA(1), (file_t *)VMA(2), H_SGAME);
    case SG_FS_FOPENWRITE:
        return FS_VM_FOpenWrite((const char *)VMA(1), (file_t *)VMA(2), H_SGAME);
    case SG_FS_FCLOSE:
        FS_VM_FClose(args[1]);
        return 0;
    case SG_FS_FILELENGTH:
        return (intptr_t)FS_FileLength(args[1]);
    case SG_FS_FILESEEK:
        return (intptr_t)FS_VM_FileSeek(args[1], args[2], args[3], H_SGAME);
    case SG_FS_FILETELL:
        return (intptr_t)FS_FileTell(args[1]);
    case TRAP_MEMSET:
        return (intptr_t)memset(VMA(1), args[2], args[3]);
	case TRAP_MEMCPY:
        return (intptr_t)memcpy(VMA(1), VMA(2), args[3]);
	case TRAP_STRNCPY:
        OUT_OF_BOUNDS(sgvm, args[1], args[3]);
        return (intptr_t)strncpy((char *)VMA(1), (const char *)VMA(2), args[3]);
	case TRAP_FLOOR:
        return FloatToInt(floor(VMF(1)));
	case TRAP_SIN:
        return FloatToInt(sin(VMF(1)));
	case TRAP_COS:
        return FloatToInt(cos(VMF(1)));
	case TRAP_ATAN2:
        return FloatToInt(atan2(VMF(1), VMF(2)));
	case TRAP_CEIL:
        return FloatToInt(ceil(VMF(1)));
    case TRAP_SQRT:
        return FloatToInt(sqrt(VMF(1)));
    case TRAP_POW:
        return FloatToInt(pow(VMF(1), VMF(2)));
    default:
        N_Error(ERR_DROP, "G_SgameSyscalls: bad call: %i", (int32_t)args[0]);
    };
    return -1;
}

static intptr_t GDR_DECL G_DllSyscall(intptr_t arg, ...)
{
#if !defined(GDRi386) || defined(__clang__)
    intptr_t args[10];
    va_list argptr;

    args[0] = args;

    va_start(argptr, arg);
    for (uint32_t i = 0; i < arraylen(args); i++) {
        args[i] = va_arg(argptr, intptr_t);
    }
    va_end(argptr);

    return G_SGameSystemCalls(args);
#else
    return G_SGameSystemCalls(&arg);
#endif
}

void G_ShutdownSGame(void)
{
    Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_SGAME);

    if (!sgvm) {
        return;
    }

    VM_Call(sgvm, 0, SGAME_SHUTDOWN);
    VM_Free(sgvm);
    sgvm = NULL;
    FS_VM_CloseFiles(H_SGAME);
}

void G_InitSGame(void)
{
    vmInterpret_t interpret;

    interpret = (vmInterpret_t)Cvar_VariableInteger("vm_sgame");
    sgvm = VM_Create(VM_SGAME, G_SGameSystemCalls, G_DllSyscall, interpret);
    if (!sgvm) {
        N_Error(ERR_DROP, "G_InitSGame: failed to load vm");
    }

    // run a quick initialization
    VM_Call(sgvm, 0, SGAME_INIT);

    // make sure everything is paged in
    Com_TouchMemory();
}

/*
G_GameCommand: see if the current console command is claimed by the sgame
*/
qboolean G_GameCommand(void)
{
    qboolean bRes;

    if (!sgvm) {
        return qfalse;
    }

    bRes = (qboolean)VM_Call(sgvm, 0, SGAME_CONSOLE_COMMAND);

    return bRes;
}

void G_SGameRender(void)
{
    VM_Call(sgvm, 0, SGAME_DRAW);
}

