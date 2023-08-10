#include "rgl_local.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "../rendergl/imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../rendergl/imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "../src/stb_sprintf.h"

#ifdef _NOMAD_DEBUG
static void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);
#endif

renderGlobals_t rg;
renderImport_t ri;
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
cvar_t *r_EXT_anisotropicFiltering;
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

static bool sdl_on = false;
static bool imgui_on = false;

Renderer::Renderer()
    : camera(-1.0f, 1.0f, -1.0f, 1.0f)
{
}

static void R_CameraInfo_f(void)
{
    ri.Con_Printf(INFO, "\n<---------- Camera Info ---------->");
    ri.Con_Printf(INFO, 
        "Rotation (Degrees): %f\n"
        "Rotation (Radians): %f\n"
        "Position: (x) %f, (y) %f, (z) %f\n"
        "Zoom Level: %f\n",
    renderer->camera.GetRotation(), glm::radians(renderer->camera.GetRotation()),
    renderer->camera.GetPos().x, renderer->camera.GetPos().y, renderer->camera.GetPos().z,
    renderer->camera.GetZoom());
}

Camera::Camera(float left, float right, float bottom, float top)
    : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPos)
                        * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
    
    m_ViewMatrix = glm::inverse(transform);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    m_ZoomLevel = left;
    m_CameraPos = glm::vec3(0.0f, 3.5f, 0.0f);
    m_AspectRatio = r_screenwidth->i / r_screenheight->i;

    ri.Cmd_AddCommand("gfx_camerainfo", R_CameraInfo_f);
}

void Camera::SetProjection(float left, float right, float bottom, float top)
{
    m_ProjectionMatrix = glm::ortho(left, right, bottom, top);
    CalculateViewMatrix();
}

void Camera::CalculateViewMatrix()
{
    EASY_FUNCTION();
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPos)
                        * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
    
    m_ViewMatrix = glm::inverse(transform);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::ZoomIn(void)
{
    if (!m_ZoomLevel)
        return;

    m_ZoomLevel -= 0.05f;
    SetProjection(-m_ZoomLevel, m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
}

void Camera::ZoomOut(void)
{
    if (m_ZoomLevel >= 15.0f)
        return;

    m_ZoomLevel += 0.05f;
    SetProjection(-m_ZoomLevel, m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
}

void Camera::RotateRight(void)
{
    m_Rotation += 0.75f;
    if (m_Rotation > 180.0f) {
        m_Rotation -= 360.0f;
    }
    else if (m_Rotation <= -180.0f) {
        m_Rotation += 360.0f;
    }
}

void Camera::RotateLeft(void)
{
    m_Rotation -= 0.75f;
    if (m_Rotation > 180.0f) {
        m_Rotation -= 360.0f;
    }
    else if (m_Rotation <= -180.0f) {
        m_Rotation += 360.0f;
    }
}

void Camera::Resize(void)
{
    m_AspectRatio = r_screenwidth->i / r_screenheight->i;
    SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
}

void Camera::MoveUp(void)
{
    m_CameraPos.x += -sin(glm::radians(m_Rotation)) * 0.25f;
    m_CameraPos.y += cos(glm::radians(m_Rotation)) * 0.25f;
}

void Camera::MoveDown(void)
{
    m_CameraPos.x -= -sin(glm::radians(m_Rotation)) * 0.25f;
    m_CameraPos.y -= cos(glm::radians(m_Rotation)) * 0.25f;
}

void Camera::MoveLeft(void)
{
    m_CameraPos.x -= cos(glm::radians(m_Rotation)) * 0.25f;
    m_CameraPos.y -= sin(glm::radians(m_Rotation)) * 0.25f;
}

void Camera::MoveRight(void)
{
    m_CameraPos.x += cos(glm::radians(m_Rotation)) * 0.25f;
    m_CameraPos.y += sin(glm::radians(m_Rotation)) * 0.25f;
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

GO_AWAY_MANGLE void RE_InitSettings_f(void)
{
    // dither
    if (r_dither->b)
        nglEnable(GL_DITHER);
    else
        nglDisable(GL_DITHER);
    
    nglDisable(GL_CULL_FACE);

    // update all the textures
    R_UpdateTextures();

    // re-calculate the camera
    ri.SDL_GetWindowSize(renderer->window, (int *)&r_screenwidth->i, (int *)&r_screenheight->i);
    renderer->camera.CalculateViewMatrix();
    renderer->camera.Resize();

    ri.SDL_GL_SetSwapInterval((r_vsync->b ? -1 : 0));
    RE_UpdateFramebuffers();
}

static char gl_extensions[ 32768 ];

GO_AWAY_MANGLE bool R_HasExtension(const char *name)
{
    const char *ptr = N_stristr(gl_extensions, name);
    if (!ptr)
        return false;
    
    ptr += strlen(name);
    return ((*ptr == ' ') || (*ptr == '\0')); // verify its a safe string
}

#define NGL( ret, name, ... )\
{ \
    n##name = (PFN##name)load(#name); \
    if (!n##name) ri.Con_Printf(INFO, "WARNING: failed to load gl proc %s", #name); \
}
GO_AWAY_MANGLE void R_InitExtensions(NGLloadproc load)
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

    r_EXT_anisotropicFiltering->b = qfalse;

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

    if (!r_useExtensions->b) {
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

    if (R_HasExtension("GL_EXT_texture_filter_anisotropic")) {
        nglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glContext.maxAnisotropy);

        if (glContext.maxAnisotropy <= 0) {
            ri.Con_Printf(INFO, "... GL_EXT_texture_filter_anisotropic not property supported");
            glContext.maxAnisotropy = 0;
        }
        else {
            ri.Con_Printf(INFO, "... GL_EXT_texture_filter_anisotropic (max: %i)", glContext.maxAnisotropy);
            r_EXT_anisotropicFiltering->b = qtrue;
            glContext.maxAnisotropy = MIN(r_EXT_anisotropicFiltering->i, glContext.maxAnisotropy);
        }
    }
    else {
        ri.Con_Printf(INFO, "... GL_EXT_texture_filter_anisotropic not found");
    }

    r_EXT_anisotropicFiltering->b = (qboolean)glContext.maxAnisotropy > 0;
    if (r_EXT_anisotropicFiltering->i != glContext.maxAnisotropy) {
        ri.Cvar_SetIntegerValue("r_EXT_anisotropicFiltering", glContext.maxAnisotropy);
    }

    if (R_HasExtension("ARB_vertex_buffer_object")) {
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

    // GL_EXT_texture_compression_s3tc
    if (R_HasExtension("GL_ARB_texture_compression") && R_HasExtension("GL_EXT_texture_compression_s3tc")) {
        if (r_textureCompression->b) {
            glContext.textureCompression = TC_S3TC_ARB;
            ri.Con_Printf(INFO, "... using GL_EXT_texture_compression_s3tc");
            ri.Cvar_SetStringValue("r_textureCompression", "TC_S3TC_ARB");
        }
        else {
            ri.Con_Printf(INFO, "... ignoring GL_EXT_texture_compression_s3tc");
            if (N_stricmp(r_textureCompression->s, "none") != 0)
                ri.Cvar_SetStringValue("r_textureCompression", "none");
        }
    }
    else {
        ri.Con_Printf(INFO, "... GL_EXT_texture_compression_s3tc not found");
        if (N_stricmp(r_textureCompression->s, "none") != 0)
            ri.Cvar_SetStringValue("r_textureCompression", "none");
    }

    // GL_S3_s3tc
    if (glContext.textureCompression == TC_NONE && r_textureCompression->b) {
        if (R_HasExtension("GL_S3_s3tc")) {
            if (r_textureCompression->b) {
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

GO_AWAY_MANGLE void R_InitGL(void)
{
    renderer->instance = ri.SDL_GL_CreateContext(renderer->window);
    ri.SDL_GL_MakeCurrent(renderer->window, renderer->instance);
    ri.SDL_GL_SetSwapInterval(-1);
    ri.SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    ri.SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    ri.SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    ri.SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    ri.SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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

GO_AWAY_MANGLE void R_GfxInfo_f(void)
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
    ri.Con_Printf(INFO, "r_drawFPS: %s", N_booltostr(r_drawFPS->b));
    ri.Con_Printf(INFO, "r_ticrate: %i", r_ticrate->i);
    ri.Con_Printf(INFO, "r_fullscreen: %s", N_booltostr(r_fullscreen->b));
    ri.Con_Printf(INFO, "r_multisampleAmount: x%i", r_multisampleAmount->i);
    ri.Con_Printf(INFO, "r_vsync: %s", N_booltostr(r_vsync->b));
    ri.Con_Printf(DEV, "r_textureMinFilter: %s", r_textureMagFilter->s);
    ri.Con_Printf(DEV, "r_textureMagFilter: %s", r_textureMinFilter->s);
    ri.Con_Printf(INFO, "r_textureFiltering: %s", r_textureFiltering->s);
    ri.Con_Printf(INFO, "r_gammaAmount: %f", r_gammaAmount->f);
    ri.Con_Printf(INFO, "r_textureDetail: %s", r_textureDetail->s);
    ri.Con_Printf(INFO, "r_EXT_anisotropicFilter: %s, %i", N_booltostr(r_EXT_anisotropicFiltering->b), r_EXT_anisotropicFiltering->i);
    ri.Con_Printf(INFO, "r_useExtensions: %s", N_booltostr(r_useExtensions->b));
    ri.Con_Printf(INFO, "r_dither: %s", N_booltostr(r_dither->b));
    ri.Con_Printf(INFO, " ");
}

GO_AWAY_MANGLE void* R_ImGuiAlloc_Callback(size_t size, void *userData)
{
    EA_UNUSED(userData);
    return ri.Mem_Alloc(size);
}

GO_AWAY_MANGLE void R_ImGuiFree_Callback(void *ptr, void *userData)
{
    EA_UNUSED(userData);
    ri.Mem_Free(ptr);
}

GO_AWAY_MANGLE void R_InitImGui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions(R_ImGuiAlloc_Callback, R_ImGuiFree_Callback);
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(renderer->window, renderer->instance);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    
    imgui_on = true;
}

static qboolean rendererInitialized = qfalse;

GO_AWAY_MANGLE void RE_Init(renderImport_t *import)
{
    memcpy(&ri, import, sizeof(*import)); // copy all the functions

    ri.Con_Printf(INFO, "RE_Init: initializing rendering engine");
    
    r_ticrate = ri.Cvar_Get("r_ticrate", "35", CVAR_PRIVATE | CVAR_ROM | CVAR_SAVE);
    ri.Cvar_SetGroup(r_ticrate, CVG_RENDERER);
    r_textureMagFilter = ri.Cvar_Get("r_textureMagFilter", "GL_NEAREST", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_textureMagFilter, CVG_RENDERER);
    r_textureMinFilter = ri.Cvar_Get("r_textureMinFilter", "GL_LINEAR", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_textureMinFilter, CVG_RENDERER);
    r_textureFiltering = ri.Cvar_Get("r_textureFiltering", "Nearest", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_textureFiltering, CVG_RENDERER);
    r_textureCompression = ri.Cvar_Get("r_textureCompression", "none", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_textureCompression, CVG_RENDERER);
    r_textureDetail = ri.Cvar_Get("r_textureDetail", "medium", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_textureDetail, CVG_RENDERER);
    r_screenwidth = ri.Cvar_Get("r_screenwidth", "1920", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_screenwidth, CVG_RENDERER);
    r_screenheight = ri.Cvar_Get("r_screenheight", "1080", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_screenheight, CVG_RENDERER);
    r_vsync = ri.Cvar_Get("r_vsync", "1", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_vsync, CVG_RENDERER);
    r_fullscreen = ri.Cvar_Get("r_fullscreen", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_fullscreen, CVG_RENDERER);
    r_drawFPS = ri.Cvar_Get("r_drawFPS", "0", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_drawFPS, CVG_RENDERER);
    r_renderapi = ri.Cvar_Get("r_renderapi", "R_OPENGL", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_renderapi, CVG_RENDERER);
    r_dither = ri.Cvar_Get("r_dither", "0", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_dither, CVG_RENDERER);
    r_multisampleAmount = ri.Cvar_Get("r_multisampleAmount", "2", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_multisampleAmount, CVG_RENDERER);
    r_multisampleType = ri.Cvar_Get("r_multisampleType", "none", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_multisampleType, CVG_RENDERER);
    r_EXT_anisotropicFiltering = ri.Cvar_Get("r_EXT_anisotropicFiltering", "0", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_EXT_anisotropicFiltering, CVG_RENDERER);
    r_gammaAmount = ri.Cvar_Get("r_gammaAmount", "2.2.", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_gammaAmount, CVG_RENDERER);
    r_bloomOn = ri.Cvar_Get("r_bloomOn", "0", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_bloomOn, CVG_RENDERER);
    r_useExtensions = ri.Cvar_Get("r_useExtensions", "1", CVAR_PRIVATE | CVAR_SAVE);
    ri.Cvar_SetGroup(r_useExtensions, CVG_RENDERER);

    if (ri.SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        ri.N_Error("RE_Init: failed to initialize SDL2, error message: %s",
            ri.SDL_GetError());
    }

    ri.Con_Printf(INFO, "alllocating memory to the SDL_Window context");
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
    R_InitGL();

    renderer->numTextures = 0;
    renderer->numVertexCaches = 0;
    renderer->numFBOs = 0;
    renderer->numShaders = 0;

    renderer->uboid = 0;
    renderer->shaderid = 0;
    renderer->vaoid = 0;
    renderer->vboid = 0;
    renderer->iboid = 0;
    renderer->textureid = 0;
    renderer->fboid = 0;

    R_InitImGui();
    sdl_on = true;

    ri.Cmd_AddCommand("gfxinfo", R_GfxInfo_f);
    ri.Cmd_AddCommand("gfxrestart", RE_InitSettings_f);

    EASY_PROFILER_ENABLE;
    profiler::startListen();

    RE_CacheTextures();
    RE_InitFrameData();
    RE_InitFramebuffers();

    rendererInitialized = qtrue;
}

void RE_Shutdown(void)
{
    if (!sdl_on) // may recurse
        return;
    if (!rendererInitialized)
        return;

    ri.Con_Printf(INFO, "RE_Shutdown: shutting down OpenGL buffers and memory, and deallocating rendering context");
    
    rendererInitialized = qfalse;
    sdl_on = false;


    RE_ShutdownFramebuffers();
    RE_ShutdownTextures();
    RE_ShutdownShaders();
//    RE_ShutdownShaderBuffers();
    for (uint32_t i = 0; i < renderer->numVertexCaches; ++i)
        R_ShutdownCache(renderer->vertexCaches[i]);
    
//    if (renderer->instance)
//        ri.SDL_GL_DeleteContext(renderer->instance);
    if (renderer->window)
        ri.SDL_DestroyWindow(renderer->window);
    if (renderer->instance)
        ri.SDL_GL_DeleteContext(renderer->instance);

    ri.SDL_Quit();
    renderer->window = NULL;
    renderer->instance = NULL;
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
        ri.Con_Error(false, "[OpenGL Error: %i] %s", id, message);
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