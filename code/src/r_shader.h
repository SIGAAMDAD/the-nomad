#ifndef _R_SHADER_
#define _R_SHADER_

#pragma once

class Shader
{
private:
    GLuint id;
    mutable google::sparse_hash_map<std::string, GLint> uniformCache;
    mutable std::unordered_map<GLenum, std::string> GLSL_Src;
    GLint GetUniformLocation(const std::string& name) const
    {
        if (uniformCache.find(name) != uniformCache.end())
            return uniformCache[name];
        
        GLint location = glGetUniformLocationARB(id, name.c_str());
        if (location == -1) {
            Con_Printf("WARNING: failed to get location of uniform %s", name);
            return 0;
        }
        uniformCache[name] = location;
        return location;
    }
    GLuint Compile(const std::string& src, GLenum type);
    void CreateProgram(const std::string& filepath);

    GLuint CompileSPIRV(const char* src, size_t len, GLenum type);
    void ProcessSPIRV(const char* vertbuf, size_t vertbufsize, const char* fragbuf, size_t fragbufsize);
public:
    Shader(const std::string& filepath); // source code
    Shader(const std::string& vertfile, const std::string& fragfile); // spirv
    ~Shader();

    inline void Bind() const
    { glUseProgramObjectARB(id); }
    inline void Unbind() const
    { glUseProgramObjectARB(0); }

    inline void SetBool(const std::string& name, bool value) const
    { Uniform1i(name, value); }
    inline void SetInt(const std::string& name, int value) const
    { Uniform1i(name, value); }
    inline void SetIntArray(const std::string& name, int* values, uint32_t count) const
    { Uniformiv(name, values, count); }
	inline void SetFloat(const std::string& name, float value) const
    { Uniform1f(name, value); }
	inline void SetFloat2(const std::string& name, const glm::vec2& value) const
    { Uniform2f(name, value); }
	inline void SetFloat3(const std::string& name, const glm::vec3& value) const
    { Uniform3f(name, value); }
	inline void SetFloat4(const std::string& name, const glm::vec4& value) const
    { Uniform4f(name, value); }
	inline void SetMat4(const std::string& name, const glm::mat4& value) const
    { UniformMat4(name, value); }

    inline void Uniform1i(const std::string& name, int value) const
    { glUniform1iARB(GetUniformLocation(name), value); }
    inline void Uniformiv(const std::string& name, int *values, uint32_t count) const
    { glUniform1ivARB(GetUniformLocation(name), count, values); }
    inline void Uniform1f(const std::string& name, float value) const
    { glUniform1fARB(GetUniformLocation(name), value); }
    inline void Uniform2f(const std::string& name, const glm::vec2& value) const
    { glUniform2fARB(GetUniformLocation(name), value.x, value.y); }
    inline void Uniform3f(const std::string& name, const glm::vec3& value) const
    { glUniform3fARB(GetUniformLocation(name), value.x, value.y, value.z); }
    inline void Uniform4f(const std::string& name, const glm::vec4& value) const
    { glUniform4fARB(GetUniformLocation(name), value.r, value.g, value.b, value.a); }
    inline void UniformMat4(const std::string& name, const glm::mat4& value) const
    { glUniformMatrix4fvARB(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }

    static Shader* Create(const std::string& filepath, const eastl::string& name);
};

#endif