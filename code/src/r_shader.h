#ifndef _R_SHADER_
#define _R_SHADER_

#pragma once

typedef struct
{
    nomad_hashtable<const char*, GLint> uniformCache;
    GLuint id;
} shader_t;

void R_ShaderClear(shader_t* shader);
shader_t* R_CreateShader(const char *filepath, const char *name);
GLint R_GetUniformLocation(shader_t* shader, const char *name);

GDR_INLINE void R_SetBool(shader_t* shader, const char* name, bool value)
{
    glUniform1iARB(R_GetUniformLocation(shader, name), (GLint)value);
}
GDR_INLINE void R_SetInt(shader_t* shader, const char* name, int value)
{
    glUniform1iARB(R_GetUniformLocation(shader, name), value);
}
GDR_INLINE void R_SetIntArray(shader_t* shader, const char* name, int* values, uint32_t count)
{
    glUniform1ivARB(R_GetUniformLocation(shader, name), count, values);
}
GDR_INLINE void R_SetFloat(shader_t* shader, const char* name, float value)
{
    glUniform1fARB(R_GetUniformLocation(shader, name), value);
}
GDR_INLINE void R_SetFloat2(shader_t* shader, const char* name, const glm::vec2& value)
{
    glUniform2fARB(R_GetUniformLocation(shader, name), value.x, value.y);
}
GDR_INLINE void R_SetFloat3(shader_t* shader, const char* name, const glm::vec3& value)
{
    glUniform3fARB(R_GetUniformLocation(shader, name), value.x, value.y, value.z);
}
GDR_INLINE void R_SetFloat4(shader_t* shader, const char* name, const glm::vec4& value)
{
    glUniform4fARB(R_GetUniformLocation(shader, name), value.r, value.g, value.b, value.a);
}
GDR_INLINE void R_SetMat4(shader_t* shader, const char* name, const glm::mat4& value)
{
    glUniformMatrix4fvARB(R_GetUniformLocation(shader, name), 1, GL_FALSE, glm::value_ptr(value));
}

#endif