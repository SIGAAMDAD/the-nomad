#include "n_shared.h"
#include "g_game.h"
#include "m_renderer.h"

static void R_GenCoords(const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, glm::vec2 tex[4])
{
    glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.y };
    glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

    tex[0] = { min.x, min.y };
    tex[1] = { max.x, min.y };
    tex[2] = { max.x, max.y };
    tex[3] = { min.x, max.y };
}

SpriteSheet::SpriteSheet(const char *filepath, const glm::vec2& spriteDims, const glm::vec2& sheetDims, uint32_t numSprites)
    : spriteSize(spriteDims), sheetSize(sheetDims), spriteCount(numSprites)
{
    texSheet = R_GetTexture(filepath);
    sprites = (sheet_sprite_t *)Z_Malloc(sizeof(sheet_sprite_t) * numSprites, TAG_STATIC, &sprites, "spriteSpread");

    if (!texSheet)
        N_Error("R_InitSpriteSheet: failed to load texture %s", filepath);
    if (sheetDims.x != sheetDims.y)
        N_Error("R_InitSpriteSheet: sprite sheets must have equal height and width, texchunk: %s", filepath);
}

SpriteSheet::~SpriteSheet()
{
    Z_Free(sprites);
}

void SpriteSheet::AddSprite(uint32_t index, const glm::vec2& coords)
{
    sheet_sprite_t *spr = &sprites[index];

    R_GenCoords(sheetSize, spriteSize, coords, spr->coords);
}

void SpriteSheet::BindSheet(void) const
{
    R_BindTexture(texSheet);
}

void SpriteSheet::UnbindSheet(void) const
{
    R_UnbindTexture();
}

void SpriteSheet::DrawSprite(uint32_t index, vertexCache_t *cache, const glm::vec2& pos, const glm::vec2& size)
{
    sheet_sprite_t* sprite = &sprites[index];

    const glm::vec4 positions[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f), // top right
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f), // bottom right
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), // bottom left
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f), // top left
    };

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x / 64.0f, pos.y / 24.0f, 0.0f))
                    * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
    glm::mat4 mvp = renderer->camera.GetVPM() * model;

    vertex_t vertices[4];
    for (uint32_t i = 0; i < 4; ++i) {
        vertices[i].pos = mvp * positions[i];
        vertices[i].texcoords = sprite->coords[i];
        vertices[i].color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    RGL_SwapVertexData(vertices, arraylen(vertices), cache);
}