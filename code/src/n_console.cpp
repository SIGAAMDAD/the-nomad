#include "n_shared.h"
#include "m_renderer.h"
#include "n_scf.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool imgui_window_open = false;
static eastl::vector<char> con_buffer;

void Con_ClearBuffer(const char *msg, int length)
{
    if (con_buffer.size() >= MAX_BUFFER_SIZE) {
        // linux console it
        eastl::vector<char>::const_iterator ptr = con_buffer.end() - MAX_BUFFER_SIZE;
        while (*ptr != '\n');
        con_buffer.erase(con_buffer.begin(), ptr);
    }
    con_buffer.insert(con_buffer.end(), msg, msg + length);
}

void Con_Printf(loglevel_t level, const char *fmt, ...)
{
    if (level == DEV) {
        fprintf(stdout, "DEV: ");
    }
    else if (level == DEBUG) {
#ifdef _NOMAD_DEBUG
        fprintf(stdout, "DEBUG: ");
#else
        return;
#endif
    }

    va_list argptr;
    int length;
    char msg[1024];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }
    fprintf(stdout, "%s\n", msg);
//    Con_ClearBuffer(msg, length);
}

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    int length;
    char msg[1024];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }
    fprintf(stdout, "%s\n", msg);
//    Con_ClearBuffer(msg, length);
}

void Con_EndFrame(void)
{
    con_buffer.push_back('\0');

    int flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    ImGui::SetWindowSize(ImVec2(N_atoi(r_screenwidth.value), (float)(N_atoi(r_screenheight.value) >> 1)));
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::Begin("Command Console", NULL, flags);
    ImGui::Text("%s", con_buffer.data());
    con_buffer.pop_back();
    ImGui::End();
    if (console_open) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void Con_Error(const char *fmt, ...)
{
    va_list argptr;

    fprintf(stderr,"ERROR: ");
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define MAX_CVAR_HASH 2048
static cvar_t cvar_list{0};

cvar_t* Cvar_Find(const char *name)
{
    cvar_t* cvar;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        if (N_strncasecmp(name, cvar->name, 79)) {
            return cvar;
        }
    }
    return NULL;
}

void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, qboolean save)
{
    cvar_t* cvar;

    if (Cvar_Find(name)) {
        Con_Printf("Cvar_Register: cvar %s already defined", name);
        return;
    }

    for (cvar = &cvar_list; cvar->next; cvar = cvar->next);
    cvar->next = (cvar_t *)Z_Malloc(sizeof(*cvar), TAG_STATIC, &cvar->next, "cvar");
    cvar = cvar->next;

    cvar->name = (char *)Z_Malloc(80, TAG_STATIC, &cvar->name, "cvar");
    N_strncpy(cvar->name, name, 79);
    cvar->name[79] = '\0';

    cvar->value = (char *)Z_Malloc(80, TAG_STATIC, &cvar->value, "cvar");
    N_strncpy(cvar->value, value, 79);
    cvar->value[79] = '\0';

    cvar->type = type;
    cvar->save = save;
}

void Cvar_ChangeValue(const char *name, const char *value)
{
    cvar_t* cvar;

    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("WARNING: attempting to change the value of non-existent cvar %s", name);
        return;
    }
    N_strncpy(cvar->value, value, 80);
    cvar->value[79] = '\0';
}

void Cvar_Register(cvar_t* cvar)
{
    cvar_t* other;

    if (Cvar_Find(cvar->name)) {
        Con_Printf("Cvar_Register: cvar %s already defined", cvar->name);
        return;
    }

    for (other = &cvar_list; other->next; other = other->next);

    other->next = cvar;
}

#define DEFAULT_CFG_NAME "nomadconfig.scf"

void Cvar_WriteCfg_f(void)
{
    cvar_t* cvar;

    json data;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        data[cvar->name] = cvar->value;
    }
    for (cvar = G_GetCvars()[0]; cvar; cvar = cvar->next) {
        data[cvar->name] = cvar->value;
    }

    char cfgpath[256];
    stbsp_snprintf(cfgpath, sizeof(cfgpath), "%s%c%s", fs_gamedir.value, PATH_SEP, DEFAULT_CFG_NAME);
    std::ofstream file(cfgpath, std::ios::out);
    if (!file.is_open()) {
        Con_Printf("WARNING: failed to open file %s to write cvar config", cfgpath);
        return;
    }
    file << data;
    file.close();
}