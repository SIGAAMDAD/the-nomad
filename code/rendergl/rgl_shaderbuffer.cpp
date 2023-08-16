#include "rgl_local.h"

extern "C" shaderBuffer_t *R_InitShaderBuffer(const shader_t *shader, const char *blockname, uint32_t bufferSize, uint32_t binding)
{
    shaderBuffer_t *buffer;

    buffer = (shaderBuffer_t *)ri.Z_Malloc(sizeof(*buffer), TAG_RENDERER, &buffer, "GLshaderBuf");
    buffer->bindingId = binding;
    buffer->bufferSize = bufferSize;

    nglGenBuffersARB(1, (GLuint *)&buffer->bufferId);
    nglBindBufferARB(GL_UNIFORM_BUFFER, buffer->bufferId);

    buffer->index = nglGetUniformBlockIndex(shader->programId, blockname);
    nglUniformBlockBinding(shader->programId, buffer->index, buffer->bindingId);

    nglBufferDataARB(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_STREAM_DRAW);
    nglBindBufferARB(GL_UNIFORM_BUFFER, 0);

    return buffer;
}

extern "C" void R_UpdateShaderBuffer(shaderBuffer_t *buffer, const void *data)
{
    R_BindShaderBuffer(buffer);
    void *buf = nglMapBufferARB(GL_UNIFORM_BUFFER, GL_MAP_WRITE_BIT);
    memcpy(buf, data, buffer->bufferSize);
    nglUnmapBufferARB(GL_UNIFORM_BUFFER);
    R_UnbindShaderBuffer();
}
