#include "rgl_local.h"

#define MAX_SHADER_HASH 1024
static shader_t* shaders[MAX_SHADER_HASH];

inline static uint32_t ShaderTypeFromString(const eastl::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment")
        return GL_FRAGMENT_SHADER;
    return 0;
}

int32_t R_GetUniformLocation(shader_t *shader, const char *name)
{
    uint64_t hash = Com_GenerateHashValue(name, RENDER_MAX_UNIFORMS);

    if (shader->uniformCache[hash] != -1)
        return shader->uniformCache[hash];
    
    int32_t location = nglGetUniformLocation(shader->programId, name);
    if (location == -1)
        ri.N_Error("R_GetUniformLocation: location was -1 for %s", name);
    
    shader->uniformCache[hash] = location;
    return location;
}

eastl::unordered_map<uint32_t, eastl::string> R_ParseShader(const eastl::string& source)
{
    eastl::unordered_map<uint32_t, eastl::string> shaderSources;

	const char *typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
	while (pos != eastl::string::npos) {
		size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
		if (eol == eastl::string::npos)
            ri.N_Error("ShaderPreProcess: syntax error");
		
        size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
		eastl::string type = source.substr(begin, eol - begin);
		if (!ShaderTypeFromString(type))
            ri.N_Error("ShaderPreProcess: invalid shader type specified");

		size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
		if (nextLinePos == eastl::string::npos)
            ri.N_Error("ShaderPreProcess: syntax error");
        
		pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line
		shaderSources[ShaderTypeFromString(type)] = (pos == eastl::string::npos)
            ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
	}
	
    return shaderSources;
}

static uint32_t R_CompileShaderSource(const char *src, uint32_t type, uint32_t id)
{
    uint32_t program;
    int32_t success, length;
    char str[1024];
    
    program = nglCreateShader(type);
    nglShaderSource(program, 1, &src, NULL);
    nglCompileShader(program);

    nglGetShaderiv(program, GL_COMPILE_STATUS, (int *)&success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));

        nglGetShaderInfoLog(program, sizeof(str), NULL, str);
        nglDeleteShader(program);
        nglDeleteProgram(id);

        ri.N_Error("R_CompileShaderSource: failed to compile shader of type %s.\nglslang error message: %s",
            (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "unknown shader type"), str);
    }
    return program;
}

shader_t* R_InitShader(const char *vertexFile, const char *fragmentFile)
{
    shader_t *shader;
    char str[1024];
    int success;
    file_t fd;

    shader = (shader_t *)Hunk_Alloc(sizeof(shader_t), "GLshader", h_low);

    fd = ri.FS_FOpenRead(vertexFile);
    if (fd == FS_INVALID_HANDLE) {
        ri.N_Error("R_InitShader: failed to open vertex shader file %s", vertexFile);
    }
    shader->vertexBufLen = ri.FS_FileLength(fd);
    shader->vertexBuf = (char *)ri.Z_Malloc(shader->vertexBufLen, TAG_RENDERER, &shader->vertexBuf, "GLvertShader");
    ri.FS_Read(shader->vertexBuf, shader->vertexBufLen, fd);
    ri.FS_FClose(fd);

    fd = ri.FS_FOpenRead(fragmentFile);
    if (fd == FS_INVALID_HANDLE) {
        ri.N_Error("R_InitShader: failed to open fragment shader file %s", fragmentFile);
    }
    shader->fragmentBufLen = ri.FS_FileLength(fd);
    shader->fragmentBuf = (char *)ri.Z_Malloc(shader->fragmentBufLen, TAG_RENDERER, &shader->fragmentBuf, "GLfragShader");
    ri.FS_Read(shader->fragmentBuf, shader->fragmentBufLen, fd);
    ri.FS_FClose(fd);

    char *filepath = (char *)ri.Alloca(strlen(vertexFile) + strlen(fragmentFile) + 1);
    stbsp_sprintf(filepath, "%s%s", vertexFile, fragmentFile);

    uint64_t hash = Com_GenerateHashValue(filepath, MAX_SHADER_HASH);
    memset(shader->uniformCache, -1, sizeof(shader->uniformCache));

    // compile
    uint32_t vertid = R_CompileShaderSource(shader->fragmentBuf, GL_VERTEX_SHADER, shader->programId);
    uint32_t fragid = R_CompileShaderSource(shader->vertexBuf, GL_FRAGMENT_SHADER, shader->programId);

    shader->programId = nglCreateProgram();
    nglUseProgram(shader->programId);

    // link
    nglAttachShader(shader->programId, vertid);
    nglAttachShader(shader->programId, fragid);
    nglLinkProgram(shader->programId);
    nglValidateProgram(shader->programId);

    // error checks
    nglGetProgramiv(shader->programId, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        nglGetProgramInfoLog(shader->programId, sizeof(str), NULL, str);
        
        nglDeleteShader(vertid);
        nglDeleteShader(fragid);
        nglDeleteProgram(shader->programId);

        ri.N_Error("R_InitShader: failed to compile and/or link shader file %s.\nglslang error message: %s", filepath, str);
    }

    // cleanup
    nglDeleteShader(vertid);
    nglDeleteShader(fragid);

    nglUseProgram(0);

    shaders[hash] = shader;
    renderer->shaders[renderer->numShaders] = shader;
    renderer->numShaders++;

    return shader;
}

void R_ShutdownShader(shader_t *shader)
{
    nglDeleteProgram(shader->programId);
    ri.Z_ChangeTag(shader->fragmentBuf, TAG_PURGELEVEL);
    ri.Z_ChangeTag(shader->vertexBuf, TAG_PURGELEVEL);
}

void R_BindShader(const shader_t *shader)
{
    if (renderer->shaderid == shader->programId)
        return; // already bound
    else if (renderer->shaderid)
        nglUseProgram(0); // unbind whatever's being used
    
    renderer->shaderid = shader->programId;
    nglUseProgram(shader->programId);
}

void R_UnbindShader(void)
{
    if (!renderer->shaderid)
        return; // already unbound
    
    renderer->shaderid = 0;
    nglUseProgram(0);
}

