#include "rgl_local.h"

extern const char *fallbackShader_basic_vs;
extern const char *fallbackShader_basic_fs;
extern const char *fallbackShader_ssao_vs;
extern const char *fallbackShader_ssao_fs;

#define GLSL_VERSION_ATLEAST(major,minor) (glContext->glslVersionMajor > (major) || (glContext->versionMajor == (major) && glContext->glslVersionMinor >= minor))

typedef struct {
    const char *name;
    uint64_t type;
} uniformInfo_t;

// these must in the same order as in uniform_t in rgl_local.h
static uniformInfo_t uniformsInfo[] = {
    {"u_ModelViewProjection", GLSL_MAT16},
    {"u_DiffuseMap", GLSL_INT},
    {"u_NormalMap", GLSL_INT},
    {"u_BaseColor", GLSL_VEC4},
    {"u_VertColor", GLSL_VEC4},
    {"u_Color", GLSL_VEC4},
    {"u_AmbientLight", GLSL_FLOAT},
    {"u_NumLights", GLSL_INT},
    {"u_Lights", GLSL_BUFFER},
};

static shaderProgram_t *hashTable[MAX_RENDER_SHADERS];

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
    cacheHashTable = ri.Malloc(sizeof(*cacheHashTable) * rg.numPrograms);
    memset(cacheHashTable, 0, sizeof(cacheHashTable));

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

    cacheHashTable = ri.Hunk_Alloc(sizeof(*cacheHashTable) * numEntries, h_low);

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

static void R_SaveShaderToCache(const shaderProgram_t *program, file_t cacheFile)
{
    shaderCacheEntry_t entry;
    GLint length;
    GLenum fmt;

    if (glContext->ARB_gl_spirv) {
        return;
    }

    memset(&entry, 0, sizeof(entry));

    nglGetProgramiv(program->programId, GL_PROGRAM_BINARY_LENGTH, &length);

    entry.data = ri.Hunk_AllocateTempMemory(length);
    entry.size = length;

    nglGetProgramBinary(program->programId, length, NULL, (GLenum *)&entry.fmt, (char *)entry.data + sizeof(GLenum));
    
    N_strncpyz(entry.name, program->name, sizeof(entry.name));
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
	char           *msg;
	static char     msgPart[1024];
	uint32_t        maxLength = 0;
	uint32_t        i;
	uint32_t        printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_INFO;

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
		msg = ri.Malloc(maxLength);

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

static int GLSL_CompileGPUShader(GLuint program, GLuint *prevShader, const GLchar *buffer, uint64_t size, GLenum shaderType)
{
    GLuint compiled;
    GLuint shader;

    // create shader
    shader = nglCreateShader(shaderType);

    // compile shader
    nglShaderSource(shader, 1, (const GLchar **)&buffer, (const GLint *)&size);

    // check if shader compiled
    nglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLSL_PrintLog(shader, GLSL_PRINTLOG_SHADER_SOURCE, qfalse);
        GLSL_PrintLog(shader, GLSL_PRINTLOG_SHADER_INFO, qfalse);
        ri.Error(ERR_DROP, "Failed to compiled shader");
        return qfalse;
    }

    if (*prevShader) {
        nglDetachShader(program, *prevShader);
        nglDeleteShader(*prevShader);
    }

    // attach shader to program
    nglAttachShader(program, shader);

    *prevShader = shader;

    return qtrue;
}

static void GLSL_LinkProgram(GLuint program)
{
	GLint linked;

	nglLinkProgram(program);

	nglGetProgramiv(program, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLSL_PrintLog(program, GLSL_PRINTLOG_PROGRAM_INFO, qfalse);
		ri.Error(ERR_DROP, "shaders failed to link");
	}
}

static int GLSL_LoadGPUShaderText(const char *name, const char *fallback, GLenum shaderType, char *dest, uint64_t destSize)
{
    char filename[MAX_GDR_PATH];
    GLchar *buffer = NULL;
    const GLchar *shaderText = NULL;
    uint64_t size;

    if (shaderType == GL_VERTEX_SHADER) {
        Com_snprintf(filename, sizeof(filename), "glsl/%s.vs.glsl", name);
    }
    else {
        Com_snprintf(filename, sizeof(filename), "glsl/f%s.fs.glsl", name);
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

static void GLSL_ShowProgramUniforms(GLuint program)
{
	uint32_t        i, count, size;
	GLenum			type;
	char            uniformName[1000];

	// query the number of active uniforms
	nglGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

	// Loop over each of the active uniforms, and set their value
	for (i = 0; i < count; i++) {
		nglGetActiveUniform(program, i, sizeof(uniformName), NULL, &size, &type, uniformName);

		ri.Printf(PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName);
	}
}


static void GLSL_PrepareHeader(GLenum shaderType, const GLchar *extra, char *dest, uint64_t size)
{
    dest[0] = '\0';

        // OpenGL version from 3.3 and up have corresponding glsl versions
    if (NGL_VERSION_ATLEAST(3, 30)) {
        N_strcat(dest, size, va("#version %s core", glContext->glsl_version_str));
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

    if (NGL_VERSION_ATLEAST(1, 30)) {
        if (shaderType == GL_VERTEX_SHADER) {
            N_strcat(dest, size, "#define attribute in\n");
            N_strcat(dest, size, "#define varying out\n");
        }
        else {
            N_strcat(dest, size, "out vec4 a_Color\n");
            N_strcat(dest, size, "#define gl_FragColor a_Color\n");
            N_strcat(dest, size, "#define varying in\n");
            N_strcat(dest, size, "#define texture2D texture\n"); // texture2D is deprecated in modern OpenGL
        }
    }

    N_strcat(dest, size, "#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n");

    // OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	N_strcat(dest, size, "#line 0\n");
}

static int GLSL_InitGPUShader2(shaderProgram_t *program, const char *name, uint32_t attribs, uint64_t size, const char *vsCode, const char *fsCode)
{
    ri.Printf(PRINT_DEVELOPER, "---------- GPU Shader ----------\n");

    if (strlen(name) >= MAX_GDR_PATH) {
        ri.Error(ERR_DROP, "GLSL_InitGPUShader2: \"%s\" is too long", name);
    }

    N_strncpyz(program->name, name, sizeof(program->name));

    program->programId = nglCreateProgram();
    program->attribBits = attribs;

    if (!(GLSL_CompileGPUShader(program->programId, &program->vertexId, vsCode, strlen(vsCode), GL_VERTEX_SHADER))) {
        ri.Printf(PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_VERTEX_SHADER\n", name);
        nglDeleteProgram(program->programId);
        return qfalse;
    }

    if (fsCode) {
        if (!(GLSL_CompileGPUShader(program->programId, &program->fragmentId, fsCode, strlen(fsCode), GL_FRAGMENT_SHADER))) {
            ri.Printf(PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_FRAGMENT_SHADER\n", name);
            nglDeleteProgram(program->programId);
            return qfalse;
        }
    }

    if (attribs & ATTRIB_POSITION)
        nglBindAttribLocation(program->programId, ATTRIB_INDEX_POSITION, "a_Position");
    if (attribs & ATTRIB_TEXCOORD)
        nglBindAttribLocation(program->programId, ATTRIB_INDEX_TEXCOORD, "a_TexCoords");
    if (attribs & ATTRIB_COLOR)
        nglBindAttribLocation(program->programId, ATTRIB_INDEX_COLOR, "a_Color");
    if (attribs & ATTRIB_ALPHA)
        nglBindAttribLocation(program->programId, ATTRIB_INDEX_ALPHA, "a_Alpha");
//    if (attribs & ATTRIB_NORMAL)
//        nglBindAttribLocation(program->programId, ATTRIB_INDEX_NORMAL, "a_Normal");
//    
    GLSL_LinkProgram(program->programId);
    
    return qtrue;
}

static int GLSL_InitGPUShader(shaderProgram_t *program, const char *name, uint32_t attribs, qboolean fragmentShader,
    const GLchar *extra, qboolean addHeader, const char *fallback_vs, const char *fallback_fs)
{
    char vsCode[32000];
    char fsCode[32000];
    char *postHeader;
    uint64_t size;

    size = sizeof(vsCode);

    if (addHeader) {
        GLSL_PrepareHeader(GL_VERTEX_SHADER, extra, vsCode, size);
        postHeader = &vsCode[strlen(vsCode)];
        size -= strlen(vsCode);
    }
    else {
        postHeader = &vsCode[0];
    }

    if (!GLSL_LoadGPUShaderText(name, fallback_vs, GL_VERTEX_SHADER, postHeader, size)) {
        return qfalse;
    }

    if (fragmentShader) {
		size = sizeof(fsCode);

		if (addHeader) {
			GLSL_GetShaderHeader(GL_FRAGMENT_SHADER, extra, fsCode, size);
			postHeader = &fsCode[strlen(fsCode)];
			size -= strlen(fsCode);
		}
		else {
			postHeader = &fsCode[0];
		}

		if (!GLSL_LoadGPUShaderText(name, fallback_fs, GL_FRAGMENT_SHADER, postHeader, size)) {
			return qfalse;
		}
	}

    return qtrue;
}

static void GLSL_InitUniforms(shaderProgram_t *program)
{
	uint32_t i, size;

	GLint *uniforms = program->uniforms;
	size = 0;

	for (i = 0; i < UNIFORM_COUNT; i++) {
		uniforms[i] = nglGetUniformLocation(program->programId, uniformsInfo[i].name);

		if (uniforms[i] == -1)
			continue;
		 
		program->uniformBufferOffsets[i] = size;

		switch(uniformsInfo[i].type) {
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

	program->uniformBuffer = ri.Malloc(size);
}

static void GLSL_FinishGPUShader(shaderProgram_t *program)
{
	GLSL_ShowProgramUniforms(program->programId);
	GL_CheckErrors();
}

static void GLSL_DeleteGPUShader(shaderProgram_t *program)
{
    if (program->programId) {
        if (program->vertexId) {
            nglDetachShader(program->programId, program->vertexId);
            nglDeleteShader(program->vertexId);
        }
        if (program->compressedVSCode) {
            ri.Free(program->compressedVSCode);
        }
        if (program->fragmentId) {
            nglDetachShader(program->programId, program->fragmentId);
            nglDeleteShader(program->fragmentId);
        }
        if (program->compressedFSCode) {
            ri.Free(program->compressedFSCode);
        }
        nglDeleteProgram(program->programId);

        if (program->uniformBuffer) {
            ri.Free(program->uniformBuffer);
        }

        memset(program, 0, sizeof(*program));
    }
}

void GLSL_SetUniformInt(shaderProgram_t *program, uint32_t uniformNum, GLint value)
{
    GLint *uniforms = program->uniforms;
    GLint *compare = (GLint *)(program->uniformBuffer + program->uniformBuffer[uniformNum]);

    if (uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_INT) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformInt: wrong type for uniform %i in program %s\n", uniformNum, program->name);
        return;
    }
    if (value == *compare)
        return;
    
    *compare = value;
    nglUniform1iARB(uniforms[uniformNum], value);
}

void GLSL_SetUniformFloat(shaderProgram_t *program, uint32_t uniformNum, GLfloat value)
{
    GLint *uniforms = program->uniforms;
    GLfloat *compare = (GLfloat *)(program->uniformBuffer + program->uniformBuffer[uniformNum]);

    if (uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_FLOAT) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformFloat: wrong type for uniform %i in program %s\n", uniformNum, program->name);
        return;
    }
    if (value == *compare)
        return;
    
    *compare = value;
    nglUniform1fARB(uniforms[uniformNum], value);
}

void GLSL_SetUniformVec2(shaderProgram_t *program, uint32_t uniformNum, const vec2_t v)
{
    GLint *uniforms = program->uniforms;
    vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBuffer[uniformNum]);

    if (uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC2) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec2: wrong type for uniform %i in program %s\n", uniformNum, program->name);
        return;
    }
    if (v[0] == compare[0] && v[1] == compare[1])
        return;
    
    compare[0] = v[0];
    compare[1] = v[1];
    nglUniform2fARB(uniforms[uniformNum], v[0], v[1]);
}

void GLSL_SetUniformVec3(shaderProgram_t *program, uint32_t uniformNum, const vec3_t v)
{
    GLint *uniforms = program->uniforms;
    vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBuffer[uniformNum]);

    if (uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC3) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec3: wrong type for uniform %i in program %s\n", uniformNum, program->name);
        return;
    }
    if (VectorCompare(v, compare))
        return;
    
    VectorCopy(compare, v);
    nglUniform3fARB(uniforms[uniformNum], v[0], v[1], v[2]);
}

void GLSL_SetUniformVec4(shaderProgram_t *program, uint32_t uniformNum, const vec4_t v)
{
    GLint *uniforms = program->uniforms;
    vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBuffer[uniformNum]);

    if (uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC4) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
        return;
    }
    if (VectorCompare4(v, compare))
        return;
    
    Vector4Copy(compare, v);
    nglUniform4fARB(uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void GLSL_SetUniformMatrix4(shaderProgram_t *program, uint32_t uniformNum, const mat4_t m)
{
    GLint *uniforms = program->uniforms;
    vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBuffer[uniformNum]);

    if (uniforms[uniformNum] == -1)
        return;
    
    if (uniformsInfo[uniformNum].type != GLSL_VEC4) {
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformMatrix4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
        return;
    }
    if (Mat4Compare(m, compare))
        return;
    
    Mat4Copy(m, compare);
    nglUniformMatrix4fvARB(uniforms[uniformNum], 1, GL_FALSE, (const GLfloat *)m);
}


enum {
    SHADER_BASIC,

    NUM_SHADER_DEFS
};

void GLSL_InitGPUShaders(void)
{
    uint64_t start, end;
    uint32_t attribs;

    ri.Printf(PRINT_INFO, "---- GLSL_InitGPUShaders ----\n");

    R_IssuePendingRenderCommands();

    start = ri.Milliseconds();

    attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD | ATTRIB_ALPHA | ATTRIB_COLOR;
    if (!GLSL_InitGPUShader(&rg.basicShader, "basic", attribs, qtrue, NULL, qtrue, fallbackShader_basic_vs, fallbackShader_basic_fs)) {
        ri.Error(ERR_FATAL, "Could not load basic shader");
    }
    GLSL_InitUniforms(&rg.basicShader);

    end = ri.Milliseconds();

    ri.Printf(PRINT_INFO, "...loaded %lu GLSL shaders in %5.2f seconds\n",
        rg.numPrograms, (end - start) / 1000.0);
}

void GLSL_ShutdownGPUShaders(void)
{
    uint32_t i;

    ri.Printf(PRINT_INFO, "---------- GLSL_ShutdownGPUShaders -----------\n");

    for (i = 0; i < ATTRIB_INDEX_COUNT; i++)
        nglDisableVertexAttribArrayARB(i);
    
    GL_BindNullProgram();

    for (i = 0; i < NUM_SHADER_DEFS; i++)
        GLSL_DeleteGPUShader(&rg.programs[i]);
}

void GLSL_BindProgram(shaderProgram_t *program)
{
    GLuint programObject = program ? program->programId : 0;

    if (GL_UseProgram(programObject)) {
        backend->pc.c_glslShaderBinds++;
        glState.currentShader = program;
    }
}
