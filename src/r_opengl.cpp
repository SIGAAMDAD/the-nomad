#include "n_shared.h"
#include "m_renderer.h"
static GLenum ShaderTypeFromString(const std::string& type)
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
	size_t typeTokenLength = strlen(typeToken);
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
    GLuint program = glCreateShader(type);
    const char* buffer = src.c_str();
    glShaderSource(program, 1, &buffer, NULL);
    glCompileShader(program);
    int success;
    glGetShaderiv(program, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca(length);
        glGetShaderInfoLog(program, length, (GLsizei *)&length, str);
        glDeleteShader(program);
        glDeleteShader(id);
        N_Error("GL_Shader::Compile: failed to compile shader of type %s, error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragement" : "unknown shader type"), str);
    }
    return program;
}

GLuint Shader::CreateProgram(const std::string& filepath)
{
    GLuint program = glCreateProgram();
    GLuint vertid = Compile(GLSL_Src[GL_VERTEX_SHADER], GL_VERTEX_SHADER);
    GLuint fragid = Compile(GLSL_Src[GL_FRAGMENT_SHADER], GL_FRAGMENT_SHADER);

    glAttachShader(program, vertid);
    glAttachShader(program, fragid);
    glLinkProgram(program);
    glValidateProgram(program);
    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *str = (char *)alloca(length);
        glGetProgramInfoLog(program, length, (GLsizei *)&length, str);
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        glDeleteProgram(program);
        N_Error("GL_Shader::Compile: failed to compile and/or link shader program file %s, error message: %s",
            filepath.c_str(), str);
    }
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    return program;
}

void Shader::Bind() const
{ glUseProgram(id); }
void Shader::Unbind() const
{ glUseProgram(0); }

Shader::Shader(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file)
        N_Error("GL_Shader::GL_Shader: failed to open shader file %s", filepath.c_str());
    
    assert(file.is_open());
    file.seekg(0L, std::ios::end);
    size_t fsize = file.tellg();
    std::string retn;
    retn.resize(fsize);
    file.seekg(0L, std::ios::beg);
    file.read(&retn[0], fsize);
    file.close();

    GLSL_Src = ShaderPreProcess(retn);

    id = CreateProgram(filepath);
    LOG_INFO("successfully compiled shader {}", filepath);
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

VertexBuffer::VertexBuffer(const void *data, size_t count)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, count, data, GL_STATIC_DRAW);
}

VertexBuffer::VertexBuffer(size_t reserve)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, reserve, NULL, GL_DYNAMIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &id);
}

void VertexBuffer::SetData(const void *data, size_t count)
{
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, count, data);
}

IndexBuffer::IndexBuffer(uint32_t *indices, size_t count)
    : NumIndices(count)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &id);
}