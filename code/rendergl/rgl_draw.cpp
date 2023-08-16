#include "rgl_local.h"

renderBackend_t backend;

extern "C" void RB_FlushVertices(void)
{
    R_BindCache(backend.frameCache);
    nglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(drawVert_t) * backend.usedVertices, backend.vertices);
    nglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, sizeof(uint32_t) * backend.usedIndices, backend.indices);
    nglDrawElements(GL_TRIANGLES, backend.usedIndices, GL_UNSIGNED_INT, NULL);
    backend.usedVertices = 0;
    backend.usedIndices = 0;
    R_UnbindCache();
}

extern "C" void RB_DrawVertices(const drawVert_t *vertices, uint32_t numVertices)
{
    const float *v, *c, *t;
    uint32_t i;

    nglBegin(GL_TRIANGLES);
    for (i = 0; i < numVertices; ++i) {
        v = (const float *)vertices[i].pos;
        c = (const float *)vertices[i].color;
        t = (const float *)vertices[i].texcoords;

//        nglColor4f(c[0], c[1], c[2], c[3]);
        nglVertex3f(v[0], v[1], v[2]);
        nglTexCoord2f(t[0], t[1]);
    }
    nglEnd();
}

extern "C" void RB_PushRect(const drawVert_t *vertices)
{
    if (backend.usedVertices + 4 >= backend.numVertices || backend.usedIndices + 6 >= backend.numIndices) {
        RB_FlushVertices();
    }

    memcpy(backend.vertices + backend.usedVertices, vertices, 4 * sizeof(drawVert_t));
    backend.usedVertices += 4;
    backend.usedIndices += 6;
}

extern "C" void RB_DrawTile(const drawTileCmd_t *cmd)
{
    if (!r_enableBuffers->i)
        RB_DrawVertices(cmd->vertices, 6);
    else
        RB_PushRect(cmd->vertices);
}

extern "C" void RB_DrawEntity(const drawRefCmd_t *cmd)
{
    const renderEntityRef_t *ref;
    drawVert_t vertices[6];
    glm::vec3 pos;
    glm::vec4 result;
    glm::mat4 model, mvp, identity = glm::mat4(1.0f);
    uint32_t i;
    const uint32_t count = r_enableBuffers->i ? 4 : 6;

    constexpr glm::vec4 positions[6] = {
        { 0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f,  0.5f, 0.0f, 1.0f}
    };

    ref = cmd->ref;
    VectorCopy(pos, ref->screenPos);

    model = glm::translate(identity, pos)
            * glm::scale(identity, glm::vec3(ref->size))
            * glm::rotate(identity, (float)DEG2RAD(ref->rotation), pos);
    mvp = rg.camera.vpm * model;

    for (i = 0; i < count; ++i) {
        result = mvp * positions[i];
        VectorCopy(vertices[i].pos, result);
    }
    RB_DrawVertices(vertices, count);
}

extern "C" void RB_DrawRect(const drawRectCmd_t *cmd)
{
    drawVert_t vertices[6];
    uint32_t i;
    const uint32_t count = r_enableBuffers->i == 1 ? 4 : 6;
    glm::vec3 pos, p;
    glm::vec4 out;
    glm::mat4 model, mvp, identity;

    identity = glm::mat4(1.0f);
    constexpr glm::vec4 positions[6] = {
        { 0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f,  0.5f, 0.0f, 1.0f}
    };

    VectorCopy(pos, cmd->pos);

    model = glm::translate(identity, pos)
            * glm::scale(identity, glm::vec3(cmd->size))
            * glm::rotate(identity, (float)DEG2RAD(cmd->rotation), pos);
    mvp = rg.camera.vpm * model;

    for (i = 0; i < count; ++i) {
        out = mvp * positions[i];
        VectorCopy(vertices[i].pos, out);

        if (cmd->filled)
            VectorCopy(vertices[i].color, colorWhite);
        else
            VectorCopy(vertices[i].color, cmd->color);
    }
    if (count == 4)
        RB_PushRect(vertices);
    else
        RB_DrawVertices(vertices, count);
}


/*
R_IsInView: returns qtrue if the position provided is within the player's fov
*/
extern "C" qboolean R_IsInView(const glm::vec3& pos)
{
    const float x = pos[0], y = pos[1];

    if ((x >= r_fovWidth->i + rg.plRef->worldPos[0] && x <= r_fovWidth->i - rg.plRef->worldPos[0])
    && (y >= r_fovHeight->i + rg.plRef->worldPos[1] && y <= r_fovHeight->i - rg.plRef->worldPos[1])) {
        return qtrue;
    }

    return qfalse;
}

