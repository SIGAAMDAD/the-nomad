#include "rgl_local.h"
#include "imgui_impl_opengl3.h"

char gl_extensions[32768];

cvar_t *vid_xpos;
cvar_t *vid_ypos;
cvar_t *r_customWidth;
cvar_t *r_customHeight;
cvar_t *r_customWindowSize;

refimport_t ri;

void RB_InitImGui(void)
{

}

// for modular renderer
void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL N_Error(errorParm_t code, const char *err, ...)
{
    char buf[4096];
    va_list argptr;

    va_start(argptr, err);
    N_vsnprintf(buf, sizeof(buf), err, argptr);
    va_end(argptr);

    ri.Error(code, "%s", buf);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Con_Printf(const char *fmt, ...)
{
    char buf[ MAXPRINTMSG ];
	va_list	argptr;
	
    va_start( argptr, fmt );
	N_vsnprintf( buf, sizeof( buf ), fmt, argptr );
	va_end( argptr );

	ri.Printf( PRINT_INFO, "%s", buf );
}

static qboolean R_HasExtension(const char *ext)
{
    const char *ptr = Q_stristr( gl_extensions, ext );
	if (ptr == NULL)
		return qfalse;
	ptr += strlen(ext);
	return ((*ptr == ' ') || (*ptr == '\0'));  // verify its complete string.
}

static void R_InitWindow(void)
{
    rg.window = ri.SDL_CreateWindow(WINDOW_TITLE, vid_xpos->i, vid_ypos->i, r_customWidth->i, r_customHeight->i,
        r_fullscreen->i);
}

static void R_InitGLContext(void)
{
    int maxTextures;

    N_strncpyz(gl_extensions, nglGetString(GL_EXTENSIONS), sizeof(gl_extensions));
    nglGetIntegerv(GL_NUM_EXTENSIONS, &glContext.numExtensions);

    nglGetIntegerv(GL_MAJOR_VERSION, &glContext.versionMajor);
    nglGetIntegerv(GL_MINOR_VERSION, &glContext.versionMinor);
    Com_snprintf(glContext.version_str, "%i %i", glContext.versionMajor, glContext.versionMinor);

    nglGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextures);
    if (maxTexture < 16) {
        ri.Error(ERR_FATAL, "Broken OpenGL installation, GL_MAX_TEXTURE_UNITS < 16");
    }
}

void R_Init(void)
{
    ri.Printf(PRINT_INFO, "---------- RE_Init ----------\n");

    backend = (renderBackend_t *)ri.Hunk

    // clear all globals
    memset(&rg, 0, sizeof(rg));
    memset(backend, 0, sizeof(*backend));

    R_InitWindow();
    R_InitGLContext();

    ImGui_ImplOpenGL3_Init(glContext.glsl_version_str);
}

void RE_Shutdown(refShutdownCode_t code)
{

}

GDR_EXPORT renderExport_t *GDR_DECL GetRenderAPI(uint32_t version, refimport_t *import)
{
    static renderExport_t re;

    ri = *import;
    memset(&re, 0, sizeof(re));

    if (version != NOMAD_VERSION_FULL) {
        ri.Error(ERR_FATAL, "GetRenderAPI: rendergl version (%i) != glnomad engine version (%i)", NOMAD_VERSION_FULL, version);
    }

    re.Shutdown = RE_Shutdown;
    re.BeginRegistration = RE_BeginRegistration;

    return &re;
}
