#include "n_shared.h"
#include "m_renderer.h"

GPUContext::GPUContext()
{
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &gpu_memory_total);
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &gpu_memory_available);

    renderer = (const char*)glGetString(GL_RENDERER);
    version = (const char*)glGetString(GL_VERSION);
    vendor = (const char*)glGetString(GL_VENDOR);

    glGetIntegerv(GL_NUM_SPIR_V_EXTENSIONS, &num_glsl_extensions);
    glsl_extensions = (const char*)glGetString(GL_SPIR_V_EXTENSIONS);

    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    extensions = (char **)Z_Malloc(sizeof(char *) * num_extensions, TAG_STATIC, &extensions, "OpenGL_EXT");
    for (int i = 0; i < num_extensions; i++) {
        const char* str = (const char*)glGetStringi(GL_EXTENSIONS, i);
        extensions[i] = (char *)Z_Malloc(strlen(str)+1, TAG_STATIC, &extensions[i], "extensionStr");
        strncpy(extensions[i], str, strlen(str));
    }
}

GPUContext::~GPUContext()
{
    for (int i = 0; i < num_extensions; i++) {
        Z_Free(extensions[i]);
    }
    Z_Free(extensions);
}