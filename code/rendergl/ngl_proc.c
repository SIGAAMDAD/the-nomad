#include "rgl_local.h"

#ifdef _NOMAD_DEBUG

void GL_LogError(const char *glFunc, GLenum error)
{
    static char msg[MAXPRINTMSG];

    Com_snprintf(msg, sizeof(msg), "error 0x%04x after operation '%s'", error, glFunc);

    nGL_glDebugMessageInsertARB( GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_ERROR_ARB, 0, GL_DEBUG_SEVERITY_HIGH_ARB, strlen(msg), msg);
}

static void GL_ErrorCheck(const char *glFunc)
{
    GLenum error;

    if ((error = nGL_glGetError()) != GL_NO_ERROR) {
        GL_LogError(glFunc, error);
    }
}

#define GLCall(x) (void)nGL_glGetError(); x; GL_ErrorCheck(#x);

void nglColorMask( GLenum red, GLenum green, GLenum blue, GLenum alpha ) {

}

void nglAlphaFunc( GLenum func, GLclampf ref ) {

}

GLenum glGetError( void ) {

}

void nglGetIntegerv( GLenum pname, GLint *data ) {

}

void nglGetFloatv( GLenum pname, GLfloat *data ) {

}

void nglEnable( GLenum mode ) {

}

void nglDisable( GLenum mode ) {

}

void nglBlendFunc( GLenum sfactor, GLenum dfactor ) {

}

const GLubyte* nglGetString( GLenum name ) {

}

const GLubyte* nglGetStringi( GLenum name, GLuint index ) {

}

void nglDepthMask( GLboolean flag ) {

}

void nglDepthFunc( GLenum func ) {

}

void nglReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {

}

void nglScissor( GLint x, GLint y, GLsizei width, GLsizei height ) {

}

void nglShadeModel( GLenum mode ) {

}

void nglStencilFunc( GLenum func, GLint ref, GLuint mask ) {

}

void nglStencilMask( GLuint mask ) {

}

void nglStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) {

}

void nglClearStencil( GLint s ) {

}

void nglViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {

}

void nglClear( GLbitfield mask ) {

}

void nglDrawArrays( GLenum mode, GLint first, GLsizei count ) {

}

void nglDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) {

}

void nglLineWidth( GLfloat width ) {

}

void nglCullFace( GLenum mode ) {

}

void nglGetBooleanv( GLenum pname, GLboolean *params ) {

}

void nglBindSampler( GLuint unit, GLuint sampler ) {

}

void nglGenSamplers( GLsizei n, GLuint *samplers ) {

}

void nglDeleteSamplers( GLsizei n, const GLuint *samplers ) {

}

void nglBlendEquation( GLenum mode ) {

}

void nglBlendFuncSeparate( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha ) {

}

void nglBlendEquationSeparate( GLenum modeRGB, GLenum modeAlpha ) {

}

void nglPolygonMode( GLenum face, GLenum mode ) {

}

void nglPixelStorei( GLenum pname, GLint param ) {

}

GLboolean nglIsEnabled( GLenum cap ) {

}

GLboolean nglIsDisabled( GLenum cap ) {

}

GLboolean nglIsProgram( GLenum program ) {

}

GLboolean nglIsShader( GLenum shader ) {

}

void nglDrawBuffer( GLenum mode ) {

}

void nglDepthRange( GLclampf zNear, GLclampf zFar ) {

}

void nglDrawBuffers( GLsizei n, const GLenum *bufs ) {

}

void nglBegin( GLenum mode ) {

}

void nglEnd( void ) {

}

void nglFlush( void ) {

}

void nglFinish( void ) {

}

void nglClearColor( GLfloat r, GLfloat g, GLfloat b, GLfloat a ) {

}

void nglColor4f( GLfloat r, GLfloat g, GLfloat b, GLfloat a ) {

}

void nglColor3f( GLfloat r, GLfloat g, GLfloat b ) {

}

void nglColor4fv( const GLfloat *values ) {

}

void nglColor3fv( const GLfloat *values ) {

}

void nglVertex3fv( const GLfloat *values ) {

}

void nglVertex3f( GLfloat x, GLfloat y, GLfloat z ) {

}

void nglVertex2f( GLfloat x, GLfloat y ) {

}

void nglVertex2fv( const GLfloat *values ) {

}

void nglTexCoord2f( GLfloat u, GLfloat v ) {

}

void nglTexCoord2fv( const GLfloat *values ) {

}

void nglPushMatrix( void ) {

}

void nglPopMatrix( void ) {

}

void nglMatrixMode( GLenum mode ) {

}

void nglMultMatrixf, const GLfloat *m ) {

}

void nglMultMatrixd, const GLdouble *m ) {

}

void nglLoadIdentity, void ) {

}

void nglLoadMatrixf, const GLfloat *m ) {

}

void nglLoadMatrixd, const GLdouble *m ) {

}

void nglEnableClientState, GLenum cap ) {

}

void nglDisableClientState, GLenum cap ) {

}

void nglTexCoordPointer, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) {

}

void nglVertexPointer, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) {

}

void nglColorPointer, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) {

}

void nglIndexPointer, GLenum type, GLsizei stride, const GLvoid *pointer ) {

}

void nglScalef, GLfloat x, GLfloat y, GLfloat z ) {

}

void nglRotatef, GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {

}

void nglTranslatef, GLfloat x, GLfloat y, GLfloat z ) {

}

void nglDrawElementsBaseVertex, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex ) {

}

void nglDebugMessageControlARB, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled ) {

}

void nglDebugMessageCallbackARB, GLDEBUGPROCARB callback, const GLvoid *userParam ) {

}

void nglDebugMessageInsertARB, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf ) {

}

void nglDebugMessageEnableAMD, GLenum category, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled ) {

}

void nglDebugMessageCallbackAMD, GLDEBUGPROCAMD callback, void *userParam ) {

}

void nglDebugMessageControlKHR, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled ) {

}

void nglDebugMessageCallbackKHR, GLDEBUGPROCKHR callback, const void *userParam ) {

}

void nglDebugMessageInsertKHR, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf ) {

}

void nglDebugMessageControl, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled ) {

}

void nglDebugMessageCallback, GLDEBUGPROC callback, const void *userParam ) {

}

void nglDebugMessageInsert, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf ) {

}

void nglObjectLabel, GLenum identifier, GLuint name, GLsizei length, const GLchar *label ) {

}

void nglPushDebugGroup, GLenum source, GLuint id, GLsizei length, const GLchar *label ) {

}

void nglPopDebugGroup, void ) {

}

void nglShaderBinary, GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length ) {

}

void nglGetProgramBinary, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary ) {

}

void nglBindAttribLocation, GLhandleARB programObj, GLuint index, const GLcharARB *name ) {

}

GLint nglGetAttribLocation, GLuint program, const GLchar *name) {

}

void nglShaderSource, GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length ) {

}

GLuint nglCreateProgram, void ) {

}

void nglLinkProgram, GLuint program ) {

}

void nglUseProgram, GLuint program ) {

}

void nglDetachShader, GLuint program, GLuint shader ) {

}

GLuint nglCreateShader, GLenum type ) {

}

void nglDeleteShader, GLuint shader ) {

}

void nglDeleteProgram, GLuint program ) {

}

void nglGetShaderiv, GLuint shader, GLenum pname, GLint *params ) {

}

void nglGetProgramiv, GLuint program, GLenum pname, GLint *params ) {

}

void nglGetShaderInfoLog, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog ) {

}

GLint nglGetUniformLocation, GLuint program, const GLchar *name ) {

}

void nglUniform1fARB, GLint location, GLfloat v0 ) {

}

void nglUniform2fARB, GLint location, GLfloat v0, GLfloat v1 ) {

}

void nglUniform3fARB, GLint location, GLfloat v0, GLfloat v1, GLfloat v2 ) {

}

void nglUniform4fARB, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) {

}

void nglUniform1fvARB, GLint location, GLsizei count, const GLfloat *value ) {

}

void nglUniform2fvARB, GLint location, GLsizei count, const GLfloat *value ) {

}

void nglUniform3fvARB, GLint location, GLsizei count, const GLfloat *value ) {

}

void nglUniform4fvARB, GLint location, GLsizei count, const GLfloat *value ) {

}

void nglUniform1iARB, GLint location, GLint v0 ) {

}

void nglUniform2iARB, GLint location, GLint v0, GLint v1 ) {

}

void nglUniform3iARB, GLint location, GLint v0, GLint v1, GLint v2 ) {

}

void nglUniform4iARB, GLint location, GLint v0, GLint v1, GLint v2, GLint v3 ) {

}

void nglUniform1ivARB, GLint location, GLsizei count, const GLint *value ) {

}

void nglUniform2ivARB, GLint location, GLsizei count, const GLint *value ) {

}

void nglUniform3ivARB, GLint location, GLsizei count, const GLint *value ) {

}

void nglUniform4ivARB, GLint location, GLsizei count, const GLint *value ) {

}

void nglUniform1uiARB, GLint location, GLuint v0 ) {

}

void nglUniform2uiARB, GLint location, GLuint v0, GLuint v1 ) {

}

void nglUniform3uiARB, GLint location, GLuint v0, GLuint v1, GLuint v2 ) {

}

void nglUniform4uiARB, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3 ) {

}

void nglUniform1uivARB, GLint location, GLsizei count, const GLuint *value ) {

}

void nglUniform2uivARB, GLint location, GLsizei count, const GLuint *value ) {

}

void nglUniform3uivARB, GLint location, GLsizei count, const GLuint *value ) {

}

void nglUniform4uivARB, GLint location, GLsizei count, const GLuint *value ) {

}

void nglUniformMatrix3fvARB, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) {

}

void nglUniformMatrix4fvARB, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value ) {

}

void nglGetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog ) {

}

void nglCompileShader, GLuint shader ) {

}

void nglAttachShader, GLuint program, GLuint shader ) {

}

void nglValidateProgram, GLuint program ) {

}

GLuint nglGetUniformBlockIndex, GLuint program, const GLchar *uniformBlockName ) {

}

void nglUniformBlockBinding, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) {

}

void nglGetShaderSource, GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source ) {

}

void nglGetActiveUniform, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) {

}

void nglTexImage2DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations ) {

}

void nglActiveTexture, GLenum texture ) {

}

void nglGenTextures, GLsizei n, GLuint *textures ) {

}

void nglDeleteTextures, GLsizei n, const GLuint *textures ) {

}

void nglBindTexture, GLenum target, GLuint texture ) {

}

void nglTexImage2D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {

}

void nglTexParameteri, GLenum target, GLenum pname, GLint param ) {

}

void nglTexParameterf, GLenum target, GLenum pname, GLfloat param ) {

}

void nglCompressedTexImage2D,GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data ) {

}

void nglCompressedTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data ) {

}

void nglTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * data ) {

}

void nglGenFramebuffers, GLsizei n, GLuint *buffers ) {

}

void nglDeleteFramebuffers, GLsizei n, const GLuint *buffers ) {

}

void nglGenRenderbuffers, GLsizei n, GLuint *buffers ) {

}

void nglDeleteRenderbuffers, GLsizei n, const GLuint *buffers ) {

}

void nglBindFramebuffer, GLenum target, GLuint buffer ) {

}

void nglBindRenderbuffer, GLenum target, GLuint buffer ) {

}

void nglRenderbufferStorage, GLenum target, GLenum internalformat, GLsizei width, GLsizei height ) {

}

void nglRenderbufferStorageMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height ) {

}

void nglBlitFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter ) {

}

void nglFramebufferRenderbuffer, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer ) {

}

void nglFramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) {

}

GLenum nglCheckFramebufferStatus, GLenum target ) {

}

void nglEnableVertexArrayAttribARB, GLuint vao, GLuint index ) {

}

void nglDisableVertexAttribArrayARB, GLuint index ) {

}

void nglVertexAttribPointerARB, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer ) {

}

void nglGetVertexAttribivARB, GLuint index, GLenum pname, GLint *params ) {

}

void nglGetVertexAttribPointervARB, GLuint index, GLenum pname, void **pointer ) {

}

void nglGenVertexArrays, GLsizei n, GLuint *arrays ) {

}

void nglDeleteVertexArrays, GLsizei n, const GLuint *arrays ) {

}

void nglBindVertexArray, GLuint array ) {

}

void nglEnableVertexAttribArray, GLuint index ) {

}

void nglEnableVertexArrayAttrib, GLuint vao, GLuint index ) {

}

void nglDisableVertexAttribArray, GLuint index ) {

}

void nglVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer ) {

}

void nglGetVertexAttribiv, GLuint index, GLenum pname, GLint *params ) {

}

void nglGetVertexAttribPointerv, GLuint index, GLenum pname, void **pointer ) {

}

void nglGenBuffersARB, GLsizei n, GLuint *buffers ) {

}

void nglDeleteBuffersARB, GLsizei n, const GLuint *buffers ) {

}

void nglBindBufferARB, GLenum target, GLuint buffer ) {

}

void nglBufferDataARB, GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage ) {

}

void nglBufferSubDataARB, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data ) {

}

void nglUnmapBufferARB, GLenum target ) {

}

void* nglMapBufferARB, GLenum target, GLbitfield access ) {

}

void nglGenBuffers, GLsizei n, GLuint *buffers ) {

}

void nglDeleteBuffers, GLsizei n, const GLuint *buffers ) {

}

void nglBindBuffer, GLenum target, GLuint buffer ) {

}

void nglBufferData, GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage ) {

}

void nglBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data ) {

}

void* nglMapBuffer, GLenum target, GLbitfield access ) {

}

void nglUnmapBuffer, GLenum target ) {

}

void* nglMapBufferRange, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ) {

}

void nglFlusMappedBufferRange( GLenum target, GLintptr offset, GLsizeiptr length ) {

}

void nglBufferStorage( GLenum target, GLsizeiptr size, const void *data, GLbitfield flags ) {

}

void nglBindBufferBase( GLenum target, GLuint first, GLsizei count, const GLuint *buffers ) {

}

void nglBindBufferRange( GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes ) {

}

void nglBindAttribLocationARB( GLhandleARB programObj, GLuint index, const GLcharARB *name) {

}

void nglGetActiveAttribARB( GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name) {

}

GLint nglGetAttribLocationARB( GLhandleARB programObj, const GLcharARB *name) {

}


#endif // !_NOMAD_DEBUG
