#include "n_shared.h"
#include "g_game.h"

SpriteSheet::SpriteSheet(const eastl::string& filepath, float spriteWidth, float spriteHeight, float sheetWidth, float sheetHeight, uint32_t numSprites)
{
    float current_x_index = 0;
    float current_y_index = 0;

    sprites.reserve(numSprites);
    for (uint32_t i = 0; i < numSprites; i++) {
        sheet_sprite_t sprite;

        // not pretty, but it'll do
        sprite.coords[0][0] = (current_x_index * spriteWidth) / sheetWidth;
        sprite.coords[0][1] = (current_y_index * spriteHeight) / sheetHeight;
        sprite.coords[1][0] = ((current_x_index + 1) * spriteWidth) / sheetWidth;
        sprite.coords[1][1] = (current_y_index * spriteHeight) / sheetHeight;
        sprite.coords[2][0] = ((current_x_index + 1) * spriteWidth) / sheetWidth;
        sprite.coords[2][1] = ((current_y_index + 1) * spriteHeight) / sheetHeight;
        sprite.coords[3][0] = (current_x_index * spriteWidth) / sheetWidth;
        sprite.coords[3][1] = ((current_y_index + 1) * spriteHeight) / sheetHeight;

        sprite.vertices[0][0] = 0.5f;
        sprite.vertices[0][1] = 0.5f;
        sprite.vertices[0][2] = 0.0f;
        sprite.vertices[1][0] = 0.5f;
        sprite.vertices[1][1] = -0.5f;
        sprite.vertices[1][2] = 0.0f;
        sprite.vertices[2][0] = -0.5f;
        sprite.vertices[2][1] = -0.5f;
        sprite.vertices[2][2] = 0.0f;
        sprite.vertices[3][0] = -0.5f;
        sprite.vertices[3][1] = 0.5f;
        sprite.vertices[3][2] = 0.0f;

        sprite.height = spriteHeight;
        sprite.width = spriteWidth;
        sprite.texXIndex = current_x_index;
        sprite.texYIndex = current_y_index;

        sprites.emplace_back(sprite);
    }

    sheet_texture = Texture2D::Create(filepath, "sprSheetTEX");

    uint8_t indices[6] = {
        0, 1, 2,
        0, 2, 3
    };

    vao = VertexArray::Create("sprSheetVAO");
    vao->Bind();
    vbo = VertexBuffer::Create(sizeof(Vertex) * 4, "sprSheetVBO");
    vbo->Bind();
    vbo->PushVertexAttrib(vao, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    vbo->PushVertexAttrib(vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));
    vbo->Unbind();
    ibo = IndexBuffer::Create(indices, 6, GL_UNSIGNED_BYTE, "sprSheetIBO");
    vao->Unbind();

    shader = Shader::Create("spritesheet.glsl", "sprShader");
}

SpriteSheet::~SpriteSheet()
{
}

void SpriteSheet::DrawSprite(uint32_t index, const glm::mat4& mvp, const glm::vec2& size)
{
    sheet_sprite_t* sprite = &sprites[index];

    Vertex vertices[4] = {
        Vertex(glm::vec3(sprite->vertices[0][0], sprite->vertices[0][1], sprite->vertices[0][2]), glm::vec2(sprite->coords[0][0], sprite->coords[0][1])),
        Vertex(glm::vec3(sprite->vertices[1][0], sprite->vertices[1][1], sprite->vertices[1][2]), glm::vec2(sprite->coords[1][0], sprite->coords[1][1])),
        Vertex(glm::vec3(sprite->vertices[2][0], sprite->vertices[2][1], sprite->vertices[2][2]), glm::vec2(sprite->coords[2][0], sprite->coords[2][1])),
        Vertex(glm::vec3(sprite->vertices[3][0], sprite->vertices[3][1], sprite->vertices[3][2]), glm::vec2(sprite->coords[3][0], sprite->coords[3][1])),
    };

    shader->Bind();
    glm::mat4 scaled_mvp = glm::scale(mvp, glm::vec3(size.x, size.y, 0.0f));
    shader->SetMat4("u_ViewProjection", renderer->camera->GetVPM());
    shader->SetMat4("u_MVP", scaled_mvp);
    sheet_texture->Bind();
    vao->Bind();
    vbo->Bind();
    ibo->Bind();

    glDrawElements(GL_TRIANGLES, ibo->GetCount(), GL_UNSIGNED_BYTE, NULL);

    ibo->Unbind();
    vao->Unbind();
    sheet_texture->Unbind();
    shader->Unbind();
}