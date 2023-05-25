#include "n_shared.h"
#include "g_zone.h"
#include "m_renderer.h"

inline static GLenum ShaderTypeFromString(const std::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment")
        return GL_FRAGMENT_SHADER;
    return 0;
}

std::unordered_map<GLenum, std::string> ShaderPreProcess(const std::string& source)
{
	std::unordered_map<GLenum, std::string> shaderSources;

	const char* typeToken = "#type";
	size_t typeTokenLength = N_strlen(typeToken);
	size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
	while (pos != std::string::npos) {
		size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
		if (eol == std::string::npos)
            N_Error("ShaderPreProcess: syntax error");
		
        size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
		std::string type = source.substr(begin, eol - begin);
		if (!ShaderTypeFromString(type))
            N_Error("ShaderPreProcess: invalid shader type specified");

		size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
		if (nextLinePos == std::string::npos)
            N_Error("ShaderPreProcess: syntax error");
        
		pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line
		shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos)
            ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
	}
	
    return shaderSources;
}

GLuint Shader::Compile(const std::string& src, GLenum type)
{
    GLuint program = glCreateShaderObjectARB(type);
    const char* buffer = src.c_str();
    glShaderSourceARB(program, 1, &buffer, NULL);
    glCompileShaderARB(program);
    int success;
    glGetShaderiv(program, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca16(length);
        glGetShaderInfoLog(program, length, (GLsizei *)&length, str);
        glDeleteShader(program);
        glDeleteShader(id);
        N_Error("Shader::Compile: failed to compile shader of type %s, error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
    return program;
}

void Shader::CreateProgram(const std::string& filepath)
{
    id = glCreateProgramObjectARB();
    GLuint vertid = Compile(GLSL_Src[GL_VERTEX_SHADER], GL_VERTEX_SHADER);
    GLuint fragid = Compile(GLSL_Src[GL_FRAGMENT_SHADER], GL_FRAGMENT_SHADER);

    glAttachShader(id, vertid);
    glAttachShader(id, fragid);
    glLinkProgramARB(id);
    glValidateProgramARB(id);
    glUseProgramObjectARB(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca16(length);
        glGetProgramInfoLog(id, length, (GLsizei *)&length, str);
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        glDeleteProgram(id);
        N_Error("Shader::Compile: failed to compile and/or link shader program file %s, error message: %s",
            filepath.c_str(), str);
    }
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgramObjectARB(0);
}

Shader::Shader(const std::string& filepath)
{
    char *buffer;
    size_t fsize = N_LoadFile(filepath.c_str(), &buffer);
    GLSL_Src = ShaderPreProcess(std::string(buffer));
    CreateProgram(filepath.c_str());
    Con_Printf("Shader::Shader: successfully compiled shader %s", filepath.c_str());
}

GLuint Shader::CompileSPIRV(const char* buf, size_t len, GLenum type)
{
    GLuint program = glCreateShaderObjectARB(type);
    glShaderBinary(1, &program, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, buf, len);
    glSpecializeShaderARB(program, "main", 0, NULL, NULL);

    int success = 0;
    glGetShaderiv(program, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca16(length);
        glGetShaderInfoLog(program, length, (GLsizei *)&length, str);
        glDeleteShader(program);
        glDeleteShader(id);
        N_Error("Shader::CompileSPIRV: failed to compile shader of type %s, error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
}

void Shader::ProcessSPIRV(const char* vertbuf, size_t vertbufsize, const char* fragbuf, size_t fragbufsize)
{
    id = glCreateProgramObjectARB();
    GLuint vertid = CompileSPIRV(vertbuf, vertbufsize, GL_VERTEX_SHADER);
    GLuint fragid = CompileSPIRV(fragbuf, fragbufsize, GL_FRAGMENT_SHADER);

    glAttachShader(id, vertid);
    glAttachShader(id, fragid);
    glLinkProgramARB(id);
    glValidateProgramARB(id);
    glUseProgramObjectARB(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca16(length);
        glGetProgramInfoLog(id, length, (GLsizei *)&length, str);
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        glDeleteProgram(id);
        N_Error("Shader::ProcessSPIRV: failed to compile and/or link shader files, error message: %s", str);
    }
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgramObjectARB(0);
}

Shader::Shader(const std::string& vertfile, const std::string& fragfile)
{
    char *vertbuf, *fragbuf;
    size_t vertbufsize, fragbufsize;

    vertbufsize = N_LoadFile(vertfile.c_str(), &vertbuf);
    fragbufsize = N_LoadFile(fragfile.c_str(), &fragbuf);
    ProcessSPIRV(vertbuf, vertbufsize, fragbuf, fragbufsize);
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

Shader* Shader::Create(const std::string& filepath, const eastl::string& name) {
    return CONSTRUCT(Shader, name.c_str(), filepath);
}