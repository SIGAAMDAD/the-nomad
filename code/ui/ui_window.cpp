// ui_window.cpp -- simple interface/abstraction layer over imgui

#include "../game/g_game.h"
#include "../rendercommon/imgui.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"

CUIWindow::CUIWindow( const char *name )
{
    // push the window
    ImGui::Begin( name, NULL,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar );
    
    // setup basic stuff
    SetPosition( 0, 0 );
    SetDimensions( ui->GetConfig().vidWidth, ui->GetConfig().vidHeight );
}

CUIWindow::~CUIWindow()
{
    // pop the window
    ImGui::End();
}

void CUIWindow::SetDimensions( int w, int h ) {
    width = w;
    height = h;
    ImGui::SetWindowSize({ (float)width, (float)height });
}

void CUIWindow::SetPosition( int xPos, int yPos ) {
    x = xPos;
    y = yPos;
    ImGui::SetWindowPos({ (float)x, (float)y });
}

void CUIWindow::SetFontScale( float scale ) {
    fontScale = scale;
    ImGui::SetWindowFontScale( scale );
}

glm::ivec2 CUIWindow::GetPosition( void ) const {
    return glm::ivec2( x, y );
}

glm::ivec2 CUIWindow::GetDimensions( void ) const {
    return glm::ivec2( width, height );
}

float CUIWindow::GetFontScale( void ) const {
    return fontScale;
}

void CUIWindow::SameLine( void ) const {
    ImGui::SameLine();
}

void CUIWindow::NewLine( void ) const {
    ImGui::NewLine();
}

int CUIWindow::DrawMenuList( const menuList_t *list ) const
{
    int index;

    index = -1;
    if (ImGui::BeginMenu( list->menuName )) {
        for (uint64_t i = 0; i < list->nitems; i++) {
            if (ImGui::MenuItem( list->items[i].name )) {
                if (ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled )) {
                    ImGui::SetItemTooltip( "%s", list->items[i].hint );
                }
                index = i;
            }
        }
        ImGui::EndMenu();
    }
    return index;
}

static int ImGuiInputTextCallback_Basic( ImGuiInputTextCallbackData *data )
{
    if (Key_IsDown(KEY_LCTRL) && Key_IsDown(KEY_A)) {
        data->SelectAll();
    }
    if (Key_IsDown(KEY_DELETE)) {
        if (data->HasSelection()) {
            // delete chars inside selection
            data->DeleteChars(data->SelectionStart, data->SelectionEnd - data->SelectionStart);
        }
    }
    if (Key_IsDown(KEY_DOWN)) {
        if (data->HasSelection()) {
            // if we have any selection, clear it
            data->ClearSelection();
        }
        data->CursorPos = strlen( data->Buf );
    }

    return 1;
}

static int ImGuiInputTextCallback_Completion( ImGuiInputTextCallbackData *data )
{
    mfield_t *field = (mfield_t *)data->UserData;

    if (Key_IsDown(KEY_LCTRL) && Key_IsDown(KEY_A)) {
        data->SelectAll();
    }
    if (Key_IsDown(KEY_DELETE)) {
        if (data->HasSelection()) {
            // delete chars inside selection
            data->DeleteChars(data->SelectionStart, data->SelectionEnd - data->SelectionStart);
        }
    }
}

void CUIWindow::TextInput( const char *label, char *buffer, uint64_t size ) const {
    ImGui::InputText( label, buffer, size, ImGuiInputTextFlags_CallbackEdit, ImGuiInputTextCallback_Basic );
}

void CUIWindow::TextInputWithCompletion( const char *label, mfield_t *buffer ) const {
    ImGui::InputText( label, buffer->buffer, buffer->widthInChars, ImGuiInputTextFlags_CallbackAlways, ImGuiInputTextCallback_Completion, buffer );
}

void CUIWindow::DrawStringCentered( const char *str ) const
{
    uint64_t length;
    float font_size;

    length = strlen(str);
    font_size = ImGui::GetFontSize() * length / 2;
    ImGui::SameLine( ImGui::GetWindowSize().x / 2.0f - font_size + (font_size / 2.15) );
    ImGui::TextUnformatted( str, str + length );
}

//
// CUIWindow::DrawString: draws a string that should be given
// through the string manager, this will handle all the fancy
// formatting
//
void CUIWindow::DrawString( const char *str ) const
{
    const char *s;
    char buffer[MAXPRINTMSG], *bufp;

    // if it's got multiple lines, we need to
    // print with a drop-down
    if (strstr(str, "\\n") != NULL) {
    }

    bufp = buffer;
    for (s = str; *s; s++) {
    }
}
