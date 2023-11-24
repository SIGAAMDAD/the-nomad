#include "g_game.h"
#include "../rendercommon/r_public.h"
#include "../sgame/sg_public.h"
#include "../engine/vm_local.h"
#include "../rendercommon/imgui.h"
#include "g_vmimgui.h"

#define VM_CHECKBOUNDS(addr1,len) VM_CheckBounds(sgvm,(addr1),(len))
#define VM_CHECKBOUNDS2(addr1,addr2,len) VM_CheckBounds2(sgvm,(addr1),(addr2),(len))

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
        Con_Printf("%s", (const char *)VMA(1));
        return 0;
    case SG_ERROR:
        N_Error(ERR_DROP, "%s", (const char *)VMA(1));
        return 0;
    case SG_SND_REGISTERSFX:
        return 0;
    case SG_SND_PLAYSFX:
        return 0;
    case SG_KEY_GETCATCHER:
        return Key_GetCatcher();
    case SG_KEY_SETCATCHER:
        Key_SetCatcher(Key_GetCatcher() & args[1]);
        return 0;
    case SG_KEY_GETKEY:
        return Key_GetKey((const char *)VMA(1));
    case SG_KEY_ISDOWN:
        return (intptr_t)Key_IsDown(args[1]);
    case SG_RE_SETCOLOR:
        re.SetColor((const float *)VMA(1));
        return 0;
    case SG_RE_ADDPOLYLISTTOSCENE:
        VM_CHECKBOUNDS(args[1], sizeof(poly_t) * args[2]);
        re.AddPolyListToScene((const poly_t *)VMA(1), args[2]);
        return 0;
    case SG_RE_ADDPOLYTOSCENE:
        VM_CHECKBOUNDS(args[2], sizeof(polyVert_t) * args[3]);
        re.AddPolyToScene(args[1], (const polyVert_t *)VMA(2), args[3]);
        return 0;
    case SG_RE_REGISTERSHADER:
        return re.RegisterShader((const char *)VMA(1));
    case SG_ADDCOMMAND:
        Cmd_AddCommand((const char *)VMA(1), NULL);
        return 0;
    case SG_REMOVECOMMAND:
        Cmd_RemoveCommand((const char *)VMA(1));
        return 0;
    case SG_ARGC:
        return Cmd_Argc();
    case SG_ARGV:
        VM_CHECKBOUNDS(args[2], args[3]);
        Cmd_ArgvBuffer(args[1], (char *)VMA(2), args[3]);
        return 0;
    case SG_ARGS:
        VM_CHECKBOUNDS(args[1], args[2]);
        Cmd_ArgsBuffer((char *)VMA(1), args[2]);
        return 0;
    case SG_CVAR_UPDATE:
        Cvar_Update((vmCvar_t *)VMA(1), sgvm->privateFlag);
        return 0;
    case SG_CVAR_SET:
        Cvar_SetSafe((const char *)VMA(1), (const char *)VMA(2));
        return 0;
    case SG_CVAR_REGISTER:
        Cvar_Register((vmCvar_t *)VMA(1), (const char *)VMA(2), (const char *)VMA(2), args[3], sgvm->privateFlag);
    case SG_FS_FOPENREAD:
        return FS_VM_FOpenRead( (const char *)VMA(1), H_SGAME );
    case SG_FS_FOPENWRITE:
        return FS_VM_FOpenWrite( (const char *)VMA(1), H_SGAME );
    case SG_FS_FOPENAPPEND:
        return FS_VM_FOpenAppend( (const char *)VMA(1), H_SGAME );
    case SG_FS_FOPENRW:
        return FS_VM_FOpenRW( (const char *)VMA(1), H_SGAME );
    case SG_FS_FILESEEK:
        return FS_VM_FileSeek( (file_t)args[1], (fileOffset_t)args[2], args[3], H_SGAME );
    case SG_FS_FILETELL:
        return FS_VM_FileTell( (file_t)args[1], H_SGAME );
    case SG_FS_FOPENFILE:
        VM_CHECKBOUNDS( args[2], sizeof(file_t) );
        return FS_VM_FOpenFile( (const char *)VMA(1), (file_t *)VMA(2), (fileMode_t)args[3], H_SGAME );
    case SG_FS_FOPENFILEWRITE:
        VM_CHECKBOUNDS( args[2], sizeof(file_t) );
        return FS_VM_FOpenFileWrite( (const char *)VMA(1), (file_t *)VMA(2), H_SGAME );
    case SG_FS_FOPENFILEREAD:
        VM_CHECKBOUNDS( args[2], sizeof(file_t) );
        return FS_VM_FOpenFileRead( (const char *)VMA(1), (file_t *)VMA(2), H_SGAME );
    case SG_FS_FCLOSE:
        FS_VM_FClose( (file_t)args[1], H_SGAME );
        return 0;
    case SG_FS_WRITEFILE:
        return FS_VM_WriteFile( (const char *)VMA(1), args[2], (file_t)args[3], H_SGAME );
    case SG_FS_WRITE:
        VM_CHECKBOUNDS( args[1], args[2] );
        return FS_VM_Write( VMA(1), args[2], (file_t)args[3], H_SGAME );
    case SG_FS_READ:
        VM_CHECKBOUNDS( args[1], args[2] );
        return FS_VM_Read( VMA(1), args[2], (file_t)args[3], H_SGAME );
    case SG_FS_FILELENGTH:
        return FS_VM_FileLength( (file_t)args[1], H_SGAME );
    case SG_FS_GETFILELIST:
        return FS_GetFileList( (const char *)VMA(1), (const char *)VMA(2), (char *)VMA(3), args[4] );
    case IMGUI_BEGIN_WINDOW:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiWindow) );
        return ImGui_BeginWindow( (ImGuiWindow *)VMA(1) );
    case IMGUI_END_WINDOW:
        ImGui_EndWindow();
        return 0;
    case IMGUI_IS_WINDOW_COLLAPSED:
        return ImGui_IsWindowCollapsed();
    case IMGUI_SET_WINDOW_COLLAPSED:
        ImGui_SetWindowCollapsed( args[1] );
        return 0;
    case IMGUI_SET_WINDOW_POS:
        ImGui_SetWindowPos( VMF(1), VMF(2) );
        return 0;
    case IMGUI_SET_WINDOW_SIZE:
        ImGui_SetWindowSize( VMF(1), VMF(2) );
        return 0;
    case IMGUI_SET_WINDOW_FONTSCALE:
        ImGui_SetWindowFontScale( VMF(1) );
        return 0;
    case IMGUI_BEGIN_MENU:
        return ImGui_BeginMenu( (const char *)VMA(1) );
    case IMGUI_END_MENU:
        ImGui_EndMenu();
        return 0;
    case IMGUI_MENU_ITEM:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiMenuItem) );
        return ImGui_MenuItem( (ImGuiMenuItem *)VMA(1) );
    case IMGUI_SET_ITEM_TOOLTIP:
        ImGui_SetItemTooltip( (const char *)VMA(1) );
        return 0;
    case IMGUI_INPUT_TEXT:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputText) );
        return ImGui_InputText( (ImGuiInputText *)VMA(1) );
    case IMGUI_INPUT_TEXT_MULTILINE:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputText) );
        return ImGui_InputTextMultiline( (ImGuiInputText *)VMA(1) );
    case IMGUI_INPUT_TEXT_WITH_HINT:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputTextWithHint) );
        return ImGui_InputTextWithHint( (ImGuiInputTextWithHint *)VMA(1) );
    case IMGUI_INPUT_FLOAT:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputFloat) );
        return ImGui_InputFloat( (ImGuiInputFloat *)VMA(1) );
    case IMGUI_INPUT_FLOAT2:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputFloat2) );
        return ImGui_InputFloat2( (ImGuiInputFloat2 *)VMA(1) );
    case IMGUI_INPUT_FLOAT3:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputFloat3) );
        return ImGui_InputFloat3( (ImGuiInputFloat3 *)VMA(1) );
    case IMGUI_INPUT_FLOAT4:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputFloat4) );
        return ImGui_InputFloat4( (ImGuiInputFloat4 *)VMA(1) );
    case IMGUI_INPUT_INT:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputInt) );
        return ImGui_InputInt( (ImGuiInputInt *)VMA(1) );
    case IMGUI_INPUT_INT2:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputInt2) );
        return ImGui_InputInt2( (ImGuiInputInt2 *)VMA(1) );
    case IMGUI_INPUT_INT3:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputInt3) );
        return ImGui_InputInt3( (ImGuiInputInt3 *)VMA(1) );
    case IMGUI_INPUT_INT4:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiInputInt4) );
        return ImGui_InputInt4( (ImGuiInputInt4 *)VMA(1) );
    case IMGUI_SLIDER_FLOAT:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderFloat) );
        return ImGui_SliderFloat( (ImGuiSliderFloat *)VMA(1) );
    case IMGUI_SLIDER_FLOAT2:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderFloat2) );
        return ImGui_SliderFloat2( (ImGuiSliderFloat2 *)VMA(1) );
    case IMGUI_SLIDER_FLOAT3:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderFloat3) );
        return ImGui_SliderFloat3( (ImGuiSliderFloat3 *)VMA(1) );
    case IMGUI_SLIDER_FLOAT4:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderFloat4) );
        return ImGui_SliderFloat4( (ImGuiSliderFloat4 *)VMA(1) );
    case IMGUI_SLIDER_INT:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderInt) );
        return ImGui_SliderInt( (ImGuiSliderInt *)VMA(1) );
    case IMGUI_SLIDER_INT2:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderInt2) );
        return ImGui_SliderInt2( (ImGuiSliderInt2 *)VMA(1) );
    case IMGUI_SLIDER_INT3:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderInt3) );
        return ImGui_SliderInt3( (ImGuiSliderInt3 *)VMA(1) );
    case IMGUI_SLIDER_INT4:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiSliderInt4) );
        return ImGui_SliderInt4( (ImGuiSliderInt4 *)VMA(1) );
    case IMGUI_COLOR_EDIT3:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiColorEdit3) );
        return ImGui_ColorEdit3( (ImGuiColorEdit3 *)VMA(1) );
    case IMGUI_COLOR_EDIT4:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiColorEdit4) );
        return ImGui_ColorEdit4( (ImGuiColorEdit4 *)VMA(1) );
    case IMGUI_ARROW_BUTTON:
        return ImGui_ArrowButton( (const char *)VMA(1), (ImGuiDir)args[2] );
    case IMGUI_CHECKBOX:
        VM_CHECKBOUNDS( args[1], sizeof(ImGuiCheckbox) );
        return ImGui_Checkbox( (ImGuiCheckbox *)VMA(1) );
    case IMGUI_GET_FONTSCALE:
        return FloatToInt( ImGui_GetFontScale() );
    case IMGUI_SET_CURSOR_POS:
        ImGui_SetCursorPos( VMF(1), VMF(2) );
        return 0;
    case IMGUI_GET_CURSOR_POS:
        ImGui_GetCursorPos( (float *)VMA(1), (float *)VMA(2) );
        return 0;
    case IMGUI_SET_CURSOR_SCREEN_POS:
        ImGui_SetCursorScreenPos( VMF(1), VMF(2) );
        return 0;
    case IMGUI_GET_CURSOR_SCREEN_POS:
        ImGui_GetCursorScreenPos( (float *)VMA(1), (float *)VMA(2) );
        return 0;
    case IMGUI_PUSH_COLOR:
        VM_CHECKBOUNDS( args[2], sizeof(vec4_t) );
        ImGui_PushColor( (ImGuiCol)args[1], (const float *)VMA(2) );
        return 0;
    case IMGUI_POP_COLOR:
        ImGui_PopColor();
        return 0;
    case IMGUI_SAMELINE:
        ImGui_SameLine( VMF(1) );
        return 0;
    case IMGUI_NEWLINE:
        ImGui_NewLine();
        return 0;
    case IMGUI_TEXT:
        ImGui_Text( (const char *)VMA(1) );
        return 0;
    case IMGUI_COLOREDTEXT:
        VM_CHECKBOUNDS( args[1], sizeof(vec4_t) );
        ImGui_ColoredText( (const float *)VMA(1), (const char *)VMA(2) );
        return 0;
    case IMGUI_SEPARATOR_TEXT:
        ImGui_SeparatorText( (const char *)VMA(1) );
        return 0;
    case IMGUI_SEPARATOR:
        ImGui_Separator();
        return 0;
    case IMGUI_PROGRESSBAR:
        ImGui_ProgressBar( VMF(1) );
        return 0;
    case SG_RE_LOADWORLDMAP:
        re.LoadWorld( (const char *)VMA(1) );
        return 0;
    case SG_G_LOADMAP:
        VM_CHECKBOUNDS( args[2], sizeof(mapinfo_t) );
        return G_LoadMap( args[1], (mapinfo_t *)VMA(2) );
    case SG_G_SETBINDNAMES:
        // we can't validate valid strings, but we can make sure they're passing in a valid pointer array
        VM_CHECKBOUNDS( args[1], sizeof(const char *) * args[2] );
        gi.bindNames = (const char **)VMA(1);
        gi.numBindNames = args[2];
        return 0;
    case TRAP_MEMSET:
        VM_CHECKBOUNDS(args[1], args[3]);
        return (intptr_t)memset(VMA(1), args[2], args[3]);
	case TRAP_MEMCPY:
        VM_CHECKBOUNDS2(args[1], args[2], args[3]);
        return (intptr_t)memcpy(VMA(1), VMA(2), args[3]);
	case TRAP_STRNCPY:
        VM_CHECKBOUNDS(args[1], args[3]);
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
    case TRAP_LOGF:
        return FloatToInt(logf(VMF(1)));
    case TRAP_POWF:
        return FloatToInt(powf(VMF(1), VMF(2)));
    case TRAP_SQRTF:
        return FloatToInt(sqrtf(VMF(1)));
    default:
        N_Error(ERR_DROP, "G_SGameSystemCalls: bad call: %lu\n", args[0]);
    };
    return -1;
}

static intptr_t GDR_DECL G_SGameDllSyscall(intptr_t arg, uint32_t numArgs, ...)
{
    va_list argptr;
    intptr_t args[MAX_VMSYSCALL_ARGS + 1];
    args[0] = arg;

    va_start(argptr, numArgs);
    for (uint32_t i = 1; i < numArgs; i++) {
        args[i] = va_arg(argptr, intptr_t);
    }
    va_end(argptr);

    return G_SGameSystemCalls(args);
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
    CTimer timer;

    timer.Run();
    interpret = (vmInterpret_t)Cvar_VariableInteger("vm_sgame");
    sgvm = VM_Create(VM_SGAME, G_SGameSystemCalls, G_SGameDllSyscall, interpret);
    if (!sgvm) {
        N_Error(ERR_DROP, "G_InitSGame: failed to load vm");
    }

    // run a quick initialization
    VM_Call(sgvm, 0, SGAME_INIT);

    timer.Stop();
    Con_Printf( "G_InitSGame: %5.5lf milliseconds\n", (double)timer.ElapsedMilliseconds().count() );

    // have the renderer touch all its images, so they are present
    // on the card even if the driver does deferred loading
//    re.EndRegistration();

    // make sure everything is paged in
    if (!Sys_LowPhysicalMemory()) {
        Com_TouchMemory();
    }

    // do not allow vid_restart for the first time
    gi.lastVidRestart = Sys_Milliseconds();
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

void G_SGameRender(stereoFrame_t stereo)
{
    VM_Call(sgvm, 2, SGAME_DRAW, gi.frametime, stereo);
}

