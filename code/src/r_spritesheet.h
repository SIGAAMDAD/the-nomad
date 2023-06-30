#ifndef _R_SPRITESHEET_
#define _R_SPRITESHEET_

#pragma once

typedef struct sheet_sprite_s
{
    glm::vec2 coords[4];
    glm::vec2 texIndex;
    uint32_t index;
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

    void DrawSprite(uint32_t index, vertexCache_t *cache, const glm::vec2& pos, const glm::vec2& size);
    void AddSprite(uint32_t index, const glm::vec2& coords);

    void BindSheet(void) const;
    void UnbindSheet(void) const;
    GDR_INLINE const texture_t* GetTexture(void) const { return texSheet; }
    GDR_INLINE const glm::vec2* GetSpriteCoords(uint32_t index) const { return sprites[index].coords; }
};

#endif