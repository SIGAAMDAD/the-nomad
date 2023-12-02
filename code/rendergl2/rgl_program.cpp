#include "rgl_local.h"

extern const char *fallbackShader_basic_vp;
extern const char *fallbackShader_basic_fp;
extern const char *fallbackShader_imgui_vp;
extern const char *fallbackShader_imgui_fp;
extern const char *fallbackShader_ssao_vp;
extern const char *fallbackShader_ssao_fp;

#define GLSL_VERSION_ATLEAST(major,minor) (glContext.glslVersionMajor > (major) || (glContext.versionMajor == (major) && glContext.glslVersionMinor >= minor))

typedef struct {
    const char *name;
    uint64_t type;
} uniformInfo_t;

// these must in the same order as in uniform_t in rgl_local.h
static uniformInfo_t uniformsInfo[] = {
    {"u_DiffuseMap", GLSL_INT},
    {"u_LightMap", GLSL_INT},
    {"u_NormalMap", GLSL_INT},
    {"u_SpecularMap", GLSL_INT},
    {"u_NumLights", GLSL_INT},
    {"u_LightInfo", GLSL_BUFFER},
    {"u_AmbientLight", GLSL_FLOAT},
    {"u_ModelViewProjection", GLSL_MAT16},
    {"u_ModelMatrix", GLSL_MAT16},
    {"u_SpecularScale", GLSL_VEC4},
    {"u_NormalScale", GLSL_VEC4},
    {"u_ColorGen", GLSL_INT},
    {"u_AlphaGen", GLSL_INT},
    {"u_Color", GLSL_VEC4},
    {"u_BaseColor", GLSL_VEC4},
    {"u_VertColor", GLSL_VEC4},
    {"u_AlphaTest", GLSL_INT},
    {"u_TCGen", GLSL_VEC4},
    {"u_ViewInfo", GLSL_VEC4}
};

static CShaderProgram *hashTable[MAX_RENDER_SHADERS];

#define SHADER_CACHE_FILE_NAME "shadercache.dat"

typedef struct {
    char name[MAX_GDR_PATH];
    uint32_t fmt;
    void *data;
    uint64_t size;
} shaderCacheEntry_t;

static shaderCacheEntry_t *cacheHashTable;

//
// R_InitShaderCache
//
static void R_InitShaderCache(void)
{
    cacheHashTable = (shaderCacheEntry_t *)ri.Malloc(sizeof(*cacheHashTable) * rg.numPrograms);
    memset(cacheHashTable, 0, sizeof(*cacheHashTable) * rg.numPrograms);

    for (uint64_t i = 0; i < rg.numPrograms; i++) {

    }
}

static void R_LoadShaderCache(void)
{
    file_t f;
    uint64_t numEntries, i;
    uint64_t hash;
    char name[MAX_GDR_PATH];
    shaderCacheEntry_t *entry;

    f = ri.FS_FOpenRead(SHADER_CACHE_FILE_NAME);
    if (f == FS_INVALID_HANDLE) {
        if (ri.FS_FileExists(SHADER_CACHE_FILE_NAME)) {
            ri.Error(ERR_DROP, "Failed to load shader cache file even though it exists");
        }
        else {
            ri.Printf(PRINT_INFO, "Failed to load shader cache file, probably doesn't exist yet\n");
        }
    }

    if (!ri.FS_Read(&numEntries, sizeof(uint64_t), f)) {
        ri.FS_FClose(f);
        ri.Printf(PRINT_DEVELOPER, "Error reading shader cache numEntries\n");
    }
    if (!numEntries) {
        ri.Error(ERR_DROP, "R_LoadShaderCache: numEntries is a funny number");
    }

    cacheHashTable = (shaderCacheEntry_t *)ri.Hunk_Alloc(sizeof(*cacheHashTable) * numEntries, h_low);

    for (i = 0; i < numEntries; i++) {
        if (!ri.FS_Read(name, sizeof(name), f)) {
            ri.FS_FClose(f);
            ri.Printf(PRINT_DEVELOPER, "Error reading shader cache entry name at %lu\n", i);
        }
        
        hash = Com_GenerateHashValue(name, MAX_RENDER_SHADERS);
        entry = &cacheHashTable[hash];
        memcpy(entry->name, name, sizeof(entry->name));

        if (!ri.FS_Read(&entry->size, sizeof(uint64_t), f)) {
            ri.FS_FClose(f);
            ri.Printf(PRINT_DEVELOPER, "Error reading shader cache entry size at %lu\n", i);
        }

        entry->data = ri.Hunk_Alloc(entry->size, h_low);
        if (!ri.FS_Read(entry->data, entry->size, f)) {
            ri.FS_FClose(f);
            ri.Printf(PRINT_DEVELOPER, "Error reading shader cache entry buffer at %lu\n", i);
        }
    }

    ri.FS_FClose(f);
}

static void R_SaveShaderToCache(const CShaderProgram *program, file_t cacheFile)
{
    shaderCacheEntry_t entry;
    GLint length;
    GLenum fmt;

    if (glContext.ARB_gl_spirv) {
        return;
    }

    memset(&entry, 0, sizeof(entry));

    nglGetProgramiv(program->GetProgram(), GL_PROGRAM_BINARY_LENGTH, &length);

    entry.data = ri.Hunk_AllocateTempMemory(length);
    entry.size = length;

    nglGetProgramBinary(program->GetProgram(), length, NULL, (GLenum *)&entry.fmt, (char *)entry.data + sizeof(GLenum));
    
    N_strncpyz(entry.name, program->GetName(), sizeof(entry.name));
    ri.FS_Write(entry.name, sizeof(entry.name), cacheFile);
    ri.FS_Write(&entry.size, sizeof(uint64_t), cacheFile);
    ri.FS_Write(&entry.fmt, sizeof(uint32_t), cacheFile);
    ri.FS_Write(entry.data, entry.size, cacheFile);

    ri.Hunk_FreeTempMemory(entry.data);
}

static int R_ShaderSavedToCache(const char *name)
{

}

typedef enum {
	GLSL_PRINTLOG_PROGRAM_INFO,
	GLSL_PRINTLOG_SHADER_INFO,
	GLSL_PRINTLOG_SHADER_SOURCE
} glslPrintLog_t;

static void GLSL_PrintLog(GLuint programOrShader, glslPrintLog_t type, qboolean developerOnly)
{
    char            *msg;
	static char     msgPart[8192];
	GLsizei         maxLength = 0;
	uint32_t        i;
	const int       printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_INFO;

	switch (type) {
	case GLSL_PRINTLOG_PROGRAM_INFO:
		ri.Printf(printLevel, "Program info log:\n");
		nglGetProgramiv(programOrShader, GL_INFO_LOG_LENGTH, &maxLength);
		break;
	case GLSL_PRINTLOG_SHADER_INFO:
		ri.Printf(printLevel, "Shader info log:\n");
		nglGetShaderiv(programOrShader, GL_INFO_LOG_LENGTH, &maxLength);
		break;
	case GLSL_PRINTLOG_SHADER_SOURCE:
		ri.Printf(printLevel, "Shader source:\n");
		nglGetShaderiv(programOrShader, GL_SHADER_SOURCE_LENGTH, &maxLength);
		break;
	};

	if (maxLength <= 0) {
		ri.Printf(printLevel, "None.\n");
		return;
	}

	if (maxLength < 1023)
		msg = msgPart;
	else
		msg = (char *)ri.Malloc(maxLength);

	switch (type) {
	case GLSL_PRINTLOG_PROGRAM_INFO:
		nglGetProgramInfoLog(programOrShader, maxLength, &maxLength, msg);
		break;
	case GLSL_PRINTLOG_SHADER_INFO:
		nglGetShaderInfoLog(programOrShader, maxLength, &maxLength, msg);
		break;
	case GLSL_PRINTLOG_SHADER_SOURCE:
		nglGetShaderSource(programOrShader, maxLength, &maxLength, msg);
		break;
	};

    ri.Printf(printLevel, "%s", msg);

	if (maxLength < 1023) {
		msgPart[maxLength + 1] = '\0';

		ri.Printf(printLevel, "%s\n", msgPart);
	}
	else {
		for(i = 0; i < maxLength; i += 1023) {
			N_strncpyz(msgPart, msg + i, sizeof(msgPart));

			ri.Printf(printLevel, "%s", msgPart);
		}

		ri.Printf(printLevel, "\n");
		ri.Free(msg);
    }
}

GDR_EXPORT int CShaderProgram::CompileGPUShader(GLuint *prevShader, const GLchar *buffer, uint64_t size, GLenum shaderType, const char *programName)
{
    GLint compiled;
    GLuint shader;

    // create shader
    shader = nglCreateShader(shaderType);

    if (shaderType == GL_VERTEX_SHADER)
        GL_SetObjectDebugName(GL_SHADER, shader, programName, "_vertexShader");
    else if (shaderType == GL_FRAGMENT_SHADER)
        GL_SetObjectDebugName(GL_SHADER, shader, programName, "_fragmentShader");
    else if (shaderType == GL_GEOMETRY_SHADER)
        GL_SetObjectDebugName(GL_SHADER, shader, programName, "_geometryShader");

    // give it the source
    nglShaderSource(shader, 1, (const GLchar **)&buffer, (const GLint *)&size);

    // compile
    nglCompileShader(shader);

    // check if shader compiled
    nglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLSL_PrintLog(shader, GLSL_PRINTLOG_SHADER_INFO, qfalse);
        ri.Error(ERR_DROP, "Failed to compiled shader");
        return qfalse;
    }

    if (*prevShader) {
        nglDetachShader(m_ProgramId, *prevShader);
        nglDeleteShader(*prevShader);
    }

    // attach shader to program
    nglAttachShader(m_ProgramId, shader);

    *prevShader = shader;

    return qtrue;
}

GDR_EXPORT void CShaderProgram::LinkProgram( void )
{
	GLint linked;

	nglLinkProgram( m_ProgramId );

	nglGetProgramiv(m_ProgramId, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLSL_PrintLog(m_ProgramId, GLSL_PRINTLOG_PROGRAM_INFO, qfalse);
		ri.Error(ERR_DROP, "shaders failed to link");
	}
}

GDR_EXPORT int CShaderProgram::LoadGPUShaderText(const char *name, const char *fallback, GLenum shaderType, char *dest, uint64_t destSize)
{
    char filename[MAX_GDR_PATH];
    GLchar *buffer = NULL;
    const GLchar *shaderText = NULL;
    uint64_t size;

    if (shaderType == GL_VERTEX_SHADER) {
        Com_snprintf(filename, sizeof(filename), "glsl/%s_vs.glsl", name);
    }
    else {
        Com_snprintf(filename, sizeof(filename), "glsl/%s_fs.glsl", name);
    }

    if (r_externalGLSL->i) {
        size = ri.FS_LoadFile(filename, (void **)&buffer);
    }
    else {
        size = 0;
        buffer = NULL;
    }

    if (!buffer) {
        if (fallback) {
            ri.Printf(PRINT_DEVELOPER, "...loading built-in '%s'\n", filename);
            shaderText = fallback;
            size = strlen(shaderText);
        }
        else {
            ri.Printf(PRINT_DEVELOPER, "couldn't load '%s'\n", filename);
            return qfalse;
        }
    }
    else {
        ri.Printf(PRINT_DEVELOPER, "...loaded '%s'\n", filename);
        shaderText = buffer;
    }

    if (size > destSize) {
        return qfalse;
    }

    N_strncpyz(dest, shaderText, size + 1);
    if (buffer)
        ri.FS_FreeFile(buffer);

    return qtrue;
}

GDR_EXPORT void CShaderProgram::ShowProgramUniforms( void )
{
	uint32_t        i;
    int32_t         count, size;
	GLenum			type;
	char            uniformName[1000];

	// query the number of active uniforms
	nglGetProgramiv(m_ProgramId, GL_ACTIVE_UNIFORMS, &count);

	// Loop over each of the active uniforms, and set their value
	for (i = 0; i < count; i++) {
		nglGetActiveUniform(m_ProgramId, i, sizeof(uniformName), NULL, &size, &type, uniformName);

		ri.Printf(PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName);
	}
}

GDR_EXPORT void GLSL_PrepareHeader(GLenum shaderType, const GLchar *extra, char *dest, uint64_t size)
{
    dest[0] = '\0';

    // OpenGL version from 3.3 and up have corresponding glsl versions
    if (NGL_VERSION_ATLEAST(3, 30)) {
        N_strcat(dest, size, "#version 330 core\n");
    }
    // otherwise, do the Quake3e method
    else if (GLSL_VERSION_ATLEAST(1, 30)) {
        if (GLSL_VERSION_ATLEAST(1, 50)) {
            N_strcat(dest, size, "#version 150\n");
        }
        else if (GLSL_VERSION_ATLEAST(1, 30)) {
            N_strcat(dest, size, "#verrsion 130\n");
        }
    }
    else {
        N_strcat(dest, size, "#define GLSL_LEGACY\n");
        N_strcat(dest, size, "#version 120\n");
    }

    if (!(NGL_VERSION_ATLEAST(1, 30))) {
        if (shaderType == GL_VERTEX_SHADER) {
            N_strcat(dest, size, "#define in attribute\n");
            N_strcat(dest, size, "#define out varying\n");
        }
        else {
            N_strcat(dest, size, "#define a_Color gl_FragColor\n");
            N_strcat(dest, size, "#define in varying\n");
            N_strcat(dest, size, "#define texture texture2D\n"); // texture2D is deprecated in modern GLSL
        }
    }

    N_strcat(dest, size, "#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n");

    // OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	N_strcat(dest, size, "#line 0\n");
}

GDR_EXPORT void GLSL_CheckAttribLocation(GLuint id, const char *name, const char *attribName, int index)
{
    GLint location;

    location = nglGetAttribLocation(id, name);
    if (location != index) {
        ri.Error(ERR_FATAL, "GetAttribLocation(%s):%i != %s (%i)", name, location, attribName, index);
    }
}

GDR_EXPORT int GLSL_InitGPUShader2(CShaderProgram *program, const char *name, uint32_t attribs, const char *vsCode, const char *fsCode)
{
    ri.Printf(PRINT_DEVELOPER, "---------- GPU Shader ----------\n");

    if (strlen(name) >= MAX_GDR_PATH) {
        ri.Error(ERR_DROP, "GLSL_InitGPUShader2: \"%s\" is too long", name);
    }

    program->SetName( name );

    program->InitProgram();
    program->SetAttribBits( attribs );

    GL_SetObjectDebugName(GL_PROGRAM, program->GetProgram(), name, "_program");

    if (!(program->CompileGPUShader(program->GetVertexId(), vsCode, strlen(vsCode), GL_VERTEX_SHADER, name))) {
        ri.Printf(PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_VERTEX_SHADER\n", name);
        program->DeleteProgram();
        return qfalse;
    }

    if (fsCode) {
        if (!(program->CompileGPUShader(program->GetFragmentId(), fsCode, strlen(fsCode), GL_FRAGMENT_SHADER, name))) {
            ri.Printf(PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_FRAGMENT_SHADER\n", name);
            program->DeleteProgram();
            return qfalse;
        }
    }

    if (attribs & ATTRIB_POSITION)
        nglBindAttribLocation(program->GetProgram(), ATTRIB_INDEX_POSITION, "a_Position");
    if (attribs & ATTRIB_TEXCOORD)
        nglBindAttribLocation(program->GetProgram(), ATTRIB_INDEX_TEXCOORD, "a_TexCoord");
    if (attribs & ATTRIB_COLOR)
        nglBindAttribLocation(program->GetProgram(), ATTRIB_INDEX_COLOR, "a_Color");
//    if (attribs & ATTRIB_NORMAL)
//        nglBindAttribLocation(program->GetProgram(), ATTRIB_INDEX_NORMAL, "a_Normal");
    
    program->LinkProgram();

//    GLSL_CheckAttribLocation(program->GetProgram(), "a_Position", "ATTRIB_INDEX_POSITION", ATTRIB_INDEX_POSITION);
//    GLSL_CheckAttribLocation(program->GetProgram(), "a_TexCoord", "ATTRIB_INDEX_TEXCOORD", ATTRIB_INDEX_TEXCOORD);
//    GLSL_CheckAttribLocation(program->GetProgram(), "a_Normal", "ATTRIB_INDEX_NORMAL", ATTRIB_INDEX_NORMAL);
//    GLSL_CheckAttribLocation(program->GetProgram(), "a_Color", "ATTRIB_INDEX_COLOR", ATTRIB_INDEX_COLOR);

    return qtrue;
}

GDR_EXPORT int GLSL_InitGPUShader(CShaderProgram *program, const char *name, uint32_t attribs, qboolean fragmentShader,
    const GLchar *extra, qboolean addHeader, const char *fallback_vs, const char *fallback_fs)
{
    char vsCode[32000];
    char fsCode[32000];
    char *postHeader;
    uint64_t size;

    rg.programs[rg.numPrograms] = program;
    rg.numPrograms++;

    size = sizeof(vsCode);

    if (addHeader) {
        GLSL_PrepareHeader(GL_VERTEX_SHADER, extra, vsCode, size);
        postHeader = &vsCode[strlen(vsCode)];
        size -= strlen(vsCode);
    }
    else {
        postHeader = &vsCode[0];
    }

    if (!program->LoadGPUShaderText(name, fallback_vs, GL_VERTEX_SHADER, postHeader, size)) {
        return qfalse;
    }

    if (fragmentShader) {
		size = sizeof(fsCode);

		if (addHeader) {
			GLSL_PrepareHeader(GL_FRAGMENT_SHADER, extra, fsCode, size);
			postHeader = &fsCode[strlen(fsCode)];
			size -= strlen(fsCode);
		}
		else {
			postHeader = &fsCode[0];
		}

		if (!program->LoadGPUShaderText(name, fallback_fs, GL_FRAGMENT_SHADER, postHeader, size)) {
			return qfalse;
		}
	}

    return GLSL_InitGPUShader2(program, name, attribs, vsCode, fsCode);
}

GDR_EXPORT void CShaderProgram::InitUniforms( void )
{
	uint32_t i, size;

	GLint *uniforms = m_Uniforms;
	size = 0;

	for (i = 0; i < UNIFORM_COUNT; i++) {
		uniforms[i] = nglGetUniformLocation(m_ProgramId, uniformsInfo[i].name);

		if (uniforms[i] == -1)
			continue;
		 
		m_UniformBufferOffsets[i] = size;

		switch (uniformsInfo[i].type) {
		case GLSL_INT:
			size += sizeof(GLint);
			break;
		case GLSL_FLOAT:
			size += sizeof(GLfloat);
			break;
		case GLSL_VEC2:
			size += sizeof(vec_t) * 2;
			break;
		case GLSL_VEC3:
			size += sizeof(vec_t) * 3;
			break;
		case GLSL_VEC4:
			size += sizeof(vec_t) * 4;
			break;
		case GLSL_MAT16:
			size += sizeof(vec_t) * 16;
			break;
		default:
			break;
		};
	}

	m_pUniformBuffer = (char *)ri.Malloc(size);
}

GDR_EXPORT void GLSL_FinishGPUShader(CShaderProgram *program)
{
    program->ShowProgramUniforms();
	GL_CheckErrors();
}

GDR_EXPORT void CShaderProgram::Shutdown( void )
{
    if (m_ProgramId) {
        if (m_VertexId) {
            nglDetachShader(m_ProgramId, m_VertexId);
            nglDeleteShader(m_VertexId);
        }
        if (m_FragmentId) {
            nglDetachShader(m_ProgramId, m_FragmentId);
            nglDeleteShader(m_FragmentId);
        }
        nglDeleteProgram(m_ProgramId);

        if (m_pUniformBuffer) {
            ri.Free( m_pUniformBuffer );
        }

        memset( this, 0, sizeof(*this) );
    }
}

GDR_EXPORT void CShaderProgram::SetUniformInt(uint32_t uniformNum, GLint value)
{
    GLint *compare = (GLint *)(m_pUniformBuffer + m_pUniformBuffer[uniformNum]);

    if (m_Uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_INT) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformInt: wrong type for uniform %i in program %s\n", uniformNum, m_szName);
        return;
    }
    if (value == *compare)
        return;
    
    *compare = value;
    nglUniform1i(m_Uniforms[uniformNum], value);
}

GDR_EXPORT void CShaderProgram::SetUniformFloat(uint32_t uniformNum, GLfloat value)
{
    GLfloat *compare = (GLfloat *)(m_pUniformBuffer + m_pUniformBuffer[uniformNum]);

    if (m_Uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_FLOAT) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformFloat: wrong type for uniform %i in program %s\n", uniformNum, m_szName);
        return;
    }
    if (value == *compare)
        return;
    
    *compare = value;
    nglUniform1f(m_Uniforms[uniformNum], value);
}

GDR_EXPORT void CShaderProgram::SetUniformVec2(uint32_t uniformNum, const glm::vec2& v)
{
    glm::vec2 *compare = (glm::vec2 *)(m_pUniformBuffer + m_pUniformBuffer[uniformNum]);

    if (m_Uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC2) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec2: wrong type for uniform %i in program %s\n", uniformNum, m_szName);
        return;
    }
    if (*compare == v) {
        return;
    }
    
    *compare = v;
    nglUniform2f(m_Uniforms[uniformNum], v[0], v[1]);
}

GDR_EXPORT void CShaderProgram::SetUniformVec3(uint32_t uniformNum, const glm::vec3& v)
{
    glm::vec3 *compare = (glm::vec3 *)(m_pUniformBuffer + m_pUniformBuffer[uniformNum]);

    if (m_Uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC3) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec3: wrong type for uniform %i in program %s\n", uniformNum, m_szName);
        return;
    }
    if (*compare == v) {
        return;
    }
    
    *compare = v;
    nglUniform3f(m_Uniforms[uniformNum], v[0], v[1], v[2]);
}

GDR_EXPORT void CShaderProgram::SetUniformVec4(uint32_t uniformNum, const glm::vec4& v)
{
    glm::vec4 *compare = (glm::vec4 *)(m_pUniformBuffer + m_pUniformBuffer[uniformNum]);

    if (m_Uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC4) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec4: wrong type for uniform %i in program %s\n", uniformNum, m_szName);
        return;
    }
    if (*compare == v)
        return;
    
    *compare = v;
    nglUniform4f(m_Uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

GDR_EXPORT void CShaderProgram::SetUniformMatrix4(uint32_t uniformNum, const glm::mat4& m)
{
    glm::mat4* compare = (glm::mat4 *)(m_pUniformBuffer + m_pUniformBuffer[uniformNum]);

    if (m_Uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_MAT16) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformMatrix4: wrong type for uniform %i in program %s\n", uniformNum, m_szName);
        return;
    }
    if (m == *compare) {
        return;
    }
    
    *compare = m;
    nglUniformMatrix4fv(m_Uniforms[uniformNum], 1, GL_FALSE, (const GLfloat *)glm::value_ptr( m ));
}


GDR_EXPORT void GLSL_InitGPUShaders(void)
{
    uint64_t start, end;
    uint64_t i;
    uint32_t attribs;

    rg.numPrograms = 0;

    ri.Printf(PRINT_INFO, "---- GLSL_InitGPUShaders ----\n");

    R_IssuePendingRenderCommands();

    start = ri.Milliseconds();

    attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD | ATTRIB_COLOR | ATTRIB_NORMAL;
    if (!GLSL_InitGPUShader(&rg.basicShader, "basic", attribs, qtrue, NULL, qtrue, fallbackShader_basic_vp, fallbackShader_basic_fp)) {
        ri.Error(ERR_FATAL, "Could not load basic shader");
    }
    rg.basicShader.InitUniforms();
    GLSL_FinishGPUShader(&rg.basicShader);

    attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD | ATTRIB_COLOR;
    if (!GLSL_InitGPUShader(&rg.imguiShader, "imgui", attribs, qtrue, NULL, qtrue, fallbackShader_imgui_vp, fallbackShader_imgui_fp)) {
        ri.Error(ERR_FATAL, "Could not load imgui shader");
    }
    rg.imguiShader.InitUniforms();
    GLSL_FinishGPUShader(&rg.imguiShader);

    end = ri.Milliseconds();

    ri.Printf(PRINT_INFO, "...loaded %lu GLSL shaders in %5.2f seconds\n",
        rg.numPrograms, (end - start) / 1000.0);
}

GDR_EXPORT void GLSL_ShutdownGPUShaders(void)
{
    uint32_t i;

    ri.Printf(PRINT_INFO, "---------- GLSL_ShutdownGPUShaders -----------\n");

    for (i = 0; i < ATTRIB_INDEX_COUNT; i++) {
        nglDisableVertexAttribArray(i);
    }
    
    GL_BindNullProgram();

    rg.basicShader.Shutdown();
}

GDR_EXPORT void GLSL_UseProgram(CShaderProgram *program)
{
    GLuint programObject = program ? program->GetProgram() : 0;

    if (GL_UseProgram(programObject)) {
        backend.pc.c_glslShaderBinds++;
        glState.currentShader = program;
    }
}
