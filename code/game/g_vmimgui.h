#ifndef __G_VMIMGUI__
#define __G_VMIMGUI__

#pragma once

typedef struct ImGuiMenuItem
{
    const char *m_pLabel;
    const char *m_pShortcut;
    uint32_t m_bUsed;
} ImGuiMenuItem;

typedef struct ImGuiInputText
{
    char m_Data[MAX_EDIT_LINE];
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputText;

typedef struct ImGuiInputTextWithHint
{
    char m_Data[MAX_EDIT_LINE];
    const char *m_pLabel;
    const char *m_pHint;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputTextWithHint;

typedef struct ImGuiInputFloat
{
    float m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputFloat;

typedef struct ImGuiInputFloat2
{
    vec2_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputFloat2;

typedef struct ImGuiInputFloat3
{
    vec3_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputFloat3;

typedef struct ImGuiInputFloat4
{
    vec4_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputFloat4;

typedef struct ImGuiInputInt
{
    int32_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputInt;

typedef struct ImGuiInputInt2
{
    ivec2_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputInt2;

typedef struct ImGuiInputInt3
{
    ivec3_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputInt3;

typedef struct ImGuiInputInt4
{
    ivec4_t m_Data;
    const char *m_pLabel;
    ImGuiInputTextFlags m_Flags;
    uint32_t m_bUsed;
} ImGuiInputInt4;

typedef struct ImGuiCheckbox
{
    const char *m_pLabel;
    uint32_t m_bPressed;
} ImGuiCheckbox;

typedef struct ImGuiRadioButton
{
    const char *m_pLabel;
    uint32_t m_bIsPressed;
} ImGuiRadioButton;

typedef struct ImGuiSliderInt
{
    const char *m_pLabel;
    int32_t m_Data;
    int32_t m_nMax;
    int32_t m_nMin;
} ImGuiSliderInt;

typedef struct ImGuiSliderInt2
{
    const char *m_pLabel;
    ivec2_t m_Data;
    int32_t m_nMax;
    int32_t m_nMin;
} ImGuiSliderInt2;

typedef struct ImGuiSliderInt3
{
    const char *m_pLabel;
    ivec3_t m_Data;
    int32_t m_nMax;
    int32_t m_nMin;
} ImGuiSliderInt3;

typedef struct ImGuiSliderInt4
{
    const char *m_pLabel;
    ivec4_t m_Data;
    int32_t m_nMax;
    int32_t m_nMin;
} ImGuiSliderInt4;

typedef struct ImGuiSliderFloat
{
    const char *m_pLabel;
    float m_Data;
    float m_nMax;
    float m_nMin;
} ImGuiSliderFloat;

typedef struct ImGuiSliderFloat2
{
    const char *m_pLabel;
    vec2_t m_Data;
    float m_nMax;
    float m_nMin;
} ImGuiSliderFloat2;

typedef struct ImGuiSliderFloat3
{
    const char *m_pLabel;
    vec3_t m_Data;
    float m_nMax;
    float m_nMin;
} ImGuiSliderFloat3;

typedef struct ImGuiSliderFloat4
{
    const char *m_pLabel;
    vec4_t m_Data;
    float m_nMax;
    float m_nMin;
} ImGuiSliderFloat4;

typedef struct ImGuiColorEdit3
{
    const char *m_pLabel;
    vec3_t m_pColor;
    ImGuiColorEditFlags m_Flags;
} ImGuiColorEdit3;

typedef struct ImGuiColorEdit4
{
    const char *m_pLabel;
    vec4_t m_pColor;
    ImGuiColorEditFlags m_Flags;
} ImGuiColorEdit4;

typedef struct ImGuiWindow
{
    const char *m_pTitle;
    uint32_t m_bClosable;
    uint32_t m_bOpen;
    ImGuiWindowFlags m_Flags;
} ImGuiWindow;

int ImGui_BeginWindow( ImGuiWindow *pWindow );
void ImGui_EndWindow( void );
int ImGui_IsWindowCollapsed( void );
void ImGui_SetWindowCollapsed( int bCollapsed );
void ImGui_SetWindowPos( float x, float y );
void ImGui_SetWindowSize( float w, float h );
void ImGui_SetWindowFontScale( float scale );

int ImGui_BeginMenu( const char *pLabel );
void ImGui_EndMenu( void );
int ImGui_MenuItem( ImGuiMenuItem *pItem );
void ImGui_SetItemTooltip( const char *pTooltip );

int ImGui_BeginTable( const char *pLabel, uint32_t nColumns );
void ImGui_TableNextRow( void );
void ImGui_TableNextColumn( void );
void ImGui_EndTable( void );

int ImGui_InputText( ImGuiInputText *pInput );
int ImGui_InputTextMultiline( ImGuiInputText *pInput );
int ImGui_InputTextWithHint( ImGuiInputTextWithHint *pInput );
int ImGui_InputFloat( ImGuiInputFloat *pInput );
int ImGui_InputFloat2( ImGuiInputFloat2 *pInput );
int ImGui_InputFloat3( ImGuiInputFloat3 *pInput );
int ImGui_InputFloat4( ImGuiInputFloat4 *pInput );
int ImGui_InputInt( ImGuiInputInt *pInput );
int ImGui_InputInt2( ImGuiInputInt2 *pInput );
int ImGui_InputInt3( ImGuiInputInt3 *pInput );
int ImGui_InputInt4( ImGuiInputInt4 *pInput );
int ImGui_SliderFloat( ImGuiSliderFloat *pSlider );
int ImGui_SliderFloat2( ImGuiSliderFloat2 *pSlider );
int ImGui_SliderFloat3( ImGuiSliderFloat3 *pSlider );
int ImGui_SliderFloat4( ImGuiSliderFloat4 *pSlider );
int ImGui_SliderInt( ImGuiSliderInt *pSlider );
int ImGui_SliderInt2( ImGuiSliderInt2 *pSlider );
int ImGui_SliderInt3( ImGuiSliderInt3 *pSlider );
int ImGui_SliderInt4( ImGuiSliderInt4 *pSlider );
int ImGui_ColorEdit3( ImGuiColorEdit3 *pEdit );
int ImGui_ColorEdit4( ImGuiColorEdit4 *pEdit );

int ImGui_ArrowButton( const char *pLabel, ImGuiDir dir );
int ImGui_Checkbox( ImGuiCheckbox *pCheckbox );
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

#endif