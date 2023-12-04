#include "rgl_local.h"

extern void R_GLDebug_Callback_ARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

void R_InitExtensions(void)
{
#ifdef _NOMAD_DEBUG
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress(#name);
#else
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress(#name);
#endif
    // win32 has IGNORE already defined
    enum { EXT_IGNORE, EXT_USING, EXT_NOTFOUND, EXT_FAILED };
    const char *ext;
    const char *result[4] = { "...ignoring %s\n", "...using %s\n", "...%s not found\n", "...%s failed to load\n" };

    // set default
    nglBufferSubDataARB = nglBufferSubData;
    nglGenBuffersARB = nglGenBuffers;
    nglDeleteBuffersARB = nglDeleteBuffers;
    nglBufferDataARB = nglBufferData;
    nglMapBufferARB = nglMapBuffer;
    nglUnmapBufferARB = nglUnmapBuffer;
    nglEnableVertexArrayAttribARB = nglEnableVertexArrayAttrib;
    nglDisableVertexAttribArrayARB = nglDisableVertexAttribArray;
    nglVertexAttribPointerARB = nglVertexAttribPointer;
    glContext.vboTarget = GL_ARRAY_BUFFER;
    glContext.iboTarget = GL_ELEMENT_ARRAY_BUFFER;

    if (!r_useExtensions->i) {
        ri.Printf(PRINT_INFO, "...Ignoring OpenGL extensions");
    }

    //
    // ARB_buffer_storage
    //
    ext = "GL_ARB_buffer_storage";
    glContext.ARB_buffer_storage = qfalse;
    if (NGL_VERSION_ATLEAST(4, 5) || R_HasExtension(ext)) {
        glContext.ARB_buffer_storage = qtrue;

        NGL_ARB_buffer_storage

        if (!nglBufferStorage) {
            ri.Printf(PRINT_INFO, result[EXT_FAILED], ext);
            glContext.ARB_buffer_storage = qfalse;
        }
        else {
            ri.Printf(PRINT_INFO, result[EXT_USING], ext);
        }
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ARB_map_buffer_range
    //
    ext = "GL_ARB_map_buffer_range";
    glContext.ARB_map_buffer_range = qfalse;
    if (NGL_VERSION_ATLEAST(4, 5) || R_HasExtension(ext)) {
        glContext.ARB_map_buffer_range = qtrue;

        NGL_ARB_map_buffer_range
        
        if (!nglMapBufferRange || !nglFlushMappedBufferRange) {
            ri.Printf(PRINT_INFO, result[EXT_FAILED], ext);
            glContext.ARB_map_buffer_range = qfalse;
        }
        else {
            ri.Printf(PRINT_INFO, result[EXT_USING], ext);
        }
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ARB_vertex_array_object
    //
    ext = "GL_ARB_vertex_array_object";
    glContext.ARB_vertex_array_object = qfalse;
    if (NGL_VERSION_ATLEAST(3, 0) || R_HasExtension(ext)) {
        if (NGL_VERSION_ATLEAST(3, 0)) {
            // force vao, core context requires it
            glContext.ARB_vertex_array_object = qtrue;
        }
        else {
            glContext.ARB_vertex_array_object = !!r_arb_vertex_array_object;
        }

        NGL_VertexArrayARB_Procs
        if (!nglVertexAttribPointerARB || !nglEnableVertexArrayAttribARB || !nglDisableVertexArrayAttribARB
        || !nglEnableVertexAttribArrayARB || !nglDisableVertexAttribArrayARB) {
            ri.Printf(PRINT_INFO, result[EXT_FAILED], ext);
            glContext.ARB_vertex_array_object = qfalse;
        }
        else {
            ri.Printf(PRINT_INFO, result[EXT_USING], ext);
        }
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ARB_gl_spirv
    //
    ext = "GL_ARB_gl_spirv";
    glContext.ARB_gl_spirv = qfalse;
    if (NGL_VERSION_ATLEAST(4, 0) || R_HasExtension(ext)) {
        NGL_GLSL_SPIRV_Procs
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ARB_vertex_buffer_object
    //
    ext = "GL_ARB_vertex_buffer_object";
    glContext.ARB_vertex_buffer_object = qfalse;
    if (NGL_VERSION_ATLEAST(3, 0) || R_HasExtension(ext)) {
        NGL_BufferARB_Procs
        
        glContext.ARB_vertex_buffer_object = qtrue;

        if (!nglGenBuffersARB || !nglDeleteBuffersARB || !nglBindBufferARB || !nglBufferDataARB || !nglBufferSubDataARB) {
            ri.Printf(PRINT_INFO, result[EXT_FAILED], ext);
            glContext.vboTarget = GL_ARRAY_BUFFER;
            glContext.iboTarget = GL_ELEMENT_ARRAY_BUFFER;
            glContext.ARB_vertex_buffer_object = qfalse;
        }
        else {
            ri.Printf(PRINT_INFO, result[EXT_USING], ext);
        }
        
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ARB_texture_filter_anisotropic
    //
    ext = "GL_ARB_texture_filter_anisotropic";
    glContext.ARB_texture_filter_anisotropic = qfalse;
    if (R_HasExtension(ext)) {
        nglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glContext.maxAnisotropy);
        glContext.ARB_texture_filter_anisotropic = qtrue;

        if (glContext.maxAnisotropy <= 0) {
            ri.Printf(PRINT_INFO, "... GL_ARB_texture_filter_anisotropic not property supported\n");
            glContext.ARB_texture_filter_anisotropic = qfalse;
        }
        else {
            ri.Printf(PRINT_INFO, "...using GL_ARB_texture_filter_anisotropic (max: %f)\n", glContext.maxAnisotropy);
        }
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    ri.Cvar_Set("r_arb_texture_filter_anisotropic", va("%f", glContext.maxAnisotropy));

    //
    // ARB_texture_float
    //
    ext = "GL_ARB_texture_float";
    glContext.ARB_texture_float = qfalse;
    if (R_HasExtension(ext)) {
        glContext.ARB_texture_float = r_arb_texture_float->i;
        ri.Printf(PRINT_INFO, result[glContext.ARB_texture_float], ext);
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // gpu memory info diangostics extensions
    //

    glContext.memInfo = MI_NONE;

    //
    // NVX_gpu_memory_info
    //
    ext = "GL_NVX_gpu_memory_info";
    if (R_HasExtension(ext)) {
        glContext.memInfo = MI_NVX;
        ri.Printf(PRINT_INFO, result[EXT_USING], ext);
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ATI_meminfo
    //
    ext = "GL_ATI_meminfo";
    if (R_HasExtension(ext)) {
        if (glContext.memInfo == MI_NONE) {
            glContext.memInfo = MI_ATI;
            ri.Printf(PRINT_INFO, result[EXT_USING], ext);
        }
        else {
            ri.Printf(PRINT_INFO, result[EXT_IGNORE], ext);
        }
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    glContext.textureCompressionRef = TCR_NONE;

    //
    // ARB_texture_compression_rgtc
    //
    ext = "GL_ARB_texture_compression_rgtc";
    if (R_HasExtension(ext)) {
        qboolean useRgtc = r_arb_texture_compression->i >= 1;
        if (useRgtc)
            glContext.textureCompressionRef|= TCR_RGTC;
        
        ri.Printf(PRINT_INFO, result[useRgtc], ext);
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    glContext.swizzleNormalmap = r_arb_texture_compression->i && !(glContext.textureCompressionRef & TCR_RGTC);

    //
    // ARB_texture_compression_bptc
    //
    ext = "GL_ARB_texture_compression_bptc";
    if (R_HasExtension(ext)) {
        qboolean useBptc = r_arb_texture_compression->i >= 2;
        if (useBptc)
            glContext.textureCompressionRef |= TCR_BPTC;
        
        ri.Printf(PRINT_INFO, result[useBptc], ext);
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    //
    // ARB_framebuffer_object
    //
    ext = "GL_ARB_framebuffer_object";
    glContext.ARB_framebuffer_object = qfalse;
    glContext.ARB_framebuffer_sRGB = qfalse;
    glContext.ARB_framebuffer_multisample = qfalse;
    glContext.ARB_framebuffer_blit = qfalse;
    if (NGL_VERSION_ATLEAST(3, 0) || R_HasExtension(ext)) {
        glContext.ARB_framebuffer_object = r_arb_framebuffer_object->i;
        glContext.ARB_framebuffer_blit = qtrue;
        glContext.ARB_framebuffer_multisample = qtrue;
        glContext.ARB_framebuffer_sRGB = qtrue;

        nglGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &glContext.maxRenderBufferSize);
        nglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &glContext.maxColorAttachments);

        NGL_FBO_Procs

        ri.Printf(PRINT_INFO, result[glContext.ARB_framebuffer_object], ext);
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }
    
    //
    // ARB_debug_output
    //
    ext = "GL_ARB_debug_output";
    if (R_HasExtension(ext)) {
        glContext.debugType = GL_DBG_ARB;
        NGL_Debug_Procs

        if (!nglDebugMessageControlARB || !nglDebugMessageInsertARB || !nglDebugMessageCallbackARB) {
            ri.Printf(PRINT_INFO, result[EXT_FAILED], ext);
        }
        else {
            ri.Printf(PRINT_INFO, result[EXT_USING], ext);

            nglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            nglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            nglEnable(GL_DEBUG_OUTPUT);

            nglDebugMessageControlARB(GL_DEBUG_SOURCE_API_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            nglDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            nglDebugMessageControlARB(GL_DEBUG_SOURCE_SHADER_COMPILER_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            nglDebugMessageControlARB(GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            nglDebugMessageControlARB(GL_DEBUG_SOURCE_OTHER_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
            nglDebugMessageControlARB(GL_DEBUG_SOURCE_THIRD_PARTY_ARB, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

            nglDebugMessageCallbackARB(R_GLDebug_Callback_ARB, NULL);
        }
    }
    else {
        ri.Printf(PRINT_INFO, result[EXT_NOTFOUND], ext);
    }

    // determine GLSL version
    N_strncpyz(glContext.glsl_version_str, (const char *)nglGetString(GL_SHADING_LANGUAGE_VERSION), sizeof(glContext.glsl_version_str));
    sscanf(glContext.glsl_version_str, "%i.%i", &glContext.glslVersionMajor, &glContext.glslVersionMinor);
    ri.Printf(PRINT_INFO, "...using GLSL version %s\n", glContext.glsl_version_str);

#undef NGL
}