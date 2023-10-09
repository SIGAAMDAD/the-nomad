#include "rgl_local.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "code/rendergl/imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "code/rendergl/imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "code/game/stb_sprintf.h"

#ifdef _NOMAD_DEBUG
static void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);
#endif

renderGlobals_t rg;
refimprot_t ri;
glContext_t glContext;

cvar_t *r_ticrate;
cvar_t *r_screenheight;
cvar_t *r_screenwidth;
cvar_t *r_vsync;
cvar_t *r_fullscreen;
cvar_t *r_native_fullscreen;
cvar_t *r_hidden;
cvar_t *r_drawFPS;
cvar_t *r_renderapi;
cvar_t *r_multisampleAmount;
cvar_t *r_multisampleType;
cvar_t *r_dither;
cvar_t *r_gammaAmount;
cvar_t *r_textureMagFilter;
cvar_t *r_textureMinFilter;
cvar_t *r_textureFiltering;
cvar_t *r_textureCompression;
cvar_t *r_textureDetail;
cvar_t *r_bloomOn;
cvar_t *r_useExtensions;
cvar_t *r_fovWidth;
cvar_t *r_fovHeight;
cvar_t *r_aspectRatio;
cvar_t *r_useFramebuffer;
cvar_t *r_enableBuffers;
cvar_t *r_enableShaders;
cvar_t *r_enableClientState;
cvar_t *r_flipTextureVertically;

cvar_t *r_EXT_anisotropicFiltering;
cvar_t *r_EXT_compiled_vertex_array;
cvar_t *r_ARB_texture_float;
cvar_t *r_ARB_texture_filter_anisotropic;
cvar_t *r_ARB_vertex_attrib_64bit;
cvar_t *r_ARB_vertex_buffer_object;
cvar_t *r_ARB_sparse_buffer;
cvar_t *r_ARB_vertex_attrib_binding;
cvar_t *r_ARB_texture_compression_bptc;
cvar_t *r_ARB_texture_compression_rgtc;
cvar_t *r_ARB_texture_compression;
cvar_t *r_ARB_pipeline_statistics_query;


static qboolean sdl_on = qfalse;
static qboolean imgui_on = qfalse;

typedef struct
{
    const char *str;
    const char *aspectRatio;
    int width, height;
    int w, h;
} vidmode_t;

static const vidmode_t videoModes[] = {
    {"3840x2160 (16:9)",  "16:9",  3840, 2160, 16, 9},
    {"2560x1440 (16:9)",  "16:9",  2560, 1440, 16, 9},
    {"1920x1080 (16:9)",  "16:9",  1920, 1080, 16, 9},
    {"1680x1050 (16:10)", "16:10", 1680, 1050, 16, 10},
    {"1600x900 (16:9)",   "16:9",  1600, 900,  16, 9},
    {"1280x800 (16:10)",  "16:10", 1280, 800,  16, 10},
    {"1280x720 (16:9)",   "16:9",  1280, 720,  16, 9},
    {"1024x768 (4:3)",    "4:3",   1024, 768,  4,  3},
    {"800x600 (4:3)",     "4:3",   800,  600,  4,  3},
};

extern "C" void RB_CameraInit(void)
{
    rg.camera.zoomLevel = 1.0f;
    RB_SetProjection(-3.0f, 3.0f, -3.0f, 3.0f);
}

extern "C" void RB_ZoomIn(void)
{
    if (rg.camera.zoomLevel < 0.1f)
        return;
    
    rg.camera.zoomLevel -= 0.05f;
    RB_SetProjection(-rg.camera.zoomLevel, rg.camera.zoomLevel, -rg.camera.zoomLevel, rg.camera.zoomLevel);
}

extern "C" void RB_ZoomOut(void)
{
    if (rg.camera.zoomLevel > 15.0f)
        return;
    
    rg.camera.zoomLevel += 0.05f;
    RB_SetProjection(-rg.camera.zoomLevel, rg.camera.zoomLevel, -rg.camera.zoomLevel, rg.camera.zoomLevel);
}

extern "C" void RB_RotateRight(void)
{
    rg.camera.rotation += 0.75f;
    if (rg.camera.rotation > 180.0f) {
        rg.camera.rotation -= 360.0f;
    }
    else if (rg.camera.rotation <= -180.0f) {
        rg.camera.rotation += 360.0f;
    }
}

extern "C" void RB_RotateLeft(void)
{
    rg.camera.rotation -= 0.75f;
    if (rg.camera.rotation > 180.0f) {
        rg.camera.rotation -= 360.0f;
    }
    else if (rg.camera.rotation <= -180.0f) {
        rg.camera.rotation += 360.0f;
    }
}

extern "C" void RB_MakeViewMatrix(void)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), rg.camera.cameraPos)
                        * glm::scale(glm::mat4(1.0f), glm::vec3(rg.camera.zoomLevel))
                        * glm::rotate(glm::mat4(1.0f), (float)DEG2RAD(rg.camera.rotation), glm::vec3(0, 0, 1));
        
    rg.camera.viewMatrix = glm::inverse(transform);
    rg.camera.vpm = rg.camera.projection * rg.camera.viewMatrix;
}

extern "C" void RB_SetProjection(float left, float right, float bottom, float top)
{
    rg.camera.projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    RB_MakeViewMatrix();
}

extern "C" void RB_CameraResize(void)
{
    r_aspectRatio->i = r_screenwidth->i / r_screenheight->i;
    RB_SetProjection(-r_aspectRatio->i * rg.camera.zoomLevel, r_aspectRatio->i * rg.camera.zoomLevel, -r_aspectRatio->i, r_aspectRatio->i);
}

extern "C" void RB_MoveUp(void)
{
    rg.camera.cameraPos[0] += -sin(DEG2RAD(rg.camera.rotation)) * 0.25f;
    rg.camera.cameraPos[1] += cos(DEG2RAD(rg.camera.rotation)) * 0.25f;
}

extern "C" void RB_MoveDown(void)
{
    rg.camera.cameraPos[0] -= -sin(DEG2RAD(rg.camera.rotation)) * 0.25f;
    rg.camera.cameraPos[1] -= cos(DEG2RAD(rg.camera.rotation)) * 0.25f;
}

extern "C" void RB_MoveLeft(void)
{
    rg.camera.cameraPos[0] -= cos(DEG2RAD(rg.camera.rotation)) * 0.25f;
    rg.camera.cameraPos[1] -= sin(DEG2RAD(rg.camera.rotation)) * 0.25f;
}

extern "C" void RB_MoveRight(void)
{
    rg.camera.cameraPos[0] += cos(DEG2RAD(rg.camera.rotation)) * 0.25f;
    rg.camera.cameraPos[1] += sin(DEG2RAD(rg.camera.rotation)) * 0.25f;
}

#ifdef RENDERLIB_USE_DLOPEN
void GDR_DECL N_Error(const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    N_vsnprintf(buf, sizeof(buf), fmt, argptr);
    va_end(argptr);

    ri.N_Error("%s", buf);
}
void GDR_DECL Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char buf[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    N_vsnprintf(buf, sizeof(buf), fmt, argptr);
    va_end(argptr);

    ri.Con_Printf(INFO, "%s", buf);
}
#endif

extern "C" void RE_InitSettings_f(void)
{
    int width, height;

    nglEnable(GL_DEPTH_TEST);
    nglEnable(GL_STENCIL_TEST);
    nglEnable(GL_TEXTURE_2D);
    nglEnable(GL_BLEND);
    
    nglDisable(GL_CULL_FACE);

    // dither
    if (r_dither->i)
        nglEnable(GL_DITHER);
    else
        nglDisable(GL_DITHER);
    
    nglDisable(GL_CULL_FACE);

    // update all the textures
    R_UpdateTextures();

    // re-calculate the camera
    ri.SDL_GetWindowSize(rg.window, &width, &height);
    RB_MakeViewMatrix();
    RB_CameraResize();

    nglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ri.Cvar_Set("r_screenwidth", va("%i", width));
    ri.Cvar_Set("r_screenheight", va("%i", height));

    ri.SDL_GL_SetSwapInterval((r_vsync->i ? -1 : 0));

    if (r_useFramebuffer->i) {
        RE_UpdateFramebuffers();
    }
    else {
        nglEnable(GL_MULTISAMPLE);
    }
}

static char gl_extensions[ 32768 ];

extern "C" qboolean R_HasExtension(const char *name)
{
    const char *ptr = N_stristr(gl_extensions, name);
    if (!ptr)
        return qfalse;
    
    ptr += strlen(name);
    return (qboolean)((*ptr == ' ') || (*ptr == '\0')); // verify its a safe string
}

#define NGL( ret, name, ... )\
{ \
    n##name = (PFN##name)load(#name); \
    if (!n##name) ri.Con_Printf(INFO, "WARNING: failed to load gl proc %s", #name); \
}

extern "C" void R_InitExtensions(NGLloadproc load)
{
    size_t len;
    const char *err;

    if (!nglGetString(GL_EXTENSIONS))
        ri.N_Error("OpenGL installation is broken. Please fix video drivers and/or restart your system");    
    
    N_strncpyz(glContext.vendor, (const char *)nglGetString(GL_VENDOR), sizeof(glContext.vendor));
    N_strncpyz(glContext.renderer, (const char *)nglGetString(GL_RENDERER), sizeof(glContext.renderer));
    len = strlen(glContext.renderer);
    if (len && glContext.renderer[len - 1] == '\n')
        glContext.renderer[len - 1] = '\0';
    
    N_strncpyz(glContext.version_str, (const char *)nglGetString(GL_VERSION), sizeof(glContext.version_str));
    N_strncpyz(gl_extensions, (const char *)nglGetString(GL_EXTENSIONS), sizeof(gl_extensions));
    N_strncpyz(glContext.extensions, gl_extensions, sizeof(glContext.extensions));

    glContext.version_f = N_atof(glContext.version_str);
    glContext.version_i = (int)(glContext.version_f * 10.001);

    glContext.textureCompression = TC_NONE;

    glContext.maxAnisotropy = 0;
    glContext.nonPowerOfTwoTextures = qfalse;

    // complete the regular config
    nglGetIntegerv(GL_MAX_TEXTURE_UNITS, &glContext.maxTextureUnits);
    nglGetIntegerv(GL_MAX_TEXTURE_SIZE, &glContext.maxTextureSize);
    nglGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &glContext.maxVertexAttribs);
    nglGetIntegerv(GL_MAX_VIEWPORT_DIMS, glContext.maxViewportDims);
    nglGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &glContext.maxVertexAttribs);
    nglGetBooleanv(GL_STEREO, (GLboolean *)&glContext.stereo);
    nglGetIntegerv(GL_MAX_SAMPLES, &glContext.maxMultisampleSamples);

    nglGetIntegerv(GL_NUM_EXTENSIONS, &glContext.numExtensions);
    nglGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &glContext.maxVertexAttribStride);
    nglGetIntegerv(GL_MAX_INTEGER_SAMPLES, &glContext.maxTextureSamples);
    nglGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &glContext.maxVertexShaderUniforms);
    nglGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &glContext.maxVertexShaderVectors);
    nglGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, &glContext.maxVertexShaderAtomicBuffers);
    nglGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &glContext.maxVertexShaderAtomicCounters);
    nglGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &glContext.maxFragmentShaderTextures);
    nglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &glContext.maxFragmentShaderUniforms);
    nglGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &glContext.maxFragmentShaderVectors);
    nglGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, &glContext.maxFragmentShaderAtomicBuffers);
    nglGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &glContext.maxFragmentShaderAtomicCounters);
    nglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &glContext.maxFramebufferColorAttachments);
    nglGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &glContext.maxFramebufferWidth);
    nglGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &glContext.maxFramebufferHeight);
    nglGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &glContext.maxFramebufferSamples);
    nglGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &glContext.maxProgramTexelOffset);
    nglGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &glContext.maxRenderbufferSize);
    nglGetIntegerv(GL_MAX_TEXTURE_LOD_BIAS, &glContext.maxTextureLOD);
    nglGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &glContext.maxUniformLocations);
    nglGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &glContext.maxUniformBufferBindings);

    ri.Con_Printf(INFO,
        "<---------- OpenGL Context Info ---------->\n"
        "GL_VENDOR: %s\n"
        "GL_VERSION: %s\n"
        "GL_RENDERER: %s\n"
        "GL_STEREO: %s\n"
        "GL_MAX_VIEWPORT_DIMS: %i, %i\n"
        "GL_MAX_TEXTURE_UNITS: %i\n"
        "GL_MAX_VERTEX_ATTRIBS: %i\n"
        "GL_MAX_TEXTURE_SIZE: %i\n"
        "GL_MAX_SAMPLES: %i\n",
    glContext.vendor, glContext.version_str, glContext.renderer, N_booltostr(glContext.stereo),
    glContext.maxViewportDims[0], glContext.maxViewportDims[1], glContext.maxTextureUnits,
    glContext.maxVertexAttribs, glContext.maxTextureSize, glContext.maxMultisampleSamples);

    ri.Con_Printf(INFO,
        "==== Extended OpenGL Context Info ====\n"
        "GL_MAX_VERTEX_ATTRIB_STRIDE: %i\n"
        "GL_MAX_INTEGER_SAMPLES: %i\n"
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS: %i\n"
        "GL_MAX_VERTEX_UNIFORM_VECTORS: %i\n"
        "GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS: %i\n"
        "GL_MAX_VERTEX_ATOMIC_COUNTERS: %i\n"
        "GL_MAX_FRAGMENT_IMAGE_UNIFORMS: %i\n"
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS: %i\n"
        "GL_MAX_FRAGMENT_UNIFORM_VECTORS: %i\n"
        "GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS: %i\n"
        "GL_MAX_FRAGMENT_ATOMIC_COUNTERS: %i\n"
        "GL_MAX_COLOR_ATTACHMENTS: %i\n"
        "GL_MAX_FRAMEBUFFER_WIDTH: %i\n"
        "GL_MAX_FRAMEBUFFER_HEIGHT: %i\n"
        "GL_MAX_FRAMEBUFFER_SAMPLES: %i\n"
        "GL_MAX_PROGRAM_TEXEL_OFFSET: %i\n"
        "GL_MAX_RENDERBUFFER_SIZE: %i\n"
        "GL_MAX_TEXTURE_LOD_BIAS: %i\n"
        "GL_MAX_UNIFORM_LOCATIONS: %i\n"
        "GL_MAX_UNIFORM_BUFFER_BINDINGS: %i\n",
    glContext.maxVertexAttribStride, glContext.maxTextureSamples, glContext.maxVertexShaderUniforms,
    glContext.maxVertexShaderUniforms, glContext.maxVertexShaderAtomicBuffers, glContext.maxVertexShaderAtomicCounters,
    glContext.maxFragmentShaderTextures, glContext.maxFragmentShaderUniforms, glContext.maxFragmentShaderVectors,
    glContext.maxFragmentShaderAtomicBuffers, glContext.maxFragmentShaderAtomicCounters,
    glContext.maxFramebufferColorAttachments, glContext.maxFramebufferWidth, glContext.maxFramebufferHeight,
    glContext.maxFramebufferSamples, glContext.maxProgramTexelOffset, glContext.maxRenderbufferSize, glContext.maxTextureLOD,
    glContext.maxUniformLocations, glContext.maxUniformBufferBindings);

    if (!r_useExtensions->i) {
        ri.Con_Printf(INFO, "****** IGNORING OPENGL EXTENSIONS ******");

        // use the default buffer functions if GL_ARB_vertex_buffer_object is disabled
        nglGenBuffersARB = nglGenBuffers;
        nglBindBufferARB = nglBindBuffer;
        nglDeleteBuffersARB = nglDeleteBuffers;
        nglBufferDataARB = nglBufferData;
        nglBufferSubDataARB = nglBufferSubData;

        return;
    }

    ri.Con_Printf(INFO, "Initializing OpenGL Extensions");

    // GL_ARB_texture_filter_anisotropic
    if (R_HasExtension("GL_ARB_texture_filter_anisotropic")) {
        nglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &glContext.maxAnisotropy);

        if (glContext.maxAnisotropy <= 0) {
            ri.Con_Printf(INFO, "... GL_ARB_texture_filter_anisotropic not property supported");
        }
        else {
            ri.Con_Printf(INFO, "... GL_ARB_texture_filter_anisotropic (max: %f)", glContext.maxAnisotropy);
            ri.Cvar_SetFloatValue("r_ARB_texture_filter_anisotropic", glContext.maxAnisotropy);
        }
    }
    else {
        ri.Con_Printf(INFO, "... GL_EXT_texture_filter_anisotropic not found");
    }

    r_ARB_texture_filter_anisotropic->i = glContext.maxAnisotropy > 0;

    // GL_ARB_vertex_buffer_object
    if (R_HasExtension("GL_ARB_vertex_buffer_object")) {
        LOAD_GL_PROCS(NGL_BufferARB_Procs);
        if (!nglGenBuffersARB || !nglDeleteBuffersARB || !nglBindBufferARB || !nglBufferDataARB || !nglBufferSubDataARB) {
            ri.Con_Printf(INFO, "WARNING: failed to load ARB_vertex_buffer_object extension procs");

            // use default buffer functions
            nglGenBuffersARB = nglGenBuffers;
            nglBindBufferARB = nglBindBuffer;
            nglDeleteBuffersARB = nglDeleteBuffers;
            nglBufferDataARB = nglBufferData;
            nglBufferSubDataARB = nglBufferSubData;
        }
        else {
            ri.Con_Printf(INFO, "... ARB_vertex_object extension loaded");
        }
    }


    // GL_ARB_texture_compression_s3tc
    if (r_ARB_texture_compression->i && R_HasExtension("GL_ARB_texture_compression_s3tc")) {
        if (r_textureCompression->i) {
            glContext.textureCompression = TC_S3TC_ARB;
            ri.Con_Printf(INFO, "... using GL_ARB_texture_compression_s3tc");
            ri.Cvar_SetStringValue("r_textureCompression", "TC_S3TC_ARB");
        }
        else {
            ri.Con_Printf(INFO, "... ignoring GL_ARB_texture_compression_s3tc");
            if (N_stricmp(r_textureCompression->s, "none") != 0)
                ri.Cvar_SetStringValue("r_textureCompression", "none");
        }
    }
    else {
        ri.Con_Printf(INFO, "... GL_ARB_texture_compression_s3tc not found");
        if (N_stricmp(r_textureCompression->s, "none") != 0)
            ri.Cvar_SetStringValue("r_textureCompression", "none");
    }

    // GL_S3_s3tc
    if (glContext.textureCompression == TC_NONE && r_textureCompression->i) {
        if (R_HasExtension("GL_S3_s3tc")) {
            if (r_textureCompression->i) {
                glContext.textureCompression = TC_S3TC;
                ri.Con_Printf(INFO, "... using GL_S3_s3tc");
                ri.Cvar_SetStringValue("r_textureCompression", "TC_S3TC");
            }
            else {
                glContext.textureCompression = TC_NONE;
                ri.Con_Printf(INFO, "... ignoring GL_S3_s3tc");
                if (N_stricmp(r_textureCompression->s, "none") != 0)
                    ri.Cvar_SetStringValue("r_textureCompression", "none");
            }
        }
        else {
            ri.Con_Printf(INFO, "... GL_S3_s3tc not found");
            if (N_stricmp(r_textureCompression->s, "none") != 0)
                ri.Cvar_SetStringValue("r_textureCompression", "none");
        }
    }
}
#undef NGL

extern "C" void R_InitGL(void)
{
    rg.context = ri.SDL_GL_CreateContext(rg.window);
    ri.SDL_GL_MakeCurrent(rg.window, rg.context);
    ri.SDL_GL_SetSwapInterval(-1);
    ri.SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    ri.SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    ri.SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    ri.SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    ri.SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (!r_useFramebuffer->i) {
        ri.SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 2);
        ri.SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, r_multisampleAmount->i);
    }

    load_gl_procs((NGLloadproc)ri.SDL_GL_GetProcAddress);

    R_InitExtensions((NGLloadproc)ri.SDL_GL_GetProcAddress);
#ifdef _NOMAD_DEBUG
    ri.Con_Printf(INFO, "turning on OpenGL debug callbacks");
    if (nglDebugMessageControlARB != NULL) {
        nglEnable(GL_DEBUG_OUTPUT);
        nglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        nglDebugMessageCallbackARB((GLDEBUGPROCARB)DBG_GL_ErrorCallback, NULL);
        uint32_t unusedIds = 0;
        nglDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, (GLuint *)&unusedIds, GL_TRUE);
    }
#endif
}

void R_GfxInfo_f(void)
{
    ri.Con_Printf(INFO, "\n\n<---------- Gfx Info ---------->");
    ri.Con_Printf(INFO, "Rendering API: %s", r_renderapi->s);
    
    if (!N_strcmp("R_OPENGL", r_renderapi->s)) {
        ri.Con_Printf(INFO, "=== OpenGL Driver Info ===");
        ri.Con_Printf(INFO, "GL_RENDERER: %s", glContext.renderer);
        ri.Con_Printf(INFO, "GL_VERSION: %s", glContext.version_str);
        ri.Con_Printf(INFO, "GL_VENDOR: %s", glContext.vendor);
    }
    ri.Con_Printf(INFO, "=== Cvars ===");
    ri.Con_Printf(INFO, "r_screenwidth: %i", r_screenwidth->i);
    ri.Con_Printf(INFO, "r_screenheight: %i", r_screenheight->i);
    ri.Con_Printf(INFO, "r_drawFPS: %s", N_booltostr(r_drawFPS->i));
    ri.Con_Printf(INFO, "r_ticrate: %i", r_ticrate->i);
    ri.Con_Printf(INFO, "r_fullscreen: %s", N_booltostr(r_fullscreen->i));
    ri.Con_Printf(INFO, "r_multisampleAmount: x%i", r_multisampleAmount->i);
    ri.Con_Printf(INFO, "r_vsync: %s", N_booltostr(r_vsync->i));
    ri.Con_Printf(DEV, "r_textureMinFilter: %s", r_textureMagFilter->s);
    ri.Con_Printf(DEV, "r_textureMagFilter: %s", r_textureMinFilter->s);
    ri.Con_Printf(INFO, "r_textureFiltering: %s", r_textureFiltering->s);
    ri.Con_Printf(INFO, "r_gammaAmount: %f", r_gammaAmount->f);
    ri.Con_Printf(INFO, "r_textureDetail: %s", r_textureDetail->s);
    ri.Con_Printf(INFO, "r_EXT_anisotropicFilter: %s, %i", N_booltostr(r_ARB_texture_filter_anisotropic->i), r_ARB_texture_filter_anisotropic->i);
    ri.Con_Printf(INFO, "r_useExtensions: %s", N_booltostr(r_useExtensions->i));
    ri.Con_Printf(INFO, "r_dither: %s", N_booltostr(r_dither->i));
    ri.Con_Printf(INFO, " ");
}

extern "C" void* R_ImGuiAlloc_Callback(size_t size, void *userData)
{
    return ri.Mem_Alloc(size);
}

extern "C" void R_ImGuiFree_Callback(void *ptr, void *userData)
{
    ri.Mem_Free(ptr);
}

extern "C" void R_InitImGui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions(R_ImGuiAlloc_Callback, R_ImGuiFree_Callback);
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplSDL2_InitForOpenGL(rg.window, rg.context);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    
    imgui_on = qtrue;
}

extern "C" void R_GetVars(void)
{
    r_aspectRatio = ri.Cvar_Get("r_aspectRatio", "16:9", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetDescription(r_aspectRatio, "Screen's aspect ratio.");

    r_fovWidth = ri.Cvar_Get("r_fovWidth", "72", CVAR_SAVE | CVAR_LATCH | CVAR_ARCHIVE_ND);
    ri.Cvar_SetGroup(r_fovWidth, CVG_RENDERER);
    r_fovHeight = ri.Cvar_Get("r_fovHeight", "24", CVAR_SAVE | CVAR_LATCH | CVAR_ARCHIVE_ND);
    ri.Cvar_SetGroup(r_fovHeight, CVG_RENDERER);
    
    r_ticrate = ri.Cvar_Get("r_ticrate", "35", CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_ticrate, CVG_RENDERER);
    ri.Cvar_SetDescription(r_ticrate, "The amount of times the game will loop per second");
    
    r_textureMagFilter = ri.Cvar_Get("r_textureMagFilter", "GL_NEAREST", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_textureMagFilter, CVG_RENDERER);
    r_textureMinFilter = ri.Cvar_Get("r_textureMinFilter", "GL_LINEAR", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_textureMinFilter, CVG_RENDERER);
    r_textureFiltering = ri.Cvar_Get("r_textureFiltering", "Nearest", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_textureFiltering, CVG_RENDERER);
    r_textureCompression = ri.Cvar_Get("r_textureCompression", "none", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_textureCompression, CVG_RENDERER);
    r_textureDetail = ri.Cvar_Get("r_textureDetail", "3", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_textureDetail, CVG_RENDERER);
    ri.Cvar_CheckRange(r_textureDetail, "0", "5", CVT_INT);
    
    r_screenwidth = ri.Cvar_Get("r_screenwidth", "1920", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_screenwidth, CVG_RENDERER);
    ri.Cvar_SetDescription(r_screenwidth, "Video rendering width");
    ri.Cvar_CheckRange(r_screenwidth, "800", "3840", CVT_INT);
    r_screenheight = ri.Cvar_Get("r_screenheight", "1080", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_screenheight, CVG_RENDERER);
    ri.Cvar_SetDescription(r_screenheight, "Video rendering height");
    ri.Cvar_CheckRange(r_screenheight, "600", "2160", CVT_INT);

    r_vsync = ri.Cvar_Get("r_vsync", "1", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_vsync, CVG_RENDERER);

    r_fullscreen = ri.Cvar_Get("r_fullscreen", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_fullscreen, CVG_RENDERER);

    r_drawFPS = ri.Cvar_Get("r_drawFPS", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_drawFPS, CVG_RENDERER);

    r_renderapi = ri.Cvar_Get("r_renderapi", "R_OPENGL", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_renderapi, CVG_RENDERER);
    ri.Cvar_SetDescription(r_renderapi,
        "   R_OPENGL = use OpenGL 3.1 rendering\n"
        "   R_VULKAN = use Vulkan rendering (NOT SUPPORTED YET)\n"
        "   R_D3D11  = use DirectX11 rendering (NOT SUPPORTED YET)");

    r_dither = ri.Cvar_Get("r_dither", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_dither, CVG_RENDERER);

    r_multisampleAmount = ri.Cvar_Get("r_multisampleAmount", "2", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_multisampleAmount, CVG_RENDERER);
    ri.Cvar_CheckRange(r_multisampleAmount, "0", "16", CVT_INT);
    ri.Cvar_SetDescription(r_multisampleAmount, "Specify the amount of samples per pixel the renderer uses for anti-aliasing.");
    r_multisampleType = ri.Cvar_Get("r_multisampleType", "none", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_multisampleType, CVG_RENDERER);
    ri.Cvar_SetDescription(r_multisampleType,
        "Specify what kind of anti-aliasing type the renderer uses.\n"
        "    MSAA = use multisampling\n"
        "    FXAA = use fast-approximate (not yet supported)\n"
        "    SSAA = use super-sampling");
    
    r_flipTextureVertically = ri.Cvar_Get("r_flipTextureVertically", "1", CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_flipTextureVertically, CVG_RENDERER);

    r_enableClientState = ri.Cvar_Get("r_enableClientState", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_enableClientState, CVG_RENDERER);
    ri.Cvar_SetDescription(r_enableClientState,
        "    0 = use either buffers or immediate mode for vertex rendering\n"
        "    1 = override both buffers and immediate mode for rendering, use nglEnableClientState instead.");

    r_gammaAmount = ri.Cvar_Get("r_gammaAmount", "2.2.", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_gammaAmount, CVG_RENDERER);
    r_bloomOn = ri.Cvar_Get("r_bloomOn", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_bloomOn, CVG_RENDERER);
    
    r_useExtensions = ri.Cvar_Get("r_useExtensions", "1", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_useExtensions, CVG_RENDERER);
    
    r_enableShaders = ri.Cvar_Get("r_enableShaders", "1", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_enableShaders, CVG_RENDERER);
    r_enableBuffers = ri.Cvar_Get("r_enableBuffers", "1", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_enableBuffers, CVG_RENDERER);
    ri.Cvar_SetDescription(r_enableBuffers,
        "    0 = force immediate (old-fashioned) rendering\n"
        "    1 = allow OpenGL 3.1 buffer objects");
    r_useFramebuffer = ri.Cvar_Get("r_useFramebuffer", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_useFramebuffer, CVG_RENDERER);
    ri.Cvar_SetDescription(r_useFramebuffer,
        "    0 = force the renderer to let OpenGL do all the post-processing (faster, but looks worse)\n"
        "    1 = let the renderer use its own custom post-processing (slower, but better looking)");
    
    r_ARB_texture_filter_anisotropic = ri.Cvar_Get("r_ARB_texture_filter_anistropic", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_ARB_texture_filter_anisotropic, CVG_RENDERER);
    ri.Cvar_SetDescription(r_ARB_texture_filter_anisotropic,
        "    0 = GL implementation doesn't support anisotropic filtering\n"
        "    1 = GL implementation does support anisotropic filtering");
    ri.Cvar_CheckRange(r_ARB_texture_filter_anisotropic, "0", "1", CVT_INT);

    r_ARB_texture_compression = ri.Cvar_Get("r_ARB_texture_compression", "0", CVAR_PRIVATE | CVAR_SAVE | CVAR_LATCH);
    ri.Cvar_SetGroup(r_ARB_texture_compression, CVG_RENDERER);
    ri.Cvar_SetDescription(r_ARB_texture_compression,
        "    0 = GL implementation doesn't support texture compression\n"
        "    1 = GL implementation does support texture compression");
}

static qboolean r_windowOpen = qfalse, r_initialized = qfalse;

extern "C" void RE_GetImport(const renderImport_t *import)
{
    memcpy(&ri, import, sizeof(*import));
}

extern "C" void RE_Init(void)
{
    ri.Con_Printf(INFO, "RE_Init: initializing rendering engine");

    R_GetVars();
    RB_CameraInit();

    ri.Con_Printf(INFO, "alllocating memory to the SDL_Window context");

    if (!r_windowOpen) {
        if (ri.SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
            ri.N_Error("RE_Init: failed to initialize SDL2, error message: %s",
                ri.SDL_GetError());
        }
        rg.window = ri.SDL_CreateWindow(
                            "The Nomad",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            r_screenwidth->i, r_screenheight->i,
                            SDL_WINDOW_OPENGL
                        );
        if (!rg.window) {
            ri.N_Error("RE_Init: failed to initialize an SDL2 window, error message: %s",
                ri.SDL_GetError());
        }
        r_windowOpen = qtrue;
    }
    R_InitGL();

    rg.numTextures = 0;
    rg.numVertexCaches = 0;
    rg.numShaders = 0;

    backend.iboId = 0;
    backend.vboId = 0;
    backend.vaoId = 0;
    backend.shaderId = 0;
    backend.texId = 0;

    R_InitImGui();

    ri.Cmd_AddCommand("gfxinfo", R_GfxInfo_f);
    ri.Cmd_AddCommand("gfxrestart", RE_InitSettings_f);

    RE_InitFrameData();
    RE_InitFramebuffers();
    RE_InitPLRef();

    r_initialized = qtrue;
}

extern "C" void RE_Shutdown(qboolean killWindow)
{
    // this may be called before RE_Init
    if (!r_initialized)
        return;

    ri.Con_Printf(INFO, "RE_Shutdown: shutting down OpenGL buffers and memory, and deallocating rendering context");
    ri.Cmd_RemoveCommand("gfxinfo");
    ri.Cmd_RemoveCommand("gfxrestart");

    RE_ShutdownFramebuffers();
    RE_ShutdownTextures();
    RE_ShutdownShaders();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (killWindow) {
        if (rg.window)
            ri.SDL_DestroyWindow(rg.window);
        if (rg.context)
            ri.SDL_GL_DeleteContext(rg.context);

        ri.SDL_Quit();
        rg.window = NULL;
        rg.context = NULL;
    }
}


#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source)
{
    switch (source) {
    case GL_DEBUG_SOURCE_API: return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION: return "Application User";
    case GL_DEBUG_SOURCE_OTHER: return "Other";
    };
    return "Unknown Source";
}

const char *DBG_GL_TypeToStr(GLenum type)
{
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
    case GL_DEBUG_TYPE_MARKER: return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP: return "Debug Push group";
    case GL_DEBUG_TYPE_POP_GROUP: return "Debug Pop Group";
    case GL_DEBUG_TYPE_OTHER: return "Other";
    };
    return "Unknown Type";
}

const char *DBG_GL_SeverityToStr(GLenum severity)
{
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: return "High";
    case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
    case GL_DEBUG_SEVERITY_LOW: return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
    };
    return "Unknown Severity";
}

static void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam)
{
    // nothing massive or useless
    if (length >= 300 || severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        ri.N_Error("[OpenGL Error: %i] %s", id, message);
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        ri.Con_Printf(WARNING, "[OpenGL Debug Log] %s Deprecated Behaviour, Id: %i", message, id);
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        ri.Con_Printf(WARNING, "[OpenGL Debug Log] %s Portability, Id: %i", message, id);
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        ri.Con_Printf(WARNING, "[OpenGL Debug Log] %s Undefined Behaviour, Id: %i", message, id);
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        ri.Con_Printf(INFO, "[OpenGL Debug Log (Performance)] %s Id: %i", message, id);
        break;
    default:
        ri.Con_Printf(INFO, "[OpenGL Debug Log] %s Id: %i", message, id);
        break;
    };
}

#endif