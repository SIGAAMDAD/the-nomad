#include "n_shared.h"
#include "g_game.h"
#include "m_renderer.h"

static void R_GenCoords(const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, vec2_t tex[4])
{
#if 0
    const glm::vec2 texcoords[4] = {
        {(coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.x) / sheetDims.x},
        {((coords.x + 1) * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.x) / sheetDims.x},
        {((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.x) / sheetDims.x},
        {(coords.x * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.x) / sheetDims.x}
    };
#else
    glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.y };
    glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

    VectorCopy(tex[0], min);
    VectorCopy(tex[2], max);

    tex[1][0] = max.x;
    tex[1][1] = min.y;

    tex[3][0] = min.x;
    tex[3][1] = max.y;

    Con_Printf("Texture Coords:");
    for (uint32_t y = 0; y < 4; y++) {
        for (uint32_t x = 0; x < 2; x++) {
            Con_Printf("tex[%i][%i]: %f", y, x, tex[y][x]);
        }
    }
#endif
}

SpriteSheet::SpriteSheet(const char *filepath, const glm::vec2& spriteDims, const glm::vec2& sheetDims, uint32_t numSprites)
    : spriteSize(spriteDims), sheetSize(sheetDims), spriteCount(numSprites)
{
    texSheet = R_GetTexture(filepath);
    sprites = (sheet_sprite_t *)Z_Malloc(sizeof(sheet_sprite_t) * numSprites, TAG_STATIC, &sprites, "spriteSpread");
    if (!texSheet)
        N_Error("R_InitSpriteSheet: failed to load texture %s", filepath);
}

SpriteSheet::~SpriteSheet()
{
    Z_Free(sprites);
}

void SpriteSheet::AddSprite(sprite_t index, const glm::vec2& coords)
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

void SpriteSheet::DrawSprite(sprite_t index, vertexCache_t *cache, const glm::vec2& pos, const glm::vec2& size)
{
    sheet_sprite_t* sprite = &sprites[index];

    const glm::vec4 positions[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f), // top right
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f), // bottom right
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), // bottom left
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f), // top left
    };

//    if (!R_CullVertex(pos))
//        return; // don't render it

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x / 64.0f, pos.y / 24.0f, 0.0f))
                    * glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 0.0f));
    glm::mat4 mvp = renderer->camera.GetVPM() * model;

//    glm::mat4 projection = glm::perspective((float)(M_PI/1.5f), (float)1024/(float)720, -1.0f, 1.0f);
//    glm::mat4 view = glm::lookAt(
//        glm::vec3(renderer->camera.GetPos().x, renderer->camera.GetPos().y, 2.0f),
//        glm::vec3(renderer->camera.GetPos().x, renderer->camera.GetPos().y, 1.0f),
//        glm::vec3(0, 1, 0)
//    );
//    glm::mat4 viewproj = view * projection;
//    glm::mat4 model =
//        glm::rotate(glm::mat4(), renderer->camera.GetRotation(), glm::vec3(0.0f, 0.0f, 0.0f)) *
//        glm::translate(glm::mat4(), glm::vec3(pos.x, pos.y, 0.0f));
//    
//    glm::mat4 mvp = viewproj * model;

//    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
//    glm::mat4 mvp = renderer->camera.GetProjection() * renderer->camera.GetViewMatrix() * model;

    vertex_t vertices[4];
    for (uint32_t i = 0; i < 4; ++i) {
        vertices[i].pos = mvp * positions[i];
        vertices[i].texcoords = { sprite->coords[i][0], sprite->coords[i][1] };
        vertices[i].color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    RGL_SwapVertexData(vertices, arraylen(vertices), cache);
}