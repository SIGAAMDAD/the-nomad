#ifndef __UI_WINDOW__
#define __UI_WINDOW__

#pragma once

class ImGuiWindow
{
public:
    ImGuiWindow( const char *name );
    ~ImGuiWindow();

    // utility
    void SetDimensions( int w, int h );
    glm::ivec2 GetDimensions( void ) const;

    void SetPosition( int xPos, int yPos );
    glm::ivec2 GetPosition( void ) const;

    void SetFontScale( float scale );
    float GetFontScale( void ) const;

    inline void SameLine( void ) const {
        ImGui::SameLine();
    }
    inline void NewLine( void ) const {
        ImGui::NewLine();
    }

    // input functions
    void TextInput( const char *label, char *buffer, uint64_t size ) const;
    void TextInputWithCompletion( const char *label, mfield_t *buffer ) const;
private:
    const char *name;

    int x, y;
    int width, height;
    float fontScale;
};

#endif