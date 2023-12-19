
// sg_syscalls.c: this file is only included in development dll builds, the .asm file is used instead when building the VM file
#include "../rendercommon/r_public.h"
#include "sg_imgui.h"
#include "../game/g_game.h"
#include "sg_local.h"

#ifdef Q3_VM
    #error NEVER use is VM build
#endif

vmRefImport_t vmi;

//static intptr_t (GDR_DECL *syscall)(intptr_t arg, uint32_t, ...) = (intptr_t (GDR_DECL *)(intptr_t, uint32_t, ...)) - 1;

void dllEntry(const vmRefImport_t *import)
{
    memcpy(&vmi, import, sizeof(vmi));
}

#if 0
void dllEntry(intptr_t (GDR_DECL *syscallptr)(intptr_t arg, uint32_t numArgs, ...))
{
    syscall = syscallptr;
}
#endif

int32_t PASSFLOAT(float x)
{
    float floatTemp;
    floatTemp = x;
    return *(int32_t *)&floatTemp;
}

void trap_Print( const char *str )
{
    vmi.trap_Print( str );
}

void trap_Error( const char *str )
{
    vmi.trap_Error( str );
}

uint32_t trap_Argc( void )
{
    return vmi.trap_Argc();
}

void trap_Argv( uint32_t n, char *buf, uint32_t bufferLength )
{
    vmi.trap_Argv( n, buf, bufferLength );
}

void trap_Args( char *buf, uint32_t bufferLength )
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

uint32_t trap_MemoryRemaining( void )
{
    return vmi.trap_MemoryRemaining();
}

void Sys_SnapVector( float *v )
{
    vmi.Sys_SnapVector( v );
}

qboolean trap_CheckWallHit( const vec3_t origin, dirtype_t dir )
{
    return vmi.trap_CheckWallHit( origin, dir );
}

int G_LoadMap( int levelIndex, mapinfo_t *info, uint32_t *soundBits, linkEntity_t *activeEnts )
{
    return vmi.G_LoadMap( levelIndex, info, soundBits, activeEnts );
}

void G_CastRay( ray_t *ray )
{
    vmi.G_CastRay( ray );
}

void G_SoundRecursive( int width, int height, float volume, const vec3_t origin )
{
    vmi.G_SoundRecursive( width, height, volume, origin );
}

void G_SetCameraData( const vec2_t origin, float zoom, float rotation )
{
    vmi.G_SetCameraData( origin, zoom, rotation );
}

int trap_Milliseconds( void )
{
    return vmi.trap_Milliseconds();
}

void trap_Key_SetCatcher( uint32_t catcher )
{
    vmi.trap_Key_SetCatcher( catcher );
}

uint32_t trap_Key_GetCatcher( void )
{
    return vmi.trap_Key_GetCatcher();
}

uint32_t trap_Key_GetKey( const char *key )
{
    return vmi.trap_Key_GetKey( key );
}

void trap_Key_ClearStates( void )
{
    vmi.trap_Key_ClearStates();
}

sfxHandle_t trap_Snd_RegisterSfx( const char *npath )
{
    return vmi.trap_Snd_RegisterSfx( npath );
}

sfxHandle_t trap_Snd_RegisterTrack( const char *npath )
{
    return vmi.trap_Snd_RegisterTrack( npath );
}

void trap_Snd_QueueTrack( sfxHandle_t track )
{
    vmi.trap_Snd_QueueTrack( track );
}

void trap_Snd_PlaySfx( sfxHandle_t sfx )
{
    vmi.trap_Snd_PlaySfx( sfx );
}

void trap_Snd_StopSfx( sfxHandle_t sfx )
{
    vmi.trap_Snd_StopSfx( sfx );
}

void trap_Snd_SetLoopingTrack( sfxHandle_t track )
{
    vmi.trap_Snd_SetLoopingTrack( track );
}

void trap_Snd_ClearLoopingTrack( void )
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

nhandle_t RE_RegisterSpriteSheet( const char *npath, uint32_t sheetWidth, uint32_t sheetHeight, uint32_t spriteWidth, uint32_t spriteHeight )
{
    return vmi.RE_RegisterSpriteSheet( npath, sheetWidth, sheetHeight, spriteWidth, spriteHeight );
}

nhandle_t RE_RegisterSprite( nhandle_t hSpriteSheet, uint32_t index )
{
    return vmi.RE_RegisterSprite( hSpriteSheet, index );
}

void RE_AddSpriteToScene( const vec3_t origin, nhandle_t hSpriteSheet, nhandle_t hSprite )
{
    vmi.RE_AddSpriteToScene( origin, hSpriteSheet, hSprite );
}

void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts )
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

file_t trap_FS_FOpenWrite( const char *npath )
{
    return vmi.FS_FOpenWrite( npath, H_SGAME );
}

void trap_FS_FClose( file_t f )
{
    vmi.FS_FClose( f, H_SGAME );
}

uint32_t trap_FS_Write( const void *data, uint32_t size, file_t f )
{
    return vmi.FS_Write( data, size, f, H_SGAME );
}

uint32_t trap_FS_Read( void *data, uint32_t size, file_t f )
{
    return vmi.FS_Read( data, size, f, H_SGAME );
}

uint32_t trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, uint32_t bufsize )
{
    return vmi.FS_GetFileList( path, extension, listbuf, bufsize );
}

uint32_t trap_FS_FileSeek( file_t f, fileOffset_t offset, uint32_t whence )
{
    return vmi.FS_FileSeek( f, offset, whence, H_SGAME );
}

uint32_t trap_FS_FileTell( file_t f )
{
    return vmi.FS_FileTell( f, H_SGAME );
}

void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags )
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

void Cvar_VariableStringBuffer( const char *var_name, char *buffer, uint32_t bufsize )
{
    vmi.Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

int ImGui_BeginWindow( ImGuiWindow *pWindow )
{
    return vmi.ImGui_BeginWindow( pWindow );
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

int ImGui_MenuItem( ImGuiMenuItem *pItem )
{
    return vmi.ImGui_MenuItem( pItem );
}

void ImGui_SetItemTooltipUnformatted( const char *pTooltip )
{
    vmi.ImGui_SetItemTooltipUnformatted( pTooltip );
}

int ImGui_BeginTable( const char *pLabel, uint32_t nColumns )
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

int ImGui_InputText( ImGuiInputText *pInput )
{
    return vmi.ImGui_InputText( pInput );
}

int ImGui_InputTextMultiline( ImGuiInputText *pInput )
{
    return vmi.ImGui_InputTextMultiline( pInput );
}

int ImGui_InputTextWithHint( ImGuiInputTextWithHint *pInput )
{
    return vmi.ImGui_InputTextWithHint( pInput );
}

int ImGui_InputFloat( ImGuiInputFloat *pInput )
{
    return vmi.ImGui_InputFloat( pInput );
}

int ImGui_InputFloat2( ImGuiInputFloat2 *pInput )
{
    return vmi.ImGui_InputFloat2( pInput );
}

int ImGui_InputFloat3( ImGuiInputFloat3 *pInput )
{
    return vmi.ImGui_InputFloat3( pInput );
}

int ImGui_InputFloat4( ImGuiInputFloat4 *pInput )
{
    return vmi.ImGui_InputFloat4( pInput );
}

int ImGui_InputInt( ImGuiInputInt *pInput )
{
    return vmi.ImGui_InputInt( pInput );
}

int ImGui_InputInt2( ImGuiInputInt2 *pInput )
{
    return vmi.ImGui_InputInt2( pInput );
}

int ImGui_InputInt3( ImGuiInputInt3 *pInput )
{
    return vmi.ImGui_InputInt3( pInput );
}

int ImGui_InputInt4( ImGuiInputInt4 *pInput )
{
    return vmi.ImGui_InputInt4( pInput );
}

int ImGui_SliderFloat( ImGuiSliderFloat *pSlider )
{
    return vmi.ImGui_SliderFloat( pSlider );
}

int ImGui_SliderFloat2( ImGuiSliderFloat2 *pSlider )
{
    return vmi.ImGui_SliderFloat2( pSlider );
}

int ImGui_SliderFloat3( ImGuiSliderFloat3 *pSlider )
{
    return vmi.ImGui_SliderFloat3( pSlider );
}

int ImGui_SliderFloat4( ImGuiSliderFloat4 *pSlider )
{
    return vmi.ImGui_SliderFloat4( pSlider );
}

int ImGui_SliderInt( ImGuiSliderInt *pSlider )
{
    return vmi.ImGui_SliderInt( pSlider );
}

int ImGui_SliderInt2( ImGuiSliderInt2 *pSlider )
{
    return vmi.ImGui_SliderInt2( pSlider );
}

int ImGui_SliderInt3( ImGuiSliderInt3 *pSlider )
{
    return vmi.ImGui_SliderInt3( pSlider );
}

int ImGui_SliderInt4( ImGuiSliderInt4 *pSlider )
{
    return vmi.ImGui_SliderInt4( pSlider );
}

int ImGui_ColorEdit3( ImGuiColorEdit3 *pEdit )
{
    return vmi.ImGui_ColorEdit3( pEdit );
}

int ImGui_ColorEdit4( ImGuiColorEdit4 *pEdit )
{
    return vmi.ImGui_ColorEdit4( pEdit );
}

int ImGui_ArrowButton( const char *pLabel, ImGuiDir dir )
{
    return vmi.ImGui_ArrowButton( pLabel, dir );
}

int ImGui_Checkbox( ImGuiCheckbox *pCheckbox )
{
    return vmi.ImGui_Checkbox( pCheckbox );
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
    vmi.ImGui_SetCursorScreenPos( x, y );
}

void ImGui_GetCursorScreenPos( float *x, float *y )
{
    vmi.ImGui_GetCursorScreenPos( x, y );
}

void ImGui_PushColor( ImGuiCol index, const vec4_t color )
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

void ImGui_SameLine( float offset_from_x ) {
    vmi.ImGui_SameLine( offset_from_x );
}

void ImGui_ProgressBar( float fraction )
{
    vmi.ImGui_ProgressBar( fraction );
}

void ImGui_TextUnformatted( const char *pText )
{
    vmi.ImGui_TextUnformatted( pText );
}

void ImGui_ColoredTextUnformatted( const vec4_t pColor, const char *pText )
{
    vmi.ImGui_ColoredTextUnformatted( pColor, pText );
}

int ImGui_BeginPopupModal( const char *pName, ImGuiWindowFlags flags )
{
    return vmi.ImGui_BeginPopupModal( pName, flags );
}

void ImGui_CloseCurrentPopup( void )
{
    vmi.ImGui_CloseCurrentPopup();
}

void ImGui_EndPopup( void )
{
    vmi.ImGui_EndPopup();
}

void ImGui_OpenPopup( const char *pName )
{
    vmi.ImGui_OpenPopup( pName );
}

