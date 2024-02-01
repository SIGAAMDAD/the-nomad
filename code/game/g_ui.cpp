#include "g_game.h"
#include "../rendercommon/r_public.h"
#include "g_sound.h"
#include "../rendercommon/imgui.h"

#if 0
typedef struct {
    qboolean menuActive;
    uint32_t menuStackDepth;
} uiState_t;

#define VM_CHECKBOUNDS(addr1,len) VM_CheckBounds(uivm,(addr1),(len))
#define VM_CHECKBOUNDS2(addr1,addr2,len) VM_CheckBounds2(uivm,(addr1),(addr2),(len))

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

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
qboolean UI_GameCommand(void)
{
    if (!uivm) {
        return qfalse;
    }

    return VM_Call(uivm, 0, UI_CONSOLE_COMMAND);
}

static void *VM_ArgPtr(intptr_t addr)
{
    if (!addr || uivm == NULL)
        return NULL;
    
    if (uivm->entryPoint)
        return (void *)(addr);
    else
        return (void *)(uivm->dataBase + (addr & uivm->dataMask));
}

static void G_GetGPUConfig(gpuConfig_t *config) {
    *config = gi.gpuConfig;
}

static intptr_t G_UISystemCalls(intptr_t *args)
{
    switch (args[0]) {
    case UI_CVAR_UPDATE:
        VM_CHECKBOUNDS(args[1], sizeof(vmCvar_t));
        Cvar_Update((vmCvar_t *)VMA(1), args[2]);
        return 0;
    case UI_CVAR_REGISTER:
        VM_CHECKBOUNDS(args[1], sizeof(vmCvar_t));
        Cvar_Register((vmCvar_t *)VMA(1), (const char *)VMA(2), (const char *)VMA(3), args[4], args[5]);
        return 0;
    case UI_CVAR_SET:
        Cvar_Set((const char *)VMA(1), (const char *)VMA(2));
        return 0;
    case UI_CVAR_VARIABLESTRINGBUFFER:
        VM_CHECKBOUNDS(args[2], args[3]);
        Cvar_VariableStringBuffer((const char *)VMA(1), (char *)VMA(2), args[3]);
        return 0;
    case UI_MILLISECONDS:
        return Sys_Milliseconds();
    case UI_PRINT:
        Con_Printf("%s", (const char *)VMA(1));
        return 0;
    case UI_ERROR:
        N_Error(ERR_DROP, "%s", (const char *)VMA(1));
        return 0;
    case UI_RE_SETCOLOR:
        if (VMA(1) != NULL) {
            VM_CHECKBOUNDS(args[1], sizeof(vec4_t));
        }
        re.SetColor((const float *)VMA(1));
        return 0;
    case UI_RE_DRAWIMAGE:
        re.DrawImage(VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), args[9]);
        return 0;
    case UI_RE_ADDPOLYTOSCENE:
        VM_CHECKBOUNDS(args[2], sizeof(polyVert_t) * args[3]);
        re.AddPolyToScene(args[1], (const polyVert_t *)VMA(2), args[3]);
        return 0;
    case UI_RE_ADDPOLYLISTTOSCENE:
        VM_CHECKBOUNDS(args[1], sizeof(poly_t) * args[2]);
        re.AddPolyListToScene((const poly_t *)VMA(1), args[2]);
        return 0;
    case UI_RE_REGISTERSHADER:
        return re.RegisterShader((const char *)VMA(1));
    case UI_RE_CLEARSCENE:
        re.ClearScene();
        return 0;
    case UI_RE_RENDERSCENE:
        VM_CHECKBOUNDS(args[1], sizeof(renderSceneRef_t));
        re.RenderScene((const renderSceneRef_t *)VMA(1));
        return 0;
    case UI_RE_ADDENTITYTOSCENE:
        VM_CHECKBOUNDS(args[1], sizeof(renderEntityRef_t));
        re.AddEntityToScene((const renderEntityRef_t *)VMA(1));
        return 0;
    case UI_RE_REGISTERSPRITESHEET:
        return re.RegisterSpriteSheet((const char *)VMA(1), args[2], args[3], args[4], args[5], args[6]);
        return 0;
    case UI_SND_REGISTERSFX:
        return Snd_RegisterSfx((const char *)VMA(1));
    case UI_SND_PLAYSFX:
        Snd_PlaySfx(args[1]);
        return 0;
    case UI_SND_STOPSFX:
        Snd_StopSfx(args[1]);
        return 0;
    case UI_SND_ADDLOOPINGTRACK:
        Snd_AddLoopingTrack(args[1]);
        return 0;
    case UI_SND_CLEARLOOPINGTRACK:
        Snd_ClearLoopingTrack(args[1]);
        return 0;
    case UI_SND_PLAYTRACK:
        Snd_PlayTrack(args[1]);
        return 0;
    case UI_ARGC:
        return Cmd_Argc();
    case UI_ARGV:
        VM_CHECKBOUNDS(args[2], args[3]);
        Cmd_ArgvBuffer(args[1], (char *)VMA(2), args[3]);
        return 0;
    case UI_KEY_GETCATCHER:
        return Key_GetCatcher();
    case UI_KEY_SETCATCHER:
        Key_SetCatcher(args[1]);
        return 0;
    case UI_KEY_ISDOWN:
        return Key_IsDown(args[1]);
    case UI_KEY_GETKEY:
        return Key_GetKey((const char *)VMA(1));
    case UI_KEY_CLEARSTATES:
        Key_ClearStates();
        return 0;
    case UI_KEY_ANYDOWN:
        return Key_AnyDown();
    case UI_MEMORY_REMAINING:
        return Hunk_MemoryRemaining();
    case UI_FS_FOPENWRITE:
        VM_CHECKBOUNDS(args[2], sizeof(file_t));
        return FS_VM_FOpenWrite((const char *)VMA(1), (file_t *)VMA(2), H_UI);
    case UI_FS_FOPENREAD:
        VM_CHECKBOUNDS(args[2], sizeof(file_t));
        return FS_VM_FOpenRead((const char *)VMA(1), (file_t *)VMA(2), H_UI);
    case UI_FS_FILESEEK:
        return FS_VM_FileSeek(args[1], args[2], args[3], H_UI);
    case UI_FS_FILETELL:
        return FS_FileTell(args[1]);
    case UI_FS_FILELENGTH:
        return FS_FileLength(args[1]);
    case UI_FS_WRITE:
        VM_CHECKBOUNDS(args[1], args[2]);
        return FS_VM_Write(VMA(1), args[2], args[3], H_UI);
    case UI_FS_READ:
        VM_CHECKBOUNDS(args[1], args[2]);
        return FS_VM_Read(VMA(1), args[2], args[3], H_UI);
    case UI_FS_WRITEFILE:
        VM_CHECKBOUNDS(args[1], args[2]);
        FS_VM_WriteFile(VMA(1), args[2], args[3], H_UI);
        return 0;
    case UI_FS_FCLOSE:
        FS_VM_FClose(args[1]);
        return 0;
    case UI_GET_GPUCONFIG:
        VM_CHECKBOUNDS(args[1], sizeof(gpuConfig_t));
        G_GetGPUConfig((gpuConfig_t *)VMA(1));
        return 0;
    case UI_GET_CLIPBOARDATA:
        VM_CHECKBOUNDS(args[1], args[2]);
        memcpy((char *)VMA(1), Sys_GetClipboardData(), args[2]);
        return 0;
    case UI_CMD_EXECUTETEXT:
        if (args[1] == EXEC_NOW
        && (!strncmp((const char *)VMA(2), "snd_restart", 11)
        || !strncmp((const char *)VMA(2), "vid_restart", 11)
        || !strncmp((const char *)VMA(2), "quit", 4)))
        {
            Con_Printf(COLOR_YELLOW "turning EXEC_NOW '%s.11s' into EXEC_INSERT\n", (const char *)VMA(2));
            args[1] = EXEC_INSERT;
        }
        Cbuf_ExecuteText((cbufExec_t)args[1], (const char *)VMA(2));
        return 0;
    case UI_UPDATE:
        SCR_UpdateScreen();
        return 0;
    case TRAP_MEMSET:
        VM_CHECKBOUNDS(args[1], args[3]);
        return (intptr_t)memset(VMA(1), args[2], args[3]);
	case TRAP_MEMCPY:
        VM_CHECKBOUNDS2(args[1], args[2], args[3]);
        return (intptr_t)memcpy(VMA(1), VMA(2), args[3]);
	case TRAP_STRNCPY:
        VM_CHECKBOUNDS2(args[1], args[2], args[3]);
        return (intptr_t)strncpy((char *)VMA(1), (const char *)VMA(2), args[3]);
	case TRAP_FLOOR:
        return FloatToInt(floor(VMF(1)));
	case TRAP_ACOS:
        return FloatToInt(acos(VMF(1)));
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
    case TRAP_STRSTR:
        return (intptr_t)strstr((const char *)VMA(1), (const char *)VMA(2));
    case TRAP_LOGF:
        return FloatToInt(logf(VMF(1)));
    case TRAP_POWF:
        return FloatToInt(powf(VMF(1), VMF(2)));
    case TRAP_SQRTF:
        return FloatToInt(sqrtf(VMF(1)));
    case TRAP_MEMMOVE:
        VM_CHECKBOUNDS2(args[1], args[2], args[3]);
        return (intptr_t)memmove(VMA(1), VMA(2), args[3]);
    default:
        N_Error(ERR_DROP, "G_UISystemCalls: bad call: %i", (int32_t)args[0]);
    };
    return -1;
}

static intptr_t GDR_DECL UI_DllSyscall(intptr_t arg, uint32_t numArgs, ...)
{
    va_list argptr;
    intptr_t args[MAX_VMSYSCALL_ARGS + 1];
    args[0] = arg;

    va_start(argptr, numArgs);
    for (uint32_t i = 1; i < numArgs; i++) {
        args[i] = va_arg(argptr, intptr_t);
    }
    va_end(argptr);

    return G_UISystemCalls(args);
}

void G_ShutdownUI(void)
{
    Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_UI);
    
    if (!uivm) {
        return;
    }
    VM_Call(uivm, 0, UI_SHUTDOWN);
    VM_Free(uivm);
    uivm = NULL;
    FS_VM_CloseFiles(H_UI);
}

void G_InitUI(void)
{
    int version;
    vmInterpret_t interpret;

    // load the dll or bytecode
    interpret = (vmInterpret_t)Cvar_VariableInteger("vm_ui");
    uivm = VM_Create(VM_UI, G_UISystemCalls, UI_DllSyscall, interpret);
    if (!uivm) {
        N_Error(ERR_DROP, "G_InitUI: failed to load vm");
    }

    // quick sanity check
    version = VM_Call(uivm, 0, UI_GETAPIVERSION);
    if (version != NOMAD_VERSION) {
        Con_Printf(COLOR_YELLOW "WARNING: User Interface has old version (%i.%i.%i (engine) vs (major version) %i (ui))\n"
                                "This could lead to some unexpected and weird bugs...\n",
        NOMAD_VERSION, NOMAD_VERSION_UPDATE, NOMAD_VERSION_PATCH,
        version);
    }

    VM_Call(uivm, 0, UI_INIT);

    Key_SetCatcher(KEYCATCH_UI);

    // make sure everything is paged in
    Com_TouchMemory();
}
#endif