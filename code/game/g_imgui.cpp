
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

int ImGui_BeginWindow( const char *pLabel, byte *pOpen, ImGuiWindowFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    if (imgui.m_bWindowOpen) {
        N_Error( ERR_DROP, "ImGui_BeginWindow: a window is already active, pop it before adding another one to the frame" );
    }

    imgui.m_bWindowOpen = true;

    return ImGui::Begin( pLabel, (bool *)pOpen, flags );
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

int ImGui_MenuItem( const char *pLabel, const char *pShortcut, byte bUsed ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    if (!imgui.m_bMenuOpen) {
        N_Error( ERR_DROP, "%s: no menu active", __func__ );
    }
    return ImGui::MenuItem( pLabel, pShortcut, (bool *)NULL, (bool)bUsed );
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

int ImGui_InputText( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }
    else if ( flags & ImGuiInputTextFlags_CallbackAlways
        || flags & ImGuiInputTextFlags_CallbackCharFilter
        || flags & ImGuiInputTextFlags_CallbackCompletion
        || flags & ImGuiInputTextFlags_CallbackEdit
        || flags & ImGuiInputTextFlags_CallbackHistory
        || flags & ImGuiInputTextFlags_CallbackResize )
    {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return ImGui::InputText( pLabel, pBuffer, nBufSize, flags );
}

int ImGui_InputTextMultiline( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }
    else if ( flags & ImGuiInputTextFlags_CallbackAlways
        || flags & ImGuiInputTextFlags_CallbackCharFilter
        || flags & ImGuiInputTextFlags_CallbackCompletion
        || flags & ImGuiInputTextFlags_CallbackEdit
        || flags & ImGuiInputTextFlags_CallbackHistory
        || flags & ImGuiInputTextFlags_CallbackResize )
    {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return ImGui::InputTextMultiline( pLabel, pBuffer, nBufSize, ImVec2( 0, 0 ), flags );
}

int ImGui_InputTextWithHint( const char *pLabel, const char *pHint, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }
    else if ( flags & ImGuiInputTextFlags_CallbackAlways
        || flags & ImGuiInputTextFlags_CallbackCharFilter
        || flags & ImGuiInputTextFlags_CallbackCompletion
        || flags & ImGuiInputTextFlags_CallbackEdit
        || flags & ImGuiInputTextFlags_CallbackHistory
        || flags & ImGuiInputTextFlags_CallbackResize )
    {
        N_Error( ERR_DROP, "%s: callbacks aren't allowed for vm modules", __func__ );
    }

    return ImGui::InputTextWithHint( pLabel, pHint, pBuffer, nBufSize, flags );
}

int ImGui_InputFloat( const char *pLabel, float *pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputFloat( pLabel, pData );
}

int ImGui_InputFloat2( const char *pLabel, vec2_t pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputFloat2( pLabel, pData );
}

int ImGui_InputFloat3( const char *pLabel, vec3_t pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputFloat3( pLabel, pData );
}

int ImGui_InputFloat4( const char *pLabel, vec4_t pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputFloat4( pLabel, pData );
}

int ImGui_InputInt( const char *pLabel, int32_t *pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputInt( pLabel, pData );
}

int ImGui_InputInt2( const char *pLabel, ivec2_t pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputInt2( pLabel, pData );
}

int ImGui_InputInt3( const char *pLabel, ivec3_t pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputInt3( pLabel, pData );
}

int ImGui_InputInt4( const char *pLabel, ivec4_t pData )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::InputInt4( pLabel, pData );
}

int ImGui_SliderFloat( const char *pLabel, float *pData, float nMax, float nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderFloat( pLabel, pData, nMin, nMax );
}

int ImGui_SliderFloat2( const char *pLabel, vec2_t pData, float nMax, float nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderFloat2( pLabel, pData, nMin, nMax );
}

int ImGui_SliderFloat3( const char *pLabel, vec3_t pData, float nMax, float nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderFloat3( pLabel, pData, nMin, nMax );
}

int ImGui_SliderFloat4( const char *pLabel, vec4_t pData, float nMax, float nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderFloat4( pLabel, pData, nMin, nMax );
}

int ImGui_SliderInt( const char *pLabel, int32_t *pData, int32_t nMax, int32_t nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderInt( pLabel, pData, nMin, nMax );
}

int ImGui_SliderInt2( const char *pLabel, ivec2_t pData, int32_t nMax, int32_t nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderInt2( pLabel, pData, nMin, nMax );
}

int ImGui_SliderInt3( const char *pLabel, ivec3_t pData, int32_t nMax, int32_t nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderInt3( pLabel, pData, nMin, nMax );
}

int ImGui_SliderInt4( const char *pLabel, ivec4_t pData, int32_t nMax, int32_t nMin )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::SliderInt4( pLabel, pData, nMin, nMax );
}

int ImGui_ColorEdit3( const char *pLabel, vec3_t pColor, ImGuiColorEditFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::ColorEdit3( pLabel, pColor, flags );
}

int ImGui_ColorEdit4( const char *pLabel, vec4_t pColor, ImGuiColorEditFlags flags )
{
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::ColorEdit4( pLabel, pColor, flags );
}

int ImGui_ArrowButton( const char *pLabel, ImGuiDir dir ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }
    return ImGui::ArrowButton( pLabel, dir );
}

int ImGui_Checkbox( const char *pLabel, byte *pPressed ) {
    if ( !ImGui_IsValid() ) {
        return false;
    }

    return ImGui::Checkbox( pLabel, (bool *)pPressed );
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

