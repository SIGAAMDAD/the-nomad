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
} sheet_sprite_t;

class SpriteSheet
{
private:
    sheet_sprite_t *sprites;
    glm::vec2 spriteSize;
    glm::vec2 sheetSize;
    uint32_t spriteCount;

    const texture_t *texSheet;
public:
    SpriteSheet(const char *filepath, const glm::vec2& spriteDims, const glm::vec2& sheetDims, uint32_t numSprites);
    ~SpriteSheet();

    void DrawSprite(sprite_t index, vertexCache_t *cache, const glm::vec2& pos, const glm::vec2& size);
    void AddSprite(sprite_t index, const glm::vec2& coords);

    void BindSheet(void) const;
    void UnbindSheet(void) const;
    GDR_INLINE const texture_t* GetTexture(void) const { return texSheet; }
    GDR_INLINE const glm::vec2* GetSpriteCoords(sprite_t index) const
    {
        static glm::vec2 coords[4];
        for (uint32_t i = 0; i < 4; ++i)
            coords[i] = glm::vec2(sprites[index].coords[i][0], sprites[index].coords[i][1]);
        return coords;
    }
};

#endif