#include "n_shared.h"
#include "m_renderer.h"

inline static GLenum ShaderTypeFromString(const eastl::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment")
        return GL_FRAGMENT_SHADER;
    return 0;
}

nomad_hashtable<GLenum, eastl::string> ShaderPreProcess(const eastl::string& source)
{
	nomad_hashtable<GLenum, eastl::string> shaderSources;

	const char* typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
	while (pos != eastl::string::npos) {
		size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
		if (eol == eastl::string::npos)
            N_Error("ShaderPreProcess: syntax error");
		
        size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
		eastl::string type = source.substr(begin, eol - begin);
		if (!ShaderTypeFromString(type))
            N_Error("ShaderPreProcess: invalid shader type specified");

		size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
		if (nextLinePos == eastl::string::npos)
            N_Error("ShaderPreProcess: syntax error");
        
		pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line
		shaderSources[ShaderTypeFromString(type)] = (pos == eastl::string::npos)
            ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
	}
	
    return shaderSources;
}

GLuint Shader::Compile(const char* src, GLenum type)
{
    GLuint program = glCreateShaderObjectARB(type);
    const char* buffer = src;
    glShaderSourceARB(program, 1, &buffer, NULL);
    glCompileShaderARB(program);
    int success;
    glGetShaderiv(program, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca(length);
        glGetShaderInfoLog(program, length, (GLsizei *)&length, str);
        glDeleteShader(program);
        glDeleteShader(id);
        N_Error("Shader::Compile: failed to compile shader of type %s, error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
    return program;
}

Shader::Shader(const char* filepath)
{
    char *buffer;
    const size_t fsize = N_LoadFile(filepath, (void **)&buffer);
    
    nomad_hashtable<GLenum, eastl::string> GLSL_Src = ShaderPreProcess(buffer);

    id = glCreateProgram();
    glUseProgram(id);

    GLuint vertid = Compile(GLSL_Src[GL_VERTEX_SHADER].c_str(), GL_VERTEX_SHADER);
    GLuint fragid = Compile(GLSL_Src[GL_FRAGMENT_SHADER].c_str(), GL_FRAGMENT_SHADER);

    glAttachShader(id, vertid);
    glAttachShader(id, fragid);
    glLinkProgramARB(id);
    glValidateProgramARB(id);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca(length);
        glGetProgramInfoLog(id, length, (GLsizei *)&length, str);
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        glDeleteProgram(id);
        N_Error("Shader::Compile: failed to compile and/or link shader file %s, error message: %s",
            filepath, str);
    }
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    glUseProgram(0);
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

Shader* Shader::Create(const char *filepath, const char *name)
{
    return CONSTRUCT(Shader, name, filepath);
}