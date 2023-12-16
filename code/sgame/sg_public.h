#ifndef __SG_PUBLIC__
#define __SG_PUBLIC__

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_types.h"

typedef struct linkEntity_s {
    vec3_t mins;
    vec3_t maxs;
    uint32_t type;
    uint32_t id;

    struct linkEntity_s *next;
    struct linkEntity_s *prev;
} linkEntity_t;

typedef struct
{
	vec3_t start;
	vec3_t end;
	vec3_t origin;
	float speed;
	float length;
	float angle;
    void *hitData;
} ray_t;

typedef enum
{
    SG_INACTIVE,
    SG_IN_LEVEL,
    SG_SHOW_LEVEL_STATS,
} sgameState_t;

/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define SGAME_IMPORT_API_VERSION 1

typedef enum
{
    SG_PRINT,
    SG_ERROR,

    SG_KEY_ISDOWN,
    SG_KEY_ANYDOWN,
    SG_KEY_SETCATCHER,
    SG_KEY_GETCATCHER,
    SG_KEY_CLEARSTATES,
    SG_KEY_GETKEY,

    SG_CASTRAY,

    SG_SETCAMERAINFO,

    SG_G_LOADMAP,
    SG_G_SETBINDNAMES,

    SG_GETGPUCONFIG,
    SG_GETCLIPBOARDDATA,
    SG_MILLISECONDS,
    SG_MEMORY_REMAINING,
    SG_SNAPVECTOR,

    SG_SENDCONSOLECOMMAND,
    SG_ARGS,
    SG_ARGC,
    SG_ARGV,
    SG_REMOVECOMMAND,
    SG_ADDCOMMAND,

    SG_CVAR_REGISTER,
    SG_CVAR_UPDATE,
    SG_CVAR_SET,
    SG_CVAR_VARIABLESTRINGBUFFER,

    SG_FS_GETFILELIST,
    SG_FS_FILELENGTH,
    SG_FS_FOPENFILEWRITE,
    SG_FS_FOPENFILEREAD,
    SG_FS_FOPENWRITE,
    SG_FS_FOPENREAD,
    SG_FS_FOPENAPPEND,
    SG_FS_FOPENFILE,
    SG_FS_FCLOSE,
    SG_FS_WRITE,
    SG_FS_READ,
    SG_FS_WRITEFILE,
    SG_FS_FILESEEK,
    SG_FS_FILETELL,
    SG_FS_FOPENRW,

    SG_SND_REGISTERSFX,
    SG_SND_REGISTERTRACK,
    SG_SND_PLAYSFX,
    SG_SND_STOPSFX,
    SG_SND_QUEUETRACK,
    SG_SND_SETLOOPINGTRACK,
    SG_SND_CLEARLOOPINGTRACK,

    SG_RE_RENDERSCENE,
    SG_RE_CLEARSCENE,
    SG_RE_ADDPOLYTOSCENE,
    SG_RE_ADDPOLYLISTTOSCENE,
    SG_RE_ADDENTITYTOSCENE,
    SG_RE_ADDLIGHTOSCENE,
    SG_RE_REGISTERSHADER,
    SG_RE_LOADWORLDMAP,

    SG_G_LINK_ENTITY,
    SG_G_UNLINK_ENTITY,
    SG_G_SOUND_RECURSIVE,
    SG_G_CHECK_WALL_COLLISION,

    IMGUI_BEGIN_WINDOW = 400,
    IMGUI_END_WINDOW,
    IMGUI_IS_WINDOW_COLLAPSED,
    IMGUI_SET_WINDOW_COLLAPSED,
    IMGUI_SET_WINDOW_POS,
    IMGUI_SET_WINDOW_SIZE,
    IMGUI_SET_WINDOW_FONTSCALE,
    IMGUI_BEGIN_MENU,
    IMGUI_END_MENU,
    IMGUI_MENU_ITEM,
    IMGUI_SET_ITEM_TOOLTIP,
    IMGUI_TEXT,
    IMGUI_TEXT_MULTILINE,
    IMGUI_TEXT_WITH_HINT,
    IMGUI_INPUT_FLOAT,
    IMGUI_INPUT_FLOAT2,
    IMGUI_INPUT_FLOAT3,
    IMGUI_INPUT_FLOAT4,
    IMGUI_INPUT_INT,
    IMGUI_INPUT_INT2,
    IMGUI_INPUT_INT3,
    IMGUI_INPUT_INT4,
    IMGUI_SLIDER_FLOAT,
    IMGUI_SLIDER_FLOAT2,
    IMGUI_SLIDER_FLOAT3,
    IMGUI_SLIDER_FLOAT4,
    IMGUI_SLIDER_INT,
    IMGUI_SLIDER_INT2,
    IMGUI_SLIDER_INT3,
    IMGUI_SLIDER_INT4,
    IMGUI_COLOR_EDIT3,
    IMGUI_COLOR_EDIT4,
    IMGUI_ARROW_BUTTON,
    IMGUI_CHECKBOX,
    IMGUI_GET_FONTSCALE,
    IMGUI_SET_CURSOR_POS,
    IMGUI_SET_CURSOR_SCREEN_POS,
    IMGUI_GET_CURSOR_POS,
    IMGUI_GET_CURSOR_SCREEN_POS,
    IMGUI_PUSH_COLOR,
    IMGUI_POP_COLOR,
    IMGUI_SAMELINE,
    IMGUI_NEWLINE,
    IMGUI_COLOREDTEXT,
    IMGUI_SEPARATOR_TEXT,
    IMGUI_SEPARATOR,
    IMGUI_PROGRESSBAR,
    IMGUI_OPEN_POPUP,
    IMGUI_CLOSE_CURRENT_POPUP,
    IMGUI_BEGIN_POPUP_MODAL,
    IMGUI_END_POPUP,
    IMGUI_BUTTON,
    IMGUI_INPUT_TEXT,
    IMGUI_INPUT_TEXT_MULTILINE,
    IMGUI_INPUT_TEXT_WITH_HINT,

    SG_FLOOR = 909,
    SG_CEIL,
    SG_ACOS,

    NUM_SGAME_IMPORT
} sgameImport_t;

typedef enum
{
    SGAME_INIT,
    // void (*SG_Init)( void );
    // called during game initialization
    // all media should be registered and loaded at this time

    SGAME_SHUTDOWN,
    // void (*SG_Shutdown)( void );
    // opportunity to flush and close and open files
    
    SGAME_LOADLEVEL,
    // qboolean (*SG_LoadLevel)( int index );
    // called when initializing a new level at set index

    SGAME_ENDLEVEL,
    
    SGAME_REWIND_TO_LAST_CHECKPOINT,

    SGAME_CONSOLE_COMMAND,
    // qboolean (*SG_ConsoleCommand)( void );
    // a console command has been issued locally that is not recognized by the
    // main game system
    // use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the command is not known to the game

    SGAME_RUNTIC,
    // void (*SG_RunTic)( int msec );
    // runs a single game tic (fixed rate of 35 per second for each mob, not fixed for players)
    // to update all entities in the currently loaded level, will draw the current game scene

    SGAME_KEY_EVENT,
    // void (*SG_KeyEven)( uint32_t key, qboolean down );

    SGAME_MOUSE_EVENT,
    // void (*SG_MouseEvent)( int dx, int dy );

    SGAME_SEND_USER_CMD,

    SGAME_EVENT_HANDLING,

    SGAME_EVENT_NONE,
    SGAME_GET_STATE,

    SGAME_EXPORT_LAST,
} sgameExport_t;

#endif
