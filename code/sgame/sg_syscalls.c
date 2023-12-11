
// sg_syscalls.c: this file is only included in development dll builds, the .asm file is used instead when building the VM file
#include "../rendercommon/r_public.h"
#include "sg_imgui.h"
#include "../game/g_game.h"
#include "sg_local.h"

#ifdef Q3_VM
    #error Never use is VM build
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

int32_t G_LoadMap( int32_t index, mapinfo_t *info )
{
    return vmi.G_LoadMap( index, info );
}

uint32_t trap_Milliseconds( void )
{
    return vmi.trap_Milliseconds();
}

void trap_GetGPUConfig(gpuConfig_t *config)
{
    vmi.trap_GetGPUConfig(config);
}

void trap_Cmd_ExecuteText(cbufExec_t exec, const char *text)
{
    vmi.trap_Cmd_ExecuteText(exec, text);
}

void RE_ClearScene( void )
{
    vmi.trap_RE_ClearScene();
}

void trap_GetClipboardData( char *buf, uint32_t bufsize )
{
    vmi.trap_GetClipboardData(buf, bufsize);
}

uint32_t trap_Key_GetCatcher(void)
{
    return vmi.trap_Key_GetCatcher();
}

void trap_Key_SetCatcher(uint32_t catcher)
{
    vmi.trap_Key_SetCatcher(catcher);
}

uint32_t trap_Key_GetKey(const char *binding)
{
    return vmi.trap_Key_GetKey(binding);
}

qboolean trap_Key_IsDown(uint32_t keynum)
{
    return vmi.trap_Key_IsDown(keynum);
}

void trap_Key_ClearStates(void)
{
    vmi.trap_Key_ClearStates();
}

qboolean trap_Key_AnyDown(void)
{
    return vmi.trap_Key_AnyDown();
}

void trap_Print(const char *str)
{
    vmi.trap_Print(str);
}

void trap_Error(const char *str)
{
    vmi.trap_Error(str);
}

void RE_SetColor(const float *rgba)
{
    vmi.trap_RE_SetColor(rgba);
}

void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts )
{
    vmi.trap_RE_AddPolyToScene(hShader, verts, numVerts);
}

void RE_AddEntityToScene( const renderEntityRef_t *ent )
{
//    vmi.trap_RE_AddEntityToScene(ent);
}

void RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys )
{
    vmi.trap_RE_AddPolyListToScene(polys, numPolys);
}

void RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader )
{
    vmi.trap_RE_DrawImage(x, y, w, h, u1, v1, u2, v2, hShader);
}

void RE_RenderScene( const renderSceneRef_t *fd )
{
    vmi.trap_RE_RenderScene(fd);
}

void RE_LoadWorldMap( const char *filename )
{
    vmi.trap_RE_LoadWorldMap( filename );
}

nhandle_t RE_RegisterShader(const char *name)
{
    return vmi.trap_RE_RegisterShader(name);
}

sfxHandle_t trap_Snd_RegisterSfx(const char *npath)
{
    return vmi.trap_Snd_RegisterSfx(npath);
}

void trap_Snd_PlaySfx(sfxHandle_t sfx)
{
    vmi.trap_Snd_PlaySfx(sfx);
}

void trap_Snd_StopSfx(sfxHandle_t sfx)
{
    vmi.trap_Snd_StopSfx(sfx);
}

void G_SetBindNames( const char **bindnames, uint32_t numbindnames )
{
    vmi.G_SetBindNames( bindnames, numbindnames );
}

void trap_UpdateScreen(void)
{
    vmi.trap_UpdateScreen();
}

void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags )
{
    vmi.trap_Cvar_Register(vmCvar, varName, defaultValue, flags);
}

void Cvar_Update( vmCvar_t *vmCvar )
{
    vmi.trap_Cvar_Update(vmCvar);
}

void Cvar_Set( const char *var_name, const char *value )
{
    vmi.trap_Cvar_Set(var_name, value);
}

void Cvar_VariableStringBuffer( const char *var_name, char *buffer, uint32_t bufsize )
{
    vmi.trap_Cvar_VariableStringBuffer(var_name, buffer, bufsize);
}

uint32_t trap_Argc( void )
{
    return vmi.trap_Argc();
}

void trap_Argv( uint32_t n, char *buffer, uint32_t bufferLength )
{
    vmi.trap_Argv(n, buffer, bufferLength);
}

void trap_Args( char *buffer, uint32_t bufferLength )
{
    return; // not meant to called
}


file_t trap_FS_FOpenRead( const char *npath )
{
    return vmi.FS_FOpenRead( npath, H_SGAME );
}

file_t trap_FS_FOpenWrite( const char *npath )
{
    return vmi.FS_FOpenWrite( npath, H_SGAME );
}

file_t trap_FS_FOpenAppend( const char *npath )
{
    return vmi.FS_FOpenAppend( npath, H_SGAME );
}

file_t trap_FS_FOpenRW( const char *npath )
{
    return vmi.FS_FOpenRW( npath, H_SGAME );
}

fileOffset_t trap_FS_FileSeek( file_t file, fileOffset_t offset, uint32_t whence )
{
    return vmi.FS_FileSeek( file, offset, whence, H_SGAME );

}

fileOffset_t trap_FS_FileTell( file_t file )
{
    return vmi.FS_FileTell( file, H_SGAME );
}

uint64_t trap_FS_FOpenFile( const char *npath, file_t *file, fileMode_t mode )
{
    return vmi.FS_FOpenFile( npath, file, mode, H_SGAME );
}

file_t trap_FS_FOpenFileWrite( const char *npath, file_t *file )
{
    return vmi.FS_FOpenFileWrite( npath, file, H_SGAME );
}

uint64_t trap_FS_FOpenFileRead( const char *npath, file_t *file )
{
    return vmi.FS_FOpenFileRead( npath, file, H_SGAME );
}

void trap_FS_FClose( file_t file )
{
    vmi.FS_FClose( file, H_SGAME );
}

uint64_t trap_FS_WriteFile( const void *buffer, uint64_t len, file_t file )
{
    return vmi.FS_WriteFile( buffer, len, file, H_SGAME );
}

uint64_t trap_FS_Write( const void *buffer, uint64_t len, file_t file )
{
    return vmi.FS_Write( buffer, len, file, H_SGAME );
}

uint64_t trap_FS_Read( void *buffer, uint64_t len, file_t file )
{
    return vmi.FS_Read( buffer, len, file, H_SGAME );
}

uint64_t trap_FS_FileLength( file_t file )
{
    return vmi.FS_FileLength( file, H_SGAME );
}

uint64_t trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, uint64_t bufsize )
{
    return vmi.FS_GetFileList( path, extension, listbuf, bufsize );
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

