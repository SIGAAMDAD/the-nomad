#ifndef __G_VMIMGUI__
#define __G_VMIMGUI__

#pragma once

int ImGui_BeginWindow( const char *pLabel, byte *pOpen, ImGuiWindowFlags windowFlags );
void ImGui_EndWindow( void );
int ImGui_IsWindowCollapsed( void );
void ImGui_SetWindowCollapsed( int bCollapsed );
void ImGui_SetWindowPos( float x, float y );
void ImGui_SetWindowSize( float w, float h );
void ImGui_SetWindowFontScale( float scale );

int ImGui_BeginMenu( const char *pLabel );
void ImGui_EndMenu( void );
int ImGui_MenuItem( const char *pLabel, const char *pShortcut, byte bUsed );
void ImGui_SetItemTooltip( const char *pTooltip );

int ImGui_BeginTable( const char *pLabel, uint32_t nColumns );
void ImGui_TableNextRow( void );
void ImGui_TableNextColumn( void );
void ImGui_EndTable( void );
int ImGui_InputText( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
int ImGui_InputTextMultiline( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
int ImGui_InputTextWithHint( const char *pLabel, const char *pHint, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
int ImGui_InputFloat( const char *pLabel, float *pData );
int ImGui_InputFloat2( const char *pLabel, vec2_t pData );
int ImGui_InputFloat3( const char *pLabel, vec3_t pData );
int ImGui_InputFloat4( const char *pLabel, vec4_t pData );
int ImGui_InputInt( const char *pLabel, int32_t *pData );
int ImGui_InputInt2( const char *pLabel, ivec2_t pData );
int ImGui_InputInt3( const char *pLabel, ivec3_t pData );
int ImGui_InputInt4( const char *pLabel, ivec4_t pData );
int ImGui_SliderFloat( const char *pLabel, float *pData, float nMax, float nMin );
int ImGui_SliderFloat2( const char *pLabel, vec2_t pData, float nMax, float nMin );
int ImGui_SliderFloat3( const char *pLabel, vec3_t pData, float nMax, float nMin );
int ImGui_SliderFloat4( const char *pLabel, vec4_t pData, float nMax, float nMin );
int ImGui_SliderInt( const char *pLabel, int32_t *pData, int32_t nMax, int32_t nMin );
int ImGui_SliderInt2( const char *pLabel, ivec2_t pData, int32_t nMax, int32_t nMin );
int ImGui_SliderInt3( const char *pLabel, ivec3_t pData, int32_t nMax, int32_t nMin );
int ImGui_SliderInt4( const char *pLabel, ivec4_t pData, int32_t nMax, int32_t nMin );
int ImGui_ColorEdit3( const char *pLabel, vec3_t pColor, ImGuiColorEditFlags flags );
int ImGui_ColorEdit4( const char *pLabel, vec4_t pColor, ImGuiColorEditFlags flags );

int ImGui_ArrowButton( const char *pLabel, ImGuiDir dir );
int ImGui_Checkbox( const char *pLabel, byte *bPressed );
int ImGui_Button( const char *pLabel );

float ImGui_GetFontScale( void );
void ImGui_SetCursorPos( float x, float y );
void ImGui_GetCursorPos( float *x, float *y );
void ImGui_SetCursorScreenPos( float x, float y );
void ImGui_GetCursorScreenPos( float *x, float *y );
void ImGui_PushColor( ImGuiCol index, const vec4_t color );
void ImGui_PopColor( void );

void ImGui_SameLine( float offsetFromStartX );
void ImGui_NewLine( void );
void ImGui_Text( const char *pText );
void ImGui_ColoredText( const vec4_t pColor, const char *pText );
void ImGui_SeparatorText( const char *pText );
void ImGui_Separator( void );
void ImGui_ProgressBar( float fraction );

void ImGui_OpenPopup( const char *pName );
int ImGui_BeginPopupModal( const char *pName, ImGuiWindowFlags flags );
void ImGui_CloseCurrentPopup( void );
void ImGui_EndPopup( void );

#endif