#ifndef _RGL_PUBLIC_
#define _RGL_PUBLIC_

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_types.h"

typedef enum {
    REF_KEEP_CONTEXT, // don't destroy window and context, just deallocate buffers and shaders
    REF_KEEP_WINDOW, // destroy context, keep window
    REF_DESTROY_WINDW,
    REF_UNLOAD_DLL
} refShutdownCode_t;

//
// refimport_t: for use with external engine system libraries
//
typedef struct {
	void *(*Malloc)(uint32_t size);
	void *(*Realloc)(void *ptr, uint32_t nsize);
    void (*FreeAll)(void);
	void (*Free)(void *ptr);
    char *(*Strdup)(const char *str);
#ifdef _NOMAD_DEBUG
    void *(*Hunk_AllocDebug)(uint64_t size, ha_pref where, const char *label, const char *file, uint64_t line);
#else
	void *(*Hunk_Alloc)(uint64_t size, const char *name, ha_pref where);
#endif
    void *(*Hunk_AllocateTempMemory)(uint64_t size);
    void (*Hunk_FreeTempMemory)(void *buf);

    void (*Sys_FreeFileList)(char **list);

    void (GDR_DECL *Printf)(int level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
    void GDR_NORETURN (GDR_DECL *Error)(errorCode_t code, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));

    void (*Cvar_VariableStringBuffer)(const char *name, char *buffer, uint64_t bufferSize);
    void (*Cvar_VariableStringBufferSafe)(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag);
    int32_t (*Cvar_VariableInteger)(const char *name);
    cvar_t *(*Cvar_Get)(const char *name, const char *value, uint32_t flags);
    void (*Cvar_SetGroup)(cvar_t *cv, cvarGroup_t group);
    void (*Cvar_SetDescription)(cvar_t *cv, const char *description);
    void (*Cvar_Set)(const char *name, const char *value);
    void (*Cvar_CheckRange)(cvar_t *var, const char *mins, const char *maxs, cvartype_t type);
    int (*Cvar_CheckGroup)(cvarGroup_t group);
    void (*Cvar_ResetGroup)( cvarGroup_t group, qboolean resetModifiedFlags );
    void (*Cvar_Reset)(const char *name);
    const char *(*Cvar_VariableString)(const char *name);

    void (*Cmd_AddCommand)(const char* name, cmdfunc_t function);
    void (*Cmd_RemoveCommand)(const char* name);
    void (*Cmd_ExecuteCommand)(const char* name);
    void (*Cmd_ExecuteString)(const char *str);
    uint32_t (*Cmd_Argc)(void);
    char *(*Cmd_ArgsFrom)(int32_t index);
    const char *(*Cmd_Argv)(uint32_t index);

    qboolean (*Key_IsDown)(uint32_t keynum);

    void (*FS_FreeFileList)(char **list);
    uint64_t (*FS_Write)(const void *buffer, uint64_t size, file_t f);
    uint64_t (*FS_Read)(void *buffer, uint64_t size, file_t);
    fileOffset_t (*FS_FileSeek)(file_t f, fileOffset_t offset, uint32_t whence);
    fileOffset_t (*FS_FileTell)(file_t f);
    uint64_t (*FS_FileLength)(file_t f);
    qboolean (*FS_FileExists)(const char *filename);
    file_t (*FS_FOpenRead)(const char *path);
    file_t (*FS_FOpenWrite)(const char *path);
    void (*FS_FClose)(file_t f);
    void (*FS_FreeFile)(void *buffer);
    uint64_t (*FS_LoadFile)(const char *path, void **buffer);
    char **(*FS_ListFiles)(const char *path, const char *extension, uint64_t *numfiles);
    void (*FS_WriteFile)(const char *npath, const void *buffer, uint64_t size);

    int (*ImGui_Init)(void *renderData, const char *renderName, SDL_Window *window, void *renderContext);
    void (*ImGui_Shutdown)(void);
    void (*ImGui_Render)(void);
    void (*ImGui_NewFrame)(void);
    void *(*ImGui_GetDrawData)(void);

    // most of this stuff is for imgui's usage
    SDL_bool (*SDL_SetHint)(const char *name, const char *value);
    void (*SDL_FreeCursor)(SDL_Cursor *cursor);
    void (*SDL_SetCursor)(SDL_Cursor *cursor);
    int (*SDL_CaptureMouse)(SDL_bool enabled);
    SDL_Window *(*SDL_GetKeyboardFocus)(void);
    Uint32 (*SDL_GetWindowFlags)(SDL_Window *window);
    void (*SDL_WarpMouseInWindow)(SDL_Window *window, int x, int y);
    Uint32 (*SDL_GetGlobalMouseState)(int *x, int *y);
    void (*SDL_GetWindowPosition)(SDL_Window *window, int *x, int *y);
    void (*SDL_GetWindowSize)(SDL_Window *window, int *w, int *h);
    int (*SDL_ShowCursor)(int toggle);
    int (*SDL_GetRendererOutputSize)(SDL_Renderer *renderer, int *w, int *h);
    Uint8 (*SDL_GameControllerGetButton)(SDL_GameController *gamecontroller, SDL_GameControllerButton button);
    Sint16 (*SDL_GameControllerGetAxis)(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis);
    SDL_GameController *(*SDL_GameControllerOpen)(int joystick_index);
    char *(*SDL_GetClipboardText)(void);
    int (*SDL_SetClipboardText)(const char *text);
    void (*SDL_SetTextInputRect)(const SDL_Rect *rect);
    const char *(*SDL_GetCurrentVideoDriver)(void);
    SDL_Cursor *(*SDL_CreateSystemCursor)(SDL_SystemCursor id);
    SDL_bool (*SDL_GetWindowWMInfo)(SDL_Window *window, SDL_SysWMinfo *info);
    int (*SDL_Init)(Uint32 flags);
    void (*SDL_Quit)(void);
    Uint64 (*SDL_GetTicks64)(void);
    Uint64 (*SDL_GetPerformanceCounter)(void);
    Uint64 (*SDL_GetPerformanceFrequency)(void);
    void (*SDL_GL_GetDrawableSize)(SDL_Window *window, int *w, int *h);
    SDL_Window *(*SDL_CreateWindow)(const char *title, int x, int y, int w, int h, Uint32 flags);
    void (*SDL_DestroyWindow)(SDL_Window *window);
    void (*SDL_GL_SwapWindow)(SDL_Window *window);
    SDL_GLContext (*SDL_GL_CreateContext)(SDL_Window *window);
    void *(*SDL_GL_GetProcAddress)(const char *proc);
    void (*SDL_GL_DeleteContext)(SDL_GLContext context);
    int (*SDL_GL_SetAttribute)(SDL_GLattr attr, int value);
    int (*SDL_GL_MakeCurrent)(SDL_Window *window, SDL_GLContext context);
    int (*SDL_GL_SetSwapInterval)(int interval);
    const char *(*SDL_GetError)(void);
} refimport_t;

typedef struct {
    // called before the library is unloaded
	// if the system is just reconfiguring, pass destroyWindow = qfalse,
	// which will keep the screen from flashing to the desktop.
	void (*Shutdown)( refShutdownCode_t code );

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// BeginRegistration makes any existing media pointers invalid
	// and returns the current gl configuration, including screen width
	// and height, which can be used by the client to intelligently
	// size display elements
	void (*BeginRegistration)( void );
    nhandle_t (*RegisterSpriteSheet)( const char *name );
	nhandle_t (*RegisterShader)( const char *name );
	void (*LoadWorld)( const char *name );

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	void (*EndRegistration)( void );

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void (*ClearScene)( void );
    void (*AddPolyAndIndicesToScene)(uint32_t numVerts, const polyVert_t *verts, uint32_t numIndices, const uint32_t *indices);
//	void (*AddRefEntityToScene)( const refEntity_t *re, qboolean intShaderTime );
//	void (*AddPolyToScene)( nhandle_t hShader , int numVerts, const polyVert_t *verts, int num );
//	int (*LightForPoint)( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
//	void (*AddLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
//	void (*AddAdditiveLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
//	void (*AddLinearLightToScene)( const vec3_t start, const vec3_t end, float intensity, float r, float g, float b );
	void (*RenderScene)( void );

	void (*SetColor)( const float *rgba );	// NULL = 1,1,1,1

	void	(*BeginFrame)( void );
	void	(*EndFrame)( void );

//	void	(*RegisterFont)(const char *fontName, int pointSize, fontInfo_t *font);
	void	(*TakeVideoFrame)( int h, int w, byte* captureBuffer, byte *encodeBuffer, qboolean motionJpeg );

	void	(*ThrottleBackend)( void );
	void	(*FinishBloom)( void );

	void	(*SetColorMappings)( void );

	qboolean (*CanMinimize)( void ); // == fbo enabled

//	const glconfig_t *(*GetConfig)( void );

	void	(*VertexLighting)( qboolean allowed );
	void	(*SyncRender)( void );

    void    (*ProcessConsoleEvents)(SDL_Event *events);
} renderExport_t;

extern refimport_t ri;
typedef renderExport_t *(GDR_DECL *GetRenderAPI_t)(uint32_t version, refimport_t *import);

#endif
