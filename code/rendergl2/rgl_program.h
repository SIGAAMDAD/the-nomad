#ifndef __RGL_PROGAM__
#define __RGL_PROGRAM__

#pragma once

class CShaderProgram
{
public:
    void Shutdown(void);

    void InitProgram(void);
    void DeleteProgram(void);

    void SetUniformInt(uint32_t uniformNum, GLint value);
    void SetUniformFloat(uint32_t uniformNum, GLfloat value);
    void SetUniformVec2(uint32_t uniformNum, const glm::vec2 &v);
    void SetUniformVec3(uint32_t uniformNum, const glm::vec3 &v);
    void SetUniformVec4(uint32_t uniformNum, const glm::vec4 &v);
    void SetUniformMatrix4(uint32_t uniformNum, const glm::mat4 &m);

    uint32_t GetProgram(void) const;
    const char *GetName( void ) const;
    void SetName( const char *pName );

    void Bind(void) const;
    void Unbind(void) const;

    int CompileGPUShader(GLuint *prevShader, const GLchar *buffer, uint64_t size, GLenum shaderType, const char *programName);
    int LoadGPUShaderText(const char *name, const char *fallback, GLenum shaderType, char *dest, uint64_t destSize);
    void LinkProgram(void);
    void ShowProgramUniforms(void);
    void InitUniforms(void);

    uint32_t GetVertexShader( void ) const;
    uint32_t GetFragmentShader( void ) const;
    uint32_t *GetVertexId( void );
    uint32_t *GetFragmentId( void );

    uint32_t GetAttribBits(void) const;
    void SetAttribBits(uint32_t bits);

private:
    char m_szName[MAX_GDR_PATH];

    uint32_t m_nVertexBufLen;
    uint32_t m_nFragmentBufLen;

    uint32_t m_ProgramId;
    uint32_t m_VertexId;
    uint32_t m_FragmentId;
    uint32_t m_AttribBits; // vertex array attribute flags

    // uniforms
    GLint m_Uniforms[UNIFORM_COUNT];
    int16_t m_UniformBufferOffsets[UNIFORM_COUNT]; // max 32767/64=511 uniforms
    char *m_pUniformBuffer;
};

GDR_EXPORT GDR_INLINE uint32_t CShaderProgram::GetAttribBits(void) const
{
    return m_AttribBits;
}

GDR_EXPORT GDR_INLINE void CShaderProgram::SetName( const char *pName )
{
    N_strncpyz( m_szName, pName, sizeof(m_szName) );
}

GDR_EXPORT GDR_INLINE uint32_t CShaderProgram::GetVertexShader( void ) const
{
    return m_VertexId;
}

GDR_EXPORT GDR_INLINE uint32_t CShaderProgram::GetFragmentShader( void ) const
{
    return m_FragmentId;
}

GDR_EXPORT GDR_INLINE uint32_t *CShaderProgram::GetVertexId( void )
{
    return &m_VertexId;
}

GDR_EXPORT GDR_INLINE uint32_t *CShaderProgram::GetFragmentId( void )
{
    return &m_FragmentId;
}

GDR_EXPORT GDR_INLINE void CShaderProgram::SetAttribBits(uint32_t bits)
{
    m_AttribBits = bits;
}

GDR_EXPORT GDR_INLINE void CShaderProgram::InitProgram(void)
{
    m_ProgramId = nglCreateProgram();
}

GDR_EXPORT GDR_INLINE void CShaderProgram::DeleteProgram(void)
{
    if (m_ProgramId != 0)
    {
        nglDeleteProgram(m_ProgramId);
        m_ProgramId = 0;
    }
}

GDR_EXPORT GDR_INLINE uint32_t CShaderProgram::GetProgram(void) const
{
    return m_ProgramId;
}

GDR_EXPORT GDR_INLINE const char *CShaderProgram::GetName( void ) const
{
    return m_szName;
}

GDR_EXPORT GDR_INLINE void CShaderProgram::Bind(void) const
{
}

GDR_EXPORT GDR_INLINE void CShaderProgram::Unbind(void) const
{
}

GDR_EXPORT void GLSL_UseProgram(CShaderProgram *program);
GDR_EXPORT void GLSL_InitGPUShaders(void);
GDR_EXPORT void GLSL_ShutdownGPUShaders(void);

#endif