
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

void trap_Print( const char *str );
void trap_Error( const char *str );
uint32_t trap_Argc( void );
void trap_Argv( uint32_t n, char *buf, uint32_t bufferLength );
void trap_Args( char *buf, uint32_t bufferLength );
void trap_SendConsoleCommand( const char *text );
void trap_AddCommand( const char *cmdName );
void trap_RemoveCommand( const char *cmdName );
uint32_t trap_MemoryRemaining( void );
void Sys_SnapVector( float *v );
int G_LoadMap( int levelIndex, mapinfo_t *info, uint32_t *soundBits, linkEntity_t *activeEnts );
void G_CastRay( ray_t *ray );
void G_SoundRecursive( int width, int height, float volume, const vec3_t origin );
int trap_Milliseconds( void );

void trap_Key_SetCatcher( uint32_t catcher );
uint32_t trap_Key_GetCatcher( void );
uint32_t trap_Key_GetKey( const char *key );
void trap_Key_ClearStates( void );

sfxHandle_t trap_Snd_RegisterSfx( const char *npath );
sfxHandle_t trap_Snd_RegisterTrack( const char *npath );
void trap_Snd_QueueTrack( sfxHandle_t track );
void trap_Snd_PlaySfx( sfxHandle_t sfx );
void trap_Snd_StopSfx( sfxHandle_t sfx );
void trap_Snd_SetLoopingTrack( sfxHandle_t track );
void trap_Snd_ClearLoopingTrack( void );

nhandle_t RE_RegisterShader( const char *npath );
void RE_LoadWorldMap( const char *npath );
void RE_ClearScene( void );
void RE_RenderScene( const renderSceneRef_t *fd );
void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );

void Sys_GetGPUConfig( gpuConfig_t *config );
uint32_t trap_FS_FOpenFile( const char *npath, file_t *f, fileMode_t mode );
file_t trap_FS_FOpenWrite( const char *npath );
void trap_FS_FClose( file_t f );
uint32_t trap_FS_Write( const void *data, uint32_t size, file_t f );
uint32_t trap_FS_Read( void *data, uint32_t size, file_t f );
uint32_t trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, uint32_t bufsize ); 
uint32_t trap_FS_FileSeek( file_t f, fileOffset_t offset, uint32_t whence );
uint32_t trap_FS_FileTell( file_t f );
void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags );
void Cvar_Update( vmCvar_t *vmCvar );
void Cvar_Set( const char *varName, const char *value );
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, uint32_t bufsize );


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

