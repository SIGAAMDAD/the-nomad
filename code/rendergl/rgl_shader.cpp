#include "rgl_local.h"

#define MAX_SHADER_HASH 1024
static shader_t* shaders[MAX_SHADER_HASH];

extern "C" int32_t R_GetUniformLocation(shader_t *shader, const char *name)
{
    int32_t location;
    uint64_t hash;
    
    hash = Com_GenerateHashValue(name, RENDER_MAX_UNIFORMS);

    if (shader->uniformCache[hash] != -1)
        return shader->uniformCache[hash];
    
    location = nglGetUniformLocation(shader->programId, name);
    if (location == -1)
        ri.N_Error("R_GetUniformLocation: location was -1 for %s", name);
    
    shader->uniformCache[hash] = location;
    return location;
}

extern "C" uint32_t R_CompileShaderSource(const char *src, uint32_t type, uint32_t id)
{
    uint32_t program, i;
    int success;
    char str[1024];
    const char *sources[1];

    sources[0] = src;
    
    program = nglCreateShader(type);
    nglShaderSource(program, 1, sources, NULL);
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

extern "C" void R_RecompileShader(shader_t *shader)
{
    uint32_t vertid, fragid;
    int success;
    char str[1024];

    R_UnbindShader();

    // load the files back into ram
    shader->fragmentBufLen = ri.FS_LoadFile(shader->fragmentFile, (void **)&shader->fragmentBuf);
    shader->vertexBufLen = ri.FS_LoadFile(shader->vertexFile, (void **)&shader->vertexBuf);

    vertid = R_CompileShaderSource(shader->vertexBuf, GL_VERTEX_SHADER, shader->programId);
    fragid = R_CompileShaderSource(shader->fragmentBuf, GL_FRAGMENT_SHADER, shader->programId);

    ri.FS_FreeFile(shader->fragmentBuf);
    ri.FS_FreeFile(shader->vertexBuf);

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

        ri.N_Error("R_RecompileShader: failed to compile and/or link shader program (fragment: %s, vertex: %s).\n"
                    "glslang error message: %s", shader->fragmentFile, shader->vertexFile, str);
    }

    // cleanup
    nglDeleteShader(vertid);
    nglDeleteShader(fragid);

    nglUseProgram(0);
}

extern "C" shader_t* R_InitShader(const char *vertexFile, const char *fragmentFile)
{
    shader_t *shader;
    char str[1024];
    int success;
    uint32_t vertid, fragid;
    char filepath[MAX_GDR_PATH*2+32];
    uint64_t hash;

    if (strlen(vertexFile) >= MAX_GDR_PATH) {
        ri.Con_Printf(WARNING, "R_InitShader: vertexFile too long");
        return NULL;
    }
    if (strlen(fragmentFile) >= MAX_GDR_PATH) {
        ri.Con_Printf(WARNING, "R_InitShader: fragmentFile too long");
        return NULL;
    }

    shader = (shader_t *)ri.Z_Malloc(sizeof(*shader), TAG_RENDERER, &shader, "GLshader");
    stbsp_sprintf(filepath, "%s%s", vertexFile, fragmentFile);

    hash = Com_GenerateHashValue(filepath, MAX_SHADER_HASH);
    memset(shader->uniformCache, -1, sizeof(shader->uniformCache));

    shader->vertexBufLen = ri.FS_LoadFile(vertexFile, (void **)&shader->vertexBuf);
    shader->fragmentBufLen = ri.FS_LoadFile(fragmentFile, (void **)&shader->fragmentBuf);

    // compile
    vertid = R_CompileShaderSource(shader->vertexBuf, GL_VERTEX_SHADER, shader->programId);
    fragid = R_CompileShaderSource(shader->fragmentBuf, GL_FRAGMENT_SHADER, shader->programId);

    ri.FS_FreeFile(shader->fragmentBuf);
    ri.FS_FreeFile(shader->vertexBuf);

    shader->programId = nglCreateProgram();

    // link
    nglAttachShader(shader->programId, vertid);
    nglAttachShader(shader->programId, fragid);
    nglLinkProgram(shader->programId);
    nglValidateProgram(shader->programId);

    nglUseProgram(shader->programId);

    // error checks
    nglGetProgramiv(shader->programId, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        memset(str, 0, sizeof(str));
        nglGetProgramInfoLog(shader->programId, sizeof(str), NULL, str);
        
        nglDeleteShader(vertid);
        nglDeleteShader(fragid);
        nglDeleteProgram(shader->programId);

        ri.N_Error("R_InitShader: failed to compile and/or link shader program (fragment: %s, vertex: %s).\n"
                    "glslang error message: %s", fragmentFile, vertexFile, str);
    }

    shader->vertexFile = ri.Z_Strdup(vertexFile);
    shader->fragmentFile = ri.Z_Strdup(fragmentFile);

    // cleanup
    nglDeleteShader(vertid);
    nglDeleteShader(fragid);

    nglUseProgram(0);

    shaders[hash] = shader;
    rg.shaders[rg.numShaders] = shader;
    rg.numShaders++;

    return shader;
}

extern "C" void R_ShutdownShader(shader_t *shader)
{
    nglDeleteProgram(shader->programId);
    ri.Z_Free(shader->fragmentFile);
    ri.Z_Free(shader->vertexFile);
    ri.Z_Free(shader);
}

extern "C" void RE_ShutdownShaders(void)
{
    uint32_t i;

    for (i = 0; i < MAX_SHADER_HASH; ++i) {
        if (shaders[i])
            nglDeleteProgram(shaders[i]->programId);
    }
}

extern "C" void R_BindShader(const shader_t *shader)
{
    if (backend.shaderId == shader->programId)
        return; // already bound
    else if (backend.shaderId)
        nglUseProgram(0); // unbind whatever's being used
    
    backend.shaderId = shader->programId;
    nglUseProgram(shader->programId);
}

extern "C" void R_UnbindShader(void)
{
    if (!backend.shaderId)
        return; // already unbound
    
    backend.shaderId = 0;
    nglUseProgram(0);
}

#if 0

GLuint MinFilterString(const char *s)
{
    if (!N_stricmp(s, "GL_LINEAR"))
        return GL_LINEAR;
    else if (!N_stricmp(s, ))
}

typedef struct
{
    char texChunk[MAX_GDR_PATH];

    uint32_t minFilter;
    uint32_t magFilter;
    uint32_t wrapS;
    uint32_t wrapT;
} texShader_t;

void R_ParseTexShader(const char **buf, const char *name)
{
    const char *tok;
    const char *name;
    texShader_t *shader;
    
    tok = COM_ParseExt(buf, qfalse);
    shader = (texShader_t *)ri.Z_Malloc(sizeof(*shader), TAG_RENDERER, &shader, "texShader");

    while (tok[0]) {
        if (!N_stricmp(tok, "minfilter")) {

        }

        tok = COM_ParseExt(buf, qfalse);
    }
}

void R_ParseShader(const char *buffer)
{
    const char *tok, **buf;

    buf = &buffer;
    tok = COM_ParseExt(buf, qfalse);

    while (tok[0]) {
        // a texture chunk
        if (strstr(tok, "/") != NULL) {
            R_ParseTexShader(buffer);
        }

        tok = COM_ParseExt(buf, qfalse);
    }
}

void RE_InitShaders(void)
{
    char **shaderList, **shaderBufs;
    uint64_t numShaders;

    shaderList = ri.FS_ListFiles("shaders", ".shader", &numShaders);
    shaderBufs = (char **)ri.Hunk_AllocateTempMemory(sizeof(*shaderBufs) * numShaders);

    for (uint64_t i = 0; i < numShaders; i++) {
        ri.FS_LoadFile(shaderList[i], &shaderBufs[i]);
    }
}
#endif

#if 0
inline static uint32_t ShaderTypeFromString(const eastl::string& type)
{
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    else if (type == "fragment")
        return GL_FRAGMENT_SHADER;
    return 0;
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
#endif