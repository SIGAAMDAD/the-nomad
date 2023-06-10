#include "n_shared.h"
#include "m_renderer.h"

static constexpr const char *TEXMAP_PATH = "Files/gamedata/RES/texmap.bmp";

Renderer* renderer;

void R_DrawIndexed(const vertexCache_t* cache, uint32_t count)
{
    R_BindCache(cache);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
    R_UnbindCache();
}

void R_BindCache(const vertexCache_t* cache)
{
    R_BindVertexArray(cache);
    R_BindVertexBuffer(cache);
    R_BindIndexBuffer(cache);
}

void R_UnbindCache(void)
{
    R_UnbindVertexBuffer();
    R_UnbindIndexBuffer();
    R_UnbindVertexArray();
}

#if 0
void R_BeginFramebuffer(framebuffer_t* fbo)
{
    R_SetFramebuffer(fbo);
}
void R_EndFramebuffer(void)
{
    R_SetFramebuffer(NULL);
}
#endif

void R_BindTexture(const texture_t* texture, uint32_t slot)
{
    if (renderer->textureid == texture->id) {
        return;
    }
    else if (renderer->textureid) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    renderer->textureid = texture->id;
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void R_UnbindTexture(void)
{
    if (renderer->textureid == 0) {
        return;
    }
    renderer->textureid = 0;
    glBindTexture(GL_TEXTURE_2D, 0);
}

void R_BindShader(const shader_t* shader)
{
    if (renderer->shaderid == shader->id) {
        return;
    }
    renderer->shaderid = shader->id;
    glUseProgram(shader->id);
}

void R_UnbindShader(void)
{
    if (renderer->shaderid == 0) {
        return;
    }
    glUseProgram(0);
    renderer->shaderid = 0;
}

void R_BindVertexArray(const vertexCache_t* cache)
{
    if (renderer->vaoid == cache->vaoId) {
        return;
    }
    renderer->vaoid = cache->vaoId;
    glBindVertexArray(cache->vaoId);
}

void R_UnbindVertexArray(void)
{
    if (renderer->vaoid == 0) {
        return;
    }
    glBindVertexArray(0);
    renderer->vaoid = 0;
}

void R_BindVertexBuffer(const vertexCache_t* cache)
{
    if (renderer->vboid == cache->vboId) {
        return;
    }
    renderer->vboid = cache->vboId;
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
}

void R_UnbindVertexBuffer(void)
{
    if (renderer->vboid == 0) {
        return;
    }
    renderer->vboid = 0;
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void R_BindIndexBuffer(const vertexCache_t* cache)
{
    if (renderer->iboid == cache->iboId) {
        return;
    }
    if (!renderer->vaoid) {
        R_BindVertexArray(cache);
    }
    renderer->iboid = cache->iboId;
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->iboId);
}

void R_UnbindIndexBuffer(void)
{
    if (renderer->iboid == 0) {
        return;
    }
    renderer->iboid = 0;
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

bool imgui_on = false;

static void R_InitVK(void)
{
    renderer->gpuContext.instance = (VKContext *)Hunk_Alloc(sizeof(VKContext), "VKContext", h_high);

    VkApplicationInfo *info = &renderer->gpuContext.instance->appInfo;
    memset(info, 0, sizeof(VkApplicationInfo));
    info->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info->pApplicationName = "The Nomad";
    info->applicationVersion = VK_MAKE_VERSION(1, 1, 0);
    info->pEngineName = "GDRLib";
    info->engineVersion = VK_MAKE_VERSION(0, 0, 1);
    info->apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo *createInfo = &renderer->gpuContext.instance->createInfo;
    memset(createInfo, 0, sizeof(VkInstanceCreateInfo));
    createInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo->pApplicationInfo = info;

    VkResult result = vkCreateInstance(createInfo, NULL, &renderer->gpuContext.instance->instance);
    if (result != VK_SUCCESS)
        N_Error("R_Init: failed to initialize a vulkan instance");

    VkPhysicalDevice device = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(renderer->gpuContext.instance->instance, &deviceCount, NULL);
    if (!deviceCount)
        N_Error("R_Init: failed to get physical device count for vulkan instance");
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(renderer->gpuContext.instance->instance, &deviceCount, devices.data());

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices.front(), &properties);

    SDL_Vulkan_CreateSurface(renderer->window, renderer->gpuContext.instance->instance, &renderer->gpuContext.instance->surface);

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(renderer->gpuContext.instance->device, 1, renderer->gpuContext.instance->surface, &presentSupport);
}

static void R_InitImGui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(renderer->window, renderer->gpuContext.context);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    
    imgui_on = true;
}

static void R_InitGL(void)
{
    renderer->gpuContext.context = SDL_GL_CreateContext(renderer->window);
    SDL_GL_MakeCurrent(renderer->window, renderer->gpuContext.context);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        N_Error("R_Init: failed to initialize OpenGL (glad)");
    
    Con_Printf(
        "OpenGL Info:\n"
        "  Version: %s\n"
        "  Renderer: %s\n"
        "  Vendor: %s\n",
    glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));

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

    Con_Printf("<-------- OpenGL Context Parameters -------->");
    // integers - only works if the order is 0-10 integer return types
    for (int i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv(params[i], &v);
        Con_Printf("%s: %i" , names[i], v);
    }
    // others
    int v[2];
    memset(v, 0, sizeof(v));
    glGetIntegerv(params[10], v);
    Con_Printf("%s: %i %i", names[10], v[0], v[1]);
    unsigned char s = 0;
    glGetBooleanv(params[11], &s);
    Con_Printf("%s: %i", names[11], (unsigned int)s);
    Con_Printf(" ");

    Con_Printf("<-------- OpenGL Extensions Info -------->");
    uint32_t extension_count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, (int *)&extension_count);
    Con_Printf("Extension Count: %i", extension_count);
    Con_Printf("Extension List:");
    for (uint32_t i = 0; i < extension_count; ++i) {
        if (i >= 30) { // dont go crazy
            Con_Printf("... (%i more extensions)", extension_count - i);
            break;
        }
        const GLubyte* name = glGetStringi(GL_EXTENSIONS, i);
        api_extensions.emplace_back(name);
        Con_Printf("%s", (const char *)name);
    }
    Con_Printf(" ");

#ifdef _NOMAD_DEBUG
    Con_Printf("turning on OpenGL debug callbacks");
    if (glDebugMessageControlARB != NULL) {
        glEnable(GL_DEBUG_OUTPUT_KHR);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB((GLDEBUGPROCARB)DBG_GL_ErrorCallback, NULL);
        GLuint unusedIds = 0;
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
    }
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_ALWAYS);

    R_InitImGui();
}


static Uint32 R_GetWindowFlags(void)
{
    Uint32 flags = 0;

    if (N_strcmp(r_renderapi.value, "R_OPENGL"))
        flags |= SDL_WINDOW_OPENGL;
    else if (N_strcmp(r_renderapi.value, "R_SDL2"))
        N_Error("R_Init: SDL2 rendering isn't yet supported");
    else if (N_strcmp(r_renderapi.value, "R_VULKAN"))
        N_Error("R_Init: Vulkan rendering isn't yet supported, will be though very soon");
    
    if (N_strcmp(r_hidden.value, "true"))
        flags |= SDL_WINDOW_HIDDEN;
    
    if (N_strcmp(r_fullscreen.value, "true") && N_strcmp(r_native_fullscreen.value, "false"))
        flags |= SDL_WINDOW_FULLSCREEN;
    
    flags |= SDL_WINDOW_RESIZABLE;
    return flags;
}

static bool sdl_on = false;

void R_InitLib(void)
{
    renderImport_t imports;
    imports.Printf = static_cast<void(*)(const char *fmt, ...)>(Con_Printf);
    imports.LPrintf = static_cast<void(*)(loglevel_t level, const char *fmt, ...)>(Con_Printf);
    imports.

    renderer = (Renderer *)Hunk_Alloc(sizeof(Renderer), "renderer", h_high);
    assert(renderer);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        N_Error("R_Init: failed to initialize SDL2, error message: %s",
            SDL_GetError());
    }

    Con_Printf("alllocating memory to the SDL_Window context");
    renderer->window = SDL_CreateWindow(
                            "The Nomad",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            atoi(r_screenwidth.value), atoi(r_screenheight.value),
                            SDL_WINDOW_OPENGL
                        );
    if (!renderer->window) {
        N_Error("R_Init: failed to initialize an SDL2 window, error message: %s",
            SDL_GetError());
    }
    assert(renderer->window);
    R_InitGL();

    renderer->numTextures = 0;
    renderer->numVertexCaches = 0;
    renderer->numFBOs = 0;
    renderer->numShaders = 0;
    sdl_on = true;
}

void R_ShutDown()
{
    if (!sdl_on)
        return;

    Con_Printf("R_ShutDown: deallocating SDL2 contexts and window");

    if (imgui_on) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    for (uint32_t i = 0; i < renderer->numShaders; i++) {
        glDeleteProgram(renderer->shaders[i]->id);
    }
    for (uint32_t i = 0; i < renderer->numFBOs; i++) {
        glDeleteFramebuffers(1, &renderer->fbos[i]->fboId);
        glDeleteFramebuffers(1, &renderer->fbos[i]->defId);
        glDeleteRenderbuffers(1, &renderer->fbos[i]->colorId);
        glDeleteRenderbuffers(1, &renderer->fbos[i]->depthId);
        glDeleteTextures(1, &renderer->fbos[i]->defTex);
    }
    for (uint32_t i = 0; i < renderer->numVertexCaches; i++) {
        glDeleteVertexArrays(1, &renderer->vertexCaches[i]->vaoId);
        glDeleteBuffersARB(1, &renderer->vertexCaches[i]->vboId);
        glDeleteBuffersARB(1, &renderer->vertexCaches[i]->iboId);
    }
    for (uint32_t i = 0; i < renderer->numTextures; i++) {
        glDeleteTextures(1, &renderer->textures[i]->id);
    }

    if (N_strcmp(r_renderapi.value, "R_OPENGL") && renderer->gpuContext.context) {
        SDL_GL_DeleteContext(renderer->gpuContext.context);
        renderer->gpuContext.context = NULL;
    }
    else if (N_strcmp(r_renderapi.value, "R_VULKAN") && renderer->gpuContext.instance) {
        vkDestroySurfaceKHR(renderer->gpuContext.instance->instance, renderer->gpuContext.instance->surface, NULL);
        vkDestroyInstance(renderer->gpuContext.instance->instance, NULL);
        renderer->gpuContext.instance = NULL;
    }
    if (renderer->window) {
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
    }
    SDL_Quit();
    sdl_on = false;
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

void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam)
{
    // nothing massive or useless
    if (length >= 300 || severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        Con_Error("[OpenGL Debug Log] %s", message);
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    case GL_DEBUG_TYPE_PORTABILITY:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    case GL_DEBUG_TYPE_PERFORMANCE:
        Con_Printf("WARNING: [OpenGL Debug Log] %s", message);
        break;
    default:
        Con_Printf("[OpenGL Debug Log] %s", message);
        break;
    };
}

#endif

struct Character
{
    GLuint texid;
    glm::ivec2 size;
    glm::ivec2 bearing;
    uint32_t advance;
};

static eastl::unordered_map<char, Character> R_LoadFont(const eastl::string filepath)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (uint8_t i = 0; i < 128; i++) {
    }
}


Camera::Camera(float left, float right, float bottom, float top)
    : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
{
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    m_CameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Camera::CalculateViewMatrix()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPos) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
    
    m_ViewMatrix = glm::inverse(transform);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::ZoomIn(void)
{
    m_ZoomLevel -= 0.05f;
    m_ProjectionMatrix = glm::ortho(m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, -m_ZoomLevel);
}
void Camera::ZoomOut(void)
{
    m_ZoomLevel += 0.05f;
    m_ProjectionMatrix = glm::ortho(m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, -m_ZoomLevel);
}

void Camera::RotateRight(void)
{
    m_Rotation += m_CameraRotationSpeed;
    if (m_Rotation > 180.0f) {
        m_Rotation -= 360.0f;
    }
    else if (m_Rotation <= -180.0f) {
        m_Rotation += 360.0f;
    }
}
void Camera::RotateLeft(void)
{
    m_Rotation -= m_CameraRotationSpeed;
    if (m_Rotation > 180.0f) {
        m_Rotation -= 360.0f;
    }
    else if (m_Rotation <= -180.0f) {
        m_Rotation += 360.0f;
    }
}
void Camera::MoveUp(void)
{
    m_CameraPos.x += -sin(glm::radians(m_Rotation)) * m_CameraSpeed;
    m_CameraPos.y += cos(glm::radians(m_Rotation)) * m_CameraSpeed;
}
void Camera::MoveDown(void)
{
    m_CameraPos.x -= -sin(glm::radians(m_Rotation)) * m_CameraSpeed;
    m_CameraPos.y -= cos(glm::radians(m_Rotation)) * m_CameraSpeed;
}
void Camera::MoveLeft(void)
{
    m_CameraPos.x -= cos(glm::radians(m_Rotation)) * m_CameraSpeed;
    m_CameraPos.y -= sin(glm::radians(m_Rotation)) * m_CameraSpeed;
}
void Camera::MoveRight(void)
{
    m_CameraPos.x += cos(glm::radians(m_Rotation)) * m_CameraSpeed;
    m_CameraPos.y += sin(glm::radians(m_Rotation)) * m_CameraSpeed;
}

#if 0

int R_DrawMenu(const char* fontfile, const nomadvector<std::string>& choices,
    const char* title)
{
    assert(fontfile);
    assert(title);
    assert(choices.size() > 0);

    std::string font_name = "Files/gamedata/RES/"+std::string(fontfile);
    int x, y;
    int NUMMENU = choices.size();
    std::vector<SDL_Texture*> menus(choices.size());
    std::vector<bool> selected(choices.size());
    std::vector<SDL_Rect> pos(choices.size());
    
    SDL_Color color[2] = {{255,255,255},{255,0,0}};
    int32_t text_size = DEFAULT_TEXT_SIZE;

    TTF_Font *font = TTF_OpenFont(font_name.c_str(), text_size);
    if (!font) {
        N_Error("R_DrawMenu: TTF_OpenFont returned NULL for font file %s", fontfile);
    }
#if 0
    for (int i = 0; i < NUMMENU; ++i) {
        menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[0]);
    }
    // get the title
    SDL_Texture* title_texture = R_GenTextureFromFont(font, title, {0, 255, 255});
#endif

    int w, h;
//    SDL_Call(SDL_QueryTexture(title_texture, NULL, NULL, &w, &h));
    SDL_Rect title_rect;
    title_rect.x = 75;
    title_rect.y = 50;
    title_rect.w = w;
    title_rect.h = h;

    int round = 0;
    for (int i = 0; i < NUMMENU; ++i) {
//        SDL_Call(SDL_QueryTexture(menus[i], NULL, NULL, &w, &h));
        pos[i].x = 100;
        pos[i].y = 150 + round;
        pos[i].w = w;
        pos[i].h = h;
        round += 70;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    SDL_Event event;
    while (1) {
        N_DebugWindowClear();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                for (auto* i : menus) {
                    SDL_DestroyTexture(i);
                }
                Game::Get()->~Game();
                exit(EXIT_SUCCESS);
            }
            switch (event.type) {
            case SDL_MOUSEMOTION: {
                y = event.motion.y;
                x = event.motion.x;
                for (int i = 0; i < NUMMENU; i++) {
                    if (x >= pos[i].x && x <= pos[i].x + pos[i].w
                     && y >= pos[i].y && y <= pos[i].y + pos[i].h) {
                        if (!selected[i]) {
                            selected[i] = true;
                            SDL_DestroyTexture(menus[i]);
//                            menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[1]);
                        }
                    }
                    else {
                        if (selected[i]) {
                            selected[i] = false;
                            SDL_DestroyTexture(menus[i]);
//                            menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[0]);
                        }
                    }
                }
                break; }
            case SDL_MOUSEBUTTONDOWN: {
                x = event.button.x;
                y = event.button.y;
                for(int i = 0; i < NUMMENU; i++) {
                    if(x >= pos[i].x && x <= pos[i].x + pos[i].w
                    && y >= pos[i].y && y <= pos[i].y + pos[i].h) {
                        for (auto* i : menus) {
                            SDL_DestroyTexture(i);
                        }
                        N_DebugWindowDraw();
                        return i;
                    }
                }
                break; }
            case SDL_KEYDOWN: {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    goto done;
                }
                break; }
            };
        }
//        R_DrawTexture(title_texture, NULL, &title_rect);
        for (int i = 0; i < NUMMENU; ++i)
//            R_DrawTexture(menus[i], NULL, &pos[i]);
        N_DebugWindowDraw();
    }
done:
    N_DebugWindowDraw();
    return -1;
}
#endif


#define MAX_RENDER_CALLS 64
#define MAX_RENDER_QUADS 64
#define MAX_RENDER_VERTICES (MAX_RENDER_QUADS*4)
#define MAX_RENDER_INDICES (MAX_RENDER_VERTICES*6)

typedef struct
{
    Vertex vertices[MAX_RENDER_VERTICES];
    uint32_t indices[MAX_RENDER_INDICES];
    uint32_t numVertices;
    uint32_t numIndices;
} renderCall_t;

typedef struct
{
    uint32_t numCalls;
    renderCall_t calls[MAX_RENDER_CALLS];

    vertexCache_t* frameCache;
//    renderCall_t* callPtr;
} renderFrame_t;

static renderFrame_t* frame;

void R_FlushFrame(void)
{
    frame->numCalls = 0;
}

void R_DrawCmd(Vertex* vertices, uint32_t numVertices, uint32_t* indices, uint32_t numIndices)
{
    if (frame->numCalls >= MAX_RENDER_CALLS) {
        R_FlushFrame();
    }
    renderCall_t* call = &frame->calls[frame->numCalls];

    memcpy(call->vertices, vertices, numVertices);
    memcpy(call->indices, indices, numIndices);
    call->numVertices = numVertices;
    call->numIndices = numIndices;
}

