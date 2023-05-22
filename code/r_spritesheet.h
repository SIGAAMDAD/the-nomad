#ifndef _R_SPRITESHEET_
#define _R_SPRITESHEET_

#pragma once

typedef struct sheet_sprite_s
{
    float width;
    float height;
    float texXIndex;
    float texYIndex;
    vec2_t coords[4];
    vec2_t vertices[4];
    uint32_t indices[6];
} sheet_sprite_t;

class SpriteSheet
{
private:
    nomadvector<sheet_sprite_t> sprites;
    float sheet_height;
    float sheet_width;

    const Texture2D* sheet_texture;

    VertexArray* vao;
    VertexBuffer* vbo;
    IndexBuffer* ibo;
    Shader* shader;
public:
    SpriteSheet(const eastl::string& filepath, float spriteWidth, float spriteHeight, float sheetWidth, float sheetHeight, uint32_t numSprites);
    ~SpriteSheet();

    void DrawSprite(uint32_t index, const glm::mat4& mvp, const glm::vec2& size);

    inline const Texture2D* GetTexture(void) const { return sheet_texture; }
};

#endif