#ifndef _NGL_
#define _NGL_

#pragma once

#include <stdint.h>

// make sure nothing else mangles
#define __gl_h__

typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef char GLchar;
typedef char GLcharARB;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef uint16_t GLushort;
typedef int16_t GLshort;
typedef unsigned char GLboolean;
typedef uint8_t GLubyte;
typedef ptrdiff_t GLsizeiptr;
typedef intptr_t GLintptr;
typedef uintptr_t GLuintptr;
typedef float GLclampf;
typedef double GLclampd;
typedef int32_t GLclampx;
typedef int64_t GLint64;
typedef int64_t GLint64EXT;
typedef uint64_t GLuint64;
typedef uint64_t GLuint64EXT;
typedef int16_t GLhalf;
typedef uint16_t GLhalfARB;
typedef int32_t GLfixed;
typedef intptr_t GLintptr;
typedef intptr_t GLintptrARB;
typedef ssize_t GLsizeiptr;
typedef ssize_t GLsizeiptrARB;


// macros, SO MANY FUCKING MACROS
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef GL_ARB_debug_output
#define GL_ARB_debug_output 1
typedef void (APIENTRY* glDebugProcARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB   0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB    0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB  0x8245
#define GL_DEBUG_SOURCE_API_ARB           0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB   0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB   0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB         0x824B
#define GL_DEBUG_TYPE_ERROR_ARB           0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB 0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB     0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB     0x8250
#define GL_DEBUG_TYPE_OTHER_ARB           0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB   0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB  0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB      0x9145
#define GL_DEBUG_SEVERITY_HIGH_ARB        0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB      0x9147
#endif

#ifndef GL_ARB_vertex_buffer_object
#define GL_ARB_vertex_buffer_object 1
#define GL_ARRAY_BUFFER_ARB                 0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB         0x8893
#define GL_STREAM_DRAW_ARB                  0x88E0
#define GL_STATIC_DRAW_ARB                  0x88E4
#define GL_DYNAMIC_DRAW_ARB                 0x88E8
#endif

#ifndef GL_VERSION_3_3
#define GL_VERSION_3_3 1
#endif

#define GL_VENDOR                                    0x1F00
#define GL_RENDERER                                  0x1F01
#define GL_VERSION                                   0x1F02
#define GL_EXTENSIONS                                0x1F03
#define GL_MAX_COLOR_ATTACHMENTS                     0x8CDF
#define GL_MAX_VERTEX_ATTRIBS                        0x8869
#define GL_STEREO                                    0x0C33
#define GL_MAX_TEXTURE_IMAGE_UNITS                   0x8872
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS          0x8B4D
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                 0x851C
#define GL_MAX_TEXTURE_SIZE                          0x0D33
#define GL_MAX_VIEWPORT_DIMS                         0x0D3A
#define GL_MAX_DRAW_BUFFERS                          0x8824
#define GL_DRAW_BUFFER0                              0x8825
#define GL_DRAW_BUFFER1                              0x8826
#define GL_DRAW_BUFFER2                              0x8827
#define GL_DRAW_BUFFER3                              0x8828
#define GL_DRAW_BUFFER4                              0x8829
#define GL_DRAW_BUFFER5                              0x882A
#define GL_DRAW_BUFFER6                              0x882B
#define GL_DRAW_BUFFER7                              0x882C
#define GL_DRAW_BUFFER8                              0x882D
#define GL_DRAW_BUFFER9                              0x882E
#define GL_DRAW_BUFFER10                             0x882F
#define GL_DRAW_BUFFER11                             0x8830
#define GL_DRAW_BUFFER12                             0x8831
#define GL_DRAW_BUFFER13                             0x8832
#define GL_DRAW_BUFFER14                             0x8833
#define GL_DRAW_BUFFER15                             0x8834
#define GL_MAJOR_VERSION                             0x821B
#define GL_MINOR_VERSION                             0x821C
#define GL_NUM_EXTENSIONS                            0x821D
#define GL_CONTEXT_FLAGS                             0x821E
#define GL_BYTE                                      0x1400
#define GL_UNSIGNED_BYTE                             0x1401
#define GL_SHORT                                     0x1402
#define GL_UNSIGNED_SHORT                            0x1403
#define GL_INT                                       0x1404
#define GL_UNSIGNED_INT                              0x1405
#define GL_FLOAT                                     0x1406
#define GL_TEXTURE_2D                                0x0DE1
#define GL_FRAGMENT_SHADER                           0x8B30
#define GL_VERTEX_SHADER                             0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS           0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS             0x8B4A
#define GL_MAX_VARYING_FLOATS                        0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS            0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS          0x8B4D
#define GL_NEAREST                                   0x2600
#define GL_LINEAR                                    0x2601
#define GL_NEAREST_MIPMAP_NEAREST                    0x2700
#define GL_LINEAR_MIPMAP_NEAREST                     0x2701
#define GL_NEAREST_MIPMAP_LINEAR                     0x2702
#define GL_LINEAR_MIPMAP_LINEAR                      0x2703
#define GL_TEXTURE_MAG_FILTER                        0x2800
#define GL_TEXTURE_MIN_FILTER                        0x2801
#define GL_TEXTURE_WRAP_S                            0x2802
#define GL_TEXTURE_WRAP_T                            0x2803
#define GL_REPEAT                                    0x2901
#define GL_CLAMP_TO_EDGE                             0x812F
#define GL_MIRRORED_REPEAT                           0x8370
#define GL_CLAMP_TO_BORDER                           0x812D
#define GL_TEXTURE0                                  0x84C0
#define GL_TEXTURE1                                  0x84C1
#define GL_TEXTURE2                                  0x84C2
#define GL_TEXTURE3                                  0x84C3
#define GL_TEXTURE4                                  0x84C4
#define GL_TEXTURE5                                  0x84C5
#define GL_TEXTURE6                                  0x84C6
#define GL_TEXTURE7                                  0x84C7
#define GL_TEXTURE8                                  0x84C8
#define GL_TEXTURE9                                  0x84C9
#define GL_TEXTURE10                                 0x84CA
#define GL_TEXTURE11                                 0x84CB
#define GL_TEXTURE12                                 0x84CC
#define GL_TEXTURE13                                 0x84CD
#define GL_TEXTURE14                                 0x84CE
#define GL_TEXTURE15                                 0x84CF
#define GL_TEXTURE16                                 0x84D0
#define GL_TEXTURE17                                 0x84D1
#define GL_TEXTURE18                                 0x84D2
#define GL_TEXTURE19                                 0x84D3
#define GL_TEXTURE20                                 0x84D4
#define GL_TEXTURE21                                 0x84D5
#define GL_TEXTURE22                                 0x84D6
#define GL_TEXTURE23                                 0x84D7
#define GL_TEXTURE24                                 0x84D8
#define GL_TEXTURE25                                 0x84D9
#define GL_TEXTURE26                                 0x84DA
#define GL_TEXTURE27                                 0x84DB
#define GL_TEXTURE28                                 0x84DC
#define GL_TEXTURE29                                 0x84DD
#define GL_TEXTURE30                                 0x84DE
#define GL_TEXTURE31                                 0x84DF
#define GL_ACTIVE_TEXTURE                            0x84E0
#define GL_RGB4                                      0x804F
#define GL_RGB5                                      0x8050
#define GL_RGB8                                      0x8051
#define GL_RGB10                                     0x8052
#define GL_RGB12                                     0x8053
#define GL_RGB16                                     0x8054
#define GL_RGBA2                                     0x8055
#define GL_RGBA4                                     0x8056
#define GL_RGBA8                                     0x8058
#define GL_RGBA12                                    0x805A
#define GL_RGBA16                                    0x805B
#define GL_DEPTH24_STENCIL8                          0x88F0
#define GL_UNSIGNED_INT_24_8                         0x84FA
#define GL_BGR                                       0x80E0
#define GL_BGRA                                      0x80E1
#define GL_POINTS                                    0x0000
#define GL_LINES                                     0x0001
#define GL_LINE_LOOP                                 0x0002
#define GL_LINE_STRIP                                0x0003
#define GL_TRIANGLES                                 0x0004
#define GL_TRIANGLE_STRIP                            0x0005
#define GL_TRIANGLE_FAN                              0x0006
#define GL_QUADS                                     0x0007
#define GL_NEVER                                     0x0200
#define GL_LESS                                      0x0201
#define GL_EQUAL                                     0x0202
#define GL_LEQUAL                                    0x0203
#define GL_GREATER                                   0x0204
#define GL_NOTEQUAL                                  0x0205
#define GL_GEQUAL                                    0x0206
#define GL_STENCIL_TEST                              0x0B90
#define GL_DEPTH_TEST                                0x0B71
#define GL_BLEND                                     0x0BE2
#define GL_BLEND_SRC                                 0x0BE1
#define GL_BLEND_DST                                 0x0BE0
#define GL_RGB					                     0x1907
#define GL_RGBA					                     0x1908
#define GL_ALWAYS                                    0x0207
#define GL_ZERO                                      0
#define GL_ONE                                       1
#define GL_SRC_COLOR                                 0x0300
#define GL_ONE_MINUS_SRC_COLOR                       0x0301
#define GL_SRC_ALPHA                                 0x0302
#define GL_ONE_MINUS_SRC_ALPHA                       0x0303
#define GL_DST_ALPHA                                 0x0304
#define GL_ONE_MINUS_DST_ALPHA                       0x0305
#define GL_DST_COLOR                                 0x0306
#define GL_ONE_MINUS_DST_COLOR                       0x0307
#define GL_SRC_ALPHA_SATURATE                        0x0308
#define GL_NONE                                      0
#define GL_READ_FRAMEBUFFER                          0x8CA8
#define GL_DRAW_FRAMEBUFFER                          0x8CA9
#define GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER        0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER        0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD
#define GL_DEPTH_STENCIL_ATTACHMENT                  0x821A
#define GL_DEPTH_STENCIL                             0x84F9
#define GL_COLOR_ATTACHMENT0                         0x8CE0
#define GL_COLOR_ATTACHMENT1                         0x8CE1
#define GL_COLOR_ATTACHMENT2                         0x8CE2
#define GL_COLOR_ATTACHMENT3                         0x8CE3
#define GL_COLOR_ATTACHMENT4                         0x8CE4
#define GL_COLOR_ATTACHMENT5                         0x8CE5
#define GL_COLOR_ATTACHMENT6                         0x8CE6
#define GL_COLOR_ATTACHMENT7                         0x8CE7
#define GL_COLOR_ATTACHMENT8                         0x8CE8
#define GL_COLOR_ATTACHMENT9                         0x8CE9
#define GL_COLOR_ATTACHMENT10                        0x8CEA
#define GL_COLOR_ATTACHMENT11                        0x8CEB
#define GL_COLOR_ATTACHMENT12                        0x8CEC
#define GL_COLOR_ATTACHMENT13                        0x8CED
#define GL_COLOR_ATTACHMENT14                        0x8CEE
#define GL_COLOR_ATTACHMENT15                        0x8CEF
#define GL_COLOR_ATTACHMENT16                        0x8CF0
#define GL_COLOR_ATTACHMENT17                        0x8CF1
#define GL_COLOR_ATTACHMENT18                        0x8CF2
#define GL_COLOR_ATTACHMENT19                        0x8CF3
#define GL_COLOR_ATTACHMENT20                        0x8CF4
#define GL_COLOR_ATTACHMENT21                        0x8CF5
#define GL_COLOR_ATTACHMENT22                        0x8CF6
#define GL_COLOR_ATTACHMENT23                        0x8CF7
#define GL_COLOR_ATTACHMENT24                        0x8CF8
#define GL_COLOR_ATTACHMENT25                        0x8CF9
#define GL_COLOR_ATTACHMENT26                        0x8CFA
#define GL_COLOR_ATTACHMENT27                        0x8CFB
#define GL_COLOR_ATTACHMENT28                        0x8CFC
#define GL_COLOR_ATTACHMENT29                        0x8CFD
#define GL_COLOR_ATTACHMENT30                        0x8CFE
#define GL_COLOR_ATTACHMENT31                        0x8CFF
#define GL_DEPTH_ATTACHMENT                          0x8D00
#define GL_STENCIL_ATTACHMENT                        0x8D20
#define GL_FRAMEBUFFER                               0x8D40
#define GL_RENDERBUFFER                              0x8D41

#define NGL_Debug_Procs \
	NGL( void, glDebugMessageCallbackARB, glDebugProcARB callback, const GLvoid *userParam ) \
    NGL( void, glDebugMessageControlARB, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled )

#define NGL_Core_Procs \
	NGL( void, glEnable, GLenum cap ) \
	NGL( void, glDisable, GLenum cap ) \
	NGL( void, glDepthFunc, GLenum func ) \
    NGL( void, glDepthMask, GLboolean flag ) \
    NGL( const GLubyte*, glGetString, GLenum name ) \
    NGL( const GLubyte*, glGetStringi, GLenum name, GLuint index) \
    NGL( void, glGetIntegerv, GLenum pname, GLuint *params ) \
    NGL( void, glGetBooleanv, GLenum pname, GLboolean *params ) \
	NGL( void, glBlendFunc, GLenum sfactor, GLenum dfactor ) \
	NGL( void, glDrawElements, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) \
	NGL( void, glDrawArrays, GLenum mode, GLint first, GLsizei count ) \
	NGL( void, glClear, GLbitfield mask ) \
	NGL( void, glClearColor, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) \
	NGL( void, glViewport, GLint x, GLint y, GLint width, GLint height )

#define NGL_FBO_Procs \
	NGL( void, glGenFramebuffers, GLsizei n, GLuint *buffers ) \
	NGL( void, glGenRenderbuffers, GLsizei n, GLuint *buffers ) \
	NGL( void, glBindFramebuffer, GLenum target, GLuint buffer ) \
	NGL( void, glBindRenderbuffer, GLenum target, GLuint buffer ) \
	NGL( void, glBlitFramebuffer, ) \
	NGL( void, glFramebufferTexture2D, GLenum target ) \
	NGL( void, glFramebufferRenderbuffer, ) \
	NGL( void, glRenderbufferStorage, ) \
	NGL( void, glRenderbufferStorageMultisample, ) \
	NGL( void, glCheckFramebufferStatus, GLenum target )

#define NGL_Texture_Procs \
	NGL( void, glGenTextures, GLsizei n, GLuint *textures ) \
	NGL( void, glDeleteTextures, GLsizei n, GLuint *textures ) \
	NGL( void, glTexImage2D, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data ) \
	NGL( void, glTexParameteri, GLenum target, GLenum pname, GLint param ) \
	NGL( void, glTexParameterf, GLenum target, GLenum pname, GLfloat param ) \
	NGL( void, glBindTexture, GLenum target, GLuint texture ) \
	NGL( void, glActiveTexture, GLenum texture )

#define NGL_VAO_Procs \
	NGL( void, glBindVertexArray, GLuint array ) \
	NGL( void, glGenVertexArrays, GLsizei n, GLuint *arrays ) \
	NGL( void, glDeleteVertexArrays, GLsizei n, GLuint *arrays ) \
	NGL( void, glEnableVertexArrayAttribARB, GLuint index ) \
	NGL( void, glDisableVertexArrayAttribARB, GLuint index ) \
	NGL( void, glVertexAttribPointerARB, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer )

#define NGL_Buffer_Procs \
	NGL( void, glBindBufferARB, GLenum target, GLuint buffer ) \
	NGL( void, glBufferDataARB, GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage ) \
	NGL( void, glGenBuffersARB, GLsizei n, GLuint *buffers) \
	NGL( void, glDeleteBuffersARB, GLsizei n, GLuint *buffers) \
	NGL( void, glBufferSubDataARB, GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data)

#define NGL(ret,name,...) extern ret(APIENTRY* n##name)(__VA_ARGS__);
	NGL_Buffer_Procs
	NGL_VAO_Procs
	NGL_Texture_Procs
	NGL_FBO_Procs
	NGL_Debug_Procs
	NGL_Core_Procs
#undef NGL

#endif