// dear imgui: Renderer Backend for modern OpenGL with shaders / programmatic pipeline
// - Desktop GL: 2.x 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the FAQ about ImTextureID!
//  [x] Renderer: Large meshes support (64k+ vertices) with 16-bit indices (Desktop OpenGL only).

// About WebGL/ES:
// - You need to '#define IMGUI_IMPL_OPENGL_ES2' or '#define IMGUI_IMPL_OPENGL_ES3' to use WebGL or OpenGL ES.
// - This is done automatically on iOS, Android and Emscripten targets.
// - For other targets, the define needs to be visible from the imgui_impl_opengl3.cpp compilation unit. If unsure, define globally or in imconfig.h.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// About GLSL version:
//  The 'glsl_version' initialization parameter should be nullptr (default) or a "#version XXX" string.
//  On computer platform the GLSL version default to "#version 130". On OpenGL ES 3 platform it defaults to "#version 300 es"
//  Only override if your GL version doesn't handle this GLSL version. See GLSL version table at the top of imgui_impl_opengl3.cpp.

#pragma once

#ifdef __cplusplus
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int16_t GLshort;
typedef uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef char GLchar;
typedef ssize_t GLsizeiptr;
#ifdef _WIN32
typedef intptr_t GLintptr;
#endif
#endif

typedef struct
{
    const GLubyte *(*glGetString)( GLenum name );
    const GLubyte *(*glGetStringi)( GLenum name, GLuint index );
    void (*glBindSampler)( GLuint unit, GLuint sampler );
    void (*glScissor)( GLint x, GLint y, GLsizei width, GLsizei height );
    void (*glViewport)( GLint x, GLint y, GLsizei width, GLsizei height );
    void (*glUniform1i)( GLint location, GLint v0 );
    GLint (*glGetAttribLocation)( GLuint program, const GLchar *name );
    GLint (*glGetUniformLocation)( GLuint program, const GLchar *name );
    void (*glUniformMatrix4fv)( GLint location, GLsizei count, GLboolean transpose, const GLfloat *value );
    void (*glEnable)( GLenum mode );
    void (*glDisable)( GLenum mode );
    void (*glGetIntegerv)( GLenum pname, GLint *data );
    void (*glBlendEquation)( GLenum mode );
    void (*glBlendFuncSeparate)( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha );
    void (*glBlendEquationSeparate)( GLenum modeRGB, GLenum modeAlpha );
    void (*glPolygonMode)( GLenum face, GLenum mode );
    void (*glPixelStorei)( GLenum pname, GLint param );
    GLboolean (*glIsEnabled)( GLenum cap );
    GLboolean (*glIsDisabled)( GLenum cap );
    GLboolean (*glIsProgram)( GLenum program );
    GLboolean (*glIsShader)( GLenum shader );
    void (*glDrawElementsBaseVertex)( GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex );
    void (*glDrawElements)( GLenum mode, GLsizei count, GLenum type, const void *indices );
    void (*glGetVertexAttribPointerv)( GLuint index, GLenum pname, void **pointer );
    void (*glVertexAttribPointer)( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer );
    void (*glEnableVertexAttribArray)( GLuint index );
    void (*glDisableVertexAttribArray)( GLuint index );
    void (*glBufferData)( GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage );
    void (*glBufferSubData)( GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data );
    void (*glGetVertexAttribiv)( GLuint index, GLenum pname, GLint *params );
    void (*glBindTexture)( GLenum target, GLuint texture );
    void (*glUseProgram)( GLuint program );
    void (*glBindBuffer)( GLenum target, GLuint buffer );
    void (*glBindVertexArray)( GLuint array );
    void (*glGenBuffers)( GLsizei n, GLuint *buffers );
    void (*glGenVertexArrays)( GLsizei n, GLuint *arrays );
    void (*glDeleteBuffers)( GLsizei n, const GLuint *buffers );
    void (*glDeleteVertexArrays)( GLsizei n, const GLuint *arrays );
    void (*glGenTextures)( GLsizei n, GLuint *textures );
    void (*glDeleteTextures)( GLsizei n, const GLuint *textures );
    void (*glTexParameteri)( GLenum target, GLenum pname, GLint param );
    void (*glActiveTexture)( GLenum texture );
    void (*glTexImage2D)( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
} imguiGL3Import_t;


// Backend API
void ImGui_ImplOpenGL3_Init(void *shaderData, const char* glsl_version, const imguiGL3Import_t *import);
void ImGui_ImplOpenGL3_Shutdown(void);
void ImGui_ImplOpenGL3_NewFrame(void);
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData *draw_data);

// (Optional) Called by Init/NewFrame/Shutdown
int ImGui_ImplOpenGL3_CreateFontsTexture(void);
void ImGui_ImplOpenGL3_DestroyFontsTexture(void);
int ImGui_ImplOpenGL3_CreateDeviceObjects(void);
void ImGui_ImplOpenGL3_DestroyDeviceObjects(void);

// Specific OpenGL ES versions
//#define IMGUI_IMPL_OPENGL_ES2     // Auto-detected on Emscripten
//#define IMGUI_IMPL_OPENGL_ES3     // Auto-detected on iOS/Android

// You can explicitly select GLES2 or GLES3 API by using one of the '#define IMGUI_IMPL_OPENGL_LOADER_XXX' in imconfig.h or compiler command-line.
#if !defined(IMGUI_IMPL_OPENGL_ES2) \
 && !defined(IMGUI_IMPL_OPENGL_ES3)

// Try to detect GLES on matching platforms
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define IMGUI_IMPL_OPENGL_ES3               // iOS, Android  -> GL ES 3, "#version 300 es"
#elif defined(__EMSCRIPTEN__) || defined(__amigaos4__)
#define IMGUI_IMPL_OPENGL_ES2               // Emscripten    -> GL ES 2, "#version 100"
#else
// Otherwise imgui_impl_opengl3_loader.h will be used.
#endif

#endif
