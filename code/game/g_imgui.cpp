
#include "../rendercommon/imgui.h"
#include "g_game.h"
#include "g_vmimgui.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

typedef struct imguiState_s
{
    uint32_t m_bWindowOpen;
    uint32_t m_bMenuOpen;
    uint32_t m_nColorStack;
    uint32_t m_bPopupOpen;

    imguiState_s( void )
    {
        memset( this, 0, sizeof(*this) );
    }
} imguiState_t;

static imguiState_t imgui;

static bool ImGui_IsValid( void )
{
    if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
        //
        // kill anything currently active
        //

        if ( imgui.m_nColorStack ) {
            ImGui::PopStyleColor( imgui.m_nColorStack );
        }
        if ( imgui.m_bMenuOpen ) {
            ImGui::EndMenu();
        }
        if ( imgui.m_bPopupOpen ) {
            ImGui::EndPopup();
        }
        if ( imgui.m_bWindowOpen ) {
            ImGui::End();
        }

        // clear state
        memset( &imgui, 0, sizeof(imgui) );

        return false;
    }
    return true;
}

void ImGui_OpenPopup( const char *pName ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::OpenPopup( pName );
    imgui.m_bPopupOpen = true;
}

int ImGui_BeginPopupModal( const char *pName, ImGuiWindowFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }
    if ( !imgui.m_bPopupOpen ) {
        N_Error( ERR_DROP, "%s: no popup is open", __func__ );
    }

    return (int)ImGui::BeginPopupModal( pName, NULL, flags );
}

void ImGui_CloseCurrentPopup( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if ( !imgui.m_bPopupOpen ) {
        N_Error( ERR_DROP, "%s: no popup is open", __func__ );
    }
    ImGui::CloseCurrentPopup();
}

void ImGui_EndPopup( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if ( !imgui.m_bPopupOpen ) {
        N_Error( ERR_DROP, "%s: no popup is open", __func__ );
    }
    ImGui::EndPopup();
}

int ImGui_BeginWindow( ImGuiWindow *pWindow )
{
    bool bOpen, bResult;

    if ( !ImGui_IsValid() ) {
        return false;
    }

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
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "ImGui_EndWindow: there must be a window active before calling this" );
    }
    imgui.m_bWindowOpen = false;

    ImGui::End();
}

int ImGui_BeginTable( const char *pLabel, uint32_t nColumns ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::BeginTable( pLabel, nColumns );
}

void ImGui_EndTable( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::EndTable();
}

void ImGui_TableNextRow( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::TableNextRow();
}

void ImGui_TableNextColumn( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::TableNextColumn();
}

int ImGui_BeginMenu( const char *pTitle ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    imgui.m_bMenuOpen = ImGui::BeginMenu( pTitle );
    return imgui.m_bMenuOpen;
}

void ImGui_EndMenu( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (!imgui.m_bMenuOpen) {
        N_Error( ERR_DROP, "%s: no menu active", __func__ );
    }

    imgui.m_bMenuOpen = false;
    ImGui::EndMenu();
}

int ImGui_MenuItem( ImGuiMenuItem *pItem ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    if (!imgui.m_bMenuOpen) {
        N_Error( ERR_DROP, "%s: no menu active", __func__ );
    }
    return (pItem->m_bUsed = ImGui::MenuItem( pItem->m_pLabel, pItem->m_pShortcut ));
}

int ImGui_InputText( ImGuiInputText *pInput )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_IsWindowCollapsed( void ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    return ImGui::IsWindowCollapsed();
}

void ImGui_SetWindowCollapsed( int bCollapsed ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowCollapsed( bCollapsed );
}

void ImGui_SetWindowPos( float x, float y ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowPos( ImVec2( x, y ) );
}

void ImGui_SetWindowSize( float w, float h ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowSize( ImVec2( w, h ) );
}

void ImGui_SetWindowFontScale( float scale ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (!imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "%s: no window active", __func__ );
    }
    ImGui::SetWindowFontScale( scale );
}

void ImGui_SetItemTooltip( const char *pTooltip )
{
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled )) {
        ImGui::SetItemTooltip( pTooltip );
    }
}

int ImGui_InputTextMultiline( ImGuiInputText *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputTextWithHint( ImGuiInputTextWithHint *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputFloat( ImGuiInputFloat *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputFloat2( ImGuiInputFloat2 *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputFloat3( ImGuiInputFloat3 *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputFloat4( ImGuiInputFloat4 *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputInt( ImGuiInputInt *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputInt2( ImGuiInputInt2 *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputInt3( ImGuiInputInt3 *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_InputInt4( ImGuiInputInt4 *pInput ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
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

int ImGui_SliderFloat( ImGuiSliderFloat *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderFloat( pSlider->m_pLabel, &pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderFloat2( ImGuiSliderFloat2 *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderFloat2( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderFloat3( ImGuiSliderFloat3 *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderFloat3( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderFloat4( ImGuiSliderFloat4 *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderFloat4( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderInt( ImGuiSliderInt *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderInt( pSlider->m_pLabel, &pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderInt2( ImGuiSliderInt2 *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderInt2( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderInt3( ImGuiSliderInt3 *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderInt3( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_SliderInt4( ImGuiSliderInt4 *pSlider ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::SliderInt4( pSlider->m_pLabel, pSlider->m_Data, pSlider->m_nMin, pSlider->m_nMax );
}

int ImGui_ColorEdit3( ImGuiColorEdit3 *pEdit ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::ColorEdit3( pEdit->m_pLabel, pEdit->m_pColor, pEdit->m_Flags );
}

int ImGui_ColorEdit4( ImGuiColorEdit4 *pEdit ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::ColorEdit4( pEdit->m_pLabel, pEdit->m_pColor, pEdit->m_Flags );
}

int ImGui_ArrowButton( const char *pLabel, ImGuiDir dir ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::ArrowButton( pLabel, dir );
}

int ImGui_Checkbox( ImGuiCheckbox *pCheckbox ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }

    bool bPressed = pCheckbox->m_bPressed;
    pCheckbox->m_bPressed = ImGui::Checkbox( pCheckbox->m_pLabel, &bPressed );
    return pCheckbox->m_bPressed;
}

int ImGui_Button( const char *pLabel ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::Button( pLabel );
}

float ImGui_GetFontScale( void ) {
    if ( !ImGui_IsValid() ) {
        return 0;
    }
    return ImGui::GetFont()->Scale;
}

void ImGui_SetCursorPos( float x, float y ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::SetCursorPos( ImVec2( x, y ) );
}

void ImGui_GetCursorPos( float *x, float *y ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    const ImVec2 pos = ImGui::GetCursorPos();
    *x = pos.x;
    *y = pos.y;
}

void ImGui_SetCursorScreenPos( float x, float y ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::SetCursorScreenPos( ImVec2( x, y ) );
}

void ImGui_GetCursorScreenPos( float *x, float *y ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    *x = pos.x;
    *y = pos.y;
}

void ImGui_PushColor( ImGuiCol index, const vec4_t color ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    imgui.m_nColorStack++;
    ImGui::PushStyleColor( index, color );
}

void ImGui_PopColor( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    if (imgui.m_nColorStack == 0) {
        N_Error( ERR_DROP, "%s: color stack underflow", __func__ );
    }
    imgui.m_nColorStack--;
    ImGui::PopStyleColor();
}

void ImGui_SameLine( float offsetFromStartX ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::SameLine( offsetFromStartX );
}

void ImGui_NewLine( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::NewLine();
}

void ImGui_Text( const char *pText ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::TextUnformatted( pText );
}

void ImGui_ColoredText( const vec4_t pColor, const char *pText ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::TextColored( pColor, pText );
}

void ImGui_SeparatorText( const char *pText ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::SeparatorText( pText );
}

void ImGui_Separator( void ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::Separator();
}

void ImGui_ProgressBar( float fraction ) {
    if ( !ImGui_IsValid() ) {
        return;
    }
    ImGui::ProgressBar( fraction );
}

