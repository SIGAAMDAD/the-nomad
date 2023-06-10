#include "rgl_public.h"
#include "rgl_local.h"
#include "../src/n_scf.h"

#ifdef __unix__
static const char *glLibraries[] = {
	"libGL.so.1",
	"libGL.so"
};
#elif defined(_WIN32)
static const char *glLibraries[] = {
	"opengl32.dll"
};
#endif

glcontext_t glContext;

#define NGL(ret,name,...) ret( APIENTRY* n##name)(__VA_ARGS__);
	NGL_Core_Procs;
	NGL_Texture_Procs;
	NGL_FBO_Procs;
	NGL_Buffer_Procs;
	NGL_VAO_Procs;
#undef NGL

typedef struct
{
    void **proc;
    const char *name;
} sym_t;

#define NGL(ret,name,...) { (void**)&n##name, VSTR(name) },
static sym_t glProcs[] = { NGL_Texture_Procs NGL_Core_Procs NGL_FBO_Procs NGL_Buffer_Procs NGL_VAO_Procs };
#undef NGL

static void GL_InitProcs(void)
{
	glContext.libhandle = NULL;
	for (uint32_t i = 0; i < arraylen(glLibraries); i++) {
		glContext.libhandle = ri.Sys_LoadLibrary(glLibraries[i]);
		if (glContext.libhandle) {
			break;
		}
	}
	if (!glContext.libhandle) {
	    ri.Error("R_Init: failed to load opengl library");
	}
	
	for (uint64_t i = 0; i < arraylen(glProcs); i++) {
		*glProcs[i].proc = ri.Sys_LoadProc(glContext.libhandle, glProcs[i].name);
		if (!*glProcs[i].proc) {
			ri.Error("R_Init: failed to load opengl proc %s", glProcs[i].name);
		}
		else {
			ri.LPrintf(DEBUG, "Successfully loaded opengl proc %s", glProcs[i].name);
		}
	}
	ri.Printf("Successfully loaded opengl procs");
	
	ri.Sys_FreeLibrary(glContext.libhandle);
}


static void GL_Init(void)
{
    // load in the procs
    GL_InitProcs();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        ri.Error("R_Init: failed to initialize SDL2, error message: %s",
            SDL_GetError());
    }

    ri.Printf("Alllocating memory to the SDL_Window context");
    glContext.window = SDL_CreateWindow(
                            "The Nomad",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            N_atoi(r_screenwidth.value), N_atoi(r_screenheight.value),
                            SDL_WINDOW_OPENGL
                        );
    if (!glContext.window) {
        ri.Error("R_Init: failed to initialize an SDL2 window, error message: %s",
            SDL_GetError());
    }
    assert(glContext.window);

    glContext.context = SDL_GL_CreateContext(glContext.window);
    SDL_GL_MakeCurrent(glContext.window, glContext.context);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    ri.Printf(
        "OpenGL Info:\n"
        "  Version: %s\n"
        "  Renderer: %s\n"
        "  Vendor: %s\n",
    nglGetString(GL_VERSION), nglGetString(GL_RENDERER), nglGetString(GL_VENDOR));

    const GLenum params[] = {
        GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
        GL_MAX_CUBE_MAP_TEXTURE_SIZE,
        GL_MAX_DRAW_BUFFERS,
        GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
        GL_MAX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_SIZE,
        GL_MAX_VARYING_FLOATS,
        GL_MAX_VERTEX_ATTRIBS,
        GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
        GL_MAX_VERTEX_UNIFORM_COMPONENTS,
        GL_MAX_VIEWPORT_DIMS,
        GL_STEREO,
    };
    const char *names[] = {
        "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
        "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
        "GL_MAX_DRAW_BUFFERS",
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
        "GL_MAX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_SIZE",
        "GL_MAX_VARYING_FLOATS",
        "GL_MAX_VERTEX_ATTRIBS",
        "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
        "GL_MAX_VIEWPORT_DIMS",
        "GL_STEREO",
    };

    ri.Printf("<-------- OpenGL Context Parameters -------->");
    
    // integers - only works if the order is 0-10 integer return types
    for (uint32_t i = 0; i < 10; i++) {
        GLuint v = 0;
        nglGetIntegerv(params[i], &v);
        ri.Printf("%s: %i" , names[i], v);
    }
    
    // others
    GLuint v[2];
    memset(v, 0, sizeof(v));
    nglGetIntegerv(params[10], v);
    ri.Printf("%s: %i %i", names[10], v[0], v[1]);
    GLboolean s = GL_FALSE;
    nglGetBooleanv(params[11], &s);
    ri.Printf("%s: %i", names[11], (unsigned int)s);
    ri.Printf(" ");

    ri.Printf("<-------- OpenGL Extensions Info -------->");
    GLuint extension_count = 0;
    nglGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
    ri.Printf("Extension Count: %i", extension_count);
    ri.Printf("Extension List:");
    glContext.extensions = (char **)ri.Z_Malloc(sizeof(char *) * extension_count, TAG_STATIC, &glContext.extensions, "GLextList");
    for (uint32_t i = 0; i < extension_count; ++i) {
        if (i >= 30) { // dont go crazy
            ri.Printf("... (%i more extensions)", extension_count - i);
            break;
        }
        const GLubyte* name = nglGetStringi(GL_EXTENSIONS, i);
        glContext.extensions[i] = (char *)ri.Z_Malloc(strlen((const char *)name)+1, TAG_STATIC, &glContext.extensions[i], "GLextList");
        N_strncpy(glContext.extensions[i], (const char *)name, strlen((const char *)name)+1);
        ri.Printf("%s", (const char *)name);
    }
    ri.Printf(" ");

#ifdef _NOMAD_DEBUG
    ri.Printf("turning on OpenGL debug callbacks");
    nglEnable(GL_DEBUG_OUTPUT_KHR);
    nglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    nglDebugMessageCallbackARB((glDebugProcARB)DBG_GL_ErrorCallback, NULL);
    GLuint unusedIds = 0;
    nglDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
#endif

    nglEnable(GL_DEPTH_TEST);
    nglEnable(GL_TEXTURE_2D);
    nglEnable(GL_STENCIL_TEST);
    nglEnable(GL_BLEND);

    nglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    nglDepthMask(GL_FALSE);
    nglDepthFunc(GL_ALWAYS);

    glState.numImages = 0;
    glState.numCaches = 0;
    glState.numFramebuffers = 0;
    glState.numShaders = 0;
    glState.initialized = true;

    memset(glState.images, 0, sizeof(glState.images));
    memset(glState.caches, 0, sizeof(glState.caches));
    memset(glState.framebuffers, 0, sizeof(glState.framebuffers));
    memset(glState.shaders, 0, sizeof(glState.shaders));
}

static void R_InitImports(renderImport_t* imports)
{
	ri.Printf = imports->Printf;
	ri.Error = imports->Error;
    ri.LPrintf = imports->LPrintf;

    ri.N_LoadFile = imports->N_LoadFile;

    ri.Cvar_Register = imports->Cvar_Register;
    ri.Cvar_RegisterName = imports->Cvar_RegisterName;
    ri.Cvar_ChangeValue = imports->Cvar_ChangeValue;
    ri.Cvar_Find = imports->Cvar_Find;
	
//	ri.FS_FOpenRead = imports->FS_FOpenRead;
//	ri.FS_FOpenWrite = imports->FS_FOpenWrite;
//	ri.FS_FileTell = imports->FS_FileTell;
//	ri.FS_FileSeek = imports->FS_FileSeek;
//	ri.FS_FClose = imports->FS_FClose;
//	ri.FS_Write = imports->FS_Write;
//	ri.FS_Read = imports->FS_Read;
//	ri.FS_FileLength = imports->FS_FileLength;
//	ri.FS_FileExists = imports->FS_FileExists;
	
	ri.Sys_LoadLibrary = imports->Sys_LoadLibrary;
	ri.Sys_LoadProc = imports->Sys_LoadProc;
	ri.Sys_FreeLibrary = imports->Sys_FreeLibrary;
	
	ri.BFF_FetchInfo = imports->BFF_FetchInfo;
}

void RE_Init(renderImport_t* imports)
{
	R_InitImports(imports);
	GL_Init();
}

void RE_Shutdown(void)
{
    if (!glState.initialized)
        return;
    
    
}