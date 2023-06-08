#include "n_shared.h"
#include "m_renderer.h"

#define MAX_FILE_HASH 1024
static shader_t* shaders[MAX_FILE_HASH];

inline static GLenum ShaderTypeFromString(const eastl::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment")
        return GL_FRAGMENT_SHADER;
    return 0;
}

GLint R_GetUniformLocation(shader_t* shader, const char *name)
{
    if (shader->uniformCache.find(name) != shader->uniformCache.end())
        return shader->uniformCache[name];
    
    GLint location = glGetUniformLocation(shader->id, name);
    if (location == -1)
        N_Error("R_GetUniformLocation: location was -1 for %s", name);
    
    shader->uniformCache[name] = location;
    return location;
}

eastl::unordered_map<GLenum, eastl::string> R_ParseShader(const eastl::string& source)
{
    eastl::unordered_map<GLenum, eastl::string> shaderSources;

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

static GLuint R_CompileShaderSource(const char *src, GLenum type, GLuint id)
{
    GLuint program;
    int success, length;
    char *str;
    
    program = glCreateShaderObjectARB(type);
    glShaderSourceARB(program, 1, &src, NULL);
    glCompileShaderARB(program);

    glGetShaderiv(program, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
        str = (char *)alloca(length);
        glGetShaderInfoLog(program, length, (GLsizei *)&length, str);

        glDeleteShader(program);
        glDeleteProgram(id);

        N_Error("R_CompileShaderSource: failed to compile shader of type %s.\nglslang error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
    return program;
}

void R_ShaderClear(shader_t* shader)
{
    shader->uniformCache.clear();
}

shader_t* R_CreateShader(const char* filepath, const char* name)
{
    shader_t* shader = (shader_t *)Hunk_Alloc(sizeof(shader_t), name, h_low);
    
    char *filebuf, *str;
    int success, length;
    const size_t fsize = N_LoadFile(filepath, (void **)&filebuf);

    eastl::unordered_map<GLenum, eastl::string> GLSL_Src = R_ParseShader(eastl::string(filebuf));

    uint64_t hash = Com_GenerateHashValue(filepath, MAX_FILE_HASH);
    shader->uniformCache = nomad_hashtable<const char*, GLint>();


    // compile
    GLuint vertid = R_CompileShaderSource(GLSL_Src[GL_VERTEX_SHADER].c_str(), GL_VERTEX_SHADER, shader->id);
    GLuint fragid = R_CompileShaderSource(GLSL_Src[GL_FRAGMENT_SHADER].c_str(), GL_FRAGMENT_SHADER, shader->id);

    shader->id = glCreateProgram();
    glUseProgram(shader->id);

    // link
    glAttachShader(shader->id, vertid);
    glAttachShader(shader->id, fragid);
    glLinkProgramARB(shader->id);
    glValidateProgramARB(shader->id);

    // error checks
    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        glGetProgramiv(shader->id, GL_INFO_LOG_LENGTH, &length);
        str = (char *)alloca(length);
        glGetProgramInfoLog(shader->id, length, (GLsizei *)&length, str);
        
        glDeleteShader(vertid);
        glDeleteShader(fragid);
        glDeleteProgram(shader->id);

        N_Error("R_CreateShader: failed to compile and/or link shader file %s.\nglslang error message: %s", filepath, str);
    }

    // cleanup
    glDeleteShader(vertid);
    glDeleteShader(fragid);

    glUseProgram(0);

    shader->hash = hash;
    shaders[hash] = shader;
    renderer->shaders[renderer->numShaders] = shader;
    renderer->numShaders++;

    return shader;
}