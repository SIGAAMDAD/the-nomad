#include "g_game.h"
#include "g_world.h"
#include "../rendercommon/r_public.h"
#include "../sgame/sg_public.h"
#include "../engine/vm_local.h"
#include "g_vmimgui.h"

#define VM_CHECKBOUNDS(addr1,len) VM_CheckBounds(sgvm,(addr1),(len))
#define VM_CHECKBOUNDS2(addr1,addr2,len) VM_CheckBounds2(sgvm,(addr1),(addr2),(len))

static void G_AddSGameCommand( const char *name )
{
    Cmd_AddCommand( name, NULL );
}

static int FloatToInt( float f )
{
    floatint_t fi;
    fi.f = f;
    return fi.i;
}

static float IntToFloat( int32_t i )
{
    floatint_t fi;
    fi.i = i;
    return fi.f;
}

static float UIntToFloat( uint32_t u )
{
    floatint_t fi;
    fi.u = u;
    return fi.f;
}

static void *VM_ArgPtr( intptr_t addr )
{
    if ( !addr || sgvm == NULL ) {
        return NULL;
    }
    
    if ( sgvm->entryPoint ) {
        return (void *)( addr );
    } else {
        return (void *)( sgvm->dataBase + ( addr & sgvm->dataMask ) );
    }
}

static intptr_t G_SGameSystemCalls( intptr_t *args )
{
    switch ( args[0] ) {
    case SG_G_CASTRAY:
        g_world.CastRay( (ray_t *)VMA( 1 ) );
        return 0;
    case SG_G_CHECK_WALL_COLLISION:
        return g_world.CheckWallHit( (const float *)VMA( 1 ), (dirtype_t)args[2] );
    case SG_G_SOUND_RECURSIVE:
        g_world.SoundRecursive( args[1], args[2], VMF( 3 ), (const float *)VMA( 4 ) );
        return 0;
    case SG_G_LINK_ENTITY:
        g_world.LinkEntity( (linkEntity_t *)VMA( 1 ) );
        return 0;
    case SG_G_UNLINK_ENTITY:
        g_world.UnlinkEntity( (linkEntity_t *)VMA( 1 ) );
        return 0;
    case SG_RE_ADDSPRITETOSCENE:
        VM_CHECKBOUNDS( args[1], sizeof(vec3_t) );
        re.AddSpriteToScene( (const float *)VMA( 1 ), args[2], args[3] );
        return 0;
    case SG_RE_REGISTERSPRITESHEET:
        return re.RegisterSpriteSheet( (const char *)VMA( 1 ), args[2], args[3], args[4], args[5] );
    case SG_RE_REGISTERSPRITE:
        return re.RegisterSprite( args[1], args[2] );
    case SG_G_SETCAMERAINFO:
        VM_CHECKBOUNDS( args[1], sizeof(vec2_t) );
        G_SetCameraData( (const float *)VMA( 1 ), VMF( 2 ), VMF( 3 ) );
        return 0;
    case SG_CVAR_UPDATE:
        VM_CHECKBOUNDS( args[1], sizeof(vmCvar_t) );
        Cvar_Update( (vmCvar_t *)VMA( 1 ), args[2] );
        return 0;
    case SG_CVAR_REGISTER:
        VM_CHECKBOUNDS( args[1], sizeof(vmCvar_t) );
        Cvar_Register( (vmCvar_t *)VMA( 1 ), (const char *)VMA( 2 ), (const char *)VMA( 3 ), args[4], args[5] );
        return 0;
    case SG_CVAR_SET:
        Cvar_Set( (const char *)VMA( 1 ), (const char *)VMA( 2 ) );
        return 0;
    case SG_CVAR_VARIABLESTRINGBUFFER:
        VM_CHECKBOUNDS( args[2], args[3] );
        Cvar_VariableStringBuffer( (const char *)VMA( 1 ), (char *)VMA( 2 ), args[3] );
        return 0;
    case SG_SYS_MILLISECONDS:
        return Sys_Milliseconds();
    case SG_PRINT:
        Con_Printf( "%s", (const char *)VMA( 1 ) );
        return 0;
    case SG_ERROR:
        N_Error( ERR_DROP, "%s", (const char *)VMA( 1 ) );
        return 0;
    case SG_RE_SETCOLOR:
        re.SetColor( (const float *)VMA( 1 ) );
        return 0;
    case SG_RE_DRAWIMAGE:
        re.DrawImage( VMF( 1 ), VMF( 2 ), VMF( 3 ), VMF( 4 ), VMF( 5 ), VMF( 6 ), VMF( 7 ), VMF( 8 ), args[9]);
        return 0;
    case SG_RE_ADDLIGHTOSCENE:
        return 0;
    case SG_RE_ADDPOLYTOSCENE:
        VM_CHECKBOUNDS( args[2], sizeof(polyVert_t) * args[3] );
        re.AddPolyToScene( args[1], (const polyVert_t *)VMA( 2 ), args[3] );
        return 0;
    case SG_RE_ADDPOLYLISTTOSCENE:
        VM_CHECKBOUNDS( args[1], sizeof(poly_t) * args[2] );
        re.AddPolyListToScene( (const poly_t *)VMA( 1 ), args[2] );
        return 0;
    case SG_RE_ADDENTITYTOSCENE:
        VM_CHECKBOUNDS( args[1], sizeof(renderEntityRef_t) );
        re.AddEntityToScene( (const renderEntityRef_t *)VMA( 1 ) );
        return 0;
    case SG_RE_REGISTERSHADER:
        return re.RegisterShader( (const char *)VMA( 1 ) );
    case SG_RE_CLEARSCENE:
        re.ClearScene();
        return 0;
    case SG_RE_RENDERSCENE:
        VM_CHECKBOUNDS( args[1], sizeof(renderSceneRef_t) );
        re.RenderScene( (const renderSceneRef_t *)VMA( 1 ) );
        return 0;
    case SG_SND_REGISTERSFX:
        return Snd_RegisterSfx( (const char *)VMA( 1 ) );
    case SG_SND_PLAYSFX:
        Snd_PlaySfx( args[1] );
        return 0;
    case SG_SND_STOPSFX:
        Snd_StopSfx( args[1] );
        return 0;
    case SG_SND_REGISTERTRACK:
        return Snd_RegisterTrack( (const char *)VMA( 1 ) );
    case SG_SND_SETLOOPINGTRACK:
        Snd_SetLoopingTrack( args[1] );
        return 0;
    case SG_SND_CLEARLOOPINGTRACK:
        Snd_ClearLoopingTrack();
        return 0;
    case SG_CMD_ADDCOMMAND:
        G_AddSGameCommand( (const char *)VMA( 1 ) );
        return 0;
    case SG_CMD_REMOVECOMMAND:
        Cmd_RemoveCommand( (const char *)VMA( 1 ) );
        return 0;
    case SG_CMD_ARGC:
        return Cmd_Argc();
    case SG_CMD_ARGV:
        VM_CHECKBOUNDS( args[2], args[3] );
        Cmd_ArgvBuffer( args[1], (char *)VMA( 2 ), args[3] );
        return 0;
    case SG_CMD_ARGS:
        return 0;
    case SG_KEY_GETCATCHER:
        return Key_GetCatcher();
    case SG_KEY_SETCATCHER:
        // don't allow the sgame module to close the console
        Key_SetCatcher( args[1] | ( Key_GetCatcher() & KEYCATCH_CONSOLE ) );
        return 0;
    case SG_KEY_ISDOWN:
        return Key_IsDown( args[1] );
    case SG_KEY_GETKEY:
        return Key_GetKey( (const char *)VMA( 1 ) );
    case SG_KEY_CLEARSTATES:
        Key_ClearStates();
        return 0;
    case SG_KEY_ANYDOWN:
        return Key_AnyDown();
    case SG_SYS_GETGPUCONFIG:
        VM_CHECKBOUNDS( args[1], sizeof(gpuConfig_t) );
        memcpy( (gpuConfig_t *)VMA( 1 ), &gi.gpuConfig, sizeof(gpuConfig_t) );
        return 0;
    case SG_SYS_GETCLIPBOARDDATA:
        return 0;
    case SG_CMD_SENDCONSOLECOMMAND:
        Cbuf_ExecuteText( EXEC_APPEND, (const char *)VMA( 1 ) );
        return 0;
    case SG_SYS_MEMORYREMAINING:
        return Hunk_MemoryRemaining();
    case SG_G_SETACTIVEMAP:
        VM_CHECKBOUNDS( args[2], sizeof(mapinfo_t) );
        VM_CHECKBOUNDS( args[3], sizeof(int32_t) * MAX_MAP_WIDTH * MAX_MAP_HEIGHT );
        VM_CHECKBOUNDS( args[4], sizeof(linkEntity_t) );
        G_SetActiveMap( args[1], (mapinfo_t *)VMA( 2 ), (int32_t *)VMA( 3 ), (linkEntity_t *)VMA( 4 ) );
        return 0;
    case SG_G_LOADMAP:
        return G_LoadMap( (const char *)VMA( 1 ) );
    case SG_FS_FOPENREAD:
        return FS_VM_FOpenRead( (const char *)VMA( 1 ), H_SGAME );
    case SG_FS_FOPENWRITE:
        return FS_VM_FOpenWrite( (const char *)VMA( 1 ), H_SGAME );
    case SG_FS_FOPENAPPEND:
        return FS_VM_FOpenAppend( (const char *)VMA( 1 ), H_SGAME );
    case SG_FS_FOPENRW:
        return FS_VM_FOpenRW( (const char *)VMA( 1 ), H_SGAME );
    case SG_FS_FILESEEK:
        return FS_VM_FileSeek( args[1], args[2], args[3], H_SGAME );
    case SG_FS_FILETELL:
        return FS_VM_FileTell( args[1], H_SGAME );
    case SG_FS_FOPENFILE:
        VM_CHECKBOUNDS( args[2], sizeof(fileHandle_t) );
        return FS_VM_FOpenFile( (const char *)VMA( 1 ), (fileHandle_t *)VMA( 2 ), (fileMode_t)args[3], H_SGAME );
    case SG_FS_FOPENFILEWRITE:
        VM_CHECKBOUNDS( args[2], sizeof(fileHandle_t) );
        return FS_VM_FOpenFileWrite( (const char *)VMA( 1 ), (fileHandle_t *)VMA( 2 ), H_SGAME );
    case SG_FS_FOPENFILEREAD:
        VM_CHECKBOUNDS( args[2], sizeof(fileHandle_t) );
        return FS_VM_FOpenFileRead( (const char *)VMA( 1 ), (fileHandle_t *)VMA( 2 ), H_SGAME );
    case SG_FS_FCLOSE:
        FS_VM_FClose( args[1], H_SGAME );
        return 0;
    case SG_FS_WRITEFILE:
        VM_CHECKBOUNDS( args[1], args[2] );
        return FS_VM_WriteFile( VMA( 1 ), args[2], args[3], H_SGAME );
    case SG_FS_WRITE:
        VM_CHECKBOUNDS( args[1], args[2] );
        return FS_VM_Write( VMA( 1 ), args[2], args[3], H_SGAME );
    case SG_FS_READ:
        VM_CHECKBOUNDS( args[1], args[2] );
        return FS_VM_Read( VMA( 1 ), args[2], args[3], H_SGAME );
    case SG_FS_FILELENGTH:
        return FS_VM_FileLength( args[1], H_SGAME );
    case SG_FS_GETFILELIST:
        VM_CHECKBOUNDS( args[3], args[4] );
        return FS_GetFileList( (const char *)VMA( 1 ), (const char *)VMA( 2 ), (char *)VMA( 3 ), args[4] );
    case SG_RE_LOADWORLDMAP:
        re.LoadWorld( (const char *)VMA( 1 ) );
        return 0;
    case SG_GETHASHSTRING:
        VM_CHECKBOUNDS( args[2], MAX_STRING_CHARS );
        UI_GetHashString( (const char *)VMA( 1 ), (char *)VMA( 2 ) );
        return 0;
    case IMGUI_BEGIN_WINDOW:
        return ImGui_BeginWindow( (const char *)VMA( 1 ), (byte *)VMA( 2 ), args[3] );
    case IMGUI_END_WINDOW:
        ImGui_EndWindow();
        return 0;
    case IMGUI_IS_WINDOW_COLLAPSED:
        return ImGui_IsWindowCollapsed();
    case IMGUI_SET_WINDOW_COLLAPSED:
        ImGui_SetWindowCollapsed( args[1] );
        return 0;
    case IMGUI_SET_WINDOW_POS:
        ImGui_SetWindowPos( VMF( 1 ), VMF( 2 ) );
        return 0;
    case IMGUI_SET_WINDOW_SIZE:
        ImGui_SetWindowSize( VMF( 1 ), VMF( 2 ) );
        return 0;
    case IMGUI_SET_WINDOW_FONTSCALE:
        ImGui_SetWindowFontScale( VMF( 1 ) );
        return 0;
    case IMGUI_BEGIN_MENU:
        return ImGui_BeginMenu( (const char *)VMA( 1 ) );
    case IMGUI_END_MENU:
        ImGui_EndMenu();
        return 0;
    case IMGUI_MENU_ITEM:
        return ImGui_MenuItem( (const char *)VMA( 1 ), (const char *)VMA( 2 ), args[3] );
    case IMGUI_SET_ITEM_TOOLTIP:
        ImGui_SetItemTooltip( (const char *)VMA( 1 ) );
        return 0;
    case IMGUI_INPUT_TEXT:
        VM_CHECKBOUNDS( args[2], args[3] );
        return ImGui_InputText( (const char *)VMA( 1 ), (char *)VMA( 2 ), args[3], args[4] );
    case IMGUI_INPUT_TEXT_MULTILINE:
        VM_CHECKBOUNDS( args[2], args[3] );
        return ImGui_InputTextMultiline( (const char *)VMA( 1 ), (char *)VMA( 2 ), args[3], args[4] );
    case IMGUI_INPUT_TEXT_WITH_HINT:
        VM_CHECKBOUNDS( args[3], args[4] );
        return ImGui_InputTextWithHint( (const char *)VMA( 1 ), (const char *)VMA( 2 ), (char *)VMA( 3 ), args[4], args[5] );
    case IMGUI_INPUT_FLOAT:
        VM_CHECKBOUNDS( args[2], sizeof(float) );
        return ImGui_InputFloat( (const char *)VMA( 1 ), (float *)VMA( 2 ) );
    case IMGUI_INPUT_FLOAT2:
        VM_CHECKBOUNDS( args[2], sizeof(vec2_t) );
        return ImGui_InputFloat2( (const char *)VMA( 1 ), (float *)VMA( 2 ) );
    case IMGUI_INPUT_FLOAT3:
        VM_CHECKBOUNDS( args[2], sizeof(vec3_t) );
        return ImGui_InputFloat3( (const char *)VMA( 1 ), (float *)VMA( 2 ) );
    case IMGUI_INPUT_FLOAT4:
        VM_CHECKBOUNDS( args[2], sizeof(vec4_t) );
        return ImGui_InputFloat4( (const char *)VMA( 1 ), (float *)VMA( 2 ) );
    case IMGUI_INPUT_INT:
        VM_CHECKBOUNDS( args[2], sizeof(int32_t) );
        return ImGui_InputInt( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ) );
    case IMGUI_INPUT_INT2:
        VM_CHECKBOUNDS( args[2], sizeof(ivec2_t) );
        return ImGui_InputInt2( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ) );
    case IMGUI_INPUT_INT3:
        VM_CHECKBOUNDS( args[2], sizeof(ivec3_t) );
        return ImGui_InputInt3( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ) );
    case IMGUI_INPUT_INT4:
        VM_CHECKBOUNDS( args[2], sizeof(ivec4_t) );
        return ImGui_InputInt4( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ) );
    case IMGUI_SLIDER_FLOAT:
        VM_CHECKBOUNDS( args[2], sizeof(float) );
        return ImGui_SliderFloat( (const char *)VMA( 1 ), (float *)VMA( 2 ), VMF( 3 ), VMF( 4 ) );
    case IMGUI_SLIDER_FLOAT2:
        VM_CHECKBOUNDS( args[2], sizeof(vec2_t) );
        return ImGui_SliderFloat2( (const char *)VMA( 1 ), (float *)VMA( 2 ), VMF( 3 ), VMF( 4 ) );
    case IMGUI_SLIDER_FLOAT3:
        VM_CHECKBOUNDS( args[2], sizeof(vec3_t) );
        return ImGui_SliderFloat3( (const char *)VMA( 1 ), (float *)VMA( 2 ), VMF( 3 ), VMF( 4 ) );
    case IMGUI_SLIDER_FLOAT4:
        VM_CHECKBOUNDS( args[2], sizeof(vec4_t) );
        return ImGui_SliderFloat4( (const char *)VMA( 1 ), (float *)VMA( 2 ), VMF( 3 ), VMF( 4 ) );
    case IMGUI_SLIDER_INT:
        VM_CHECKBOUNDS( args[2], sizeof(int32_t) );
        return ImGui_SliderInt( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ), args[3], args[4] );
    case IMGUI_SLIDER_INT2:
        VM_CHECKBOUNDS( args[2], sizeof(ivec2_t) );
        return ImGui_SliderInt2( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ), args[3], args[4] );
    case IMGUI_SLIDER_INT3:
        VM_CHECKBOUNDS( args[2], sizeof(ivec3_t) );
        return ImGui_SliderInt3( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ), args[3], args[4] );
    case IMGUI_SLIDER_INT4:
        VM_CHECKBOUNDS( args[2], sizeof(ivec4_t) );
        return ImGui_SliderInt4( (const char *)VMA( 1 ), (int32_t *)VMA( 2 ), args[3], args[4] );
    case IMGUI_COLOR_EDIT3:
        VM_CHECKBOUNDS( args[2], sizeof(vec3_t) );
        return ImGui_ColorEdit3( (const char *)VMA( 1 ), (float *)VMA( 2 ), args[3] );
    case IMGUI_COLOR_EDIT4:
        VM_CHECKBOUNDS( args[2], sizeof(vec4_t) );
        return ImGui_ColorEdit4( (const char *)VMA( 1 ), (float *)VMA( 2 ), args[3] );
    case IMGUI_ARROW_BUTTON:
        return ImGui_ArrowButton( (const char *)VMA( 1 ), (ImGuiDir)args[2] );
    case IMGUI_CHECKBOX:
        VM_CHECKBOUNDS( args[2], sizeof(byte) );
        return ImGui_Checkbox( (const char *)VMA( 1 ), (byte *)VMA( 2 ) );
    case IMGUI_GET_FONTSCALE:
        return FloatToInt( ImGui_GetFontScale() );
    case IMGUI_SET_CURSOR_POS:
        ImGui_SetCursorPos( VMF( 1 ), VMF( 2 ) );
        return 0;
    case IMGUI_GET_CURSOR_POS:
        ImGui_GetCursorPos( (float *)VMA( 1 ), (float *)VMA( 2 ) );
        return 0;
    case IMGUI_SET_CURSOR_SCREEN_POS:
        ImGui_SetCursorScreenPos( VMF( 1 ), VMF( 2 ) );
        return 0;
    case IMGUI_GET_CURSOR_SCREEN_POS:
        ImGui_GetCursorScreenPos( (float *)VMA( 1 ), (float *)VMA( 2 ) );
        return 0;
    case IMGUI_PUSH_COLOR:
        VM_CHECKBOUNDS( args[2], sizeof(vec4_t) );
        ImGui_PushColor( (ImGuiCol)args[1], (const float *)VMA( 2 ) );
        return 0;
    case IMGUI_POP_COLOR:
        ImGui_PopColor();
        return 0;
    case IMGUI_SAMELINE:
        ImGui_SameLine( VMF( 1 ) );
        return 0;
    case IMGUI_NEWLINE:
        ImGui_NewLine();
        return 0;
    case IMGUI_TEXT:
        ImGui_Text( (const char *)VMA( 1 ) );
        return 0;
    case IMGUI_COLOREDTEXT:
        VM_CHECKBOUNDS( args[1], sizeof(vec4_t) );
        ImGui_ColoredText( (const float *)VMA( 1 ), (const char *)VMA( 2 ) );
        return 0;
    case IMGUI_SEPARATOR_TEXT:
        ImGui_SeparatorText( (const char *)VMA( 1 ) );
        return 0;
    case IMGUI_SEPARATOR:
        ImGui_Separator();
        return 0;
    case IMGUI_PROGRESSBAR:
        ImGui_ProgressBar( VMF( 1 ) );
        return 0;
    case IMGUI_OPEN_POPUP:
        ImGui_OpenPopup( (const char *)VMA( 1 ) );
        return 0;
    case IMGUI_CLOSE_CURRENT_POPUP:
        ImGui_CloseCurrentPopup();
        return 0;
    case IMGUI_BEGIN_POPUP_MODAL:
        return ImGui_BeginPopupModal( (const char *)VMA( 1) , args[2] );
    case IMGUI_END_POPUP:
        ImGui_EndPopup();
        return 0;
    case IMGUI_BUTTON:
        return ImGui_Button( (const char *)VMA( 1 ) );
    case TRAP_MEMCPY:
        VM_CHECKBOUNDS2( args[1], args[2], args[3] );
        return (intptr_t)memcpy( VMA( 1 ), VMA( 2 ), args[3] );
    case TRAP_MEMSET:
        VM_CHECKBOUNDS( args[1], args[3] );
        return (intptr_t)memset( VMA( 1 ), args[2], args[3] );
    case TRAP_MEMMOVE:
        VM_CHECKBOUNDS2( args[1], args[2], args[3] );
        return (intptr_t)memmove( VMA( 1 ), VMA( 2 ), args[3] );
    case SG_FLOOR:
        return FloatToInt( floor( VMF( 1 ) ) );
    case SG_ACOS:
        return FloatToInt( acos( VMF( 1 ) ) );
    case SG_CEIL:
        return FloatToInt( ceil( VMF( 1 ) ) );
    case TRAP_POW:
        return FloatToInt( pow( VMF( 1 ), VMF( 2 ) ) );
    case TRAP_LOGF:
        return FloatToInt( logf( VMF( 1 ) ) );
    case TRAP_COS:
        return FloatToInt( cos( VMF( 1 ) ) );
    case TRAP_ATAN2:
        return FloatToInt( atan2( VMF( 1 ), VMF( 2 ) ) );
    case TRAP_POWF:
        return FloatToInt( powf( VMF( 1 ), VMF( 2 ) ) );
    case TRAP_SIN:
        return FloatToInt( sin( VMF( 1 ) ) );
    case TRAP_SQRT:
        return FloatToInt( sqrt( VMF( 1 ) ) );
    case TRAP_SQRTF:
        return FloatToInt( sqrtf( VMF( 1 ) ) );
    case TRAP_STRNCPY:
        VM_CHECKBOUNDS2( args[1], args[2], args[3] );
        return (intptr_t)strncpy( (char *)VMA( 1 ), (const char *)VMA( 2 ), args[3] );
    default:
        N_Error( ERR_DROP, "G_SGameSystemCalls: bad call: %lu\n", args[0] );
    };
    return -1;
}

static intptr_t GDR_DECL G_SGameDllSyscall( intptr_t arg, uint32_t numArgs, ... )
{
    va_list argptr;
    intptr_t args[MAX_VMSYSCALL_ARGS + 1];
    args[0] = arg;

    va_start( argptr, numArgs );
    for (uint32_t i = 1; i < numArgs; i++) {
        args[i] = va_arg(argptr, intptr_t);
    }
    va_end(argptr);

    return G_SGameSystemCalls(args);
}

void G_ShutdownSGame( void )
{
    Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_SGAME );

    if ( !sgvm ) {
        return;
    }

    VM_Call( sgvm, 0, SGAME_SHUTDOWN );
    VM_Free( sgvm );
    sgvm = NULL;
    FS_VM_CloseFiles( H_SGAME );
}

void G_InitSGame( void )
{
    vmInterpret_t interpret;
    CTimer timer;

    timer.Run();
    interpret = (vmInterpret_t)Cvar_VariableInteger( "vm_sgame" );
    sgvm = VM_Create( VM_SGAME, G_SGameSystemCalls, G_SGameDllSyscall, interpret );
    if ( !sgvm ) {
        N_Error( ERR_DROP, "G_InitSGame: failed to load vm" );
    }

    // run a quick initialization
    VM_Call( sgvm, 0, SGAME_INIT );

    timer.Stop();
    Con_Printf( "G_InitSGame: %5.5lf milliseconds\n", (double)timer.ElapsedMilliseconds().count() );

    // have the renderer touch all its images, so they are present
    // on the card even if the driver does deferred loading
//    re.EndRegistration();

    // make sure everything is paged in
    if ( !Sys_LowPhysicalMemory() ) {
        Com_TouchMemory();
    }

    // do not allow vid_restart for the first time
    gi.lastVidRestart = Sys_Milliseconds();

    // set state to active
    gi.state = GS_MENU;
}

/*
* G_SGameCommand: see if the current console command is claimed by the sgame
*/
qboolean G_SGameCommand( void )
{
    qboolean bRes;

    if ( !sgvm ) {
        return qfalse;
    }

    bRes = (qboolean)VM_Call( sgvm, 0, SGAME_CONSOLE_COMMAND );

    return bRes;
}


