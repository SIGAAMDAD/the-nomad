#include "rgl_local.h"

renderBackend_t backend;

extern "C" void RB_FlushVertices(void)
{
    R_DrawCache(backend.frameCache);
}

extern "C" void RB_DrawVertices(const drawVert_t *vertices, uint32_t numVertices)
{
    const float *v, *c, *t;
    uint32_t i;

    if (r_enableBuffers->i) {
        R_PushVertices(backend.frameCache, vertices, numVertices);
    }

    if (r_enableClientState->i) {
        nglEnableClientState(GL_VERTEX_ARRAY);
        nglEnableClientState(GL_COLOR_ARRAY);
        nglEnableClientState(GL_NORMAL_ARRAY);

        nglVertexPointer(3, GL_FLOAT, sizeof(drawVert_t), vertices[0].pos);
        nglTexCoordPointer(2, GL_FLOAT, sizeof(drawVert_t), vertices[0].texcoords);

        nglDrawArrays(GL_TRIANGLES, 0, numVertices);

        nglDisableClientState(GL_VERTEX_ARRAY);
        nglDisableClientState(GL_COLOR_ARRAY);
        nglDisableClientState(GL_NORMAL_ARRAY);
        return;
    }

    nglBegin(GL_TRIANGLE_FAN);
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
    R_PushVertices(backend.frameCache, vertices, 6);
    backend.usedVertices += 4;
}

extern "C" void RB_DrawTile(const drawTileCmd_t *cmd)
{
    if (!r_enableBuffers->i)
        RB_DrawVertices(cmd->vertices, 4);
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

    for (i = 0; i < 6; ++i) {
        result = mvp * positions[i];
        VectorCopy(vertices[i].pos, result);
    }
    RB_DrawVertices(vertices, 6);
}

extern "C" void RB_DrawRect(const drawRectCmd_t *cmd)
{
    drawVert_t vertices[6];
    uint32_t i;
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

    for (i = 0; i < 6; ++i) {
        out = mvp * positions[i];
        VectorCopy(vertices[i].pos, out);

        if (cmd->filled)
            VectorCopy(vertices[i].color, colorWhite);
        else
            VectorCopy(vertices[i].color, cmd->color);
    }
    RB_PushRect(vertices);
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

