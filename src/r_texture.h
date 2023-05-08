#ifndef _R_TEXTURE_
#define _R_TEXTURE_

#pragma once

class Texture2D
{
private:
    GLuint id;

    byte* buffer;
    int width;
    int height;
    int n;

    bool multisampled;
public:
    Texture2D(const eastl::string& filepath, bool _multisample = false);
    ~Texture2D();

    inline void Bind(uint32_t slot = 0) const {
        glBindTexture((multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), id);
    }
    inline void Unbind() const {
        glBindTexture((multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), 0);
    }
    inline byte* GetData() {
        return buffer;
    }
    inline int GetWidth() const { return width; }
    inline int GetHeight() const { return height; }
    inline GLuint GetID() const { return id; }
    inline bool IsMultisampled() const { return multisampled; }
    
    static Texture2D* Create(const eastl::string& filepath, const eastl::string& name, bool multisample = false);
};

#endif