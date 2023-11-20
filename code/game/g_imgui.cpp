#include "g_game.h"
#include "../rendercommon/imgui.h"
#include "g_vmimgui.h"

#ifdef __GCC__
#pragma GCC diagnostic ignore "-Wformat"
#endif

typedef struct imguiState_s
{
    uint32_t m_bWindowOpen;
    uint32_t m_bMenuOpen;
    uint32_t m_nColorStack;

    imguiState_s( void )
    {
        memset( this, 0, sizeof(*this) );
    }
} imguiState_t;

static imguiState_t imgui;

bool ImGui_BeginWindow( ImGuiWindow *pWindow )
{
    bool bOpen, bResult;

    if (imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "ImGui_BeginWindow: a window is already active, pop it before adding another one to the frame" );
    }

    imgui.m_bWindowOpen = true;
    bOpen = true;

    bResult = ImGui::Begin( pWindow->m_pTitle, pWindow->m_bClosable ? &bOpen : NULL, pWindow->m_Flags );

    pWindow->m_bOpen = bOpen;

    return bResult;
}

void ImGui_EndWindow( void )
{
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "ImGui_EndWindow: there must be a window active before calling this" );
    }
    imgui.m_bWindowOpen = false;

    ImGui::End();
}

bool ImGui_BeginTable( const char *pLabel, uint32_t nColumns ) {
    return ImGui::BeginTable( pLabel, nColumns );
}

void ImGui_EndTable( void ) {
    ImGui::EndTable();
}

void ImGui_TableNextRow( void ) {
    ImGui::TableNextRow();
}

void ImGui_TableNextColumn( void ) {
    ImGui::TableNextColumn();
}

bool ImGui_BeginMenu( const char *pTitle ) {
    imgui.m_bMenuOpen = ImGui::BeginMenu( pTitle );
    return imgui.m_bMenuOpen;
}

void ImGui_EndMenu( void ) {
    if (!imgui.m_bMenuOpen) {
        N_Error( ERR_DROP, "%s: no menu active", __func__ );
    }

    imgui.m_bMenuOpen = false;
    ImGui::EndMenu();
}

bool ImGui_MenuItem( ImGuiMenuItem *pItem ) {
    if (!imgui.m_bMenuOpen) {
        N_Error( ERR_DROP, "%s: no menu active", __func__ );
    }
    return (pItem->m_bUsed = ImGui::MenuItem( pItem->m_pLabel, pItem->m_pShortcut ));
}

bool ImGui_InputText( ImGuiInputText *pInput )
{
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }
    return (pInput->m_bUsed = ImGui::InputText( pInput->m_pLabel, pInput->m_Data, sizeof(pInput->m_Data), pInput->m_Flags ));
}

bool ImGui_IsWindowCollapsed( void ) {
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    return ImGui::IsWindowCollapsed();
}

void ImGui_SetWindowCollapsed( bool bCollapsed ) {
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowCollapsed( bCollapsed );
}

void ImGui_SetWindowPos( float x, float y ) {
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowPos( ImVec2( x, y ) );
}

void ImGui_SetWindowSize( float w, float h ) {
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowSize( ImVec2( w, h ) );
}

void ImGui_SetWindowFontScale( float scale ) {
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowFontScale( scale );
}

void ImGui_SetItemTooltip( const char *pTooltip )
{
    if (ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled )) {
        ImGui::SetItemTooltip( pTooltip );
    }
}

bool ImGui_InputTextMultiline( ImGuiInputText *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputTextMultiline( pInput->m_pLabel, pInput->m_Data, sizeof(pInput->m_Data), ImVec2( 0, 0 ), pInput->m_Flags ));
}

bool ImGui_InputTextWithHint( ImGuiInputTextWithHint *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputTextWithHint( pInput->m_pLabel, pInput->m_pHint, pInput->m_Data, sizeof(pInput->m_Data), pInput->m_Flags ));
}

bool ImGui_InputFloat( ImGuiInputFloat *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputFloat( pInput->m_pLabel, &pInput->m_Data, 0.0f, 0.0f, "%.3f", pInput->m_Flags ));
}

bool ImGui_InputFloat2( ImGuiInputFloat2 *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputFloat2( pInput->m_pLabel, pInput->m_Data, "%.3f", pInput->m_Flags ));
}

bool ImGui_InputFloat3( ImGuiInputFloat3 *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputFloat3( pInput->m_pLabel, pInput->m_Data, "%.3f", pInput->m_Flags ));
}

bool ImGui_InputFloat4( ImGuiInputFloat4 *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputFloat4( pInput->m_pLabel, pInput->m_Data, "%.3f", pInput->m_Flags ));
}

bool ImGui_InputInt( ImGuiInputInt *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputInt( pInput->m_pLabel, &pInput->m_Data, 1, 100, pInput->m_Flags ));
}

bool ImGui_InputInt2( ImGuiInputInt2 *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputInt2( pInput->m_pLabel, pInput->m_Data, pInput->m_Flags ));
}

bool ImGui_InputInt3( ImGuiInputInt3 *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputInt3( pInput->m_pLabel, pInput->m_Data, pInput->m_Flags ));
}

bool ImGui_InputInt4( ImGuiInputInt4 *pInput ) {
    if (pInput->m_Flags & ImGuiInputTextFlags_CallbackAlways
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCharFilter
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackCompletion
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackEdit
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackHistory
    || pInput->m_Flags & ImGuiInputTextFlags_CallbackResize) {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return (pInput->m_bUsed = ImGui::InputInt4( pInput->m_pLabel, pInput->m_Data, pInput->m_Flags ));
}

bool ImGui_SliderFloat( ImGuiSliderFloat *pSlider ) {
    return ImGui::SliderFloat( pSlider->m_pLabel, &pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderFloat2( ImGuiSliderFloat2 *pSlider ) {
    return ImGui::SliderFloat2( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderFloat3( ImGuiSliderFloat3 *pSlider ) {
    return ImGui::SliderFloat3( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderFloat4( ImGuiSliderFloat4 *pSlider ) {
    return ImGui::SliderFloat4( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderInt( ImGuiSliderInt *pSlider ) {
    return ImGui::SliderInt( pSlider->m_pLabel, &pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderInt2( ImGuiSliderInt2 *pSlider ) {
    return ImGui::SliderInt2( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderInt3( ImGuiSliderInt3 *pSlider ) {
    return ImGui::SliderInt3( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_SliderInt4( ImGuiSliderInt4 *pSlider ) {
    return ImGui::SliderInt4( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

bool ImGui_ColorEdit3( ImGuiColorEdit3 *pEdit ) {
    return ImGui::ColorEdit3( pEdit->m_pLabel, pEdit->m_pColor, pEdit->m_Flags );
}

bool ImGui_ColorEdit4( ImGuiColorEdit4 *pEdit ) {
    return ImGui::ColorEdit4( pEdit->m_pLabel, pEdit->m_pColor, pEdit->m_Flags );
}

bool ImGui_ArrowButton( const char *pLabel, ImGuiDir dir ) {
    return ImGui::ArrowButton( pLabel, dir );
}

bool ImGui_Checkbox( ImGuiCheckbox *pCheckbox ) {
    bool bPressed = pCheckbox->m_bPressed;
    pCheckbox->m_bPressed = ImGui::Checkbox( pCheckbox->m_pLabel, &bPressed );
    return pCheckbox->m_bPressed;
}

bool ImGui_Button( const char *pLabel ) {
    return ImGui::Button( pLabel );
}

float ImGui_GetFontScale( void ) {
    return ImGui::GetFont()->Scale;
}

void ImGui_SetCursorPos( float x, float y ) {
    ImGui::SetCursorPos( ImVec2( x, y ) );
}

void ImGui_GetCursorPos( float *x, float *y ) {
    const ImVec2 pos = ImGui::GetCursorPos();
    *x = pos.x;
    *y = pos.y;
}

void ImGui_SetCursorScreenPos( float x, float y ) {
    ImGui::SetCursorScreenPos( ImVec2( x, y ) );
}

void ImGui_GetCursorScreenPos( float *x, float *y ) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    *x = pos.x;
    *y = pos.y;
}

void ImGui_PushColor( ImGuiCol index, const vec4_t color ) {
    imgui.m_nColorStack++;
    ImGui::PushStyleColor( index, color );
}

void ImGui_PopColor( void ) {
    if (imgui.m_nColorStack == 0) {
        N_Error( ERR_DROP, "%s: color stack underflow", __func__ );
    }
    imgui.m_nColorStack--;
    ImGui::PopStyleColor();
}

void ImGui_SameLine( float offsetFromStartX ) {
    ImGui::SameLine( offsetFromStartX );
}

void ImGui_NewLine( void ) {
    ImGui::NewLine();
}

void ImGui_Text( const char *pText ) {
    ImGui::TextUnformatted( pText );
}

void ImGui_ColoredText( const vec4_t pColor, const char *pText ) {
    ImGui::TextColored( pColor, pText );
}

void ImGui_SeparatorText( const char *pText ) {
    ImGui::SeparatorText( pText );
}

void ImGui_Separator( void ) {
    ImGui::Separator();
}

void ImGui_ProgressBar( float fraction ) {
    ImGui::ProgressBar( fraction );
}

