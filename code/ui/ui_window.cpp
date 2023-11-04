// ui_window.cpp -- simple interface/abstraction layer over imgui

#include "../game/g_game.h"
#include "../rendercommon/imgui.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"

ImGuiWindow::ImGuiWindow( const char *name )
{
    // push the window
    ImGui::Begin( name, NULL,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGUiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoTitleBar );
    
    // setup basic stuff
    SetPosition( 0, 0 );
    SetDimensions( ui->GetConfig().vidWidth, ui->GetConfig().vidHeight );
}

ImGuiWindow::~ImGuiWindow()
{
    // pop the window
    ImGui::End();
}

void ImGuiWindow::SetDimensions( int w, int h ) {
    width = w;
    height = h;
    ImGui::SetWindowSize({ (float)width, (float)height });
}

void ImGuiWindow::SetPosition( int xPos, int yPos ) {
    x = xPos;
    y = yPos;
    ImGui::SetWindowPos({ (float)x, (float)y });
}

void ImGuiWindow::SetFontScale( float scale ) {
    fontScale = scale;
    ImGui::SetWindowFontScale( scale );
}

glm::ivec2 ImGuiWindow::GetPosition( void ) const {
    return glm::ivec2( x, y );
}

glm::ivec2 ImGuiWindow::GetDimensions( void ) const {
    return glm::ivec2( w, h );
}

float ImGuiWindow::GetFontScale( void ) const {
    return fontScale;
}

static int ImGuiInputTextCallback_Basic( ImGuiInputTextCallbackData *data )
{
    if (Key_IsDown(KEY_LCTRL) && Key_IsDown(KEY_A)) {
        data->SelectAll();
    }
    if (Key_IsDown(KEY_DELETE)) {
        if (data->HasSelection()) {
            // delete chars inside selection
            data->DeleteChars(data->SelectionStart(), data->SelectionEnd() - data->SelectionStart());
        }
    }
    if (Key_IsDown(KEY_DOWN)) {
        if (data->HasSelection()) {
            // if we have any selection, clear it
            data->ClearSelection();
        }
        data->CursorPos = strlen( data->Buf );
    }
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
            data->DeleteChars(data->SelectionStart(), data->SelectionEnd() - data->SelectionStart());
        }
    }
    if (Key_IsDown(KEY_DOWN)) {
        if (data->HasSelection()) {
            // if we have any selection, clear it
            data->ClearSelection();
            data->CursorPos = strlen( data->Buf );

            field->cursor = data->CursorPos;
        }
        if (Con_HistoryGetNext(field)) {
            data->CursorPos = field->cursor;
        }
    }
    if (Key_IsDown(KEY_UP)) {
        if (Con_HistoryGetPrev(field)) {
            data->CursorPos = field->cursor;
        }
    }
}

void ImGuiWindow::TextInput( const char *label, char *buffer, uint64_t size ) const {
    ImGui::InputText( label, buffer, size, ImGuiInputTextFlags_CallbackEdit, ImGuiInputTextCallback_Basic );
}

void ImGuiWindow::TextInputWithCompletion( const char *label, mfield_t *buffer ) const {
    ImGui::InputText( label, buffer->buffer, buffer->widthInChars, ImGuiInputTextFlags_CallbackAlways, ImGuiInputTextCallback_Completion, buffer );
}
