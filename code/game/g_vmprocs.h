#ifndef __G_VMPROCS__
#define __G_VMPROCS__

#pragma once

#include "../engine/n_common.h"

#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
typedef struct vmRefImport_s vmRefImport_t;
struct vmRefImport_s
{
    void (*trap_Print)( const char *str );
    void (*trap_Error)( const char *str );

    uint32_t (*trap_Argc)( void );
    void (*trap_Argv)( uint32_t n, char *buf, uint32_t bufferLength );
    void (*trap_Args)( char *buf, uint32_t bufferLength );
    void (*trap_SendConsoleCommand)( const char *text );
    void (*trap_AddCommand)( const char *cmdName );
    void (*trap_RemoveCommand)( const char *cmdName );

    uint32_t (*trap_MemoryRemaining)( void );

    void (*Sys_SnapVector)( float *v );

    nhandle_t (*G_LoadMap)( const char *mapname );
    void (*G_SetActiveMap)( nhandle_t mapHandle, mapinfo_t *info, int32_t *soundBits, linkEntity_t *activeEnts );
    void (*G_CastRay)( ray_t *ray );
#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
    void (*G_SoundRecursive)( int32_t width, int32_t height, float volume, const vec3_t *origin );
    qboolean (*trap_CheckWallHit)( const vec3_t *origin, dirtype_t dir );
    void (*G_SetCameraData)( const vec2_t *origin, float zoom, float rotation );
#else
    void (*G_SoundRecursive)( int32_t width, int32_t height, float volume, const vec3_t origin );
    qboolean (*trap_CheckWallHit)( const vec3_t origin, dirtype_t dir );
    void (*G_SetCameraData)( const vec2_t origin, float zoom, float rotation );
#endif

    int (*trap_Milliseconds)( void );

    void (*trap_Key_SetCatcher)( int32_t catcher );
    uint32_t (*trap_Key_GetCatcher)( void );
    uint32_t (*trap_Key_GetKey)( const char *key );
    void (*trap_Key_ClearStates)( void );

    sfxHandle_t (*trap_Snd_RegisterSfx)( const char *npath );
    sfxHandle_t (*trap_Snd_RegisterTrack)( const char *npath );
    void (*trap_Snd_PlaySfx)( sfxHandle_t sfx );
    void (*trap_Snd_StopSfx)( sfxHandle_t sfx );
    void (*trap_Snd_SetLoopingTrack)( sfxHandle_t track );
    void (*trap_Snd_ClearLoopingTrack)( void );

    nhandle_t (*RE_RegisterShader)( const char *npath );
    nhandle_t (*RE_RegisterSpriteSheet)( const char *npath, uint32_t sheetWidth, uint32_t sheetHeight, uint32_t spriteWidth, uint32_t spriteHeight );
    nhandle_t (*RE_RegisterSprite)( nhandle_t hSpriteSheet, uint32_t index );
    void (*RE_LoadWorldMap)( const char *npath );
    void (*RE_ClearScene)( void );
    void (*RE_RenderScene)( const renderSceneRef_t *fd );
#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
    void (*RE_AddSpriteToScene)( const vec3_t *origin, nhandle_t hSpriteSheet, nhandle_t hSprite );
#else
    void (*RE_AddSpriteToScene)( const vec3_t origin, nhandle_t hSpriteSheet, nhandle_t hSprite );
#endif
    void (*RE_AddPolyToScene)( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );

    void (*Sys_GetGPUConfig)( gpuConfig_t *config );

    void (*Cvar_Register)( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags );
    void (*Cvar_Update)( vmCvar_t *vmCvar );
    void (*Cvar_Set)( const char *varName, const char *value );
    void (*Cvar_VariableStringBuffer)( const char *var_name, char *buffer, uint32_t bufsize );

    file_t (*FS_FOpenRead)( const char *npath, handleOwner_t owner );
    file_t (*FS_FOpenWrite)( const char *npath, handleOwner_t owner );
    file_t (*FS_FOpenAppend)( const char *npath, handleOwner_t owner );
    file_t (*FS_FOpenRW)( const char *npath, handleOwner_t owner );
    fileOffset_t (*FS_FileSeek)( file_t file, fileOffset_t offset, uint32_t whence, handleOwner_t owner );
    fileOffset_t (*FS_FileTell)( file_t file, handleOwner_t owner );
    uint32_t (*FS_FOpenFile)( const char *npath, file_t *file, fileMode_t mode, handleOwner_t owner );
    file_t (*FS_FOpenFileWrite)( const char *npath, file_t *file, handleOwner_t owner );
    uint32_t (*FS_FOpenFileRead)( const char *npath, file_t *file, handleOwner_t owner );
    void (*FS_FClose)( file_t file, handleOwner_t owner );
    uint32_t (*FS_WriteFile)( const void *buffer, uint32_t len, file_t file, handleOwner_t owner );
    uint32_t (*FS_Write)( const void *buffer, uint32_t len, file_t file, handleOwner_t owner );
    uint32_t (*FS_Read)( void *buffer, uint32_t len, file_t file, handleOwner_t owner );
    uint32_t (*FS_FileLength)( file_t file, handleOwner_t owner );
    uint32_t (*FS_GetFileList)( const char *path, const char *extension, char *listbuf, uint32_t bufsize );

    int (*ImGui_BeginWindow)( const char *pLabel, byte *pOpen, ImGuiWindowFlags windowFlags );
    int (*ImGui_BeginPopupModal)( const char *pName, ImGuiWindowFlags flags );
    int (*ImGui_IsWindowCollapsed)( void );
    int (*ImGui_BeginMenu)( const char *pLabel );
    int (*ImGui_MenuItem)( const char *pLabel, const char *pShortcut, byte bUsed );
    int (*ImGui_BeginTable)( const char *pLabel, uint32_t nColumns );
    int (*ImGui_InputText)( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
    int (*ImGui_InputTextMultiline)( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
    int (*ImGui_InputTextWithHint)( const char *pLabel, const char *pHint, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
    int (*ImGui_InputFloat)( const char *pLabel, float *pData );
#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
    int (*ImGui_InputFloat2)( const char *pLabel, vec2_t *pData );
    int (*ImGui_InputFloat3)( const char *pLabel, vec3_t *pData );
    int (*ImGui_InputFloat4)( const char *pLabel, vec4_t *pData );
    int (*ImGui_InputInt2)( const char *pLabel, ivec2_t *pData );
    int (*ImGui_InputInt3)( const char *pLabel, ivec3_t *pData );
    int (*ImGui_InputInt4)( const char *pLabel, ivec4_t *pData );
    int (*ImGui_SliderFloat2)( const char *pLabel, vec2_t *pData, float nMax, float nMin );
    int (*ImGui_SliderFloat3)( const char *pLabel, vec3_t *pData, float nMax, float nMin );
    int (*ImGui_SliderFloat4)( const char *pLabel, vec4_t *pData, float nMax, float nMin );
    int (*ImGui_SliderInt2)( const char *pLabel, ivec2_t *pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt3)( const char *pLabel, ivec3_t *pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt4)( const char *pLabel, ivec4_t *pData, int32_t nMax, int32_t nMin );
    int (*ImGui_ColorEdit3)( const char *pLabel, vec3_t *pColor, ImGuiColorEditFlags flags );
    int (*ImGui_ColorEdit4)( const char *pLabel, vec4_t *pColor, ImGuiColorEditFlags flags );
#else
    int (*ImGui_InputFloat2)( const char *pLabel, vec2_t pData );
    int (*ImGui_InputFloat3)( const char *pLabel, vec3_t pData );
    int (*ImGui_InputFloat4)( const char *pLabel, vec4_t pData );
    int (*ImGui_InputInt2)( const char *pLabel, ivec2_t pData );
    int (*ImGui_InputInt3)( const char *pLabel, ivec3_t pData );
    int (*ImGui_InputInt4)( const char *pLabel, ivec4_t pData );
    int (*ImGui_SliderFloat2)( const char *pLabel, vec2_t pData, float nMax, float nMin );
    int (*ImGui_SliderFloat3)( const char *pLabel, vec3_t pData, float nMax, float nMin );
    int (*ImGui_SliderFloat4)( const char *pLabel, vec4_t pData, float nMax, float nMin );
    int (*ImGui_SliderInt2)( const char *pLabel, ivec2_t pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt3)( const char *pLabel, ivec3_t pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt4)( const char *pLabel, ivec4_t pData, int32_t nMax, int32_t nMin );
    int (*ImGui_ColorEdit3)( const char *pLabel, vec3_t pColor, ImGuiColorEditFlags flags );
    int (*ImGui_ColorEdit4)( const char *pLabel, vec4_t pColor, ImGuiColorEditFlags flags );
#endif
    int (*ImGui_SliderFloat)( const char *pLabel, float *pData, float nMax, float nMin );
    int (*ImGui_InputInt)( const char *pLabel, int32_t *pData );
    int (*ImGui_SliderInt)( const char *pLabel, int32_t *pData, int32_t nMax, int32_t nMin );
    int (*ImGui_ArrowButton)( const char *pLabel, ImGuiDir dir );
    int (*ImGui_Checkbox)( const char *pLabel, byte *bPressed );
    int (*ImGui_Button)( const char *pLabel );
    float (*ImGui_GetFontScale)( void );
    void (*ImGui_EndWindow)( void );
    void (*ImGui_SetWindowCollapsed)( int bCollapsed );
    void (*ImGui_SetWindowPos)( float x, float y );
    void (*ImGui_SetWindowSize)( float w, float h );
    void (*ImGui_SetWindowFontScale)( float scale );
    void (*ImGui_EndMenu)( void );
    void (*ImGui_SetItemTooltipUnformatted)( const char *pTooltip );
    void (*ImGui_TableNextRow)( void );
    void (*ImGui_TableNextColumn)( void );
    void (*ImGui_EndTable)( void );
    void (*ImGui_SetCursorPos)( float x, float y );
    void (*ImGui_GetCursorPos)( float *x, float *y );
    void (*ImGui_SetCursorScreenPos)( float x, float y );
    void (*ImGui_GetCursorScreenPos)( float *x, float *y );
    void (*ImGui_PopColor)( void );
    void (*ImGui_SameLine)( float offsetFromStartX );
    void (*ImGui_NewLine)( void );
    void (*ImGui_TextUnformatted)( const char *pText );
#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
    void (*ImGui_ColoredTextUnformatted)( const vec4_t *pColor, const char *pText );
    void (*ImGui_PushColor)( ImGuiCol index, const vec4_t *color );
#else
    void (*ImGui_ColoredTextUnformatted)( const vec4_t pColor, const char *pText );
    void (*ImGui_PushColor)( ImGuiCol index, const vec4_t color );
#endif
    void (*ImGui_SeparatorText)( const char *pText );
    void (*ImGui_Separator)( void );
    void (*ImGui_ProgressBar)( float fraction );
    void (*ImGui_OpenPopup)( const char *pName );
    void (*ImGui_CloseCurrentPopup)( void );
    void (*ImGui_EndPopup)( void );
};
#endif

#endif
