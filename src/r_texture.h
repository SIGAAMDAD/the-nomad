#ifndef _R_TEXTURE_
#define _R_TEXTURE_

#pragma once

struct Texture2DSetup
{
    GLenum magFilter;
    GLenum minFilter;
    GLenum wrapS;
    GLenum wrapT;
    bool genMipmap;

    inline Texture2DSetup(GLenum _magFilter, GLenum _minFilter, GLenum _wrapS, GLenum _wrapT, bool _genMipmap)
        : magFilter(_magFilter), minFilter(_minFilter), wrapS(_wrapS), wrapT(_wrapT), genMipmap(_genMipmap)
    {
    }
};

const inline Texture2DSetup DEFAULT_TEXTURE_SETUP = Texture2DSetup(GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true);

class Texture2D
{
private:
    GLuint id;

    byte* buffer;
    int width;
    int height;
    int n;
public:
    Texture2D(const Texture2DSetup& _setup, const eastl::string& filepath);
    ~Texture2D();

    inline void Bind(uint32_t slot = 0) const
    {
        glActiveTexture(GL_TEXTURE0+slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }
    inline void Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    inline byte* GetData() { return buffer; }
    inline int GetWidth() const { return width; }
    inline int GetHeight() const { return height; }
    inline GLuint GetID() const { return id; }
    
    static Texture2D* Create(const Texture2DSetup& setup, const eastl::string& filepath, const eastl::string& name);
};

#endif