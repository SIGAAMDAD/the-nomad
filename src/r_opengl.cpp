#include "n_shared.h"
#include "n_scf.h"
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

GLuint GL_Shader::Compile(const std::string& src, GLenum type)
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

GLuint GL_Shader::CreateProgram(const std::string& filepath)
{
    GLuint program = glCreateProgram();

    std::cout << "vertex source: \n" << GLSL_Src[GL_VERTEX_SHADER] << std::endl;
    std::cout << "fragment source: \n" << GLSL_Src[GL_FRAGMENT_SHADER] << std::endl; 
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

void GL_Shader::Bind() const
{ glUseProgram(id); }
void GL_Shader::Unbind() const
{ glUseProgram(0); }

GL_Shader::GL_Shader(const std::string& filepath)
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

GL_Shader::~GL_Shader()
{
    glDeleteProgram(id);
}


std::shared_ptr<VertexBuffer> VertexBuffer::Create(size_t reserve)
{
    switch (scf::renderer::api) {
    case scf::R_OPENGL: return std::make_shared<GL_VertexBuffer>(reserve);
    };

    if (scf::renderer::api != scf::R_SDL2)
        N_Error("VertexBuffer::Create: unknown rendering api");
    else
        N_Error("VertexBuffer::Create: SDL2 doesn't support Vertex Buffer Objects");
    
    return NULL;
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(float *vertices, size_t size)
{
    switch (scf::renderer::api) {
    case scf::R_OPENGL: return std::make_shared<GL_VertexBuffer>(vertices, size);
    };

    if (scf::renderer::api != scf::R_SDL2)
        N_Error("VertexBuffer::Create: unknown rendering api");
    else
        N_Error("VertexBuffer::Create: SDL2 doesn't support Vertex Buffer Objects");
    
    return NULL;
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t* indices, size_t size)
{
    switch (scf::renderer::api) {
    case scf::R_OPENGL: return std::make_shared<GL_IndexBuffer>(indices, size);
    };

    if (scf::renderer::api != scf::R_SDL2)
        N_Error("IndexBuffer::Create: unknown rendering api");
    else
        N_Error("IndexBuffer::Create: SDL2 doesn't support Index Buffer Objects");
    
    return NULL;
}

std::shared_ptr<VertexArray> VertexArray::Create()
{
    switch (scf::renderer::api) {
    case scf::R_OPENGL: return std::make_shared<GL_VertexArray>();
    };

    if (scf::renderer::api != scf::R_SDL2)
        N_Error("VertexArray::Create: unknown rendering api");
    else
        N_Error("VertexArray::Create: SDL2 doesn't support Vertex Array Objects");
    
    return NULL;
}

std::shared_ptr<Shader> Shader::Create(const std::string& filepath)
{
	switch (scf::renderer::api) {
	case scf::R_OPENGL:  return std::make_shared<GL_Shader>(filepath);
	};
    assert(false);
    if (!false) {
        N_Error("Shader::Create: unknown rendering api");
    }
    
	return NULL;
}

static GLenum ShaderDataTypeToGLBaseType(ShaderDataType type)
{
	switch (type) {
	case ShaderDataType::Float:    return GL_FLOAT;
	case ShaderDataType::Float2:   return GL_FLOAT;
	case ShaderDataType::Float3:   return GL_FLOAT;
	case ShaderDataType::Float4:   return GL_FLOAT;
	case ShaderDataType::Mat3:     return GL_FLOAT;
	case ShaderDataType::Mat4:     return GL_FLOAT;
	case ShaderDataType::Int:      return GL_INT;
	case ShaderDataType::Int2:     return GL_INT;
	case ShaderDataType::Int3:     return GL_INT;
	case ShaderDataType::Int4:     return GL_INT;
	case ShaderDataType::Bool:     return GL_BOOL;
	};
    if (!false) {
        N_Error("unknown ShaderDataType");
    }
        
	return 0;
}

GL_VertexArray::GL_VertexArray()
{
	glGenVertexArrays(1, &id);
}

GL_VertexArray::~GL_VertexArray()
{
	glDeleteVertexArrays(1, &id);
}

void GL_VertexArray::Bind() const
{
	glBindVertexArray(id);
}

void GL_VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

void GL_VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
//		HZ_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

	glBindVertexArray(id);
	vertexBuffer->Bind();

	const auto& layout = vertexBuffer->GetLayout();
	for (const auto& element : layout) {
		switch (element.Type) {
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:
		{
			glEnableVertexAttribArray(vertexBufferIndex);
			glVertexAttribPointer(vertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			vertexBufferIndex++;
			break;
		}
		case ShaderDataType::Int:
		case ShaderDataType::Int2:
		case ShaderDataType::Int3:
		case ShaderDataType::Int4:
		case ShaderDataType::Bool: {
			glEnableVertexAttribArray(vertexBufferIndex);
			glVertexAttribIPointer(vertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToGLBaseType(element.Type),
				layout.GetStride(),
				(const void*)element.Offset);
			vertexBufferIndex++;
			break; }
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4: {
			uint8_t count = element.GetComponentCount();
			for (uint8_t i = 0; i < count; i++) {
				glEnableVertexAttribArray(vertexBufferIndex);
				glVertexAttribPointer(vertexBufferIndex,
					count,
					ShaderDataTypeToGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(element.Offset + sizeof(float) * count * i));
				glVertexAttribDivisor(vertexBufferIndex, 1);
				vertexBufferIndex++;
			}
			break; }
		default:
			N_Error("unkown ShaderDataType");
		};
	}
    vertexBuffers.push_back(vertexBuffer);
}

void GL_VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& _indexBuffer)
{
	glBindVertexArray(id);

	indexBuffer = _indexBuffer;
}

GL_VertexBuffer::GL_VertexBuffer(size_t reserve)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, reserve, NULL, GL_DYNAMIC_DRAW);
}

GL_VertexBuffer::GL_VertexBuffer(float* vertices, size_t size)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

GL_VertexBuffer::~GL_VertexBuffer()
{
    glDeleteBuffers(1, &id);
}

void GL_VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void GL_VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GL_VertexBuffer::SetData(const void *data, size_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

GL_IndexBuffer::GL_IndexBuffer(uint32_t* indices, size_t count)
    : NumIndices(count)
{
    glGenBuffers(1, &id);

    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

GL_IndexBuffer::~GL_IndexBuffer()
{
    glDeleteBuffers(1, &id);
}

void GL_IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void GL_IndexBuffer::Unbind() const
{ 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}