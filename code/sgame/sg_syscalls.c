
// sg_syscalls.c: this file is only included in development dll builds, the .asm file is used instead when building the VM file
#include "../rendercommon/r_public.h"
#include "sg_imgui.h"
#include "../game/g_game.h"
#include "sg_local.h"

#ifdef Q3_VM
    #error NEVER use is VM build
#endif

vmRefImport_t vmi;

//static intptr_t (GDR_DECL *syscall)(intptr_t arg, int, ...) = (intptr_t (GDR_DECL *)(intptr_t, int, ...)) - 1;

void dllEntry(const vmRefImport_t *import)
{
    memcpy(&vmi, import, sizeof(vmi));
}

#if 0
void dllEntry(intptr_t (GDR_DECL *syscallptr)(intptr_t arg, int numArgs, ...))
{
    syscall = syscallptr;
}
#endif

int32_t PASSFLOAT( float x )
{
    float floatTemp;
    floatTemp = x;
    return *(int32_t *)&floatTemp;
}

void Print( const char *str )
{
    vmi.trap_Print( str );
}

void Error( const char *str )
{
    vmi.trap_Error( str );
}

int trap_Argc( void )
{
    return vmi.trap_Argc();
}

void trap_Argv( int n, char *buf, int bufferLength )
{
    vmi.trap_Argv( n, buf, bufferLength );
}

void trap_Args( char *buf, int bufferLength )
{
    vmi.trap_Args( buf, bufferLength );
}

void trap_SendConsoleCommand( const char *text )
{
    vmi.trap_SendConsoleCommand( text );
}

void trap_AddCommand( const char *cmdName )
{
    vmi.trap_AddCommand( cmdName );
}

void trap_RemoveCommand( const char *cmdName )
{
    vmi.trap_RemoveCommand( cmdName );
}

int Sys_MemoryRemaining( void )
{
    return vmi.trap_MemoryRemaining();
}

void Sys_SnapVector( float *v )
{
    vmi.Sys_SnapVector( v );
}

qboolean G_CheckWallHit( const vec3_t *origin, dirtype_t dir )
{
    return vmi.trap_CheckWallHit( origin, dir );
}

nhandle_t G_LoadMap( const char *mapname )
{
    return vmi.G_LoadMap( mapname );
}

void G_SetActiveMap( nhandle_t mapHandle, mapinfo_t *info, int *soundBits, linkEntity_t *activeEnts )
{
    vmi.G_SetActiveMap( mapHandle, info, soundBits, activeEnts );
}

void G_CastRay( ray_t *ray )
{
    vmi.G_CastRay( ray );
}

void G_SoundRecursive( int width, int height, float volume, const vec3_t *origin )
{
    vmi.G_SoundRecursive( width, height, volume, origin );
}

void G_SetCameraData( const vec2_t *origin, float zoom, float rotation )
{
    vmi.G_SetCameraData( origin, zoom, rotation );
}

int Sys_Milliseconds( void )
{
    return vmi.trap_Milliseconds();
}

void trap_Key_SetCatcher( int catcher )
{
    vmi.trap_Key_SetCatcher( catcher );
}

int trap_Key_GetCatcher( void )
{
    return vmi.trap_Key_GetCatcher();
}

int trap_Key_GetKey( const char *key )
{
    return vmi.trap_Key_GetKey( key );
}

void trap_Key_ClearStates( void )
{
    vmi.trap_Key_ClearStates();
}

sfxHandle_t Snd_RegisterSfx( const char *npath )
{
    return vmi.trap_Snd_RegisterSfx( npath );
}

sfxHandle_t Snd_RegisterTrack( const char *npath )
{
    return vmi.trap_Snd_RegisterTrack( npath );
}

void Snd_PlaySfx( sfxHandle_t sfx )
{
    vmi.trap_Snd_PlaySfx( sfx );
}

void Snd_StopSfx( sfxHandle_t sfx )
{
    vmi.trap_Snd_StopSfx( sfx );
}

void Snd_SetLoopingTrack( sfxHandle_t track )
{
    vmi.trap_Snd_SetLoopingTrack( track );
}

void Snd_ClearLoopingTrack( void )
{
    vmi.trap_Snd_ClearLoopingTrack();
}

nhandle_t RE_RegisterShader( const char *npath )
{
    return vmi.RE_RegisterShader( npath );
}

void RE_LoadWorldMap( const char *npath )
{
    vmi.RE_LoadWorldMap( npath );
}

void RE_ClearScene( void )
{
    vmi.RE_ClearScene();
}

void RE_RenderScene( const renderSceneRef_t *fd )
{
    vmi.RE_RenderScene( fd );
}

nhandle_t RE_RegisterSpriteSheet( const char *npath, int sheetWidth, int sheetHeight, int spriteWidth, int spriteHeight )
{
    return vmi.RE_RegisterSpriteSheet( npath, sheetWidth, sheetHeight, spriteWidth, spriteHeight );
}

nhandle_t RE_RegisterSprite( nhandle_t hSpriteSheet, int index )
{
    return vmi.RE_RegisterSprite( hSpriteSheet, index );
}

void RE_AddSpriteToScene( const vec3_t *origin, nhandle_t hSpriteSheet, nhandle_t hSprite )
{
    vmi.RE_AddSpriteToScene( origin, hSpriteSheet, hSprite );
}

void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, int numVerts )
{
    vmi.RE_AddPolyToScene( hShader, verts, numVerts );
}

void Sys_GetGPUConfig( gpuConfig_t *config )
{
    vmi.Sys_GetGPUConfig( config );
}

uint32_t trap_FS_FOpenFile( const char *npath, file_t *f, fileMode_t mode )
{
    return vmi.FS_FOpenFile( npath, f, mode, H_SGAME );
}

file_t trap_FS_FOpenRead( const char *npath )
{
    return vmi.FS_FOpenRead( npath, H_SGAME );
}

file_t trap_FS_FOpenWrite( const char *npath )
{
    return vmi.FS_FOpenWrite( npath, H_SGAME );
}

void trap_FS_FClose( file_t f )
{
    vmi.FS_FClose( f, H_SGAME );
}

int trap_FS_Write( const void *data, int size, file_t f )
{
    return vmi.FS_Write( data, size, f, H_SGAME );
}

int trap_FS_Read( void *data, int size, file_t f )
{
    return vmi.FS_Read( data, size, f, H_SGAME );
}

int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize )
{
    return vmi.FS_GetFileList( path, extension, listbuf, bufsize );
}

int trap_FS_FileSeek( file_t f, fileOffset_t offset, int whence )
{
    return vmi.FS_FileSeek( f, offset, whence, H_SGAME );
}

int trap_FS_FileTell( file_t f )
{
    return vmi.FS_FileTell( f, H_SGAME );
}

void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags )
{
    vmi.Cvar_Register( vmCvar, varName, defaultValue, flags );
}

void Cvar_Update( vmCvar_t *vmCvar )
{
    vmi.Cvar_Update( vmCvar );
}

void Cvar_Set( const char *varName, const char *value )
{
    vmi.Cvar_Set( varName, value );
}

void Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize )
{
    vmi.Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

int ImGui_BeginWindow( const char *pLabel, byte *pOpen, ImGuiWindowFlags windowFlags )
{
    return vmi.ImGui_BeginWindow( pLabel, pOpen, windowFlags );
}

void ImGui_EndWindow( void )
{
    vmi.ImGui_EndWindow();
}

int ImGui_IsWindowCollapsed( void )
{
    return vmi.ImGui_IsWindowCollapsed();
}

void ImGui_SetWindowCollapsed( int bCollapsed )
{
    vmi.ImGui_SetWindowCollapsed( bCollapsed );
}

void ImGui_SetWindowPos( float x, float y )
{
    vmi.ImGui_SetWindowPos( x, y );
}

void ImGui_SetWindowSize( float w, float h )
{
    vmi.ImGui_SetWindowSize( w, h );
}

void ImGui_SetWindowFontScale( float scale )
{
    vmi.ImGui_SetWindowFontScale( scale );
}

int ImGui_BeginMenu( const char *pLabel )
{
    return vmi.ImGui_BeginMenu( pLabel );
}

void ImGui_EndMenu( void )
{
    vmi.ImGui_EndMenu();
}

int ImGui_MenuItem( const char *pLabel, const char *pShortcut, byte bUsed )
{
    return vmi.ImGui_MenuItem( pLabel, pShortcut, bUsed );
}

void ImGui_SetItemTooltipUnformatted( const char *pTooltip )
{
    vmi.ImGui_SetItemTooltipUnformatted( pTooltip );
}

int ImGui_BeginTable( const char *pLabel, int nColumns )
{
    return vmi.ImGui_BeginTable( pLabel, nColumns );
}

void ImGui_TableNextRow( void )
{
    vmi.ImGui_TableNextRow();
}

void ImGui_TableNextColumn( void )
{
    vmi.ImGui_TableNextColumn();
}

void ImGui_EndTable( void )
{
    vmi.ImGui_EndTable();
}

int ImGui_InputText( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags )
{
    return vmi.ImGui_InputText( pLabel, pBuffer, nBufSize, flags );
}

int ImGui_InputTextMultiline( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags )
{
    return vmi.ImGui_InputTextMultiline( pLabel, pBuffer, nBufSize, flags );
}

int ImGui_InputTextWithHint( const char *pLabel, const char *pHint, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags )
{
    return vmi.ImGui_InputTextWithHint( pLabel, pHint, pBuffer, nBufSize, flags );
}

int ImGui_InputFloat( const char *pLabel, float *pData )
{
    return vmi.ImGui_InputFloat( pLabel, pData );
}

int ImGui_InputFloat2( const char *pLabel, vec2_t *pData )
{
    return vmi.ImGui_InputFloat2( pLabel, pData );
}

int ImGui_InputFloat3( const char *pLabel, vec3_t *pData )
{
    return vmi.ImGui_InputFloat3( pLabel, pData );
}

int ImGui_InputFloat4( const char *pLabel, vec4_t *pData )
{
    return vmi.ImGui_InputFloat4( pLabel, pData );
}

int ImGui_InputInt( const char *pLabel, int *pData )
{
    return vmi.ImGui_InputInt( pLabel, pData );
}

int ImGui_InputInt2( const char *pLabel, ivec2_t *pData )
{
    return vmi.ImGui_InputInt2( pLabel, pData );
}

int ImGui_InputInt3( const char *pLabel, ivec3_t *pData )
{
    return vmi.ImGui_InputInt3( pLabel, pData );
}

int ImGui_InputInt4( const char *pLabel, ivec4_t *pData )
{
    return vmi.ImGui_InputInt4( pLabel, pData );
}

int ImGui_SliderFloat( const char *pLabel, float *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderFloat( pLabel, pData, nMax, nMin );
}

int ImGui_SliderFloat2( const char *pLabel, vec2_t *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderFloat2( pLabel, pData, nMax, nMin );
}

int ImGui_SliderFloat3( const char *pLabel, vec3_t *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderFloat3( pLabel, pData, nMax, nMin );
}

int ImGui_SliderFloat4( const char *pLabel, vec4_t *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderFloat4( pLabel, pData, nMax, nMin );
}

int ImGui_SliderInt( const char *pLabel, int *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderInt( pLabel, pData, nMax, nMin );
}

int ImGui_SliderInt2( const char *pLabel, ivec2_t *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderInt2( pLabel, pData, nMax, nMin );
}

int ImGui_SliderInt3( const char *pLabel, ivec3_t *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderInt3( pLabel, pData, nMax, nMin );
}

int ImGui_SliderInt4( const char *pLabel, ivec4_t *pData, float nMax, float nMin )
{
    return vmi.ImGui_SliderInt4( pLabel, pData, nMax, nMin );
}

int ImGui_ColorEdit3( const char *pLabel, vec3_t *pColor, ImGuiColorEditFlags flags )
{
    return vmi.ImGui_ColorEdit3( pLabel, pColor, flags );
}

int ImGui_ColorEdit4( const char *pLabel, vec4_t *pColor, ImGuiColorEditFlags flags )
{
    return vmi.ImGui_ColorEdit4( pLabel, pColor, flags );
}

int ImGui_ArrowButton( const char *pLabel, ImGuiDir dir )
{
    return vmi.ImGui_ArrowButton( pLabel, dir );
}

int ImGui_Checkbox( const char *pLabel, byte *bPressed )
{
    return vmi.ImGui_Checkbox( pLabel, bPressed );
}

int ImGui_Button( const char *pLabel )
{
    return vmi.ImGui_Button( pLabel );
}

float ImGui_GetFontScale( void )
{
    return vmi.ImGui_GetFontScale();
}

void ImGui_SetCursorPos( float x, float y )
{
    vmi.ImGui_SetCursorPos( x, y );
}

void ImGui_GetCursorPos( float *x, float *y )
{
    vmi.ImGui_GetCursorPos( x, y );
}

void ImGui_SetCursorScreenPos( float x, float y )
{
    vmi.ImGui_SetCursorPos( x, y );
}

void ImGui_GetCursorScreenPos( float *x, float *y )
{
    vmi.ImGui_GetCursorScreenPos( x, y );
}

void ImGui_PushColor( ImGuiCol index, const vec4_t *color )
{
    vmi.ImGui_PushColor( index, color );
}

void ImGui_PopColor( void )
{
    vmi.ImGui_PopColor();
}

void ImGui_NewLine( void )
{
    vmi.ImGui_NewLine();
}

void ImGui_SeparatorText( const char *pText )
{
    vmi.ImGui_SeparatorText( pText );
}

void ImGui_Separator( void )
{
    vmi.ImGui_Separator();
}

void ImGui_ProgressBar( float fraction )
{
    vmi.ImGui_ProgressBar( fraction );
}

void ImGui_SameLine( float offset_from_x )
{
    vmi.ImGui_SameLine( offset_from_x );
}

void ImGui_TextUnformatted( const char *pText )
{
    vmi.ImGui_TextUnformatted( pText );
}

void ImGui_ColoredTextUnformatted( const vec4_t *pColor, const char *pText )
{
    vmi.ImGui_ColoredTextUnformatted( pColor, pText );
}

int ImGui_BeginPopupModal( const char *pName, ImGuiWindowFlags flags )
{
    return vmi.ImGui_BeginPopupModal( pName, flags );
}

void ImGui_EndPopup( void )
{
    vmi.ImGui_EndPopup();
}

void ImGui_OpenPopup( const char *pName )
{
    vmi.ImGui_OpenPopup( pName );
}

void ImGui_CloseCurrentPopup( void )
{
    vmi.ImGui_CloseCurrentPopup();
}
